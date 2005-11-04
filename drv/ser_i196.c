/*!
 * \file
 * <!--
 * Copyright (C) 2003,2004 Develer S.r.l. (http://www.develer.com/)
 * Copyright (C) 2000 Bernardo Innocenti <bernie@codewiz.org>
 * This file is part of DevLib - See README.devlib for information.
 * -->
 *
 * \version $Id$
 *
 * \author Bernardo Innocenti <bernie@develer.com>
 *
 * \brief CPU specific serial I/O driver
 */

/*#*
 *#* $Log$
 *#* Revision 1.6  2005/11/04 16:20:02  bernie
 *#* Fix reference to README.devlib in header.
 *#*
 *#* Revision 1.5  2004/12/13 11:51:08  bernie
 *#* DISABLE_INTS/ENABLE_INTS: Convert to IRQ_DISABLE/IRQ_ENABLE.
 *#*
 *#* Revision 1.4  2004/08/25 14:12:08  rasky
 *#* Aggiornato il comment block dei log RCS
 *#*
 *#* Revision 1.3  2004/06/03 11:27:09  bernie
 *#* Add dual-license information.
 *#*
 *#* Revision 1.2  2004/05/23 18:21:53  bernie
 *#* Trim CVS logs and cleanup header info.
 *#*
 *#*/

#include "hw.h"
#include "serhw.h"

#define SER_HW_ENABLE_TX \
	ATOMIC( \
		if (!ser_sending) \
		{ \
			ser_sending = true; \
			(INT_PEND1 |= INT1F_TI) \
		} \
	);

static volatile bool ser_sending;

// Serial TX intr
INTERRUPT(0x30) void TI_interrupt(void)
{
	if (CANT_SEND)
	{
		ser_sending = false;
		return;
	}

	/* Can we send two bytes at the same time? */
	if (SP_STAT & SPSF_TX_EMPTY)
	{
		SBUF = fifo_pop(&ser_txfifo);

		if (CANT_SEND)
		{
			ser_sending = false;
			return;
		}
	}

	SBUF = fifo_pop(&ser_txfifo);
}

INTERRUPT(0x32) void RI_interrupt(void)
{
	ser_status |= SP_STAT &
		(SPSF_OVERRUN_ERROR | SPSF_PARITY_ERROR | SPSF_FRAMING_ERROR);
	if (fifo_isfull(&ser_rxfifo))
		ser_status |= SERRF_RXFIFOOVERRUN;
	else
		fifo_push(&ser_rxfifo, SBUF);
}

static void ser_setbaudrate(unsigned long rate)
{
	// Calcola il periodo per la generazione del baud rate richiesto
	uint16_t baud = (uint16_t)(((CLOCK_FREQ / 16) / rate) - 1) | 0x8000;
	BAUD_RATE = (uint8_t)baud;
	BAUD_RATE = (uint8_t)(baud >> 8);
}

static void ser_hw_init(void)
{
	// Inizializza la porta seriale
	SP_CON = SPCF_RECEIVE_ENABLE | SPCF_MODE1;
	ioc1_img |= IOC1F_TXD_SEL | IOC1F_EXTINT_SRC;
	IOC1 = ioc1_img;

	// Svuota il buffer di ricezione
	{
		uint8_t dummy = SBUF;
	}

	// Abilita gli interrupt
	INT_MASK1 |= INT1F_TI | INT1F_RI;
}


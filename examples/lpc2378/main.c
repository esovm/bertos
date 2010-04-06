// Emtpy main.c file generated by the wizard
#include <cpu/irq.h>
#include <cfg/debug.h>

#include <io/lpc23xx.h>

#define STATUS_LED (1<<19)
#define LED_ON()  do { IOCLR1 =  STATUS_LED; } while (0)
#define LED_OFF() do { IOSET1 =  STATUS_LED; } while (0)

static void init(void)
{
	IRQ_ENABLE;
	kdbg_init();
	kprintf("NXP LPC2378 BeRTOS port test\n");
	/* Turn off boot led */
	IODIR0 = (1<<21);	
	IOCLR0 = (1<<21);	
	/* Init status led */
	IODIR1 |= STATUS_LED;
	LED_OFF();
}

int main(void)
{
	char spinner[] = {'/', '-', '\\', '|'};
	int i = 0;

	init();
	while (1)
	{
		i++;
		kprintf("BeRTOS is up & running: %c\r",
			spinner[i % countof(spinner)]);
		LED_ON();
		for (int j = 0; j < 200000; j++ );
		
		LED_OFF();
		for (int j = 0; j < 200000; j++ );
	}

	return 0;
}

/*
 * init.c
 *
 * Created: 14.06.2019 19:30:23
 *  Author: ThePetrovich
 */ 

#include "../kernel.h"
#include "../hal.h"

void init()
{
	kernel_setFlag(KFLAG_INIT, 1);
	hal_setupPins();
	hal_uart_init(12);
	//sd_init();
	wdt_reset();
	kernel_checkMCUCSR();
	
	hal_enableInterrupts();
	delay_ms(10);
	
	if(hal_checkBit_m(JUMPER_PIN, JUMPER_IN)){
		kernel_setFlag(KFLAG_DEBUG, 1);
		hal_writePin(&LED_DBG_PORT, LED_DBG, HIGH);
	}
	else hal_writePin(&LED_DBG_PORT, LED_DBG, LOW);
}
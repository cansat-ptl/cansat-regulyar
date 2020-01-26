/*
 * kernel_config.h
 *
 * Created: 30.07.2019 22:31:10
 *  Author: Admin
 */ 


#ifndef KERNEL_CONFIG_H_
#define KERNEL_CONFIG_H_

#ifndef KERNELconfig
#define KERNELconfig

#include "../drivers/devices/sensors/ds18b20/ds18b20.h"
#include "../drivers/interfaces/spi.h"
#include "../drivers/interfaces/twi.h"
#include "../drivers/devices/sensors/bmp280/bmp280.h"
#include "../drivers/devices/sensors/adxl345/adxl345.h"
#include "../drivers/devices/sensors/dht11/DHT.h"

	//Kernel modules - set to 0 to disable
	#define KERNEL_SD_MODULE 0
	#define KERNEL_WDT_MODULE 1
	#define KERNEL_UTIL_MODULE 0
	#define KERNEL_DEBUG_MODULE 1
	#define KERNEL_CLI_MODULE 1
	#define KERNEL_TIMER_MODULE 1

	//Task manager settings
	#define MAX_TASK_QUEUE_SIZE 4
	#define MAX_HIGHPRIO_CALL_QUEUE_SIZE 8
	#define MAX_NORMPRIO_CALL_QUEUE_SIZE 0
	#define MAX_LOWPRIO_CALL_QUEUE_SIZE 10
	
	//Timer settings
	#define MAX_TIMER_COUNT 3
	#define KERNEL_TIMER_PRESCALER 3
	#define TICKRATE_MS 1
	
	/*             Prescaler values             */
	/* 0 - timer disabled                       */
	/* 1 - no prescaling (F_TIMER = F_CPU)      */
	/* 2 - F_CPU / 8                            */
	/* 3 - F_CPU / 64                           */
	/* 4 - F_CPU / 256                          */
	/* 5 - F_CPU / 1024                         */
	/* 6 & 7 - external clock (datasheet)       */
	
	//Debug logging options
	#define LOGGING 1
	#define VERBOSE 0
	#define PROFILING 0
	
	//CLI options
	#define RX0_BUFFER_SIZE 64
	#define MAX_CMD_WORD_SIZE 16

	//HAL uart settings - comment these lines to use built-in uart driver, or specify your own uart functions
	//#define hal_uart_init(ubrr) uart0_init(ubrr)
	//#define hal_uart_putc(c) uart0_putc(c)
	//#define hal_uart_puts(s) uart0_puts(s)
	//#define hal_uart_enableInterruptsRX() rx0_enableInterrupts()
	//#define hal_uart_disableInterruptsRX() rx0_disableInterrupts()
	
	//HAL timer selection - uncomment the timer to select it
	#define HAL_USE_TIMER0
	//#define HAL_USE_TIMER1A
#endif
#endif /* KERNEL_CONFIG_H_ */
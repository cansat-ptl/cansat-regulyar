/*
 * kernel.h
 *
 * Created: 11.05.2019 21:15:24
 *  Author: ThePetrovich
 */ 

#ifndef KERNEL_H_
#define KERNEL_H_

#define KERNEL_VER "0.6.0-bleeding"
#define KERNEL_TIMESTAMP __TIMESTAMP__

#define SDCARD_MOD_VER "0.0.5-rc1"
#define SDCARD_MOD_TIMESTAMP __TIMESTAMP__

#define CMD_MOD_VER "0.0.2-bleeding"
#define CMD_MOD_TIMESTAMP __TIMESTAMP__

#include "types.h"
#include "hal.h"
#include "kernel_config.h"
#include <avr/common.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define ERR_QUEUE_OVERFLOW 1
#define ERR_QUEUE_END 2
#define ERR_WDT_RESET 3
#define ERR_BOD_RESET 4
#define ERR_KRN_RETURN 5
#define ERR_DEVICE_FAIL 6
	
#define KPRIO_HIGH 0
#define KPRIO_NORM 1
#define KPRIO_LOW 2
	
#define KFLAG_INIT 0
#define KFLAG_TIMER_SET 1
#define KFLAG_TIMER_EN 2
#define KFLAG_TIMER_ISR 3
#define KFLAG_SD_INIT 4
#define KFLAG_HAL_ERROR 5
#define KFLAG_LOG_SD 13
#define KFLAG_LOG_UART 14
#define KFLAG_DEBUG 15
	
#define KSTATE_ACTIVE 1
#define KSTATE_SUSPENDED 0

#define KTASK_SINGLERUN 0
#define KTASK_REPEATED 1

void init();
int systemInit();
void initTaskManager();

void kernel_setFlag(uint8_t flag, uint8_t value);
uint8_t kernel_checkFlag(uint8_t flag);
uint64_t kernel_getUptime();

uint8_t kernel_addCall(kTask t_ptr, uint8_t t_priority); 
uint8_t kernel_addTask(uint8_t taskType, kTask t_ptr, uint16_t t_delay, uint8_t t_priority, uint8_t startupState);
uint8_t kernel_removeCall(uint8_t t_priority);
uint8_t kernel_removeTask(uint8_t position);
uint8_t kernel_setTaskState(kTask t_pointer, uint8_t state);
void kernel_clearTaskQueue();
void kernel_clearCallQueue(uint8_t t_priority);
void kernel_checkMCUCSR();
uint8_t kernelInit();

void kernel_stopTimer();
void kernel_startTimer();
void kernel_setupTimer();

#ifdef KERNEL_TIMER_MODULE
	uint8_t kernel_setTimer(kTimerISR t_pointer, uint32_t t_delay);
	uint8_t kernel_removeTimer(kTimerISR t_pointer);
	void kernel_timerService();
#endif

#ifdef KERNEL_SD_MODULE
	void sd_putc(char data);
	void sd_puts(char * data);
	void sd_flush();
	void sd_readPtr();
	void sd_init();
#endif

#ifdef KERNEL_WDT_MODULE
	void wdt_saveMCUCSR(void) __attribute__((naked)) __attribute__((section(".init3")));
	//void wdt_disableWatchdog();
	void wdt_enableWatchdog();
#endif

#ifdef KERNEL_UTIL_MODULE
	#define util_getArrayLength_m(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))
	uint8_t util_strCompare(char * a, char * b, uint8_t len);
#endif

//#ifdef KERNEL_CLI_MODULE
	#define ERR_EMPTY_STRING 20
	#define ERR_COMMAND_NOT_FOUND 21
	void kernel_initCLI();
	void kernel_registerCommand(const char * c_keyword, kCmdHandler c_ptr);
//#endif


#ifdef KERNEL_DEBUG_MODULE
	#define DBG_MOD_VER "0.6.0-bleeding"
	#define DBG_MOD_TIMESTAMP __TIMESTAMP__
	#define PGM_ON 1
	#define PGM_OFF 0
	#define PGM_PUTS 2
	#define L_NONE 0
	#define L_INFO 1
	#define L_WARN 2
	#define L_ERROR 3
	#define L_FATAL 4
	void debug_sendMessage(char * buffer, uint8_t level, const char * format, va_list args);
	void debug_sendMessageSD(char * buffer, uint8_t level, const char * format, va_list args);
	void debug_sendMessage_p(char * buffer, uint8_t level, const char * format, va_list args);
	void debug_sendMessageSD_p(char * buffer, uint8_t level, const char * format, va_list args);
	void debug_puts(uint8_t level, const char * message);
	void debug_putsSD(uint8_t level, const char * message);
	void debug_logMessage(uint8_t pgm, uint8_t level, const char * format, ...);
#endif

#endif /* KERNEL_H_ */
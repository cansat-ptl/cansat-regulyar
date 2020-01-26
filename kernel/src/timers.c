/*
 * timers.c
 *
 * Created: 20.07.2019 10:41:13
 *  Author: WorldSkills-2019
 */ 
#include "../kernel.h"

#if KERNEL_TIMER_MODULE == 1
static volatile struct kTimerStruct_t kTimerQueue[MAX_TIMER_COUNT];
static volatile uint8_t kTimerIndex = 0;

uint8_t kernel_setTimer(kTimerISR t_pointer, uint32_t t_delay)
{
	if(!kernel_checkFlag(KFLAG_TIMER_EN)) return 2;
		
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	for(int i = 0; i <= kTimerIndex; i++){
		if(kTimerQueue[i].tsrPointer == t_pointer){
			kTimerQueue[i].period = t_delay;
			kTimerQueue[i].repeatPeriod = t_delay;
			
			hal_enableInterrupts();
			hal_statusReg = sreg;
			return 0;
		}
	}
	if(kTimerIndex < MAX_TIMER_COUNT){
		kTimerIndex++;
		kTimerQueue[kTimerIndex].tsrPointer = t_pointer;
		kTimerQueue[kTimerIndex].period = t_delay;
		kTimerQueue[kTimerIndex].repeatPeriod = t_delay;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return MAX_TIMER_COUNT;
	}
}

uint8_t kernel_removeTimer(kTimerISR t_pointer)
{
	if(!kernel_checkFlag(KFLAG_TIMER_EN)) return 2;
	
	uint8_t position;
	
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	kTimerIndex--;
	for(position = 0; position < MAX_TIMER_COUNT-1; position++){
		if(t_pointer == kTimerQueue[position].tsrPointer)
		break;
	}
	
	if(position != MAX_TIMER_COUNT-1){
		kTimerQueue[position].tsrPointer = NULL;
		kTimerQueue[position].period = 0;
		for(int j = position; j < MAX_TIMER_COUNT-1; j++){
			kTimerQueue[j] = kTimerQueue[j+1];
		}
		kTimerQueue[MAX_TIMER_COUNT-1].tsrPointer = NULL;
		kTimerQueue[MAX_TIMER_COUNT-1].period = 0;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 0;
}

inline void kernel_timerService()
{
	for(int i = 0; i < MAX_TIMER_COUNT; i++){
		if(kTimerQueue[i].tsrPointer == NULL) continue;
		else {
			if(kTimerQueue[i].period != 0)
				kTimerQueue[i].period--;
			else {
				if(kTimerQueue[i].tsrPointer != NULL) (kTimerQueue[i].tsrPointer)(); //Additional NULLPTR protection
				kTimerQueue[i].period = kTimerQueue[i].repeatPeriod;
			}
		}
	}
}
#endif

/*
 * kernel.c
 *
 * Created: 11.05.2019 21:12:52
 *  Author: ThePetrovich
 */ 

#include "../kernel.h"
#include "../hal.h"

static volatile uint16_t kflags = 0;
static volatile uint16_t kflags_mirror __attribute__ ((section (".noinit")));
static uint32_t e_time = 0;
extern uint8_t mcucsr_mirror;

static uint8_t kCallIndex[3] = {0, 0, 0};
static volatile uint8_t kTaskIndex = 0; //Index of the last task in queue
#if MAX_HIGHPRIO_CALL_QUEUE_SIZE != 0
	static volatile kTask kCallQueueP0[MAX_HIGHPRIO_CALL_QUEUE_SIZE];
#endif
#if MAX_NORMPRIO_CALL_QUEUE_SIZE != 0
	static volatile kTask kCallQueueP1[MAX_NORMPRIO_CALL_QUEUE_SIZE];
#endif
#if MAX_LOWPRIO_CALL_QUEUE_SIZE != 0
	static volatile kTask kCallQueueP2[MAX_LOWPRIO_CALL_QUEUE_SIZE]; //TODO: variable number of priorities (might be hard to implement)
#endif
static volatile struct kTaskStruct_t kTaskQueue[MAX_TASK_QUEUE_SIZE];

void idle()
{
	hal_nop();
}
void init();

void kernel_setFlag(uint8_t flag, uint8_t value)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	uint8_t nvalue = !!value;
	kflags ^= (-1 * nvalue ^ kflags) & (1 << flag);
	hal_enableInterrupts();
	hal_statusReg = sreg;
} 

uint8_t kernel_checkFlag(uint8_t flag)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	uint8_t flag_tmp = hal_checkBit_m(kflags, flag);
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return flag_tmp;
}

uint64_t kernel_getUptime()
{
	return e_time;
}

static inline volatile kTask* kernel_getCallQueuePointer(uint8_t t_priority){
	switch(t_priority){
		case KPRIO_HIGH:
			#if MAX_HIGHPRIO_CALL_QUEUE_SIZE != 0
				return kCallQueueP0;
			#else
				return NULL;
			#endif
		break;
		case KPRIO_NORM:
			#if MAX_NORMPRIO_CALL_QUEUE_SIZE != 0 
				return kCallQueueP1;
			#else
				return NULL;
			#endif
		break;
		case KPRIO_LOW:
			#if MAX_LOWPRIO_CALL_QUEUE_SIZE != 0
				return kCallQueueP2;
			#else
				return NULL;
			#endif
		break;
		default:
			return NULL;
		break;
	}
}

static inline uint8_t kernel_getMaxQueueSize(uint8_t t_priority){
	switch(t_priority){
		case KPRIO_HIGH:
			return MAX_HIGHPRIO_CALL_QUEUE_SIZE;
		break;
		case KPRIO_NORM:
			return MAX_NORMPRIO_CALL_QUEUE_SIZE;
		break;
		case KPRIO_LOW:
			return MAX_LOWPRIO_CALL_QUEUE_SIZE;
		break;
		default:
			return 0;
		break;
	}
}

static inline void kernel_resetTaskByPosition(uint8_t position){
	kTaskQueue[position].pointer = idle;
	kTaskQueue[position].delay = 0;
	kTaskQueue[position].repeatPeriod = 0;
	kTaskQueue[position].priority = KPRIO_LOW;
	kTaskQueue[position].state = KSTATE_ACTIVE;
}

static inline void kernel_addCall_i(kTask t_ptr, uint8_t t_priority)
{			
	uint8_t maxsize = kernel_getMaxQueueSize(t_priority);

	if(kCallIndex[t_priority] < maxsize){
		volatile kTask* ptr = kernel_getCallQueuePointer(t_priority);
		
		(ptr)[kCallIndex[t_priority]] = t_ptr;
		kCallIndex[t_priority]++;
	}
}

uint8_t kernel_addCall(kTask t_ptr, uint8_t t_priority)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	uint8_t maxsize = kernel_getMaxQueueSize(t_priority);
	if(maxsize == 0) return 1;

	if(kCallIndex[t_priority] < maxsize){
		volatile kTask* ptr = kernel_getCallQueuePointer(t_priority);
		if(ptr == NULL) return 1;
		
		(ptr)[kCallIndex[t_priority]] = t_ptr;
		kCallIndex[t_priority]++;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return ERR_QUEUE_OVERFLOW;
	}
}

uint8_t kernel_addTask(uint8_t taskType, kTask t_ptr, uint16_t t_delay, uint8_t t_priority, uint8_t startupState)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	if(t_delay == 0) t_delay = 1;
		
	for(int i = 0; i <= kTaskIndex; i++){
		if(kTaskQueue[i].pointer == t_ptr){
			kTaskQueue[i].repeatPeriod = t_delay - 1;
			kTaskQueue[i].priority = t_priority;
			kTaskQueue[i].state = startupState;
			if(taskType == KTASK_REPEATED) kTaskQueue[i].repeatPeriod = t_delay - 1;
			else kTaskQueue[i].repeatPeriod = 0;
			
			hal_enableInterrupts();
			hal_statusReg = sreg;
			return 0;
		}
	}
	if(kTaskIndex < MAX_TASK_QUEUE_SIZE){
		kTaskQueue[kTaskIndex].pointer = t_ptr;
		kTaskQueue[kTaskIndex].delay = t_delay - 1;
		kTaskQueue[kTaskIndex].priority = t_priority;
		kTaskQueue[kTaskIndex].state = startupState;
		if(taskType == KTASK_REPEATED) kTaskQueue[kTaskIndex].repeatPeriod = t_delay - 1;
		else kTaskQueue[kTaskIndex].repeatPeriod = 0;
		kTaskIndex++;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {	
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return ERR_QUEUE_OVERFLOW;
	}
}

inline uint8_t kernel_removeCall(uint8_t t_priority)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	uint8_t maxsize = kernel_getMaxQueueSize(t_priority);
	if(maxsize == 0) return 1;
	volatile kTask* ptr = kernel_getCallQueuePointer(t_priority);
	if(ptr == NULL) return 1;
	
	if(kCallIndex[t_priority] != 0){
		kCallIndex[t_priority]--;
		for(int i = 0; i < maxsize-1; i++){
			(ptr)[i] = (ptr)[i+1];
		}
		(ptr)[maxsize-1] = idle;
	}	
	else {
		(ptr)[0] = idle;
	}
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 0;
}

inline uint8_t kernel_removeTask(uint8_t position)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	kTaskIndex--;
	kernel_resetTaskByPosition(position);
	for(int j = position; j < MAX_TASK_QUEUE_SIZE-1; j++){
		kTaskQueue[j] = kTaskQueue[j+1];
	}
	kernel_resetTaskByPosition(MAX_TASK_QUEUE_SIZE-1);
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 0;
}

uint8_t kernel_removeTaskByPtr(kTask t_pointer)
{
	uint8_t position;
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	kTaskIndex--;
	for(position = 0; position < MAX_TASK_QUEUE_SIZE-1; position++){
		if(t_pointer == kTaskQueue[position].pointer)
			break;
	}
	
	if(position != MAX_TASK_QUEUE_SIZE-1){
		kernel_resetTaskByPosition(position);
		for(int j = position; j < MAX_TASK_QUEUE_SIZE-1; j++){
			kTaskQueue[j] = kTaskQueue[j+1];
		}
		kernel_resetTaskByPosition(MAX_TASK_QUEUE_SIZE-1);
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {
		 hal_enableInterrupts();
		 hal_statusReg = sreg;
		 return 1;
	}
}

void kernel_clearCallQueue(uint8_t t_priority)
{
	#if LOGGING == 1
		if(kernel_checkFlag(KFLAG_DEBUG) && VERBOSE)
			debug_logMessage(PGM_PUTS, L_WARN, (char *)PSTR("kernel: Call queue cleared\r\n"));
	#endif
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	uint8_t maxsize = kernel_getMaxQueueSize(t_priority);
	if(maxsize == 0) return;	
	volatile kTask* ptr = kernel_getCallQueuePointer(t_priority);
	if(ptr == NULL) return;
	
	for(int i = 0; i < maxsize; i++){
		(ptr)[i] = idle;
	}
	kCallIndex[t_priority] = 0;
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
}

void kernel_clearTaskQueue()
{
	if(kernel_checkFlag(KFLAG_DEBUG) && VERBOSE)
		debug_logMessage(PGM_PUTS, L_WARN, (char *)PSTR("kernel: Task queue cleared\r\n"));
		
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	for(int i = 0; i < MAX_TASK_QUEUE_SIZE; i++){
		kernel_resetTaskByPosition(i);
	}
	kTaskIndex = 0;
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
}

uint8_t kernel_setTaskState(kTask t_pointer, uint8_t state)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	for(int i = 0; i < MAX_TASK_QUEUE_SIZE-1; i++){
		if(kTaskQueue[i].pointer == t_pointer){
			kTaskQueue[i].state = state;
			hal_enableInterrupts();
			return 0;
		}
	}
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 1;
}

inline static void kernel_taskManager()
{
	#if MAX_HIGHPRIO_CALL_QUEUE_SIZE != 0
	if((kCallQueueP0[0] != idle)){
		#if PROFILING == 1
			uint64_t startTime = kernel_getUptime();
		#endif
		
		(kCallQueueP0[0])();
		kernel_removeCall(0);
		
		#if PROFILING == 1
			debug_logMessage(PGM_ON, L_INFO, PSTR("kernel: Task exec time: %u ms\r\n"), (unsigned int)(kernel_getUptime()-startTime));
		#endif
	}
	#endif
	#if MAX_NORMPRIO_CALL_QUEUE_SIZE != 0
	else if((kCallQueueP1[0] != idle)){	
		#if PROFILING == 1
			uint64_t startTime = kernel_getUptime();
		#endif
		
		(kCallQueueP1[0])();
		kernel_removeCall(1);
		
		#if PROFILING == 1
			debug_logMessage(PGM_ON, L_INFO, PSTR("kernel: Task exec time: %u ms\r\n"), (unsigned int)(kernel_getUptime()-startTime));
		#endif
	}
	#endif
	#if MAX_LOWPRIO_CALL_QUEUE_SIZE != 0
	else if(kCallQueueP2[0] != idle){		
		#if PROFILING == 1
			uint64_t startTime = kernel_getUptime();
		#endif
		
		(kCallQueueP2[0])();
		kernel_removeCall(2);

		#if PROFILING == 1
			debug_logMessage(PGM_ON, L_INFO, PSTR("kernel: Task exec time: %u ms\r\n"), (unsigned int)(kernel_getUptime()-startTime));
		#endif
	}
	#endif
	idle();
}

static uint8_t kernel()
{
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                        [DONE]\r\n"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Starting task manager"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                  [DONE]\r\n"));
	#endif
	#if KERNEL_CLI_MODULE == 1
		kernel_initCLI();
	#endif
	kernel_setFlag(KFLAG_INIT, 0);
	while(1){
		wdt_reset();
		kernel_taskManager();
		hal_switchBit(&LED_KRN_PORT, LED_KRN);
		hal_enableInterrupts();
	}
	return ERR_KRN_RETURN;
}

void kernel_startTimer()
{
	if(kernel_checkFlag(KFLAG_TIMER_SET)){
		hal_startSystemTimer();
		kernel_setFlag(KFLAG_TIMER_EN, 1);
	}
}

void kernel_stopTimer()
{
	if(kernel_checkFlag(KFLAG_TIMER_SET)){
		hal_stopSystemTimer();
		kernel_setFlag(KFLAG_TIMER_EN, 0);
	}
}

void kernel_setupTimer()
{
	hal_setupSystemTimer();
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                         [DONE]\r\n"));
	#endif
	kernel_setFlag(KFLAG_TIMER_SET, 1);
}

uint8_t kernelInit()
{
	init();
	kernel_setFlag(KFLAG_INIT, 1);
	kernel_setFlag(KFLAG_TIMER_SET, 0);
	kernel_setFlag(KFLAG_TIMER_EN, 0);
	#if LOGGING == 1
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("\r\n[INIT]kernel: Starting up CanSat kernel v%s\r\n\r\n"), KERNEL_VER);
	#endif
	
	wdt_reset();
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up PRIORITY_HIGH queue"));
	#endif
	kernel_clearCallQueue(0);
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("         [DONE]\r\n"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up PRIORITY_NORM queue"));
	#endif
	kernel_clearCallQueue(1);
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("         [DONE]\r\n"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up PRIORITY_LOW queue"));
	#endif
	kernel_clearCallQueue(2);
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("          [DONE]\r\n"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up task queue"));
	#endif
	wdt_reset();
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                  [DONE]\r\n"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("\r\n----------------------Memory usage----------------------\r\n"));
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: HIGHPRIO queue size:             %d\r\n"), MAX_HIGHPRIO_CALL_QUEUE_SIZE);
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: NORMPRIO queue size:             %d\r\n"), MAX_NORMPRIO_CALL_QUEUE_SIZE);
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: LOWPRIO queue size:              %d\r\n"), MAX_LOWPRIO_CALL_QUEUE_SIZE);
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: Task queue size:                 %d\r\n"), MAX_TASK_QUEUE_SIZE);
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: HIGHPRIO queue memory usage:     %d bytes\r\n"), MAX_HIGHPRIO_CALL_QUEUE_SIZE * sizeof(void*));
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: NORMPRIO queue memory usage:     %d bytes\r\n"), MAX_NORMPRIO_CALL_QUEUE_SIZE * sizeof(void*));
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: LOWPRIO queue memory usage:      %d bytes\r\n"), MAX_LOWPRIO_CALL_QUEUE_SIZE * sizeof(void*));
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: Task queue memory usage:         %d bytes\r\n"), MAX_TASK_QUEUE_SIZE * sizeof(kTaskQueue[0]));
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: Total Task Manager memory usage: %d bytes\r\n"), MAX_TASK_QUEUE_SIZE * sizeof(kTaskQueue[0]) +\
		MAX_HIGHPRIO_CALL_QUEUE_SIZE * sizeof(void*) +\
		MAX_NORMPRIO_CALL_QUEUE_SIZE * sizeof(void*) +\
		MAX_LOWPRIO_CALL_QUEUE_SIZE * sizeof(void*));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("--------------------------------------------------------\r\n\r\n"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Starting timer"));
	#endif
	wdt_reset();
	kernel_setupTimer();
	kernel_startTimer();
	kernel_setFlag(KFLAG_TIMER_EN, 1);
	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Starting kernel"));
	#endif
	kernel();
	return 0;
}

void kernel_checkMCUCSR()
{
	if(hal_checkBit_m(mcucsr_mirror, WDRF))
		hal_setBit_m(kflags, WDRF);
	if(hal_checkBit_m(mcucsr_mirror, BORF))
		hal_setBit_m(kflags, BORF);
	return;
}


inline static void kernel_taskService()
{
	for(int i = 0; i < MAX_TASK_QUEUE_SIZE; i++){
		if(kTaskQueue[i].pointer == idle) continue;
		else {
			if(kTaskQueue[i].delay != 0 && kTaskQueue[i].state == KSTATE_ACTIVE)
				kTaskQueue[i].delay--;
			else {
				if(kTaskQueue[i].state == KSTATE_ACTIVE && kTaskQueue[i].pointer != NULL){
					kernel_addCall_i(kTaskQueue[i].pointer, kTaskQueue[i].priority);
					if(kTaskQueue[i].repeatPeriod == 0) 
						kernel_removeTask(i);
					else 
						kTaskQueue[i].delay = kTaskQueue[i].repeatPeriod;
				}
			}
		}
	}
	e_time++;
}

ISR(HAL_TIMER_INTERRUPT_vect)
{
	hal_setBit_m(kflags, KFLAG_TIMER_ISR);
	hal_disableInterrupts();
	
	kernel_taskService();
	#ifndef USE_EXTERNAL_TIMER_ISR
		#if KERNEL_TIMER_MODULE == 1
			kernel_timerService();
		#endif
	#endif
	
	hal_enableInterrupts();
	hal_clearBit_m(kflags, KFLAG_TIMER_ISR);
}

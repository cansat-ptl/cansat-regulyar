/*
 * types.h
 *
 * Created: 12.05.2019 18:36:04
 *  Author: ThePetrovich
 */ 

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

#define CMD_MAX_WORD_SIZE 16

typedef char kCmdWord_t[CMD_MAX_WORD_SIZE];
typedef void (*kTask)(void);
typedef void (*kTimerISR)(void);
typedef void (*kCmdHandler)(void);
typedef uint8_t byte;

struct kTaskStruct_t {
	kTask pointer;
	uint16_t delay;
	uint16_t repeatPeriod;
	uint8_t priority;
	uint8_t state;
};

struct kTimerStruct_t {
	kTimerISR tsrPointer;
	uint32_t period;
	uint32_t repeatPeriod;
};

struct kCommandStruct_t {
	kCmdWord_t keyword;
	kCmdHandler handler;
	uint8_t length;
};

#endif /* TYPES_H_ */
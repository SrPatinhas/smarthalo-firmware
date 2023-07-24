#ifndef PERIPHERAL_WHITELED_H
#define PERIPHERAL_WHITELED_H

#include "../peripheral.h"

int Peripheral_WhiteLED_init(Peripheral* _perf);

typedef struct {
	int (*turnOn)();
	int (*turnOff)();
	int (*blink)(int delay);
} Peripheral_WhiteLED_API;

#endif

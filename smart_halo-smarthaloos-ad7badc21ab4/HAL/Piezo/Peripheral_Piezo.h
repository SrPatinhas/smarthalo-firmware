#ifndef PERIPHERAL_PIEZO_H
#define PERIPHERAL_PIEZO_H

#include "../peripheral.h"

int Peripheral_Piezo_init(Peripheral* _perf);

typedef struct {
	//limited set of functionality exposed - only what we need for this example
	int (*play)(int frequency, int duration, int volume);
	int (*turnOff)();
} Peripheral_Piezo_API;


#endif
#ifndef PERIPHERAL_HALOLED_H
#define PERIPHERAL_HALOLED_H

#include "../peripheral.h"

int Peripheral_HaloLED_init(Peripheral* _perf);

typedef struct {
	//limited set of functionality exposed - only what we need for this example
	int (*setHaloColour)(int hue, int saturation, int lightness);
	int (*flash)(int hue, int saturation, int lightness, int onDuration, int offDuration, int repeat);
	int (*turnOff)();
} Peripheral_HaloLED_API;

#endif
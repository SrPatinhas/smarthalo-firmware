#ifndef ACCGYRO_H
#define ACCGYRO_H

#include "../peripheral.h"

int Peripheral_AccGyro_init(Peripheral* _perf);

typedef struct {
	// accelerometer functions
	int (*getXLowStatus)();
	int (*getXHighStatus)();
	int (*getYLowStatus)();
	int (*getYHighStatus)();
	int (*getRawAcceleration)();

	//magnetometer functions
	int (*getRawMagneticField)();
	int (*getMagnetometerStatus)();

	//temperature functions
	int (*isTemperatureDataReady)();
	int (*getRawTemperature)();
} Peripheral_AccelGyro_API;

#endif
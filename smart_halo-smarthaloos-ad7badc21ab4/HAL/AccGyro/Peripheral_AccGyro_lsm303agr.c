#include "Peripheral_AccGyro.h"

static Peripheral_AccelGyro_API _myAPI; 

static char* _get_name() {
	return "AccGyro";
}

static char* _get_part_number() {
	return "lsm303agr";
}

static int _return1() {
	return 1;
}

// this function is exposed in peripherala.h and is therefore public
int Peripheral_AccGyro_init(Peripheral* _perf) {
	//set the function pointers to our private functions
	_perf->Peripheral_getName = _get_name;
	_perf->Peripheral_getPartNumber = &_get_part_number;

	//wire up the static API
	_myAPI.getXLowStatus = &_return1;
	_myAPI.getXHighStatus = &_return1;
	_myAPI.getYLowStatus = &_return1;
	_myAPI.getYHighStatus = &_return1;
	_myAPI.getRawAcceleration = &_return1;
	_myAPI.getRawMagneticField = &_return1;
	_myAPI.getMagnetometerStatus = &_return1;
	_myAPI.isTemperatureDataReady = &_return1;
	_myAPI.getRawTemperature = &_return1;

	_perf->Peripheral_API = (void*) &_myAPI;
	return 0;
}
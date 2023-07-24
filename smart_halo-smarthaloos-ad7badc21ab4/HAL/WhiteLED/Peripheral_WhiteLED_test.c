#include "Peripheral_WhiteLED.h"

static Peripheral_WhiteLED_API _myAPI; 

static char* _get_name() {
	return "WhiteLED";
}

static char* _get_part_number() {
	return "test";
}

static int _turn_on() {
	return 1;
}

static int _turn_off() {
	return 1;
}

static int _blink(int delay) {
	return 1;
}

int Peripheral_WhiteLED_init(Peripheral* _perf) {
	//set the function pointers to our private functions
	_perf->Peripheral_getName = &_get_name;
	_perf->Peripheral_getPartNumber = &_get_part_number;

	//wire up the static API
	_myAPI.turnOn = &_turn_on;
	_myAPI.turnOff = &_turn_off;
	_myAPI.blink = &_blink;

	_perf->Peripheral_API = (void*) &_myAPI;
	return 0;
}
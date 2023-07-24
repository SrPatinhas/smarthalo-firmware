#include <stdio.h>
#include "Peripheral_HaloLED.h"

static Peripheral_HaloLED_API _myAPI; 

static char* _get_name() {
	return "HaloLED";
}

static char* _get_part_number() {
	return "test";
}

static int _set_halo_colour(int hue, int saturation, int lightness) {
	return 1;
}

/*
	Possibly this functionality should be held at a higher level.  It presumably uses
		_set_halo_colour to deliver this service.  This is a line we have to draw at some point.
*/
static int _flash(int hue, int saturation, int lightness, int onDuration, int offDuration, int repeat) {
	printf("\tPeripheral %s flashing halo %i times.\n", _get_name(), repeat);
	return 1;
}

static int _turn_off(int delay) {
	return 1;
}

int Peripheral_HaloLED_init(Peripheral* _perf) {
	//set the function pointers to our private functions
	_perf->Peripheral_getName = &_get_name;
	_perf->Peripheral_getPartNumber = &_get_part_number;

	//wire up the static API
	_myAPI.setHaloColour = &_set_halo_colour;
	_myAPI.flash = &_flash;
	_myAPI.turnOff = &_turn_off;

	_perf->Peripheral_API = (void*) &_myAPI;
	return 0;
}
#include <stdio.h>
#include "Peripheral_Piezo.h"

static Peripheral_Piezo_API _myAPI; 

static char* _get_name() {
	return "Piezo";
}

static char* _get_part_number() {
	return "test";
}

static int _play(int frequency, int duration, int volume) {
	printf("\tPeripheral piezo playing %i for %ims at %i percent volume.\n", frequency, duration, volume);
	return 1;
}

static int _turn_off() {
	return 1;
}

int Peripheral_Piezo_init(Peripheral* _perf) {
	//set the function pointers to our private functions
	_perf->Peripheral_getName = &_get_name;
	_perf->Peripheral_getPartNumber = &_get_part_number;

	//wire up the static API
	_myAPI.play = &_play;
	_myAPI.turnOff = &_turn_off;

	_perf->Peripheral_API = (void*) &_myAPI;
	return 0;
}

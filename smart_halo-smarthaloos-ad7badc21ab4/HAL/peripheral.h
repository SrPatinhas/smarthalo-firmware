#ifndef PERIPHERAL_H
#define PERIPHERAL_H

typedef struct peripheral_type {
	//"public" functions
	//char* (*Peripheral_getName)(struct peripheral_type* self);
	char* (*Peripheral_getName)();
	char* (*Peripheral_getPartNumber)();

	/*void* allows us to pass back ANY struct.  In practice we'll pass back 
		a struct describing the Peripheral-specific functions*/
	void* Peripheral_API;	
	//"public" data could go here as well
} Peripheral;

/*
	factory method to get an "instance" of peripheral (actually a struct)
		- it's up to the consumer to manage the memory for this (i.e. whether it is on the heap or stack)

	Question: do we want to namespace the functions like below?
		- we could also o 
*/

int Peripheral_init(Peripheral* _perf);



//include every Peripheral API here
//should we enforce API in the file name so we know it's an interface?
#include "AccGyro/Peripheral_AccGyro.h"
#include "WhiteLED/Peripheral_WhiteLED.h"
#include "HaloLED/Peripheral_HaloLED.h"
#include "Piezo/Peripheral_Piezo.h"

#endif
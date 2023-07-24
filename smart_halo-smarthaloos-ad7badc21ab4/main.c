/*
	This is an entry point into the firmware.  As a result, this file has "god mode", i.e.
		- it knows all of the libraries available
		- it knows all of the HW available
		- it can call/use any of the code

	Note that I said "an entry point".  There may be multiple files like this, i.e.
		- one per hardware (on SH2, this corresponds to /Src/main.c)
		- we may have multiple of these as our test harnesses
		- we may have one for local development (i.e. without HW)

	Despite this file's omnipotence, we will use it's power sparingly.  It should be used mainly as an orchestrator, i.e.
		- it's responsible for gathering a list of peripherals (HW)
		- it's resonsible for gathering a list of apps available
		- it's responsible for initializing the OS level (including passing in the peripherals)
		- it may or may not be responsible for initializing the apps (the OS may do that)

*/


#include <stdio.h>
#include <string.h>

#include "SmartHaloOS/SmartHaloOS.h"
#include "HAL/peripheral.h"
#include "Applications/application.h"

int main() {

	//init peripherals.  Note that this main() keeps them all on its stack
	Peripheral peripherals[4];

	Peripheral_AccGyro_init(&peripherals[0]);
	Peripheral_WhiteLED_init(&peripherals[1]);
	Peripheral_HaloLED_init(&peripherals[2]);
	Peripheral_Piezo_init(&peripherals[3]);

	//init OS
	SmartHaloOS SHOS;
	SmartHaloOS_init(&SHOS);

	//after initializing, we pass all peripherals into the OS
	int num_peripherals = sizeof(peripherals)/sizeof(peripherals[0]);
	for(int x=0; x<num_peripherals; x++) {
		//SmartHaloOS_registerPeripheral(&peripherals[x]);
		SHOS.registerPeripheral(&peripherals[x]);
	}

	//search for applications
	/* Don't know how this is going to work, for now we'll just include them*/
	SmartHalo_Application app_alarm;
	Application_Alarm_init(&SHOS);

	//start scheduler
	SHOS.startScheduler();
	return 0; //return for now since we don't have a scheduler


	/* We should never get here as control is now taken by the scheduler */
	while (1) {
	}

}






#include <stdio.h>	//TODO: replace this with an internal debug output handler
#include <string.h>

#include "SmartHaloOS.h"
/*
	Unlike peripherals and applications, we include the submodules in the implementation of the OS
		to keep them isolated.
*/
#include "MessageBroker.h" 	
#include "Graphics.h"
#include "Sounds.h"

//in a real implementation, we would let FreeRTOUS keep track of apps
static const char* _app_name;
static void (*_app_function)();


static int _register_peripheral(Peripheral* thePeripheral) {
	char* peripheralName = thePeripheral->Peripheral_getName();
	char* peripheralPartNum = thePeripheral->Peripheral_getPartNumber();

	//TODO: replace this with a debug log
	printf("SmartHaloOS Registered peripheral: %s-%s\n", peripheralName, peripheralPartNum);

	if( strcmp(peripheralName, "AccGyro")==0 ) {
		//there is an implicit cast from a void* to a Peripheral_AccelGyro_API* below
		Peripheral_AccelGyro_API* theAPI = thePeripheral->Peripheral_API;

		//TODO: replace these with a debug logs
		/*
		printf("\tAccGyro getXlowStatus(): %i\n", theAPI->getXLowStatus());
		printf("\tAccGyro getXHighStatus(): %i\n", theAPI->getXHighStatus());
		printf("\tAccGyro getYLowStatus(): %i\n", theAPI->getYLowStatus());
		printf("\tAccGyro getYHighStatus(): %i\n", theAPI->getYHighStatus());
		printf("\tAccGyro getRawAcceleration(): %i\n", theAPI->getRawAcceleration());
		printf("\tAccGyro getRawMagneticField(): %i\n", theAPI->getRawMagneticField());
		printf("\tAccGyro getMagnetometerStatus(): %i\n", theAPI->getMagnetometerStatus());
		printf("\tAccGyro isTemperatureDataReady(): %i\n", theAPI->isTemperatureDataReady());
		printf("\tAccGyro getRawTemperature(): %i\n", theAPI->getRawTemperature());
		*/
	}

	if( strcmp(peripheralName, "WhiteLED")==0 ) {
		//do nothing for now
	}

	if( strcmp(peripheralName, "HaloLED")==0 ) {
		Peripheral_HaloLED_API* theAPI = thePeripheral->Peripheral_API;
		Graphics_Register_Halo_Peripheral(theAPI);
	}

	if( strcmp(peripheralName, "Piezo")==0 ) {
		Peripheral_Piezo_API* theAPI = thePeripheral->Peripheral_API;
		Sounds_Register_Piezo_Peripheral(theAPI);
	}

	return 1;
}

static int _register_application(const char* name, void (*taskFunction)() ) {

	_app_name = name;
	_app_function = taskFunction;
	printf("SmartHaloOS Registered application: %s\n", _app_name);

	return 1;
}

static void _start_scheduler() {
	//in a real implementation, we'd hand off execution to FreeRTOS here
	printf("SmartHaloOS Starting scheduler\n");

	//for this example we will just call the single application's  execution function once
	_app_function();
}


int SmartHaloOS_init(SmartHaloOS* _os) {

	_os->registerPeripheral = &_register_peripheral;
	_os->registerApplication = &_register_application;
	_os->startScheduler = &_start_scheduler;

	//Wire up the submodule functions to the exposed functionality
	//	not sure if this is the best way to include things internally...

	//Message broker functionality
	_os->subscribeToEvent = &MessageBroker_subscribeToEvent;

	//Graphics functionality
	_os->Graphics_haloFlash = &Graphics_Halo_flash;

	//Sound functionality 
	_os->Sound_playAlarmSound = &Sound_play_alarm_sound;


	return 0;

}



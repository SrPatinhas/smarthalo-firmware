#ifndef SMARTHALO_OS_H
#define SMARTHALO_OS_H

/*
	This is the main interface of the SmartHaloOS.  Since it's a wrapper for FreeRTOS, you can
		expect to see a lot of things that map directly to FreeRTOS concepts, like tasks, 
		queues, sempahores, etc.  We don't want to directly expose FreeRTOS to applications
		because we'd like to enforce isolation between applications and system-level functions.
		In addition, by choosing which RTOS functionalities we expose, we are forced to take a
		"safety-first" approach - only exposing functionality as they are needed.

	In addition to RTOS stuff, there will be functionality exposed to control access to our 
		resources (i.e. hardware).  This includes modules like graphics, sounds, communciation,
		sensors, etc.  We'll also have helper functionality such as event monitor, message broker,
		memory managers, etc.  The end goal is to expose a set of functionality which makes it
		easy to create an app for SmartHalo.
*/


//TBH peripheral.h might belong in the OS layer.
#include "../HAL/peripheral.h" 

typedef struct SmartHaloOS_type {

	//RTOS/system functions
	int (*registerPeripheral)(Peripheral* thePeripheral);
	int (*registerApplication)(const char* name, void (*taskFunction)() );
	void (*startScheduler)();

	//message broker functions
		//event names will probably be exposed here via an enum
		//the message queue is fake - in a real implementation we'd have a queue type exposed here
	int (*subscribeToEvent)(char* eventName, int fakeMessageQueue);	

	//Graphic functionality
	int (*Graphics_haloFlash)(int hue, int saturation, int lightness, int repeat);

	//Sound functionality
	int (*Sound_playAlarmSound)();

} SmartHaloOS;


int SmartHaloOS_init(SmartHaloOS* theOS);

#endif
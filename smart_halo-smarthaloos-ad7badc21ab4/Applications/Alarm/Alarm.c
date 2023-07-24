//in a real implementation, we wouldn't include stdio
#include <stdio.h>

#include "Alarm.h"

const char* APP_NAME = "Alarm";

static SmartHaloOS* _shos;

//in a real app, we'd make this a queue type from the OS layer
static int _fakeMessageQueue;
static int _deltaX;
static int _deltaY;

void _processIncomingMessages() {
	/*
		Normally we would check the entire message queue and and aggregate the movement deltas
			were going to pretend the delta is large.
	*/

	_deltaX = 999;
	_deltaY = 999;
}

static void _alarmTaskFunction(void* params) {
	///do stuff here
	printf("Alarm app checking for movement.\n");

	/*
		What follows is a contrived example to show how an application COULD work.
			In a real RTOS task, this would be inside a never-ending while loop.
	*/
	_processIncomingMessages();
	if( _deltaX > 50 || _deltaY > 50 ) {
		//at this point, we need to set off the alarm warning
		printf("Alarm app movement limit detected, activating alarm warning\n");
		_shos->Graphics_haloFlash(1,1,100,3);
		_shos->Sound_playAlarmSound();
	}
} 


int Application_Alarm_init(SmartHaloOS* _smartHaloOS) {
	
	_shos = _smartHaloOS;

	/*
		In a real app, we'd set up a real message queue (provided by the OS layer)
			but  this is just a fake app.
	*/
	_fakeMessageQueue = 1;
	_deltaX = 0;
	_deltaY = 0;

	_shos->registerApplication(APP_NAME, &_alarmTaskFunction);
	_shos->subscribeToEvent("MovementEvent", _fakeMessageQueue);

	return 0;
}


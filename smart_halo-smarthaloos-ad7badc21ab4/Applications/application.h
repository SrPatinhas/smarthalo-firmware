#ifndef SMARTHALO_APPLICATION_H
#define SMARTHALO_APPLICATION_H

#include "../SmartHaloOS/SmartHaloOS.h"

/*
	An application - mainly just a wrapper around FreeRTOS tasks
*/
typedef struct {

	//maps directly to a FreeRTOS taskfunction: https://freertos.org/implementing-a-FreeRTOS-task.html
	int (*taskFunction)(void* params);
	const char* name;

} SmartHalo_Application;

//we'll include applications here, but probably we should have some sort of manifest
#include "Alarm/Alarm.h"

#endif
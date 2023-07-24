/*
 * SH_Temperature_monitoring.c
 *
 *  Created on: May 13, 2016
 *      Author: Sean Beitz
 */
#include "SH_Includes.h"

#ifndef SH_FIRMWARE_CODE_SH_TEMPERATURE_MONITORING_C_
#define SH_FIRMWARE_CODE_SH_TEMPERATURE_MONITORING_C_

float led_brightness_correction_for_temperature()
{
	int8_t battery_temperature = read_battery_temperature();

	//@@@ convert battery temperature to float in DEG C;

	if(battery_temperature >= MAXIMUM_OPERATING_TEMPERATURE)
	{// battery too damn hot, risks exploding


		return 0.0f;
	}
	else if((battery_temperature < MAXIMUM_OPERATING_TEMPERATURE) & (battery_temperature > MINIMUM_CONTROL_TEMPERATURE))
	{//battery getting hot, start lowering output current
		float temperature_correction = 0.0f;

		// @@@ make compensation PD or PI
		//proportional compensation, returns a float between 0 and 1;
		temperature_correction =  ((MAXIMUM_OPERATING_TEMPERATURE - battery_temperature)/(MAXIMUM_OPERATING_TEMPERATURE - MINIMUM_CONTROL_TEMPERATURE));


		// @@@ send error message for SmartHalo starting to overheat
		// warn user that leds will dim

		//extra safety check
		if(temperature_correction > 1.0f) 		return 1.0f;
		else if(temperature_correction < 0.0f) 	return 0.0f;
		else 									return temperature_correction;
	}
	else if((battery_temperature > MINIMUM_OPERATING_TEMPERATURE) & (battery_temperature <= MINIMUM_CONTROL_TEMPERATURE))
	{//normal operating temperatures
		return 1.0f;
	}
	else if(battery_temperature < MINIMUM_OPERATING_TEMPERATURE)
	{//battery too cold, risk reducing life expectancy of battery
		return 0.0f;
	}
	else
	{//in case readings are unexpected values
		// @@@ send error message
		return 0.0f;
	}

}



int8_t read_battery_temperature()
{
	int8_t battery_temperature = 0;

	//read battery temperature

	//++++++++++++++++++++++++++++++++++++++++
	//@@@ THIS READS THE ACCELEROMETER TEMPERATURE!!!!!!!!!!!!!!!!
	//++++++++++++++++++++++++++++++++++++++++

	battery_temperature = ACCELEROMETER_MAGNETOMETER_retrieve_temperature_sensor_data();

	return battery_temperature;
}



bool thermal_cutoff_check()
{
	int8_t battery_temperature = 0;
	battery_temperature = read_battery_temperature();

	if(battery_temperature >= MAXIMUM_OPERATING_TEMPERATURE)
	{// battery too damn hot, risks exploding

		return true;
	}
return false;

}




#endif /* SH_FIRMWARE_CODE_SH_TEMPERATURE_MONITORING_C_ */

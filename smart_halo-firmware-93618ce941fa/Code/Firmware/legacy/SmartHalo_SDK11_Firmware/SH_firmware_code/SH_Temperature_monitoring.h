/*
 * SH_Temperature_monitoring.h
 *
 *  Created on: May 13, 2016
 *      Author: Sean Beitz
 */


#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifndef SH_FIRMWARE_CODE_SH_TEMPERATURE_MONITORING_H_
#define SH_FIRMWARE_CODE_SH_TEMPERATURE_MONITORING_H_

#define SAFETY_MARGIN 2.5f

#ifdef LOW_TEMP_BATTERY

#define MAXIMUM_OPERATING_TEMPERATURE 			 (60.0f - SAFETY_MARGIN)
#define MINIMUM_CONTROL_TEMPERATURE 			  40.0f
#define MINIMUM_OPERATING_TEMPERATURE 			(-20.0f + SAFETY_MARGIN)

#define MAXIMUM_CHARGING_TEMPERATURE			(45.0f - SAFETY_MARGIN)
#define MINIMUM_CHARGING_TEMPERATURE			(0.0f + SAFETY_MARGIN)


#elif HIGH_TEMP_BATTERY

#define MAXIMUM_OPERATING_TEMPERATURE 			 (80.0f - SAFETY_MARGIN)
#define MINIMUM_CONTROL_TEMPERATURE 			  60.0f
#define MINIMUM_OPERATING_TEMPERATURE 			(-10.0f + SAFETY_MARGIN)

#define MAXIMUM_CHARGING_TEMPERATURE			(45.0f - SAFETY_MARGIN)
#define MINIMUM_CHARGING_TEMPERATURE			(0.0f + SAFETY_MARGIN)

#else
#error "Battery is not defined!"
#endif


float led_brightness_correction_for_temperature();



int8_t read_battery_temperature();


bool thermal_cutoff_check();


#endif /* SH_FIRMWARE_CODE_SH_TEMPERATURE_MONITORING_H_ */

/*
 * SH_Mouvement_Dectection.h
 *
 *  Created on: 2016-03-30
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_MOUVEMENT_DETECTION_H_
#define SH_FIRMWARE_CODE_SH_MOUVEMENT_DETECTION_H_

#include "SH_Accelerometer_Magnetometer.h"
#include "SH_typedefs.h"

#define ODR_FOR_SLEEP_MODE	SECOND_DATA_RATE

#define ODR_FOR_NORMAL_MODE	SEVENTH_DATA_RATE

#define FULL_SCALE_FOR_SLEEP_MODE	16

#define FULL_SCALE_FOR_NORMAL_MODE	2

#define POWER_MODE_NORMAL_MODE	HIGH_RESOLUTION_MODE

#define POWER_MODE_SLEEP_MODE	LOW_POWER_MODE

#define HIGH_VALUE_X_AXIS	10

#define LOW_VALUE_X_AXIS	10

#define HIGH_VALUE_Y_AXIS

#define LOW_VALUE_Y_AXIS

#define HIGH_VALUE_Z_AXIS

#define LOW_VALUE_Z_AXIS

void SH_initialization_of_accelerometer();

void SH_set_accelerometer_to_sleep_mode_while_enabling_interrupts();
void SH_set_accelerometer_to_sleep_mode_while_disabling_interrupts();
void SH_set_accelerometer_to_normal_mode();
void SH_start_looking_for_movement_data();
bool SH_retrieve_data_from_accelerometer_fifo();
int8_t SH_get_temperature_sensor_data();
SH_event_type analyze_data ();
void SH_reset_int2();


#endif /* SH_FIRMWARE_CODE_SH_MOUVEMENT_DETECTION_H_ */

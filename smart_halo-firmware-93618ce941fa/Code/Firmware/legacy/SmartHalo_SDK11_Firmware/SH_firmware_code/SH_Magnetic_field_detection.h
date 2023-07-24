/*
 * SH_Magnetic_field_detection.h
 *
 *  Created on: 2016-04-19
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_MAGNETIC_FIELD_DETECTION_H_
#define SH_FIRMWARE_CODE_SH_MAGNETIC_FIELD_DETECTION_H_

#define ODR_SLEEP_MODE_MAGNETOMETER 10
#define ODR_NORMAL_MODE_MAGNETOMETER 100
#define SYSTEM_MODE_SLEEP_MODE_MAGNETOMETER IDLE_MODE
#define SYSTEM_MODE_NORMAL_MODE_MAGNETOMETER CONTINUOUS_MODE

#define NUMBER_OF_DATA_IN_ARRAY	32
#define DEGREES_IN_CIRCLE		360

void SH_Magnetometer_initialization();

void SH_Magnetometer_normal_mode();

void SH_Magnetometer_read_data();

bool SH_Magnetometer_retrieve_data();

void SH_Magnetometer_analyze_data();

uint16_t SH_Magnetometer_where_is_north(SH_magnetometer_data average_data);

uint8_t SH_Magnetometer_as_the_crow_flies(uint16_t degrees_to_north);

void SH_Magnetometer_sleep_mode();

SH_magnetometer_data* magnetometer_get_local_stored_values();

uint16_t SH_Magnetometer_heading(SH_magnetometer_data average_data_magnetometer);

#endif /* SH_FIRMWARE_CODE_SH_MAGNETIC_FIELD_DETECTION_H_ */

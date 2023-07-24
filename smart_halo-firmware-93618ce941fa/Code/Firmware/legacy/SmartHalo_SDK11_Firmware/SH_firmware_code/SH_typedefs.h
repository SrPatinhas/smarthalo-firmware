/*
 * SH_typedefs.h
 *
 *  Created on: Feb 23, 2016
 *      Author: Sean Beitz
 */


#include "SHP_Frames_and_commands.h"
#include <stdbool.h>
#include "SH_TWI.h"



#ifndef SH_FIRMWARE_CODE_SH_TYPEDEFS_H_
#define SH_FIRMWARE_CODE_SH_TYPEDEFS_H_

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */

#define MAX_BUFFER_SIZE 					50
#define ANIMATION_MAX_BUFFER_SIZE			63
#define ON 	1
#define OFF 0

#define SH_TWI_ERROR_WITH_DATA_NACK	182
#define SH_TWI_ERROR_WITH_ADDRESS_NACK 183

//define a type for function pointer array
//typedef bool (*func_ptr_t)(void* optional_parameter);
typedef bool (*func_ptr_t)(bool,uint8_t,uint8_t);

//structure holding the information of a parsed SHP message
typedef struct SH_BLE_message{
	bool message_error; //set to true if message is invalid
	uint8_t priority; // for use later in priority queue
	uint8_t device_address;
	uint8_t destination_address;
	uint8_t protocol_version;
	uint8_t lentgh;
	uint8_t command;
	uint8_t data[MAX_PAYLOAD_LENGTH-1]; //command and data are the payload, -1 for command byte
}SH_BLE_message_t;

//structure holding the information of accelerometer axis data
typedef struct SH_accelerometer_data{
	int16_t x_axis; // X axis data of the accelerometer
	int16_t y_axis; // Y axis data of the accelerometer
	int16_t z_axis; // Z axis data of the accelerometer
}SH_accelerometer_data;

//structure holding the information of accelerometer axis data
typedef struct SH_magnetometer_data{
	int16_t x_axis; // X axis data of the accelerometer
	int16_t y_axis; // Y axis data of the accelerometer
	int16_t z_axis; // Z axis data of the accelerometer
}SH_magnetometer_data;

typedef struct SH_as_the_crow_flies_params {
	uint8_t relative_heading; //must be value from 0 to 100, position on the halo to indicate leds
	uint8_t distance_to_target; //must be value from 0 to 100, distance to destination
}SH_as_the_crow_flies_params_t;


//buffer used for management of tasks
typedef struct SHP_command_buffer{
	SH_BLE_message_t queue_array[MAX_BUFFER_SIZE];
	int8_t rear; //must always be initialised as -1
}SHP_command_buffer_t;

//buffer used for management of twi communication
typedef struct SH_twi_command_buffer_t{
	SH_twi_command_t twi_command;
	struct SH_twi_command_buffer_t * next; //must always be initialised at NULL
}SH_twi_command_buffer_t;

//typedef SHP_string char[];

//buffer used for management of incoming messages
typedef struct SHP_message_buffer{
	char* queue_array[MAX_BUFFER_SIZE];
	int8_t rear;//must always be initialised as -1
}SHP_message_buffer_t;


typedef struct SH_animation_function_pointer{
	func_ptr_t animation_function_pointer;
	bool reset;
	uint8_t parameter_1;
	uint8_t parameter_2;
}SH_animation_function_pointer_t;


//buffer that hold function pointers to sequence the animations without blocking the cpu
typedef struct SH_animations_buffer{
	//array of pointers towards animation functions to be executed
	SH_animation_function_pointer_t func_ptr_array[ANIMATION_MAX_BUFFER_SIZE];
	//bool reset_function_array[ANIMATION_MAX_BUFFER_SIZE];
	int16_t rear;//must always be initialised as -1
}SH_animations_buffer_t;

typedef struct SH_calibration_setting{
	int16_t soft_iron_offset;
	int16_t hard_iron_offset;
}SH_calibration_setting_t;

typedef enum SH_display_substate SH_display_substate;
enum SH_display_substate	{TURN_BY_TURN_SUBSTATE =0, AS_THE_CROW_FLIES_SUBSTATE, GOAL_COMPLETION_SUBSTATE};


typedef enum SH_event_type SH_event_type;
enum SH_event_type	{MOVEMENT =0, ACCIDENTAL_MOVEMENT, SUSTAINED_MOVEMENT};


//used for i2c communication
typedef enum bit_number bit_number;
enum bit_number {FIRST_BIT=0, SECOND_BIT, THIRD_BIT, FOURTH_BIT, FIFTH_BIT, SIXTH_BIT, SEVENTH_BIT, EIGHTH_BIT};

#endif /* SH_FIRMWARE_CODE_SH_TYPEDEFS_H_ */

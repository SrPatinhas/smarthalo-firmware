/*
 * SH_Task_handler.h
 *
 *  Created on: Feb 1, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SH_TASK_HANDLER_H_
#define SH_FIRMWARE_CODE_SH_TASK_HANDLER_H_

#define ALARM_MODE_AUTO					true
#define ALARM_MODE_MANUAL				false

//handles the tasks held in the buffers
void task_handler(SHP_command_buffer_t* LOW_priority_tasks_to_be_handled,
		SHP_command_buffer_t* HIGH_priority_tasks_to_be_handled,
		SH_animations_buffer_t* animation_function_buffer,
		SH_animations_buffer_t* LED_animation_function_buffer,
		ble_nus_t * p_nus);

//manages the actions taken by the system when a command is received
void handle_command(SHP_command_buffer_t* tasks_to_be_handled,
		uint8_t index_position,
		SH_animations_buffer_t* animation_function_buffer,
		SH_animations_buffer_t* LED_animation_function_buffer,
		ble_nus_t * p_nus);

//returns bool true if front led is set to blinking mode
bool get_front_led_status();


SH_as_the_crow_flies_params_t get_as_the_crow_flies_current_params();


#endif /* SH_FIRMWARE_CODE_SH_TASK_HANDLER_H_ */

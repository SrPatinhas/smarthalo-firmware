/*
 * SH_Priority_queue.h
 *
 *  Created on: Feb 10, 2016
 *      Author: Sean Beitz
 */
#include "SH_typedefs.h"



#ifndef SH_FIRMWARE_CODE_SH_PRIORITY_QUEUE_H_
#define SH_FIRMWARE_CODE_SH_PRIORITY_QUEUE_H_






//inserts a parsed message into the command buffer to be handled by the taks handler
void insert_command(SHP_command_buffer_t* command_buffer, SH_BLE_message_t* command_to_insert);


//inserts a raw SHP message into a buffer to be parsed
void insert_unparsed_message(SHP_message_buffer_t* message_buffer, uint8_t* message_to_insert);


//removes the command in position 0 from the buffer, shifts all other elements down one postion
void delete_command(SHP_command_buffer_t* command_buffer);


//removes the raw SHP message in position 0 from the buffer, shifts all other elements down one postion
void delete_unparsed_message(SHP_message_buffer_t* message_buffer);

//inserts the pointer to the function of the next animation to be displayed
void insert_animation_function_pointer(SH_animations_buffer_t* animation_function_buffer,
		void* pointer_of_function_to_insert,bool reset, uint8_t parameter_1,uint8_t parameter_2);

//removes the function pointer in position 0 from the buffer, shifts all other elements down one postion
void delete_animation_function_pointer(SH_animations_buffer_t* animation_function_buffer);

//removes all functions pointers from the animation buffer
void flush_animation_buffer(SH_animations_buffer_t* animation_function_buffer);



#endif /* SH_FIRMWARE_CODE_SH_PRIORITY_QUEUE_H_ */

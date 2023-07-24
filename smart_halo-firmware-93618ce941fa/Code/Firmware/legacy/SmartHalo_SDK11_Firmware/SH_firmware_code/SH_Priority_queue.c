#include "stdafx.h"
#include "SH_Priority_queue.h"
#include "SH_BLE.h"
#include "nrf_soc.h"
#include "SH_typedefs.h"


void insert_command(SHP_command_buffer_t* command_buffer, SH_BLE_message_t* command_to_insert)
{
	//enter critical region
//	sd_nvic_critical_region_enter(false);
	 if (command_buffer->rear >= MAX_BUFFER_SIZE - 1)
	    {
	       // printf("\nQueue overflow no more elements can be inserted");
		 ///to do: send error message @@@
	        return;
	    }
	 else
	    {
	        command_buffer->rear++;
	        command_buffer->queue_array[command_buffer->rear] = *command_to_insert;
	        return;
	    }
	// sd_nvic_critical_region_exit(false);
	//exit critical region
}


//insert pointer to message (array of uint8_t) and copy data of array to structure
void insert_unparsed_message(SHP_message_buffer_t* message_buffer, uint8_t* message_to_insert)
{
	//enter critical region
	//sd_nvic_critical_region_enter(false);
	if (message_buffer->rear >= MAX_BUFFER_SIZE - 1)
	 {
	       // printf("\nQueue overflow no more elements can be inserted");
		 ///to do: send error message @@@
		return;
	 }
	else
	 {
		message_buffer->rear++;
		//printf("%d",message_buffer->rear);
		message_buffer->queue_array[message_buffer->rear] = (char*)malloc(MAX_PACKET_LENGTH);
		memcpy(message_buffer->queue_array[message_buffer->rear],message_to_insert,MAX_PACKET_LENGTH);
		return;
	}
	//sd_nvic_critical_region_exit(false);
	//exit critical region
}


void delete_command(SHP_command_buffer_t* command_buffer)
{
	//enter critical region
	//sd_nvic_critical_region_enter(false);
	if (command_buffer->rear < 0)
	{
	 ///to do: send error message @@@
		return;
	}
	else if(command_buffer->rear == 0)
	{
		command_buffer->rear--;
	}
	 else
	{
		 //this may cause an assert failure
		 for(int iter = 0;iter < command_buffer->rear; ++iter )
		 {
			 command_buffer->queue_array[iter] = command_buffer->queue_array[iter + 1];
		 }
		 command_buffer->rear--;
	}
	 //sd_nvic_critical_region_exit(false);
	//exit critical region
}


void delete_unparsed_message(SHP_message_buffer_t* message_buffer)
{

	//entering critical region could cause some messages to be lost because
	//interrupts will be disabled for a significant quantity of time
	//enter critical region
	//sd_nvic_critical_region_enter(false);
	if (message_buffer->rear < 0)
	{
	 ///to do: send error message @@@
		return;
	}
	else if(message_buffer->rear == 0)
	{
		free(message_buffer->queue_array[message_buffer->rear]);
		message_buffer->rear--;
	}
	else
	{
		 for(int iter = 0;iter < message_buffer->rear; ++iter )
		 {
			 memcpy(message_buffer->queue_array[iter],message_buffer->queue_array[iter + 1],MAX_PACKET_LENGTH);
		 }
		 free(message_buffer->queue_array[message_buffer->rear]);
		 message_buffer->rear--;
	}
	//sd_nvic_critical_region_exit(false);
		 //exit critical region
}


void insert_animation_function_pointer(SH_animations_buffer_t* animation_function_buffer,
		void* pointer_of_function_to_insert,bool reset, uint8_t parameter_1,uint8_t parameter_2)
{



	if (animation_function_buffer->rear >= MAX_BUFFER_SIZE - 1)
	 {
	       // printf("\nQueue overflow no more elements can be inserted");
		 ///to do: send error message @@@
		return;
	 }
	else
	 {
		animation_function_buffer->rear++;
		animation_function_buffer->func_ptr_array[animation_function_buffer->rear].animation_function_pointer = pointer_of_function_to_insert;
		animation_function_buffer->func_ptr_array[animation_function_buffer->rear].reset = reset;
		animation_function_buffer->func_ptr_array[animation_function_buffer->rear].parameter_1 = parameter_1;
		animation_function_buffer->func_ptr_array[animation_function_buffer->rear].parameter_2 = parameter_2;
		return;
	}

}


void delete_animation_function_pointer(SH_animations_buffer_t* animation_function_buffer)
{
	if (animation_function_buffer->rear < 0)
	{
	 ///to do: send error message @@@
		return;
	}
	else if(animation_function_buffer->rear == 0)
	{
		//free(animation_function_buffer->queue_array[animation_function_buffer->rear]);
		animation_function_buffer->rear--;
	}
	else
	{
		 for(int iter = 0;iter < animation_function_buffer->rear; ++iter )
		 {
			// memcpy(animation_function_buffer->func_ptr_array[iter],animation_function_buffer->func_ptr_array[iter + 1],sizeof(void*));

			 animation_function_buffer->func_ptr_array[iter] = animation_function_buffer->func_ptr_array[iter + 1];

		 }
		 //free(animation_function_buffer->queue_array[animation_function_buffer->rear]);
		 animation_function_buffer->rear--;
	}
}


void flush_animation_buffer(SH_animations_buffer_t* animation_function_buffer)
{
	while(animation_function_buffer->rear >= 0)
	{
		delete_animation_function_pointer(animation_function_buffer);
	}
}

//@@@do a clear all buffers for memory management










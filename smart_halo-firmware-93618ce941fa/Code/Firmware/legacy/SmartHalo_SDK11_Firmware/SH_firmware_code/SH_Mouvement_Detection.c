/*
 * SH_Mouvement_Detection.c
 *
 *  Created on: 2016-03-30
 *      Author: SmartHalo
 */

#include "SH_typedefs.h"
#include "SH_Mouvement_Detection.h"

#include "SH_app_fifo_accelerometer.h"
#include "SH_Accelerometer_Magnetometer.h"
#include "SH_Accelerometer_INT1.h"
#include "SH_Accelerometer_INT2.h"
#include "AcceleroTest.h"
#include "MagnetoTest.h"
static int8_t temperature=0;
static SH_accelerometer_data average_data_array[FIFO_NUM_ENTRIES];
static uint8_t average_data_samples=0;

//static void look_which_axis_interrupted(void);

static void init_fifo_buffer_for_accelerometer_data(void);

static void start_collecting_data_from_accelerometer();

static void average_data();

// Create a FIFO structure
sh_app_fifo_t fifo_accelerometer_data;

// Create a buffer for the FIFO
SH_accelerometer_data buffer[FIFO_NUM_ENTRIES];

//static bool begun = false;

/*static bool movement_on_x_down_axis=false;
static bool movement_on_x_up_axis=false;
static bool movement_on_y_down_axis=false;
static bool movement_on_y_up_axis=false;
static bool movement_on_z_down_axis=false;
static bool movement_on_z_up_axis=false;*/


void SH_initialization_of_accelerometer(){

	//if (!begun){

		//reboot_acclerometer();
		SH_set_accelerometer_to_sleep_mode_while_enabling_interrupts();
	//	SH_set_accelerometer_to_normal_mode();
		SH_enable_interrupts_accelerometer();
		ACCELEROMETER_Set_FIFO_Mode(FIFO_MDE_BYPASS);
		ACCELEROMETER_MAGNETOMETER_set_temperature_sensor(true);
		ACCELEROMETER_set_full_scale_value(FULL_SCALE_FOR_NORMAL_MODE);
		ACCELEROMETER_enable_x_axis(true);
		ACCELEROMETER_enable_y_axis(true);
		ACCELEROMETER_enable_z_axis(true);
		ACCELEROMETER_initizalize_int2(); //accelerometer interrupt initialization

		ACCELEROMETER_initialize_int1_for_FIFO();
		init_fifo_buffer_for_accelerometer_data();
		SH_Accelerometer_INT2_initialisation(); //gpiote initialization
		//SH_start_looking_for_movement_data();
		SH_reset_int2();
		//begun = true;
	//}
}






void SH_start_looking_for_movement_data(){
	SH_set_accelerometer_to_normal_mode();
	start_collecting_data_from_accelerometer();
}

void SH_shutdown_accelerometer(){
	ACCELEROMETER_set_data_rate_configuration(POWER_DOWN_MODE_DATA_RATE);
}

void fifo (){

	Get_FIFO_Mode();
	FIFO_Get_Depth();
	FIFO_Set_Depth(0);
	FIFO_Get_Thr();
	FIFO_Get_Overrun();
	FIFO_Get_Stored();
}

static void init_fifo_buffer_for_accelerometer_data(){

	// Initialize FIFO structure
	uint32_t err_code = SH_app_fifo_init(&fifo_accelerometer_data, buffer, (uint16_t)sizeof(buffer));
	APP_ERROR_CHECK(err_code);
}

void SH_set_accelerometer_to_sleep_mode_while_enabling_interrupts(){

	ACCELEROMETER_set_power_mode(POWER_MODE_SLEEP_MODE);
	ACCELEROMETER_set_data_rate_configuration(ODR_FOR_SLEEP_MODE);
	ACCELEROMETER_set_full_scale_value(FULL_SCALE_FOR_SLEEP_MODE);
	SH_enable_interrupts_accelerometer();
	SH_enable_disable_interrupt_from_accelerometer_int1(true);
	SH_enable_disable_interrupt_from_accelerometer_int2(true);

}

void SH_set_accelerometer_to_sleep_mode_while_disabling_interrupts(){

	ACCELEROMETER_set_power_mode(POWER_MODE_SLEEP_MODE);
	ACCELEROMETER_set_data_rate_configuration(ODR_FOR_SLEEP_MODE);
	ACCELEROMETER_set_full_scale_value(FULL_SCALE_FOR_SLEEP_MODE);
	SH_disable_interrupts_from_accelerometer();
	SH_enable_disable_interrupt_from_accelerometer_int1(false);
	SH_enable_disable_interrupt_from_accelerometer_int2(false);
}

void SH_set_accelerometer_to_normal_mode(){

	ACCELEROMETER_set_power_mode(POWER_MODE_NORMAL_MODE);
	ACCELEROMETER_set_data_rate_configuration(ODR_FOR_NORMAL_MODE);
	ACCELEROMETER_set_full_scale_value(FULL_SCALE_FOR_NORMAL_MODE);
}

static void start_collecting_data_from_accelerometer(){

	ACCELEROMETER_Set_FIFO_Mode(FIFO_MDE_MODE);
}

bool SH_retrieve_data_from_accelerometer_fifo(){

	SH_accelerometer_data data;
	uint32_t err_code;

	SH_app_fifo_flush(&fifo_accelerometer_data);

	while((fifo_accelerometer_data.write_pos < FIFO_NUM_ENTRIES) && ACCELEROMETER_get_axis_value()){
		if (ACCELEROMETER_get_axis_value()){
			data.x_axis = ACCELEROMETER_x_axis_data();
			data.y_axis = ACCELEROMETER_y_axis_data();
			data.z_axis = ACCELEROMETER_z_axis_data();
			// Add an element to the FIFO
			err_code = SH_app_fifo_put(&fifo_accelerometer_data, data);
			APP_ERROR_CHECK(err_code);
		}
	}

	//Clear and Reset FIFO register
	ACCELEROMETER_Set_FIFO_Mode(FIFO_MDE_BYPASS);

	average_data();

	if (average_data_samples<FIFO_NUM_ENTRIES){
		start_collecting_data_from_accelerometer();
		return false;
	}
	else {
		//Reset interrupt 2
		ACCELEROMETER_get_src_int2();
		SH_set_accelerometer_to_sleep_mode_while_enabling_interrupts();
		return true;

	}
}

void SH_reset_int2(){
#ifndef TEST_HALO_BOX
	ACCELEROMETER_get_src_int2();
#endif
}

static void average_data(){

	SH_accelerometer_data data_array[FIFO_NUM_ENTRIES];
	uint32_t number_of_data = FIFO_NUM_ENTRIES;
	SH_app_fifo_read(&fifo_accelerometer_data, data_array, &number_of_data);

	average_data_array[average_data_samples].x_axis = 0;
	average_data_array[average_data_samples].y_axis = 0;
	average_data_array[average_data_samples].z_axis = 0;

	for (uint8_t i=0; i<FIFO_NUM_ENTRIES; i++){
		average_data_array[average_data_samples].x_axis +=  data_array[i].x_axis;
		average_data_array[average_data_samples].y_axis +=  data_array[i].y_axis;
		average_data_array[average_data_samples].z_axis +=  data_array[i].z_axis;
	}

	average_data_array[average_data_samples].x_axis = average_data_array[average_data_samples].x_axis/FIFO_NUM_ENTRIES;
	average_data_array[average_data_samples].y_axis = average_data_array[average_data_samples].y_axis/FIFO_NUM_ENTRIES;
	average_data_array[average_data_samples].z_axis = average_data_array[average_data_samples].z_axis/FIFO_NUM_ENTRIES;

	average_data_samples++;
}

SH_event_type analyze_data (){
	return MOVEMENT;
	average_data_samples = 0;
}

int8_t SH_get_temperature_sensor_data(){

	temperature = ACCELEROMETER_MAGNETOMETER_retrieve_temperature_sensor_data();
	return temperature;
}

/*
static void look_which_axis_interrupted(void){

	movement_on_x_down_axis=false;
	movement_on_x_up_axis=false;
	movement_on_y_down_axis=false;
	movement_on_y_up_axis=false;
	movement_on_z_down_axis=false;
	movement_on_z_up_axis=false;

	if (is_int2_z_high_value_event()){
		movement_on_z_up_axis = true;
	}
	if (is_int2_z_low_value_event()){
		movement_on_z_down_axis = true;
	}
	if (is_int2_x_high_value_event()){
		movement_on_x_up_axis = true;
	}
	if (is_int2_x_low_value_event()){
		movement_on_x_down_axis = true;
	}
	if (is_int2_y_high_value_event()){
		movement_on_y_up_axis = true;
	}
	if (is_int2_y_low_value_event()){
		movement_on_y_down_axis = true;
	}
}*/


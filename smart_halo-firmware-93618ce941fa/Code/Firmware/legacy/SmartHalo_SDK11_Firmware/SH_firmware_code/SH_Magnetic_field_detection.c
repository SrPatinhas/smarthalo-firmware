/*
 * SH_Magnetic_field_detection.c
 *
 *  Created on: 2016-04-19
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "SH_Accelerometer_Magnetometer.h"
#include "SH_Magnetic_field_detection.h"
#include "SH_typedefs.h"
#include "vector.h"
//#include <tgmath.h>
#include "AcceleroTest.h"
#include "MagnetoTest.h"
#include <math.h>

// @@@ this may be much too precise (affects calculation time)
#define M_PI 3.14159265358979323846

#define FILTER_SHIFT 6


static SH_magnetometer_data data_array[NUMBER_OF_DATA_IN_ARRAY];
static uint16_t heading =0;

void SH_Magnetometer_initialization(){

	MAGNETOMETER_set_ODR(ODR_SLEEP_MODE_MAGNETOMETER);
	MAGNETOMETER_set_system_mode(SYSTEM_MODE_NORMAL_MODE_MAGNETOMETER);
	MAGNETOMETER_set_low_power_mode(true);
	MAGNETOMETER_temperature_compensation();
	MAGNETOMETER_set_interrupt_for_drdy(true);
	nrf_delay_ms(10); //delay necessary to clear bits
	clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,  CFG_REG_A_M, FIRST_BIT);
	clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,  CFG_REG_A_M, SECOND_BIT);
}

void SH_Magnetometer_normal_mode(){

	MAGNETOMETER_set_ODR(ODR_NORMAL_MODE_MAGNETOMETER);
	MAGNETOMETER_set_system_mode(SYSTEM_MODE_NORMAL_MODE_MAGNETOMETER);
	MAGNETOMETER_set_low_power_mode(false);

}

void SH_Magnetometer_sleep_mode()
{
	MAGNETOMETER_set_ODR(ODR_SLEEP_MODE_MAGNETOMETER);
	MAGNETOMETER_set_system_mode(SYSTEM_MODE_SLEEP_MODE_MAGNETOMETER);
	MAGNETOMETER_set_low_power_mode(true);
}

void SH_Magnetometer_read_data(){

	if (MAGNETOMETER_get_axis_value()){
		SH_Magnetometer_retrieve_data();
	}
}

bool SH_Magnetometer_retrieve_data(){

	static uint8_t i = 0;

	if (i<NUMBER_OF_DATA_IN_ARRAY){
		data_array[i].x_axis = MAGNETOMETER_x_axis_data();
		data_array[i].y_axis = MAGNETOMETER_y_axis_data();
		data_array[i].z_axis = MAGNETOMETER_z_axis_data();
		i++;
	}

	else{
		i=0;
		return true;
	}
	return false;
}

void SH_Magnetometer_analyze_data(){

	SH_magnetometer_data average_data;

	for (uint8_t i=0; i<NUMBER_OF_DATA_IN_ARRAY; i++){
		average_data.x_axis += data_array[i].x_axis;
		average_data.y_axis += data_array[i].y_axis;
		average_data.z_axis += data_array[i].z_axis;
	}

	average_data.x_axis = average_data.x_axis/NUMBER_OF_DATA_IN_ARRAY;
	average_data.y_axis = average_data.y_axis/NUMBER_OF_DATA_IN_ARRAY;
	average_data.z_axis = average_data.z_axis/NUMBER_OF_DATA_IN_ARRAY;

	heading = SH_Magnetometer_heading(average_data);
}


uint16_t SH_Magnetometer_heading(SH_magnetometer_data average_data_magnetometer){

    vector a, m;
    vector from = {0, 1 , 0};
    int16_t _filt_ax = 0, _filt_ay = 0, _filt_az =0;
    int16_t a_x= 0, a_y= 0, a_z= 0;

    if(ACCELEROMETER_get_axis_value()){
    	a_x = ACCELEROMETER_x_axis_data();
    	a_y = ACCELEROMETER_y_axis_data();//y and z are inverted because
    	a_z = ACCELEROMETER_z_axis_data();
    }

    // Perform simple lowpass filtering
    // Intended to stabilize heading despite
    // device vibration such as on a UGV
    _filt_ax += a_x - (_filt_ax >> FILTER_SHIFT);
    _filt_ay += a_y - (_filt_ay >> FILTER_SHIFT);
    _filt_az += a_z - (_filt_az >> FILTER_SHIFT);

    a.x = (float) (_filt_ax >> FILTER_SHIFT);
    a.y = (float) (_filt_ay >> FILTER_SHIFT);
    a.z = (float) (_filt_az >> FILTER_SHIFT);

    m.x = average_data_magnetometer.x_axis;
    m.y = average_data_magnetometer.y_axis;
    m.z = average_data_magnetometer.z_axis;

    ////////////////////////////////////////////////
    // compute heading
    ////////////////////////////////////////////////

    vector temp_a = a;
   //  vector gavity_vector = a;
    // normalize
    vector_normalize(&temp_a);
   // vector_normalize(&gavity_vector);
    //vector_normalize(&m);

    // compute E and N
    vector E;
    vector N;
    vector_cross(&m,&temp_a,&E); //vector orthogonal to mag and acc vectors, results in east
    vector_normalize(&E);
    vector_cross(&temp_a,&E,&N);

 //   printf("x: %f, y: %f, z: %f \r\n", a.x ,a.y ,a.z  );
   // vector temptemp ={0,0,0};

    //temp_a.x = -temp_a.x;
    // temptemp.y = temp_a.y;
   //// temp_a.y = -temp_a.z;//inverted z and y because these calculations suppose that y is the vertical axis while the accelerometer supposes that z is vertical
   // temp_a.z = -temptemp.y;
   // printf("xn: %f, yn: %f, zn: %f \r\n", temp_a.x ,temp_a.y ,temp_a.z  );
   // printf("xfrom: %f, yfrom: %f, zfrom: %f \r\n", from.x ,from.y ,from.z  );

   // from.y = temp_a.y;


    //https://www.maplesoft.com/support/help/maple/view.aspx?path=MathApps%2FProjectionOfVectorOntoPlane
    float temp;
    temp = vector_dot(&from,&temp_a)/vector_dot(&temp_a,&temp_a);
   // temp = temp * sqrt(vector_dot(&temp_a,&temp_a));
    temp_a.x = temp * temp_a.x;
    temp_a.y = temp * temp_a.y;
    temp_a.z = temp * temp_a.z;
    from.x = from.x - temp_a.x;
    from.y = from.y - temp_a.y;
    from.z = from.z - temp_a.z;

    //vector_normalize(&from);

     // compute heading
    printf("xfrom: %f, yfrom: %f, zfrom: %f \r\n", from.x ,from.y ,from.z  );
   // printf("Ex: %f, Ey: %f, Ez: %f \r\n", E.x ,E.y ,E.z  );
    printf("Nx: %f, Ny: %f, Nz: %f \r\n", N.x ,N.y ,N.z  );
    float heading = atan2(vector_dot(&E,&from), vector_dot(&N,&from)) * 180/M_PI;
    if (heading < 0) heading += 360;

    return (uint16_t)heading;
}

uint8_t SH_Magnetometer_as_the_crow_flies (uint16_t degrees_to_north){

	uint16_t degrees_to_destination;
	uint8_t percentage_of_led_to_open;

	if (heading >degrees_to_north){
		degrees_to_destination = (heading + degrees_to_north)%DEGREES_IN_CIRCLE;
	}
	else{
		degrees_to_destination = (degrees_to_north - heading)%DEGREES_IN_CIRCLE;
	}
	percentage_of_led_to_open = degrees_to_destination * 100 / DEGREES_IN_CIRCLE;

	return percentage_of_led_to_open;
}

//debug only
SH_magnetometer_data* magnetometer_get_local_stored_values()
{
	return data_array;
}























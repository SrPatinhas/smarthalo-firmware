/*
 * SH_Accelerometer_Magnetometer.c
 *
 *  Created on: 2016-02-22
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "SH_typedefs.h"
#include "SH_Accelerometer_Magnetometer.h"
#include "SH_TWI.h"
#include "AcceleroTest.h"
#include "MagnetoTest.h"
#include "SH_Accelero_Magneto_typedefs.h"

static void ACCELEROMETER_cfg_int2();
static void ACCELEROMETER_set_int2_ths();
static void ACCELEROMETER_set_int2_duration();

static uint8_t check_dataready_A(void);
static uint8_t check_dataready_M(void);

static void ACCELEROMETER_set_a_lsb();
static uint8_t get_binary_for_value_with_full_scale_and_int2_ths_a_lsb(uint32_t value);
static uint8_t get_binary_for_value_with_full_scale_and_int2_duration_a_lsb(uint8_t value);

static AcceleroFullScaleValue full_scale_accelerometer = FS2G;		//2,4,8 or 16 g
static AcceleroODRmode ODR_accelerometer = THIRD_DATA_RATE;
static MagnetoODRmode odr_magnetometer = ODR_10_HZ;
static Accelerometer_Magnetometer_System_mode system_mode_magnetometer = IDLE_MODE;
static Accelerometer_Power_Mode acceleretor_power_mode = HIGH_RESOLUTION_MODE;

static float act_ths_a_lsb_value;   	//mg
static float act_dur_a_lsb_value;		//s
static float int_ths_a_lsb;
static float int2_duration_a_lsb;

static int8_t l_temperature_sensor_value;
static int8_t h_temperature_sensor_value;

static int8_t out_x_l_a_value;
static int8_t out_x_h_a_value;
static int8_t out_y_l_a_value;
static int8_t out_y_h_a_value;
static int8_t out_z_l_a_value;
static int8_t out_z_h_a_value;

static int8_t mag_out_x_l_a_value;
static int8_t mag_out_x_h_a_value;
static int8_t mag_out_y_l_a_value;
static int8_t mag_out_y_h_a_value;
static int8_t mag_out_z_l_a_value;
static int8_t mag_out_z_h_a_value;

static uint8_t int2_src_value;

//Functions for initialization of the Accelerometer

//ACCELEROMTER_SET_FULL_SCALE_VALUE
void ACCELEROMETER_set_full_scale_value(AcceleroFullScaleValue full_scale_value){

	full_scale_accelerometer=full_scale_value;

	switch (full_scale_accelerometer){
	case FS2G:
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS1);
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS0);
		break;
	case FS4G:
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS1);
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS0);
		break;
	case FS8G:
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS1);
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS0);
		break;
	case FS16G:
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS1);
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG4_A, FS0);
		break;
	default:
		break;
	}

	ACCELEROMETER_set_a_lsb();
}

//ACCELEROMETER_SET_DATA_RATE_CONFIGURATION
void ACCELEROMETER_set_data_rate_configuration(AcceleroODRmode ODR_value){

	ODR_accelerometer = ODR_value;

	//Set the LSB for the variables that depends on the ODR value
	act_dur_a_lsb_value = (float) ACT_DUR_A_LSB_INITIAL_VALUE/ODR_accelerometer;
	int2_duration_a_lsb = (float) 1/ODR_accelerometer;
	//ACCELEROMETER_set_int2_durantion(); @@TO DO ADD THIS lINE TO ADJUST THE VALUE OF DURATION
	uint8_t data;

	switch (ODR_accelerometer){

	case POWER_DOWN_MODE_DATA_RATE:
		data = POWER_DOWN_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case FIRST_DATA_RATE :
		data = _1_HZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case SECOND_DATA_RATE :
		data = _10_HZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case THIRD_DATA_RATE :
		data = _25_HZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case FOURTH_DATA_RATE :
		data = _50_HZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case FIFTH_DATA_RATE :
		data = _100_HZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case SIXTH_DATA_RATE :
		data = _200_HZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case SEVENTH_DATA_RATE :
		data = _400_HZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case LOW_POWER_MODE_ONLY_DATA_RATE :
		data = _1_620KHZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case HR_NORMAL_DATA_RATE :
		data = _1_344KHZ_OR_5_376KHZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	case LOW_POWER_DATA_RATE :
		data = _1_344KHZ_OR_5_376KHZ_MODE_ACCELEROMETER;
		modify_x_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, data);
	break;

	default : break;
	}
}

void reboot_acclerometer()
{
	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG5_A , BOOT);//reset memory of accelerometer
}


//ACCELEROMETER_SET_POWER_MODE
void ACCELEROMETER_set_power_mode(Accelerometer_Power_Mode accelerometer_power_mode_value){

	acceleretor_power_mode = accelerometer_power_mode_value;

	switch ((int)acceleretor_power_mode){

	case LOW_POWER_MODE :
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, LPen);
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG4_A, HR);
	break;

	case NORMAL_MODE:
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, LPen);
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG4_A, HR);
	break;

	case HIGH_RESOLUTION_MODE:
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, LPen);
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG4_A, HR);
	break;

	default:	break;
	}
}

/* INITIALIZING MODULES */

//INTERUPTS

//ACCELEROMETER_INITIALIZE_INT1_FOR_FIFO
void ACCELEROMETER_initialize_int1_for_FIFO(){
	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG3_A, I1_OVERRUN);
}

//ACCELEROMETER_INITIALIZE_INT2
void ACCELEROMETER_initizalize_int2(){

	ACCELEROMETER_cfg_int2();
	ACCELEROMETER_set_int2_ths();
	ACCELEROMETER_set_int2_duration();
}

static void ACCELEROMETER_set_int2_ths(){
	uint8_t ths_value;
	ths_value = get_binary_for_value_with_full_scale_and_int2_ths_a_lsb(INT_2_THS_VALUE);
	ths_value++;
	nrf_delay_ms(100);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_THS_A, ths_value);
	nrf_delay_ms(100);
}

static void ACCELEROMETER_set_int2_duration(){
	uint8_t duration_value;
	duration_value = get_binary_for_value_with_full_scale_and_int2_duration_a_lsb(INT_2_DURATION_VALUE);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_DURATION_A, duration_value);
}

static void ACCELEROMETER_cfg_int2(){

	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG5_A , LIR_INT2); //latching interrupt, cleared by reading buffer
	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG2_A ,  HPIS2); //high pass filter
	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG2_A ,  HPM0); //high pass filter
	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG2_A ,  HPM1); //high pass filter
	if (INT_2_Z_UP){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, ZHIE_ZUPE);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, ZHIE_ZUPE);
	}

	if (INT_2_Z_DOWN){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, ZLIE_ZDOWNE);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, ZLIE_ZDOWNE);
	}

	if (INT_2_Y_UP){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, YHIE_YUPE);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, YHIE_YUPE);
	}

	if (INT_2_Y_DOWN){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, YLIE_YDOWNE);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, YLIE_YDOWNE);
	}
	if (INT_2_X_UP){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, XHIE_XUPE);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, XHIE_XUPE);
	}

	if (INT_2_X_DOWN){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, XLIE_XDOWNE);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, XLIE_XDOWNE);
	}

	if (INT_2_6D_DETECTION){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, SIXD);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, SIXD);
	}

	if(INT_2_AOI_EVENT){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, AOI);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_CFG_A, AOI);
	}
}

void SH_disable_interrupts_from_accelerometer(){
	clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG6_A, I2_INT2);
}

void SH_enable_interrupts_accelerometer(){
	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG6_A, I2_INT2);

}


void SH_initialization_of_accelerometer22()
{//test code, not for production
	uint8_t buffer[1];
	reboot_acclerometer();
	set_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A , 0x27);
	buffer[0] = check_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG2_A , 0x05);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG3_A , 0x40);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG4_A , 0x88);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG5_A , 0x00);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, INT1_THS_A , 0x03);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, INT1_DURATION_A , 0x14);
	set_register(ACCELEROMETER_DRIVER_ADDRESS, INT1_CFG_A , 0x2A);

	buffer[0]++;
}

//FIFO

void ACCELEROMETER_Set_FIFO_Mode(uint8_t mode)
{
  uint8_t data;

  /* Disable FIFO */
  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG5_A);
  data = (data & ~FIFO_ENABLE);
  set_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG5_A, data);

  /* Set mode */
  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_CTRL_REG_A);
  data = (data & ~FIFO_MODE_MASK) | (mode << FIFO_SHIFT);
  set_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_CTRL_REG_A, data);

  /* Enable FIFO */
  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG5_A);
  data = (data | FIFO_ENABLE);
  set_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG5_A, data);
}

uint8_t FIFO_Set_Depth(uint8_t depth)
{
  uint8_t data;

  /* set FIFO watermark */
  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_CTRL_REG_A);
  data = (data & 0xE0) | (depth << 0);
  set_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_CTRL_REG_A, data);

  return (depth);
}

//TEMPERATURE SENSOR

void ACCELEROMETER_MAGNETOMETER_set_temperature_sensor(bool enable_temperature_sensor){

	if (enable_temperature_sensor){
		//Block data update only if we set the temperature sensor
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,  CTRL_REG4_A, BDU);
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,  TEMP_CFG_REG_A, TEMP_EN1);
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, TEMP_CFG_REG_A, TEMP_EN0);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, TEMP_CFG_REG_A, TEMP_EN1);
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, TEMP_CFG_REG_A, HR);
	}
}

//BDU

void ACCELEROMETER_set_bdu(){

	if (ACCELEROMETER_BLOCK_DATA_UPDATE){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG4_A, BDU);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG4_A, BDU);
	}
}

//AXIS

void ACCELEROMETER_enable_z_axis(bool enable_z_axis){

	if (enable_z_axis){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, Zen);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, Zen);
	}
}

void ACCELEROMETER_enable_y_axis(bool enable_y_axis){

	if (enable_y_axis){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, Yen);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, Yen);
	}
}

void ACCELEROMETER_enable_x_axis(bool enable_x_axis){

	if (enable_x_axis){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,CTRL_REG1_A, Xen);
	}
	else{
		clear_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG1_A, Xen);
	}
}

/* Function for initialization of the Magnetometer */

void MAGNETOMETER_set_ODR(MagnetoODRmode ODR_value){
	odr_magnetometer = ODR_value;

	switch(odr_magnetometer){
		case ODR_10_HZ :
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, FOURTH_BIT);
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, THIRD_BIT);
		break;
		case ODR_20_HZ :
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, FOURTH_BIT);
			set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, THIRD_BIT);
		break;
		case ODR_50_HZ :
			set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, FOURTH_BIT);
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, THIRD_BIT);
		break;
		case ODR_100_HZ :
			set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, FOURTH_BIT);
			set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, THIRD_BIT);
		break;
	}
}

void MAGNETOMETER_set_low_power_mode(bool enable_low_power_mode){

	if (enable_low_power_mode){
		set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, LP);
	}
	else{
		clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, LP);
	}
}

void MAGNETOMETER_set_system_mode(Accelerometer_Magnetometer_System_mode mode){
	system_mode_magnetometer = mode;

	switch(system_mode_magnetometer){
		case CONTINUOUS_MODE ://Continuous mode
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, MD1);
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, MD0);
		break;
		case SINGLE_MODE : //Single mode
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, MD1);
			set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, MD0);
		break;
		case IDLE_MODE : //Idle
			set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, MD1);
			clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, MD0);
		break;
	}
}

void MAGNETOMETER_temperature_compensation(){

	set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_A_M, COMP_TEMP_EN);
}

void MAGNETOMETER_set_interrupt_for_drdy(bool enable_interrupts){

	if (enable_interrupts){
		set_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_C_M, INT_MAG);
	}
	else{
		clear_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS,CFG_REG_C_M, INT_MAG);
	}
}

/* LSB ACCELEROMETER */

//ACCELEROMETER_SET_A_LSB
static void ACCELEROMETER_set_a_lsb(){

	act_ths_a_lsb_value = (float) full_scale_accelerometer/ACT_THS_A_LSB_INITIAL_VALUE;


	switch (full_scale_accelerometer) {
	case FS2G:
		int_ths_a_lsb = (float) INTTHS_16mg;
		break;

	case FS4G:
		int_ths_a_lsb = (float) INTTHS_32mg;
		break;

	case FS8G:
		int_ths_a_lsb = (float) INTTHS_62mg;
		break;

	case FS16G:
		int_ths_a_lsb = (float) INTTHS_186mg;
		break;

	default:
		break;
	}
	//ACCELEROMETER_set_int2_ths();@@ TO DO ADD THIS LINE TO ADJUST THE VALUE OF THS
}

static uint8_t get_binary_for_value_with_full_scale_and_int2_ths_a_lsb(uint32_t value){

	uint8_t binary_value_with_lsb;
	binary_value_with_lsb = value/int_ths_a_lsb;
	return binary_value_with_lsb;
}

static uint8_t get_binary_for_value_with_full_scale_and_int2_duration_a_lsb(uint8_t value){

	uint8_t binary_value_with_lsb;
	binary_value_with_lsb = value/int2_duration_a_lsb;
	return binary_value_with_lsb;
}

/*RETRIEVE_DATA*/

//ACCELEROMETER_AXIS_DATA

/* Return whether data is ready on acc sensor */
static uint8_t check_dataready_A(void)
{
	if (check_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, STATUS_REG_A, ZYXDA)){
	   return(true);
	}
	return (false);
}

bool ACCELEROMETER_get_axis_value(){
	if  (check_dataready_A()){
		out_x_l_a_value = check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_X_L_A);
		out_x_h_a_value = check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_X_H_A);
		out_y_l_a_value = check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_Y_L_A);
		out_y_h_a_value = check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_Y_H_A);
		out_z_l_a_value = check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_Z_L_A);
		out_z_h_a_value = check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_Z_H_A);
		return (1);
	}
	return (0);
}

int16_t ACCELEROMETER_x_axis_data(){

	uint8_t data_l = out_x_l_a_value;
	uint8_t data_h = out_x_h_a_value;
	int16_t x_axis_data;

	x_axis_data =(int16_t) ((((int16_t)data_h << BIT_SHIFT_DATA)& 0xFF00) | data_l);
	x_axis_data = (int16_t) x_axis_data>>BIT_SHIFT_FOR_12_BIT;

	return x_axis_data;
}

int16_t ACCELEROMETER_y_axis_data(){

	uint8_t data_l = out_y_l_a_value;
	uint8_t data_h = out_y_h_a_value;
	int16_t y_axis_data;

	y_axis_data = (int16_t)((((int16_t)data_h << BIT_SHIFT_DATA)& 0xFF00) | data_l);
	y_axis_data = (int16_t) y_axis_data>>BIT_SHIFT_FOR_12_BIT;

	return y_axis_data;
}

int16_t ACCELEROMETER_z_axis_data(){

	uint8_t data_l = out_z_l_a_value;
	uint8_t data_h = out_z_h_a_value;
	int16_t z_axis_data;

	z_axis_data = (int16_t)((((int16_t)data_h << BIT_SHIFT_DATA)& 0xFF00) | data_l);
	z_axis_data = (int16_t) z_axis_data>>BIT_SHIFT_FOR_12_BIT;

	return z_axis_data;
}

//MAGNETOMETER AXIS DATA

static uint8_t check_dataready_M(void){

	if (check_bit_in_register(MAGNETOMETER_DRIVER_ADDRESS, STATUS_REG_M, ZYXDA)){
	   return(1);
	}
	return (0);
}

bool MAGNETOMETER_get_axis_value(){
	if  (check_dataready_M()){
		mag_out_x_l_a_value = check_register(MAGNETOMETER_DRIVER_ADDRESS, OUTX_L_REG_M);
		mag_out_x_h_a_value = check_register(MAGNETOMETER_DRIVER_ADDRESS, OUTX_H_REG_M);
		mag_out_y_l_a_value = check_register(MAGNETOMETER_DRIVER_ADDRESS, OUTY_L_REG_M);
		mag_out_y_h_a_value = check_register(MAGNETOMETER_DRIVER_ADDRESS, OUTY_H_REG_M);
		mag_out_z_l_a_value = check_register(MAGNETOMETER_DRIVER_ADDRESS, OUTZ_L_REG_M);
		mag_out_z_h_a_value = check_register(MAGNETOMETER_DRIVER_ADDRESS, OUTZ_H_REG_M);
		return (1);
	}
	return (0);
}

int16_t MAGNETOMETER_x_axis_data(){

	uint8_t data_l = mag_out_x_l_a_value;
	uint8_t data_h = mag_out_x_h_a_value;
	int16_t x_axis_data;

	x_axis_data =(int16_t) (((data_h << BIT_SHIFT_DATA) & 0xFF00)|data_l);
	//x_axis_data |= (int8_t)data_l;

	// 2s complement
	//if(x_axis_data>=0)return x_axis_data;
	//else return -(~x_axis_data + 1);
	return x_axis_data;
}

int16_t MAGNETOMETER_y_axis_data(){

	uint8_t data_l = mag_out_y_l_a_value;
	uint8_t data_h = mag_out_y_h_a_value;
	int16_t y_axis_data;

	y_axis_data =(int16_t) (((data_h << BIT_SHIFT_DATA) & 0xFF00)|data_l);
	// 2s complement
	//if(y_axis_data>=0)return y_axis_data;
	//else return -(~y_axis_data + 1);
	return y_axis_data;
}

int16_t MAGNETOMETER_z_axis_data(){

	uint8_t data_l = mag_out_z_l_a_value;
	uint8_t data_h = mag_out_z_h_a_value;
	int16_t z_axis_data;

	z_axis_data =(int16_t) (((data_h << BIT_SHIFT_DATA) & 0xFF00)|data_l);
	// 2s complement
//	if(z_axis_data>=0)return z_axis_data;
//	else return -(~z_axis_data + 1);
	return z_axis_data;
}

//INTERRUPT 2 DATA

void ACCELEROMETER_get_src_int2(){

	int2_src_value = check_register(ACCELEROMETER_DRIVER_ADDRESS, INT2_SRC_A);
}

bool is_int2_active(){

	ACCELEROMETER_get_src_int2();
	return CHECK_BIT(int2_src_value, IA);
}

bool is_int2_z_high_value_event(){

	ACCELEROMETER_get_src_int2();
	return CHECK_BIT(int2_src_value, ZH);
}

bool is_int2_z_low_value_event(){

	ACCELEROMETER_get_src_int2();
	return CHECK_BIT(int2_src_value, ZL);
}

bool is_int2_y_high_value_event(){

	ACCELEROMETER_get_src_int2();
	return CHECK_BIT(int2_src_value, YH);
}

bool is_int2_y_low_value_event(){

	ACCELEROMETER_get_src_int2();
	return CHECK_BIT(int2_src_value, YL);
}

bool is_int2_x_high_value_event(){

	ACCELEROMETER_get_src_int2();
	return CHECK_BIT(int2_src_value, YH);
}

bool is_int2_x_low_value_event(){

	ACCELEROMETER_get_src_int2();
	return CHECK_BIT(int2_src_value, YL);
}

//TEMPERATURE SENSOR DATA

int8_t ACCELEROMETER_MAGNETOMETER_retrieve_temperature_sensor_data(){

	//set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,  CTRL_REG4_A, BDU);

	l_temperature_sensor_value  = (int8_t) check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_TEMP_L_A);
	h_temperature_sensor_value  = (int8_t) check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_TEMP_H_A);

	if (check_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, STATUS_REG_AUX_A, TDA)){
		set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS,  CTRL_REG4_A, BDU);
		l_temperature_sensor_value  = (int8_t) check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_TEMP_L_A);
		h_temperature_sensor_value  = (int8_t) check_register(ACCELEROMETER_DRIVER_ADDRESS, OUT_TEMP_H_A);
		//ACCELEROMETER_set_bdu();
	}

	//ACCELEROMETER_set_bdu();
	return h_temperature_sensor_value + TEMPERATURE_OFFSET;
}

/* FIFO routines */

uint8_t Get_FIFO_Mode(void){

  uint8_t data;
  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_CTRL_REG_A);
  return (data & FIFO_MODE_MASK);
}

uint16_t FIFO_Get_Depth(void)
{
  return (FIFO_NUM_ENTRIES);
}

uint8_t FIFO_Get_Thr(void)
{
  uint8_t data;

  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_SRC_REG_A);
  data = (data & 0x80) >> 7;

  return (data);
}

uint8_t FIFO_Get_Overrun(void)
{
  uint8_t data;

  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_SRC_REG_A);
  data = (data & 0x40) >> 6;

  return (data);
}

uint8_t FIFO_Get_Stored(void)
{
  uint8_t data;

  data = check_register(ACCELEROMETER_DRIVER_ADDRESS, FIFO_SRC_REG_A);
  data = (data & 0x1F);

  return (data);
}




/* CLICK DETECTION */

//INITIALIZE_CLICK_DETECTION
void SH_initialize_click_detection(){

	uint8_t data;

	set_bit_in_register(ACCELEROMETER_DRIVER_ADDRESS, CTRL_REG6_A, I2_CLICKen);
	data = CLICK_THS;
	set_register(ACCELEROMETER_DRIVER_ADDRESS, CLICK_THS_A, data );
	data = CLICK_TIME_LATENCY;
	set_register(ACCELEROMETER_DRIVER_ADDRESS, TIME_LATENCY_A, data );
	data = CLICK_TIME_LIMIT;
	set_register(ACCELEROMETER_DRIVER_ADDRESS, TIME_LIMIT_A, data );
	data = CLICK_TIME_WINDOW;
	set_register(ACCELEROMETER_DRIVER_ADDRESS, TIME_WINDOW_A, data );

}

//ENABLE_CLICK_DETECTION
void SH_enable_click_detection(bool enable_click_detection){

	uint8_t data;

	if(enable_click_detection){
		data = CLICK_CFG;
		set_register(ACCELEROMETER_DRIVER_ADDRESS, CLICK_CFG_A, data);
	}
	else {
		data=0b000000;
		set_register(ACCELEROMETER_DRIVER_ADDRESS, CLICK_CFG_A, data);
	}
}



/* FACTORY CALIBRATIONS */

//function to calibrate the lsm303 on the fly
void recalibrate_accel_magneto()
{

	/* code taken from:
	  https://learn.adafruit.com/lsm303-accelerometer-slash-compass-breakout/calibration
	 * */

//	sensors_event_t accelEvent;
//	sensors_event_t magEvent;
//
//	accel.getEvent(&accelEvent);
//	mag.getEvent(&magEvent);
//
//	if (accelEvent.acceleration.x < AccelMinX) AccelMinX = accelEvent.acceleration.x;
//	if (accelEvent.acceleration.x > AccelMaxX) AccelMaxX = accelEvent.acceleration.x;
//
//	if (accelEvent.acceleration.y < AccelMinY) AccelMinY = accelEvent.acceleration.y;
//	if (accelEvent.acceleration.y > AccelMaxY) AccelMaxY = accelEvent.acceleration.y;
//
//	if (accelEvent.acceleration.z < AccelMinZ) AccelMinZ = accelEvent.acceleration.z;
//	if (accelEvent.acceleration.z > AccelMaxZ) AccelMaxZ = accelEvent.acceleration.z;
//
//	if (magEvent.magnetic.x < MagMinX) MagMinX = magEvent.magnetic.x;
//	if (magEvent.magnetic.x > MagMaxX) MagMaxX = magEvent.magnetic.x;
//
//	if (magEvent.magnetic.y < MagMinY) MagMinY = magEvent.magnetic.y;
//	if (magEvent.magnetic.y > MagMaxY) MagMaxY = magEvent.magnetic.y;
//
//	if (magEvent.magnetic.z < MagMinZ) MagMinZ = magEvent.magnetic.z;
//	if (magEvent.magnetic.z > MagMaxZ) MagMaxZ = magEvent.magnetic.z;


}

//function to fetch the factory calibration stored in the lsm303
//SH_calibration_setting_t* get_factory_offsets()
//{
//	SH_calibration_setting_t current_calibration_settings;
//
//	//access storage of calibration settings and then return them in current_calibration_settings
//
//	return &current_calibration_settings;
//}

#ifdef FACTORY_PROGRAMMED_FIRMWARE //to make sure this is only called in the factory firmware
//function to calibrate the lsm303 in factory
//should only be called in factory mode!!!!!
//void factory_calibration()
//{
//	calibrate_soft_iron();
//	calibrate_hard_iron();
//}
//
////factory calibration of soft iron offsets
//static void calibrate_soft_iron()
//{
//
//
//}
//
//
////factory calibration of hard iron offsets
//static void calibrate_hard_iron()
//{
//
//
//}

#endif //FACTORY_PROGRAMMED_FIRMWARE






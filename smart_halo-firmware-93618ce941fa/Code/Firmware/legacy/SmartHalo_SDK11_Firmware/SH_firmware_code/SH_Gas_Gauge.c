/*
 * SH_Gas_Gauge.c

 *
 *  Created on: 2016-05-11
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "SH_Gas_Gauge.h"
#include "SH_TWI.h"
#include "SH_GasGauge_INT.h"

#define MIXED_MODE_FOR_GAS_GAUGE			0
#define INITIALISATION_MODE_FOR_GAS_GAUGE 	MIXED_MODE_FOR_GAS_GAUGE
#define POWER_SAVING_MODE_FOR_GAS_GAUGE 	1

#define OPERATING_MODE_FOR_GAS_GAUGE	1
#define INITIALISATION_RUN				OPERATING_MODE_FOR_GAS_GAUGE
#define GAS_GAUGE_IN_STANDBY		0

#define LOW_LEVEL_SOC_GAS_GAUGE 1.00
#define LOW_VOlTAGE_GAS_GAUGE_TRESHOLD 3.00

#define ENABLE_ALARM_INTERRUPT 	1
#define DISABLE_ALARM_INTERRUPT	0

#define BIT_SHIFT_DATA 8
#define RSENSE	10
#define CNOM	1950
#define RI		190
#define VALUE_OF_REG_VM_CNF ((RI*CNOM)/977.78)
#define VALUE_OF_REG_CC_CNF ((RSENSE * CNOM)/49.556)
#define SOC_CST_FOR_ALARM 	2.00
#define VLT_CST_FOR_ALARM	17.60
#define LSB_VOLTAGE_VALUE	(2.2/1000)
#define LSB_CURRENT_VALUE	(5.88/1000000)
#define LSB_PERCENTAGE_SOC_VALUE (1/512)
#define ZERO_FOR_TEMPERATURE 40

/*
 * ENABLE_ALARM_INTERRUPT
 * Send 1 to enable the alarm interrupt in the register
 * Send 0 to disable the alarm interrupt in the register
 */
static void set_alarm_interrupt (bool enable_interrupt);

/*
 * SET_VMODE
 * Send 1 to enable power saving mode (mixed mode isn't active)
 * Send 0 to enable normal mode (mixed mode is active)
 */
static void set_vmode (bool is_power_saving_mode);

/*
 * SET_GG_TO_RUN
 * Send 1 to enable gas gauge
 * Send 0 to put gas gauge in standby mode
 */
static void set_gg_to_run (bool enable_gas_gauge);

/*
 * Get the value of the register for the voltage of the battery
 */
static uint16_t get_voltage_of_battery();

/*
 * Get the value of the register for the state of charge of the battery
 */
static uint16_t get_soc();

/*
 * Get the value of the register for the temperature of the battery
 * (available only when gas gauge is in mixed mode)
 */
static uint8_t get_temperature();
//static uint16_t get_current();

//Set VMODE
static void set_vmode (bool is_power_saving_mode){

	if (is_power_saving_mode){
		set_bit_in_register(ADDRESS_OF_GAS_GAUGE,REG_MODE, VMODE);
	}
	else{
		clear_bit_in_register(ADDRESS_OF_GAS_GAUGE,REG_MODE, VMODE);
	}
}

//Set Gas gauge to run
static void set_gg_to_run (bool enable_gas_gauge){

	if (enable_gas_gauge){
		set_bit_in_register(ADDRESS_OF_GAS_GAUGE,REG_MODE, GG_RUN);
	}
	else{
		clear_bit_in_register(ADDRESS_OF_GAS_GAUGE,REG_MODE, GG_RUN);
	}
}

//Set relative SOC alarm level
void set_relative_soc_alarm_level(float percentage_alarm_soc_level){

	float value_of_register_in_float;
	uint8_t value_of_register;

	value_of_register_in_float = (SOC_CST_FOR_ALARM * percentage_alarm_soc_level);
	value_of_register = (uint8_t) value_of_register_in_float;

	set_register(ADDRESS_OF_GAS_GAUGE, REG_ALARM_SOC, value_of_register);
}

//Set low voltage threshold for the alarm
void set_low_voltage_threshold(float low_voltage_value_in_v){

	float value_of_register_in_float;
	uint8_t value_of_register;

	value_of_register_in_float = (low_voltage_value_in_v * 1000)/VLT_CST_FOR_ALARM;
	value_of_register = (uint8_t) value_of_register_in_float;

	set_register(ADDRESS_OF_GAS_GAUGE, REG_ALARM_VOLTAGE, value_of_register);
}

//Reset the alarm interrupt
void reset_alarm_interrupt(){

	clear_bit_in_register(ADDRESS_OF_GAS_GAUGE,REG_CTRL, ALM_SOC);
	clear_bit_in_register(ADDRESS_OF_GAS_GAUGE,REG_CTRL, ALM_VOLT);
}

//Enable alarm interrupt
static void set_alarm_interrupt (bool enable_interrupt){

	if (enable_interrupt){
		set_bit_in_register(ADDRESS_OF_GAS_GAUGE, REG_MODE, ALM_ENA);
	}
	else{
		clear_bit_in_register(ADDRESS_OF_GAS_GAUGE, REG_MODE, ALM_ENA);
	}
	//ENABLE or DISABLE the interrupt pin
	SH_enable_disable_interrupt_from_gas_gauge_int(enable_interrupt);
}

//Get voltage of the battery
static uint16_t get_voltage_of_battery(){

	uint8_t data[2];
	uint16_t voltage_battery;

	SH_twi_read_two_registers(ADDRESS_OF_GAS_GAUGE, REG_VOLTAGE_L, data, 2);
	voltage_battery = (uint16_t) ((((uint16_t)data[1] << BIT_SHIFT_DATA)& 0xFF00) | data[0]);
	voltage_battery = (uint16_t) voltage_battery;

	return voltage_battery;

}

//Get SOC
static uint16_t get_soc(){

	uint8_t data[2];
	uint16_t soc_battery;

	SH_twi_read_two_registers(ADDRESS_OF_GAS_GAUGE, REG_SOC_L, data, 2);

	soc_battery = (uint16_t) ((((uint16_t)data[1] << BIT_SHIFT_DATA)& 0xFF00) | data[0]);
	soc_battery = (uint16_t) soc_battery;

	return soc_battery;

}

//Get temperature
static uint8_t get_temperature(){

	uint8_t data[1];
	uint8_t battery_temperature;

	SH_twi_read(ADDRESS_OF_GAS_GAUGE, REG_TEMPERATURE, data, 1);

	battery_temperature = data[0];

	return battery_temperature;

}

/*static uint16_t get_current(){

	uint8_t data[2];
	uint16_t battery_current;

	SH_twi_read_two_registers(ADDRESS_OF_GAS_GAUGE, REG_CURRENT_L, data, 2);

	(uint16_t) ((((uint16_t)data[1] << BIT_SHIFT_DATA)& 0xFF00) | data[0]);
	battery_current = (uint16_t) battery_current;

	return battery_current;
}*/


//Is the voltage low
bool is_there_a_low_condition_with_voltage(){

	if (check_bit_in_register(ADDRESS_OF_GAS_GAUGE, REG_CTRL, ALM_VOLT)){
		return true;
	}
	else {
		return false;
	}
}


//Is the SOC low
bool is_there_a_low_condition_with_soc(){

	if (check_bit_in_register(ADDRESS_OF_GAS_GAUGE, REG_CTRL, ALM_SOC)){
		return true;
	}
	else {
		return false;
	}
}

//Current voltage of the battery in V
float  current_voltage_of_the_battery(){

	uint16_t register_value_for_voltage;
	float voltage;
	register_value_for_voltage = get_voltage_of_battery();

	voltage = (float)(register_value_for_voltage * LSB_VOLTAGE_VALUE);

	return voltage;
}

//Current SOC of the battery in %
float  current_soc_of_the_battery(){

	uint16_t register_value_for_soc;
	float soc;
	register_value_for_soc = get_soc();

	soc = (float)(register_value_for_soc * LSB_PERCENTAGE_SOC_VALUE);

	return soc;
}

//Current temperature of the battery
int8_t  current_temperature_of_the_battery(){

	uint8_t register_value_for_temperature;
	int8_t temperature;
	register_value_for_temperature = get_temperature();

	temperature = (int8_t)(register_value_for_temperature - ZERO_FOR_TEMPERATURE);

	return temperature;
}

//Initialisation of gas gauge
void initialisation_of_gas_gauge(){
	set_vmode(INITIALISATION_MODE_FOR_GAS_GAUGE);
	set_gg_to_run(INITIALISATION_RUN);
}

//Put gas gauge in low power mode
void put_gas_gauge_in_low_power_mode(){
	set_vmode(POWER_SAVING_MODE_FOR_GAS_GAUGE);
}

//Put gas gauge in standby mode
void put_gas_gauge_in_standby_mode(){
	set_gg_to_run(GAS_GAUGE_IN_STANDBY);
}

//Initialization of alarm interrupt
void initialisation_of_alarm_interrupt(){
	set_relative_soc_alarm_level(LOW_LEVEL_SOC_GAS_GAUGE);
	set_low_voltage_threshold(LOW_VOlTAGE_GAS_GAUGE_TRESHOLD);
	enable_alarm_interrupt();
	//Initialize GAS GAUGE interrupt pin
	SH_Gas_Gauge_INT_initialisation();
}

//Disable alarm interrupt
void disable_alarm_interrupt(){
	set_alarm_interrupt(DISABLE_ALARM_INTERRUPT);
}

//Enable alarm interrupt
void enable_alarm_interrupt(){
	set_alarm_interrupt(ENABLE_ALARM_INTERRUPT);
}

//Put gas gauge in mixed mode
void put_gas_gauge_in_mixed_mode(){
	set_vmode(MIXED_MODE_FOR_GAS_GAUGE);
}

//Put gas gauge in operating mode
void put_gas_gauge_in_operating_mode(){
	set_gg_to_run(OPERATING_MODE_FOR_GAS_GAUGE);
}

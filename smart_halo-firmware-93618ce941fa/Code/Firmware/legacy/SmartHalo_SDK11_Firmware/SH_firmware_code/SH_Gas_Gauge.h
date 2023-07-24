/*
 * SH_Gas_Gauge.h
 *
 *  Created on: 2016-05-11
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_GAS_GAUGE_H_
#define SH_FIRMWARE_CODE_SH_GAS_GAUGE_H_

#include "stdafx.h"
#include "SH_typedefs.h"

#define ADDRESS_OF_GAS_GAUGE 0b1110000


//BIT OF REG_MODE
#define VMODE			FIRST_BIT
#define CLR_VM_ADJ		SECOND_BIT
#define CLR_CC_ADJ		THIRD_BIT
#define ALM_ENA			FOURTH_BIT
#define GG_RUN			FIFTH_BIT
#define FORCE_CC		SIXTH_BIT
#define FORCE_VM		SEVENTH_BIT

//BIT OF REG_CTRL
#define IO0DATA			FIRST_BIT
#define GG_RST			SECOND_BIT
#define GG_VM			THIRD_BIT
#define BATFAIL			FOURTH_BIT
#define PORDET			FIFTH_BIT
#define ALM_SOC			SIXTH_BIT
#define ALM_VOLT		SEVENTH_BIT

typedef enum  Reg_Gas_Gauge
{
	REG_MODE			=	0,
	REG_CTRL			=	1,
	REG_SOC_L 			= 	2,
	REG_SOC_H			=	3,
	REG_COUNTER_L		=	4,
	REG_COUNTER_H		=	5,
	REG_CURRENT_L		=	6,
	REG_CURRENT_H		=	7,
	REG_VOLTAGE_L		=	8,
	REG_VOLTAGE_H		=	9,
	REG_TEMPERATURE		=	10,
	REG_CC_ADJ_HIGH		=	11,
	REG_VM_ADJ_HIGH		=	12,
	REG_OCV_L			=	13,
	REG_OCV_H			=	14,
	REG_CC_CNF_L		=	15,
	REG_CC_CNF_H		=	16,
	REG_VM_CNF_L		=	17,
	REG_VM_CNF_H		=	18,
	REG_ALARM_SOC		=	19,
	REG_ALARM_VOLTAGE	=	20,
	REG_CURRENT_THRES	=	21,
	REG_RELAX_COUNT		=	22,
	REG_RELAX_MAX		=	23,
	REG_ID				=	24,
	REG_CC_ADJ_LOW		=	25,
	REG_VM_ADJ_LOW		=	26,
	ACC_CC_ADJ_L		=	27,
	ACC_CC_ADJ_H		=	28,
	ACC_VM_ADJ_L		=	29,
	ACC_VM_ADJ_H		=	30
}Reg_Gas_Gauge;

/*
 * Get current voltage of the battery in V
 */
float  current_voltage_of_the_battery();

/*
 * Get current SOC of the battery in percentage
 * more accurate if the gas gauge is in mixed mode
 */
float  current_soc_of_the_battery();
/*
 * Get current temperature of the battery in degrees
 * Available only if the gas gauge is in mixed mode
 */
int8_t  current_temperature_of_the_battery();

/*
 * Initialization of gas gauge
 * with the initialization defined value
 * + initialization of the interrupt pin
 */
void initialisation_of_gas_gauge();

/*
 * Set GG to low-power mode to save power consuption
 * In voltage mode with no current sensing, a voltage conversion is made every 4 s and a
 * temperature conversion every 16 s. This mode provides the lowest power consumption.
 */
void put_gas_gauge_in_low_power_mode();

/*
 * Set GG to mixed-mode
 * In mixed mode, current is measured continuously (except for a conversion cycle every 4 s
 * and every 16 s seconds for measuring voltage and temperature respectively). This provides
 * the highest accuracy from the gas gauge
 */
void put_gas_gauge_in_mixed_mode();

/*
 * Normal behavior for the GG
 */
void put_gas_gauge_in_operating_mode();

/*
 * Put Gas gauge in standby mode
 * Standby mode. Accumulator and counter
 * registers are frozen, gas gauge and battery
 * monitor functions are in standby.
 */
void put_gas_gauge_in_standby_mode();

/*
 * Initialisation of alarm interrupt
 *
 * FYI:
 * At power-up, or when the STC3115 is reset, the SOC and voltage alarms are enabled
 * (ALM_ENA bit = 1). The ALM pin is high-impedance directly after POR and is driven low if
 * the SOC and/or the voltage is below the default thresholds (1% SOC, 3.00 V voltage), after
 * the first OCV measurement and SOC estimation.
 * + initialization of the interrupt pin
 */
void initialisation_of_alarm_interrupt();

/*
 * Disable the alarm interrupt
 */
void disable_alarm_interrupt();

/*
 * Enable the alarm interrupt
 */
void enable_alarm_interrupt();


#endif /* SH_FIRMWARE_CODE_SH_GAS_GAUGE_H_ */


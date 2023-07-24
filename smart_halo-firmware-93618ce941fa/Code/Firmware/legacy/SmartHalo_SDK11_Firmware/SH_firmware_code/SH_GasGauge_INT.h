/*
 * SH_GasGauge_INT.h
 *
 *  Created on: 2016-07-23
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_GASGAUGE_INT_H_
#define SH_FIRMWARE_CODE_SH_GASGAUGE_INT_H_

#include "SH_pinMap.h"
#include <stdbool.h>
#define PIN_INT_GAS_GAUGE	BATMON_ALARM_PIN

/*
 * Initialization of the int pin for the alarm of the battery monitor with the gpiote driver
 * Interrupts when toggle event on the pin occurs
 */
void SH_Gas_Gauge_INT_initialisation(void);

/*
 * Enable or disable the interrupts on the pin
 */
void SH_enable_disable_interrupt_from_gas_gauge_int(bool enable);

/*
 * True if the interrupt of the gas gauge is high, false otherwise
 */
bool SH_check_for_flag_gas_gauge_int();


#endif /* SH_FIRMWARE_CODE_SH_GASGAUGE_INT_H_ */

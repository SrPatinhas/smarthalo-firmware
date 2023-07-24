/*
 * SH_Accelerometer_INT1.h
 *
 *  Created on: 2016-03-18
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_ACCELEROMETER_INT1_H_
#define SH_FIRMWARE_CODE_SH_ACCELEROMETER_INT1_H_

#include "SH_pinMap.h"
#define PIN_INT1	ACCEL_INT_1_PIN

/*
 * Initialization of the int1 pin with the gpiote driver
 * Interrupts when toggle event on the pin occurs
 */
void SH_Accelerometer_INT1_initialisation(void);

/*
 * Enable or disable the interrupts on the pin
 */
void SH_enable_disable_interrupt_from_accelerometer_int1(bool enable);

/*
 * True if the interrupt1 is high, false otherwise
 */
bool SH_check_for_flag_accelerometer_int1();

#endif /* SH_FIRMWARE_CODE_SH_ACCELEROMETER_INT1_H_ */

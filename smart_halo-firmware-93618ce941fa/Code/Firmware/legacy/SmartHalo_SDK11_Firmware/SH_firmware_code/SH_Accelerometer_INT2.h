/*
 * SH_Accelerometer_INT2.h
 *
 *  Created on: 2016-04-13
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_ACCELEROMETER_INT2_H_
#define SH_FIRMWARE_CODE_SH_ACCELEROMETER_INT2_H_

#include <stdbool.h>
#include "SH_pinMap.h"

#define PIN_INT2	ACCEL_INT_2_PIN

/*
 * Initialization of the int2 pin with the gpiote driver
 * Interrupts when toggle event on the pin occurs
 */
void SH_Accelerometer_INT2_initialisation(void);

/*
 * Enable or disable the interrupts on the pin
 */
void SH_enable_disable_interrupt_from_accelerometer_int2(bool enable);


/*
 * True if the interrupt2 is high, false otherwise
 */
bool SH_check_for_flag_accelerometer_int2();

//resets the falg value after the main state machine checks it
void SH_reset_flag_accelerometer_int2();


#endif /* SH_FIRMWARE_CODE_SH_ACCELEROMETER_INT2_H_ */

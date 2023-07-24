/*
 * FrontLEDDriver.h
 *
 *  Created on: Aug 9, 2019
 */

#ifndef PERIPHERALS_FRONTLEDDRIVER_H_
#define PERIPHERALS_FRONTLEDDRIVER_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

bool init_FrontLEDDriver(void);
void setFrontLEDPercentage_FrontLEDDriver(uint8_t percentage);

#endif /* PERIPHERALS_FRONTLEDDRIVER_H_ */

/**
 * @file battery.h
 * @author Felix Cormier
 * @date May 18 2021
 * @brief Driver for reading the battery voltage via ADC.
 */

#ifndef INC_BATTERY_H_
#define INC_BATTERY_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"

#define ADC_TIMEOUT           0xFFFFFFFF
#define ADC_MAX_VALUE         0xFFFF

HAL_StatusTypeDef batteryGetVoltage(ADC_HandleTypeDef *hadc, double *voltage);

#ifdef __cplusplus
 }
#endif

#endif /* INC_BATTERY_H_ */

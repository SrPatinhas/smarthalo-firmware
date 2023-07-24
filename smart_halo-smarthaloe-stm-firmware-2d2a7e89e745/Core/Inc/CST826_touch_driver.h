/**
 * @file CST826_touch_driver.h
 * @author Felix Cormier
 * @date May 17 2021
 * @brief Driver for the Hynitron CST826 capacitive touch controller.
 */

#ifndef INC_CST826_TOUCH_DRIVER_H_
#define INC_CST826_TOUCH_DRIVER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"

#define TOUCH_CHIP_ADDRESS        0x2A
#define TOUCH_POLLING_TIMEOUT     140000000    // 1 second @ 140 MHz

typedef enum {
  TOUCH_DATA_REG  = 0x02,
  TOUCH_UPDATE_REG = 0x00
} TouchRegister;

typedef enum {
  NOT_VALID_TOUCH = 0,
  IS_VALID_TOUCH = 1
} TouchStatus;

typedef struct {
  uint16_t x;
  uint16_t y;
} TouchCoord;

HAL_StatusTypeDef touchInit(I2C_HandleTypeDef* hi2c);

#ifdef __cplusplus
 }
#endif

#endif /* INC_CST826_TOUCH_DRIVER_H_ */

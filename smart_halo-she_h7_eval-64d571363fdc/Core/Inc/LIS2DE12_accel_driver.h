/**
 * @file LIS2DE12_accel_driver.h
 * @author Felix Cormier
 * @date May 13 2021
 * @brief Driver for the ST Microelectronics LIS2DE12 accelerometer.
 */

#ifndef INC_LIS2DE12_ACCEL_DRIVER_H_
#define INC_LIS2DE12_ACCEL_DRIVER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"

// Pin 3 (SA0) connected to GND
#define HARDWARE_SELECT_BIT       0
#define ACCEL_CHIP_ADDRESS        (0x30 + HARDWARE_SELECT_BIT * 2)
#define ACCEL_POLLING_TIMEOUT     140000000    // 1 second @ 140 MHz

typedef struct {
    double x_g;
    double y_g;
    double z_g;
} Axis;

typedef enum {
    STATUS_REG_AUX  = 0x07,
    OUT_TEMP_L      = 0x0C,
    OUT_TEMP_H      = 0x0D,
    WO_AM_I         = 0x0F,
    CTRL_REG0       = 0x1E,
    TEMP_CFG_REG    = 0x1F,
    CTRL_REG1       = 0x20,
    CTRL_REG2       = 0x21,
    CTRL_REG3       = 0x22,
    CTRL_REG4       = 0x23,
    CTRL_REG5       = 0x24,
    CTRL_REG6       = 0x25,
    REFERENCE       = 0x26,
    STATUS_REG      = 0x27,
    FIFO_READ_START = 0x28,
    OUT_X_H         = 0x29,
    OUT_Y_H         = 0x2B,
    OUT_Z_H         = 0x2D,
    FIFO_CTRL_REG   = 0x2E,
    FIFO_SRC_REG    = 0x2F,
    INT1_CFG        = 0x30,
    INT1_SRC        = 0x31,
    INT1_THS        = 0x32,
    INT1_DURATION   = 0x33,
    INT2_CFG        = 0x34,
    INT2_SRC        = 0x35,
    INT2_THS        = 0x36,
    INT2_DURATION   = 0x37,
    CLICK_CFG       = 0x38,
    CLICK_SRC       = 0x39,
    CLICK_THS       = 0x3A,
    TIME_LIMIT      = 0x3B,
    TIME_LATENCY    = 0x3C,
    TIME_WINDOW     = 0x3D,
    ACT_TS          = 0x3E,
    ACT_DUR         = 0x3F
} AccelRegister;

HAL_StatusTypeDef accelInit(I2C_HandleTypeDef* hi2c);
HAL_StatusTypeDef accelReadID(I2C_HandleTypeDef* hi2c, uint8_t* pData);
HAL_StatusTypeDef accelReadXYZ(I2C_HandleTypeDef* hi2c, Axis * pAxis);
HAL_StatusTypeDef accelEnableTempSensor(I2C_HandleTypeDef* hi2c);
HAL_StatusTypeDef accelDisableTempSensor(I2C_HandleTypeDef* hi2c);

#ifdef __cplusplus
 }
#endif

#endif /* INC_LIS2DE12_ACCEL_DRIVER_H_ */

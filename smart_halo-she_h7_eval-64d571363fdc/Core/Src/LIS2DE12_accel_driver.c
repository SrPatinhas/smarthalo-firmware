/**
 * @file LIS2DE12_accel_driver.c
 * @author Felix Cormier
 * @date May 13 2021
 * @brief Driver for the ST Microelectronics LIS2DE12 accelerometer.
 *
 * Sensor datasheet:
 * https://www.st.com/content/ccc/resource/technical/document/datasheet/group0/72/cb/80/a7/5e/fe/46/9d/DM00153214/files/DM00153214.pdf/jcr:content/translations/en.DM00153214.pdf
 *
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdio.h"
#include "main.h"
#include "LIS2DE12_accel_driver.h"

/* Static function declarations */
static HAL_StatusTypeDef accelReadReg(I2C_HandleTypeDef* hi2c, AccelRegister Address, uint8_t* pData);
static HAL_StatusTypeDef accelWriteRegWithData(I2C_HandleTypeDef* hi2c, AccelRegister reg_adr, uint8_t data);
static HAL_StatusTypeDef accelWriteRegNoData(I2C_HandleTypeDef* hi2c, AccelRegister reg_adr);

uint8_t full_scale_g = 2;

/**
 * @brief Initializes the accelerometer registers to the desired state.
 * @param hi2c an I2C peripheral handler.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef accelInit(I2C_HandleTypeDef* hi2c)
{
  HAL_StatusTypeDef status = 0;
  
  // Disable pull-up
  status |= accelWriteRegWithData(hi2c, CTRL_REG0, 0x90);
  // Set data rate (rate of update) to 100 Hz and enable all axes
  status |= accelWriteRegWithData(hi2c, CTRL_REG1, 0x5F);
  // Disable filters
  status |= accelWriteRegWithData(hi2c, CTRL_REG2, 0x00);
  // Disable interrupts
  status |= accelWriteRegWithData(hi2c, CTRL_REG3, 0x00);
  // Continuous update, 2g full scale, self-test disabled
  status |= accelWriteRegWithData(hi2c, CTRL_REG4, 0x00);
  // Disable FIFO, disable other features
  status |= accelWriteRegWithData(hi2c, CTRL_REG5, 0x00);
  // Disable more features
  status |= accelWriteRegWithData(hi2c, CTRL_REG6, 0x00);
  // Set reference level to 0 for interrupt generation
  status |= accelWriteRegWithData(hi2c, REFERENCE, 0x00);
  // Set FIFO to bypass mode, trigger on INT1
  status |= accelWriteRegWithData(hi2c, FIFO_CTRL_REG, 0x00);
  // Activate all axes on INT1.
  status |= accelWriteRegWithData(hi2c, INT1_CFG, 0x3F);
  // Set INT1 threshold to half the full range
  status |= accelWriteRegWithData(hi2c, INT1_THS, 0x40);
  // Set INT1 duration to 10 ms (1 sample @ 100 Hz)
  status |= accelWriteRegWithData(hi2c, INT1_DURATION, 0x01);
  /* Don't need to configure INT2 */
  /* Don't need to configure Clicks */
  /* Don't need to configure Sleep-to-wake and return-to-sleep (ACT) */

  // Enable temp sensor
  status |= accelEnableTempSensor(hi2c);

  return status;
}

/**
 * @brief Reads the 8-bit ID of the device. 
 * @param hi2c an I2C peripheral handler.
 * @param pData a pointer to a uint8_t where the ID should be written.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef accelReadID(I2C_HandleTypeDef* hi2c, uint8_t* pData)
{
  return accelReadReg(hi2c, WO_AM_I, pData);
}

/**
 * @brief Reads the acceleration on all 3 axes. 
 * @param hi2c an I2C peripheral handler.
 * @param pAxis a pointer to an Axis type where the results will be written.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef accelReadXYZ(I2C_HandleTypeDef* hi2c, Axis * pAxis)
{
  HAL_StatusTypeDef status = 0;
  uint8_t data;
  status |= accelReadReg(hi2c, OUT_X_H, &data);
  pAxis->x_g = (int8_t) data / 255.0 * full_scale_g;
  status |= accelReadReg(hi2c, OUT_Y_H, &data);
  pAxis->y_g = (int8_t) data / 255.0 * full_scale_g;
  status |= accelReadReg(hi2c, OUT_Z_H, &data);
  pAxis->z_g = (int8_t) data / 255.0 * full_scale_g;

  return status;
}

/**
 * @brief Enables the temperature sensor on the accelerometer.
 * @param hi2c an I2C peripheral handler.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef accelEnableTempSensor(I2C_HandleTypeDef* hi2c)
{
  HAL_StatusTypeDef status = 0;
  // Set TEMP_EN[1:0] to 11
  status |= accelWriteRegWithData(hi2c, TEMP_CFG_REG, 0xC0);
  // Set BDU = 1
  status |= accelWriteRegWithData(hi2c, CTRL_REG4, 0x80);
  return status;
}

/**
 * @brief Disables the temperature sensor on the accelerometer.
 * @param hi2c an I2C peripheral handler.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef accelDisableTempSensor(I2C_HandleTypeDef* hi2c)
{
  HAL_StatusTypeDef status = 0;
  // Set TEMP_EN[1:0] to 00
  status |= accelWriteRegWithData(hi2c, TEMP_CFG_REG, 0x00);
  // Set BDU = 0
  status |= accelWriteRegWithData(hi2c, CTRL_REG4, 0x00);
  return status;
}

/**
 * @brief Reads the temperature from the on-chip temp sensor
 * @param hi2c an I2C peripheral handler.
 * @param temp a pointer to a double where the temperature will be stored (in celcius).
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef accelReadTempSensorCelsius(I2C_HandleTypeDef* hi2c, int16_t* temp)
{
  HAL_StatusTypeDef status = 0;
  uint8_t temp_data[2];
  status |= accelReadReg(hi2c, OUT_TEMP_H, &temp_data[1]);
  status |= accelReadReg(hi2c, OUT_TEMP_L, &temp_data[0]);
  *temp = (int16_t)((uint16_t) temp_data[1]) << 8 | temp_data[0];
  return status;
}

//*******************************************************//
//                  Static Functions
//*******************************************************//

static HAL_StatusTypeDef accelReadReg(I2C_HandleTypeDef* hi2c, AccelRegister Address, uint8_t* pData)
{
  HAL_StatusTypeDef status = 0;
  status |= accelWriteRegNoData(hi2c, Address);
  status |= HAL_I2C_Master_Receive(hi2c, ACCEL_CHIP_ADDRESS, pData, 1, ACCEL_POLLING_TIMEOUT);
  return status;
}

static HAL_StatusTypeDef accelWriteRegWithData(I2C_HandleTypeDef* hi2c, AccelRegister reg_adr, uint8_t data) {
  // Send register address plus one byte to write to that register
  uint8_t reg_plus_data[2] = {(uint8_t) reg_adr, data};
  return HAL_I2C_Master_Transmit(hi2c, ACCEL_CHIP_ADDRESS, reg_plus_data, 2, ACCEL_POLLING_TIMEOUT);
}

static HAL_StatusTypeDef accelWriteRegNoData(I2C_HandleTypeDef* hi2c, AccelRegister reg_adr) {
  // Only send register address as data
  uint8_t reg_only = (uint8_t) reg_adr;
  return HAL_I2C_Master_Transmit(hi2c, ACCEL_CHIP_ADDRESS, &reg_only, 1, ACCEL_POLLING_TIMEOUT);
}

#ifdef __cplusplus
 }
#endif

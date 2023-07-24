/**
 * @file CST826_touch_driver.c
 * @author Felix Cormier
 * @date May 17 2021
 * @brief Driver for the Hynitron CST826 capacitive touch controller.
 *
 * Was written based on code sent over by ShineWorld Innovations (LCD Manufacturer)
 *
 * Controller datasheet:
 * http://surenoo.tech/download/04_STP/Controller/CST826.pdf
 *
 * Original code can be found in SHe TechPack -> EE -> Spec Sheets
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "CST826_touch_driver.h"

/* Static function declarations */
static HAL_StatusTypeDef touchWriteRegNoData(I2C_HandleTypeDef* hi2c, TouchRegister reg_adr);
static HAL_StatusTypeDef touchReadRegMultiple(I2C_HandleTypeDef* hi2c, TouchRegister Address, uint8_t* pData, uint8_t len);

HAL_StatusTypeDef touchInit(I2C_HandleTypeDef* hi2c) {
  // Could push update to the CST826 here
  HAL_StatusTypeDef status = 0;
  return status;
}

HAL_StatusTypeDef touchGetData(I2C_HandleTypeDef* hi2c, TouchStatus * touch_status, TouchCoord *coord) {
  HAL_StatusTypeDef status = 0;
  uint8_t lvalue[5];
  uint32_t model = 0;

  // Read touch data into lvalue
  status |= touchReadRegMultiple(hi2c, TOUCH_DATA_REG, lvalue, 5);

  // LSByte is the number of touches
  model = lvalue[0];
  
  // Only support 2 simultaneous touches
  if ((model == 0)||(model > 2))
  {
    coord->x = 0;
    coord->y = 0;
    *touch_status = NOT_VALID_TOUCH;
    return status;
  }

  // Calculate coordinates
  uint16_t x_pos_0 = (((uint16_t)(lvalue[1]&0x0f))<<8) | lvalue[2];
  uint16_t y_pos_0 = (((uint16_t)(lvalue[3]&0x0f))<<8) | lvalue[4];
  
  coord->x = x_pos_0;
  coord->y = y_pos_0;
  *touch_status = IS_VALID_TOUCH;

  return status;    
}

//*******************************************************//
//                  Static Functions
//*******************************************************//

static HAL_StatusTypeDef touchReadRegMultiple(I2C_HandleTypeDef* hi2c, TouchRegister Address, uint8_t* pData, uint8_t len)
{
  HAL_StatusTypeDef status = 0;
  status |= touchWriteRegNoData(hi2c, Address);
  status |= HAL_I2C_Master_Receive(hi2c, TOUCH_CHIP_ADDRESS, pData, len, TOUCH_POLLING_TIMEOUT);
  return status;
}

static HAL_StatusTypeDef touchWriteRegNoData(I2C_HandleTypeDef* hi2c, TouchRegister reg_adr) {
  // Only send register address as data
  uint8_t reg_only = (uint8_t) reg_adr;
  return HAL_I2C_Master_Transmit(hi2c, TOUCH_CHIP_ADDRESS, &reg_only, 1, TOUCH_POLLING_TIMEOUT);
}

#ifdef __cplusplus
 }
#endif
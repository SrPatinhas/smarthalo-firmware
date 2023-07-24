/*
 * SHe_utilities.c
 *
 *  Created on: May 11, 2021
 *      Author: Sean Beitz
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "SHe_utilities.h"
#include "stdio.h"
#include "main.h"

 /*
  * This folder contains various functions and features that will need to
  * be inserted at an appropriate location once we have SHe prototypes and start that project
  *
  * */

 //LDO and Power Supply Enables
#define _LCD_LDO_DISABLE() HAL_GPIO_WritePin(LDO_LCD_EN_GPIO_Port, LDO_LCD_EN_Pin, GPIO_PIN_RESET)
#define _LCD_LDO_ENABLE() HAL_GPIO_WritePin(LDO_LCD_EN_GPIO_Port, LDO_LCD_EN_Pin, GPIO_PIN_SET)
#define _NRF_DISABLE() HAL_GPIO_WritePin(LDO_NRF_EN_GPIO_Port, LDO_NRF_EN_Pin, GPIO_PIN_RESET)
#define _NRF_ENABLE() HAL_GPIO_WritePin(LDO_NRF_EN_GPIO_Port, LDO_NRF_EN_Pin, GPIO_PIN_SET)
#define _LDO_BZR_DISABLE() HAL_GPIO_WritePin(LDO_BZR_EN_GPIO_Port, LDO_BZR_EN_Pin, GPIO_PIN_RESET)
#define _LDO_BZR_ENABLE() HAL_GPIO_WritePin(LDO_BZR_EN_GPIO_Port, LDO_BZR_EN_Pin, GPIO_PIN_SET)

//Power IO
#define _READ_POWERGOOD() HAL_GPIO_ReadPin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET)

#define _LCD_ENABLE() HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET)


//VBAT_AN
//CHARGE_STAT2
//CHARGE_nTE
//CHARGE_CE
//
//TP_RESET
//PGOOD_VIN
//PWM_BL

#ifdef __cplusplus
}
#endif

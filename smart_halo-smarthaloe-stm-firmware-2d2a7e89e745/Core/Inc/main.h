/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define QSPI_FLASH_IO2_Pin GPIO_PIN_2
#define QSPI_FLASH_IO2_GPIO_Port GPIOE
#define CAN_TX_Pin GPIO_PIN_9
#define CAN_TX_GPIO_Port GPIOB
#define I2C_LIGHT_SDA_Pin GPIO_PIN_7
#define I2C_LIGHT_SDA_GPIO_Port GPIOB
#define PGOOD_VIN_Pin GPIO_PIN_4
#define PGOOD_VIN_GPIO_Port GPIOB
#define UART_BLE_RX_Pin GPIO_PIN_3
#define UART_BLE_RX_GPIO_Port GPIOB
#define SPI_LCD_nCS_Pin GPIO_PIN_15
#define SPI_LCD_nCS_GPIO_Port GPIOA
#define MCU_SWCLK_Pin GPIO_PIN_14
#define MCU_SWCLK_GPIO_Port GPIOA
#define MCU_SWDIO_Pin GPIO_PIN_13
#define MCU_SWDIO_GPIO_Port GPIOA
#define LCD_B6_Pin GPIO_PIN_8
#define LCD_B6_GPIO_Port GPIOB
#define I2C_LIGHT_SCL_Pin GPIO_PIN_6
#define I2C_LIGHT_SCL_GPIO_Port GPIOB
#define TP_INT_Pin GPIO_PIN_5
#define TP_INT_GPIO_Port GPIOD
#define LCD_B7_Pin GPIO_PIN_2
#define LCD_B7_GPIO_Port GPIOD
#define QSPI_FLASH_nCS_Pin GPIO_PIN_11
#define QSPI_FLASH_nCS_GPIO_Port GPIOC
#define QSPI_FLASH_IO1_Pin GPIO_PIN_10
#define QSPI_FLASH_IO1_GPIO_Port GPIOC
#define LCD_R5_Pin GPIO_PIN_12
#define LCD_R5_GPIO_Port GPIOA
#define REV_ID_0_Pin GPIO_PIN_4
#define REV_ID_0_GPIO_Port GPIOE
#define LCD_R6_Pin GPIO_PIN_1
#define LCD_R6_GPIO_Port GPIOE
#define LCD_B5_Pin GPIO_PIN_5
#define LCD_B5_GPIO_Port GPIOB
#define LCD_G7_Pin GPIO_PIN_3
#define LCD_G7_GPIO_Port GPIOD
#define TP_RESET_Pin GPIO_PIN_12
#define TP_RESET_GPIO_Port GPIOC
#define CHARGE_nTE_Pin GPIO_PIN_9
#define CHARGE_nTE_GPIO_Port GPIOA
#define LCD_R4_Pin GPIO_PIN_11
#define LCD_R4_GPIO_Port GPIOA
#define REV_ID_1_Pin GPIO_PIN_5
#define REV_ID_1_GPIO_Port GPIOE
#define SPI_LCD_MOSI_Pin GPIO_PIN_7
#define SPI_LCD_MOSI_GPIO_Port GPIOD
#define ACCEL_INT2_Pin GPIO_PIN_4
#define ACCEL_INT2_GPIO_Port GPIOD
#define CAN_RX_Pin GPIO_PIN_0
#define CAN_RX_GPIO_Port GPIOD
#define I2C_ACCEL_SCL_Pin GPIO_PIN_8
#define I2C_ACCEL_SCL_GPIO_Port GPIOA
#define LCD_B4_Pin GPIO_PIN_10
#define LCD_B4_GPIO_Port GPIOA
#define LDO_BZR_EN_Pin GPIO_PIN_2
#define LDO_BZR_EN_GPIO_Port GPIOC
#define REV_ID_2_Pin GPIO_PIN_6
#define REV_ID_2_GPIO_Port GPIOE
#define BZR_Pin GPIO_PIN_1
#define BZR_GPIO_Port GPIOD
#define I2C_ACCEL_SDA_Pin GPIO_PIN_9
#define I2C_ACCEL_SDA_GPIO_Port GPIOC
#define LCD_G6_Pin GPIO_PIN_7
#define LCD_G6_GPIO_Port GPIOC
#define LCD_G2_Pin GPIO_PIN_0
#define LCD_G2_GPIO_Port GPIOC
#define LCD_G5_Pin GPIO_PIN_1
#define LCD_G5_GPIO_Port GPIOC
#define QSPI_FLASH_IO0_Pin GPIO_PIN_3
#define QSPI_FLASH_IO0_GPIO_Port GPIOC
#define LDO_LCD_EN_Pin GPIO_PIN_8
#define LDO_LCD_EN_GPIO_Port GPIOC
#define LCD_HSYNC_Pin GPIO_PIN_6
#define LCD_HSYNC_GPIO_Port GPIOC
#define PWM_PIEZO_VOLUME_Pin GPIO_PIN_0
#define PWM_PIEZO_VOLUME_GPIO_Port GPIOA
#define LCD_VSYNC_Pin GPIO_PIN_4
#define LCD_VSYNC_GPIO_Port GPIOA
#define LCD_R7_Pin GPIO_PIN_4
#define LCD_R7_GPIO_Port GPIOC
#define QSPI_FLASH_CLK_Pin GPIO_PIN_2
#define QSPI_FLASH_CLK_GPIO_Port GPIOB
#define LCD_PCLK_Pin GPIO_PIN_14
#define LCD_PCLK_GPIO_Port GPIOE
#define UART_DBG_TX_Pin GPIO_PIN_15
#define UART_DBG_TX_GPIO_Port GPIOD
#define LDO_NRF_EN_Pin GPIO_PIN_15
#define LDO_NRF_EN_GPIO_Port GPIOB
#define QSPI_FLASH_IO3_Pin GPIO_PIN_1
#define QSPI_FLASH_IO3_GPIO_Port GPIOA
#define SPI_LCD_SCK_Pin GPIO_PIN_5
#define SPI_LCD_SCK_GPIO_Port GPIOA
#define LCD_DE_Pin GPIO_PIN_5
#define LCD_DE_GPIO_Port GPIOC
#define REV_ID_3_Pin GPIO_PIN_7
#define REV_ID_3_GPIO_Port GPIOE
#define LCD_G3_Pin GPIO_PIN_11
#define LCD_G3_GPIO_Port GPIOE
#define UART_DBG_RX_Pin GPIO_PIN_14
#define UART_DBG_RX_GPIO_Port GPIOD
#define LCD_B3_Pin GPIO_PIN_10
#define LCD_B3_GPIO_Port GPIOD
#define CAN_STBY_Pin GPIO_PIN_14
#define CAN_STBY_GPIO_Port GPIOB
#define ACCEL_INT1_Pin GPIO_PIN_2
#define ACCEL_INT1_GPIO_Port GPIOA
#define VBAT_AN_Pin GPIO_PIN_6
#define VBAT_AN_GPIO_Port GPIOA
#define LCD_R3_Pin GPIO_PIN_0
#define LCD_R3_GPIO_Port GPIOB
#define UART_BLE_TX_Pin GPIO_PIN_8
#define UART_BLE_TX_GPIO_Port GPIOE
#define LCD_G4_Pin GPIO_PIN_10
#define LCD_G4_GPIO_Port GPIOB
#define I2C_TP_SDA_Pin GPIO_PIN_13
#define I2C_TP_SDA_GPIO_Port GPIOD
#define CHARGE_STAT1_Pin GPIO_PIN_3
#define CHARGE_STAT1_GPIO_Port GPIOA
#define VIN_AN_Pin GPIO_PIN_7
#define VIN_AN_GPIO_Port GPIOA
#define PWM_BL_Pin GPIO_PIN_9
#define PWM_BL_GPIO_Port GPIOE
#define CHARGE_CE_Pin GPIO_PIN_11
#define CHARGE_CE_GPIO_Port GPIOB
#define LCD_nRESET_Pin GPIO_PIN_12
#define LCD_nRESET_GPIO_Port GPIOB
#define CHARGE_STAT2_Pin GPIO_PIN_8
#define CHARGE_STAT2_GPIO_Port GPIOD
#define I2C_TP_SCL_Pin GPIO_PIN_12
#define I2C_TP_SCL_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */

#define LIGHT_SENSOR_I2C      hi2c1
#define PIEZO_TIMER           htim14
#define PWM_PIEZO_VOLUME      htim2
#define PWM_PIEZO_VOLUME_CH   TIM_CHANNEL_1
#define BATTERY_ADC_CH        ADC_CHANNEL_3
#define VIN_ADC_CH            ADC_CHANNEL_7


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void die();
bool is_block_erased(uint8_t *buf, size_t len);
bool CheckFirmwareMagic(void *buf);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OLED_VDD_PWR_EN_Pin GPIO_PIN_13
#define OLED_VDD_PWR_EN_GPIO_Port GPIOC
#define BLE_RX_Pin GPIO_PIN_2
#define BLE_RX_GPIO_Port GPIOA
#define QSPI_SCLK_Pin GPIO_PIN_3
#define QSPI_SCLK_GPIO_Port GPIOA
#define QSPI_IO3_Pin GPIO_PIN_6
#define QSPI_IO3_GPIO_Port GPIOA
#define QSPI_IO2_Pin GPIO_PIN_7
#define QSPI_IO2_GPIO_Port GPIOA
#define QSPI_IO1_Pin GPIO_PIN_0
#define QSPI_IO1_GPIO_Port GPIOB
#define QSPI_IO0_Pin GPIO_PIN_1
#define QSPI_IO0_GPIO_Port GPIOB
#define QSPI_nCS_Pin GPIO_PIN_11
#define QSPI_nCS_GPIO_Port GPIOB
#define OLED_VCC_PWR_nEN_Pin GPIO_PIN_7
#define OLED_VCC_PWR_nEN_GPIO_Port GPIOC
#define CPU_DEBUG_TX_Pin GPIO_PIN_9
#define CPU_DEBUG_TX_GPIO_Port GPIOA
#define BLE_TX_Pin GPIO_PIN_15
#define BLE_TX_GPIO_Port GPIOA
#define CPU_DEBUG_RX_Pin GPIO_PIN_7
#define CPU_DEBUG_RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define QSPI_SCLK_Pin GPIO_PIN_3
#define QSPI_SCLK_GPIO_Port GPIOA
#define QSPI_IO3_Pin GPIO_PIN_6
#define QSPI_IO3_GPIO_Port GPIOA
#define QSPI_IO2_Pin GPIO_PIN_7
#define QSPI_IO2_GPIO_Port GPIOA
#define QSPI_IO1_Pin GPIO_PIN_0
#define QSPI_IO1_GPIO_Port GPIOB
#define QSPI_IO0_Pin GPIO_PIN_1
#define QSPI_IO0_GPIO_Port GPIOB
#define QSPI_nCS_Pin GPIO_PIN_11
#define QSPI_nCS_GPIO_Port GPIOB

// Lifted from -stm-firmware
#define BLE_EN_GPIO_Port            GPIOC
#define BLE_EN_Pin                  GPIO_PIN_15
#define BLE_RX_GPIO_Port            GPIOA
// #define BLE_RX_Pin                  GPIO_PIN_2
#define BLE_TX_GPIO_Port            GPIOA
// #define BLE_TX_Pin                  GPIO_PIN_15
#define EN_VLED_GPIO_Port           GPIOC
#define EN_VLED_Pin                 GPIO_PIN_14
#define OLED_CS_Pin                 GPIO_PIN_4
#define OLED_DC_Pin                 GPIO_PIN_6
#define OLED_RES_Pin                GPIO_PIN_5
#define SENSORS_EN_GPIO_Port        GPIOB
#define SENSORS_EN_Pin              GPIO_PIN_8
#define VBZ_EN_GPIO_Port            GPIOA
#define VBZ_EN_Pin                  GPIO_PIN_1
#define VBZ_LDO_EN_GPIO_Port        GPIOH
#define VBZ_LDO_EN_Pin              GPIO_PIN_1

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

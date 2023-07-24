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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "FreeRTOS.h"
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
void SystemClock_Config(void);  // needed by wakeup code

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define configUSE_TRACE_FACILITY 1
#define OLED_VDD_PWR_EN_Pin GPIO_PIN_13
#define OLED_VDD_PWR_EN_GPIO_Port GPIOC
#define EN_VLED_Pin GPIO_PIN_14
#define EN_VLED_GPIO_Port GPIOC
#define BLE_EN_Pin GPIO_PIN_15
#define BLE_EN_GPIO_Port GPIOC
#define REV_ID_0_Pin GPIO_PIN_0
#define REV_ID_0_GPIO_Port GPIOH
#define VBZ_LDO_EN_Pin GPIO_PIN_1
#define VBZ_LDO_EN_GPIO_Port GPIOH
#define TOUCH_I2C_SCL_Pin GPIO_PIN_0
#define TOUCH_I2C_SCL_GPIO_Port GPIOC
#define TOUCH_I2C_SDA_Pin GPIO_PIN_1
#define TOUCH_I2C_SDA_GPIO_Port GPIOC
#define VBAT_AN_Pin GPIO_PIN_2
#define VBAT_AN_GPIO_Port GPIOC
#define OLED_SDIN_Pin GPIO_PIN_3
#define OLED_SDIN_GPIO_Port GPIOC
#define FRONTLED_Pin GPIO_PIN_0
#define FRONTLED_GPIO_Port GPIOA
#define VBZ_EN_Pin GPIO_PIN_1
#define VBZ_EN_GPIO_Port GPIOA
#define BLE_RX_Pin GPIO_PIN_2
#define BLE_RX_GPIO_Port GPIOA
#define QSPI_SCLK_Pin GPIO_PIN_3
#define QSPI_SCLK_GPIO_Port GPIOA
#define REV_ID_1_Pin GPIO_PIN_4
#define REV_ID_1_GPIO_Port GPIOA
#define nCHG_Pin GPIO_PIN_5
#define nCHG_GPIO_Port GPIOA
#define nCHG_EXTI_IRQn EXTI9_5_IRQn
#define QSPI_IO3_Pin GPIO_PIN_6
#define QSPI_IO3_GPIO_Port GPIOA
#define QSPI_IO2_Pin GPIO_PIN_7
#define QSPI_IO2_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_4
#define OLED_CS_GPIO_Port GPIOC
#define OLED_RES_Pin GPIO_PIN_5
#define OLED_RES_GPIO_Port GPIOC
#define QSPI_IO1_Pin GPIO_PIN_0
#define QSPI_IO1_GPIO_Port GPIOB
#define QSPI_IO0_Pin GPIO_PIN_1
#define QSPI_IO0_GPIO_Port GPIOB
#define INT_1_XL_Pin GPIO_PIN_2
#define INT_1_XL_GPIO_Port GPIOB
#define INT_1_XL_EXTI_IRQn EXTI2_IRQn
#define LED_I2C_SCL_Pin GPIO_PIN_10
#define LED_I2C_SCL_GPIO_Port GPIOB
#define QSPI_nCS_Pin GPIO_PIN_11
#define QSPI_nCS_GPIO_Port GPIOB
#define OLED_SCLK_Pin GPIO_PIN_13
#define OLED_SCLK_GPIO_Port GPIOB
#define LED_I2C_SDA_Pin GPIO_PIN_14
#define LED_I2C_SDA_GPIO_Port GPIOB
#define OLED_DC_Pin GPIO_PIN_6
#define OLED_DC_GPIO_Port GPIOC
#define OLED_VCC_PWR_EN_Pin GPIO_PIN_7
#define OLED_VCC_PWR_EN_GPIO_Port GPIOC
#define INT_MAG_Pin GPIO_PIN_8
#define INT_MAG_GPIO_Port GPIOA
#define INT_MAG_EXTI_IRQn EXTI9_5_IRQn
#define CPU_DEBUG_TX_Pin GPIO_PIN_9
#define CPU_DEBUG_TX_GPIO_Port GPIOA
#define BZR_Pin GPIO_PIN_10
#define BZR_GPIO_Port GPIOA
#define REV_ID_2_Pin GPIO_PIN_11
#define REV_ID_2_GPIO_Port GPIOA
#define REV_ID_3_Pin GPIO_PIN_12
#define REV_ID_3_GPIO_Port GPIOA
#define BLE_TX_Pin GPIO_PIN_15
#define BLE_TX_GPIO_Port GPIOA
#define PRE_USB_RESET_Pin GPIO_PIN_10
#define PRE_USB_RESET_GPIO_Port GPIOC
#define PRE_USB_RESET_EXTI_IRQn EXTI15_10_IRQn
#define LED_SDB_Pin GPIO_PIN_2
#define LED_SDB_GPIO_Port GPIOD
#define INT_2_XL_Pin GPIO_PIN_3
#define INT_2_XL_GPIO_Port GPIOB
#define INT_2_XL_EXTI_IRQn EXTI3_IRQn
#define SENSORS_I2C_SCL_Pin GPIO_PIN_6
#define SENSORS_I2C_SCL_GPIO_Port GPIOB
#define CPU_DEBUG_RX_Pin GPIO_PIN_7
#define CPU_DEBUG_RX_GPIO_Port GPIOB
#define BOOT_Pin GPIO_PIN_3
#define BOOT_GPIO_Port GPIOH
#define SENSORS_EN_Pin GPIO_PIN_8
#define SENSORS_EN_GPIO_Port GPIOB
#define SENSORS_I2C_SDA_Pin GPIO_PIN_9
#define SENSORS_I2C_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define FLASH_ID              	0x1540EF

#define CONSOLE_MAX_DATA_SIZE 	2048

/* FreeRTOS priority ------------ */
#define PRIORITY_IDLE 				(0)
#define PRIORITY_LOW 					(1)
#define PRIORITY_BELOW_NORMAL	(2)
#define PRIORITY_NORMAL 			(3)
#define PRIORITY_ABOVE_NORMAL (4)
#define PRIORITY_HIGH 				(5)
#define PRIORITY_REAL_TIME 		(6)

#define FREE_HEAP_CRITICAL_MIN     10            // Enable check of Free Heap
#define FREE_HEAP_WASTE_MAX        20            // Enable check of Free Heap

#define HALO_LED_I2C 		hi2c2
#define SENSOR_I2C 			hi2c1
#define OLED_SPI			hspi2
#define DEBUG_PORT			huart1
#define BLE_UART 			huart2
#define WHITE_LED_TIMER		htim2
#define WHITE_LED_TIMER_CH	TIM_CHANNEL_1
#define PIEZO_TIMER			htim6
#define PIEZO_EN_TIMER		htim15
#define PIEZO_EN_TIMER_CH	TIM_CHANNEL_1

#define SPI_EVENT_COMPLETED	0x01
#define SPI_EVENT_ERROR		0x02

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

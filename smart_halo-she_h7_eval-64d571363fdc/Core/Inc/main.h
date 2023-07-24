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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_RESET_Pin GPIO_PIN_6
#define LCD_RESET_GPIO_Port GPIOB
#define CHARGE_STAT1_Pin GPIO_PIN_3
#define CHARGE_STAT1_GPIO_Port GPIOE
#define CHARGE_CE_Pin GPIO_PIN_5
#define CHARGE_CE_GPIO_Port GPIOI
#define CHARGE_nTE_Pin GPIO_PIN_0
#define CHARGE_nTE_GPIO_Port GPIOE
#define CHARGE_STAT2_Pin GPIO_PIN_8
#define CHARGE_STAT2_GPIO_Port GPIOI
#define LDO_NRF_EN_Pin GPIO_PIN_4
#define LDO_NRF_EN_GPIO_Port GPIOE
#define LDO_LCD_EN_Pin GPIO_PIN_6
#define LDO_LCD_EN_GPIO_Port GPIOI
#define PGOOD_VIN_Pin GPIO_PIN_1
#define PGOOD_VIN_GPIO_Port GPIOE
#define LDO_BZR_EN_Pin GPIO_PIN_9
#define LDO_BZR_EN_GPIO_Port GPIOI
#define BZR_GPIO_Pin GPIO_PIN_10
#define BZR_GPIO_GPIO_Port GPIOI
#define BZR_Pin GPIO_PIN_1
#define BZR_GPIO_Port GPIOF
#define VBAT_AN_Pin GPIO_PIN_4
#define VBAT_AN_GPIO_Port GPIOA
#define REV_ID_0_Pin GPIO_PIN_4
#define REV_ID_0_GPIO_Port GPIOC
#define REV_ID_1_Pin GPIO_PIN_15
#define REV_ID_1_GPIO_Port GPIOI
#define REV_ID_2_Pin GPIO_PIN_5
#define REV_ID_2_GPIO_Port GPIOC
#define REV_ID_3_Pin GPIO_PIN_0
#define REV_ID_3_GPIO_Port GPIOJ
#define CAN_STBY_Pin GPIO_PIN_8
#define CAN_STBY_GPIO_Port GPIOH
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

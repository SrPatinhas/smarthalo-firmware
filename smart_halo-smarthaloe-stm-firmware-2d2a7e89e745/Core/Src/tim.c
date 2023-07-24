/**
  ******************************************************************************
  * File Name          : TIM.c
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
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

/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim15;

/* TIM2 init function */
void MX_TIM2_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 3000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim2);

}
/* TIM6 init function */
void MX_TIM6_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 1000;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 290;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}
/* TIM15 init function */
void MX_TIM15_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 0;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 1000;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim15);

}
//
//void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle)
//{
//
//  if(tim_pwmHandle->Instance==TIM2)
//  {
//  /* USER CODE BEGIN TIM2_MspInit 0 */
//
//  /* USER CODE END TIM2_MspInit 0 */
//    /* TIM2 clock enable */
//    __HAL_RCC_TIM2_CLK_ENABLE();
//  /* USER CODE BEGIN TIM2_MspInit 1 */
//
//  /* USER CODE END TIM2_MspInit 1 */
//  }
//  else if(tim_pwmHandle->Instance==TIM15)
//  {
//  /* USER CODE BEGIN TIM15_MspInit 0 */
//
//  /* USER CODE END TIM15_MspInit 0 */
//    /* TIM15 clock enable */
//    __HAL_RCC_TIM15_CLK_ENABLE();
//  /* USER CODE BEGIN TIM15_MspInit 1 */
//
//  /* USER CODE END TIM15_MspInit 1 */
//  }
//}

//void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
//{
//
//  if(tim_baseHandle->Instance==TIM6)
//  {
//  /* USER CODE BEGIN TIM6_MspInit 0 */
//
//  /* USER CODE END TIM6_MspInit 0 */
//    /* TIM6 clock enable */
//    __HAL_RCC_TIM6_CLK_ENABLE();
//
//    /* TIM6 interrupt Init */
//    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
//    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
//  /* USER CODE BEGIN TIM6_MspInit 1 */
//
//  /* USER CODE END TIM6_MspInit 1 */
//  }
//}
//void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
//{
//
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  if(timHandle->Instance==TIM2)
//  {
//  /* USER CODE BEGIN TIM2_MspPostInit 0 */
//
//  /* USER CODE END TIM2_MspPostInit 0 */
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    /**TIM2 GPIO Configuration
//    PA0     ------> TIM2_CH1
//    */
//    GPIO_InitStruct.Pin = FRONTLED_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
//    HAL_GPIO_Init(FRONTLED_GPIO_Port, &GPIO_InitStruct);
//
//  /* USER CODE BEGIN TIM2_MspPostInit 1 */
//
//  /* USER CODE END TIM2_MspPostInit 1 */
//  }
//  else if(timHandle->Instance==TIM15)
//  {
//  /* USER CODE BEGIN TIM15_MspPostInit 0 */
//
//  /* USER CODE END TIM15_MspPostInit 0 */
//
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    /**TIM15 GPIO Configuration
//    PA1     ------> TIM15_CH1N
//    */
//    GPIO_InitStruct.Pin = VBZ_EN_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF14_TIM15;
//    HAL_GPIO_Init(VBZ_EN_GPIO_Port, &GPIO_InitStruct);
//
//  /* USER CODE BEGIN TIM15_MspPostInit 1 */
//
//  /* USER CODE END TIM15_MspPostInit 1 */
//  }
//
//}

//void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle)
//{
//
//  if(tim_pwmHandle->Instance==TIM2)
//  {
//  /* USER CODE BEGIN TIM2_MspDeInit 0 */
//
//  /* USER CODE END TIM2_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_TIM2_CLK_DISABLE();
//  /* USER CODE BEGIN TIM2_MspDeInit 1 */
//
//  /* USER CODE END TIM2_MspDeInit 1 */
//  }
//  else if(tim_pwmHandle->Instance==TIM15)
//  {
//  /* USER CODE BEGIN TIM15_MspDeInit 0 */
//
//  /* USER CODE END TIM15_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_TIM15_CLK_DISABLE();
//  /* USER CODE BEGIN TIM15_MspDeInit 1 */
//
//  /* USER CODE END TIM15_MspDeInit 1 */
//  }
//}

//void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
//{
//
//  if(tim_baseHandle->Instance==TIM6)
//  {
//  /* USER CODE BEGIN TIM6_MspDeInit 0 */
//
//  /* USER CODE END TIM6_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_TIM6_CLK_DISABLE();
//
//    /* TIM6 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
//  /* USER CODE BEGIN TIM6_MspDeInit 1 */
//
//  /* USER CODE END TIM6_MspDeInit 1 */
//  }
//}

/* USER CODE BEGIN 1 */
bool HAL_TIM_SET_PRESCALER(TIM_HandleTypeDef* tim, uint32_t u32Value)
{
	if (tim == NULL) return false;
	__HAL_TIM_SET_PRESCALER(tim,u32Value);
	return true;
}
bool HAL_TIM_SET_STATE(TIM_HandleTypeDef* tim, uint32_t u32Channel, bool bState)
{
	if (tim == NULL) return false;

	if(tim->Instance == TIM15)
	{
		if (true == bState)
		{
			HAL_TIMEx_PWMN_Start(tim, u32Channel);
		}
		else
		{
			HAL_TIMEx_PWMN_Stop(tim, u32Channel);
		}
	}
	else
	{
		if (true == bState)
		{
			HAL_TIM_PWM_Start(tim, u32Channel);
		}
		else
		{
			HAL_TIM_PWM_Stop(tim, u32Channel);
		}
	}
	return true;
}
bool HAL_TIM_SET_VALUE(TIM_HandleTypeDef* tim, uint32_t u32Channel, uint32_t u32Value)
{
	if (tim == NULL) return false;
	__HAL_TIM_SET_COMPARE(tim, u32Channel, u32Value);
	return true;
}
bool HAL_TIM_SET_period(TIM_HandleTypeDef* tim, uint32_t u32Channel, uint32_t u32Value)
{
  if (tim == NULL) return false;
  __HAL_TIM_SET_COMPARE(tim, u32Channel, u32Value);
  return true;
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

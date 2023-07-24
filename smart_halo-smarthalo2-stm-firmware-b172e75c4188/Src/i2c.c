/**
  ******************************************************************************
  * File Name          : I2C.c
  * Description        : This file provides code for the configuration
  *                      of the I2C instances.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include "main.h"
#include "semphr.h"
#include <SystemUtilitiesTask.h>
#include <inttypes.h>
#include "device_telemetry.h"

typedef struct
{
    SemaphoreHandle_t 	xSemaphorePort;
    StaticSemaphore_t 	xSemaphoreBufferPort;
    SemaphoreHandle_t 	xSemaphoreComplete;
    StaticSemaphore_t 	xSemaphoreBuffer;
}oI2C_t;
oI2C_t oI2C1;
oI2C_t oI2C2;
oI2C_t oI2C3;
/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
DMA_HandleTypeDef hdma_i2c2_tx;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x40202EBE;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }

}
/* I2C2 init function */
void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x107075B0;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }

}
/* I2C3 init function */
void MX_I2C3_Init(void)
{

  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x40202EBE;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = SENSORS_I2C_SCL_Pin|SENSORS_I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

        //Create semaphore
        if (oI2C1.xSemaphorePort == NULL)
        {
            oI2C1.xSemaphorePort = xSemaphoreCreateBinaryStatic(&oI2C1.xSemaphoreBufferPort);
            configASSERT(oI2C1.xSemaphorePort);
            xSemaphoreGive(oI2C1.xSemaphorePort);
        }

        if (oI2C1.xSemaphoreComplete == NULL)
        {
            oI2C1.xSemaphoreComplete = xSemaphoreCreateBinaryStatic(&oI2C1.xSemaphoreBuffer);
            configASSERT(oI2C1.xSemaphoreComplete);
        }

  /* USER CODE END I2C1_MspInit 1 */
  }
  else if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C2_MspInit 0 */

  /* USER CODE END I2C2_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB14     ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = LED_I2C_SCL_Pin|LED_I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C2 clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();

    /* I2C2 DMA Init */
    /* I2C2_TX Init */
    hdma_i2c2_tx.Instance = DMA1_Channel4;
    hdma_i2c2_tx.Init.Request = DMA_REQUEST_3;
    hdma_i2c2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_i2c2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c2_tx.Init.Mode = DMA_NORMAL;
    hdma_i2c2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_i2c2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2cHandle,hdmatx,hdma_i2c2_tx);

    /* I2C2 interrupt Init */
    HAL_NVIC_SetPriority(I2C2_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_SetPriority(I2C2_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
  /* USER CODE BEGIN I2C2_MspInit 1 */

        //Create semaphore
        if (oI2C2.xSemaphorePort == NULL)
        {
            oI2C2.xSemaphorePort = xSemaphoreCreateBinaryStatic(&oI2C2.xSemaphoreBufferPort);
            configASSERT(oI2C2.xSemaphorePort);
            xSemaphoreGive(oI2C2.xSemaphorePort);
        }

        if (oI2C2.xSemaphoreComplete == NULL)
        {
            oI2C2.xSemaphoreComplete = xSemaphoreCreateBinaryStatic(&oI2C2.xSemaphoreBuffer);
            configASSERT(oI2C2.xSemaphoreComplete);
        }

  /* USER CODE END I2C2_MspInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspInit 0 */

  /* USER CODE END I2C3_MspInit 0 */

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2C3 GPIO Configuration
    PC0     ------> I2C3_SCL
    PC1     ------> I2C3_SDA
    */
    GPIO_InitStruct.Pin = TOUCH_I2C_SCL_Pin|TOUCH_I2C_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* I2C3 clock enable */
    __HAL_RCC_I2C3_CLK_ENABLE();
  /* USER CODE BEGIN I2C3_MspInit 1 */
        //Create semaphore
        if (oI2C3.xSemaphorePort == NULL)
        {
            oI2C3.xSemaphorePort = xSemaphoreCreateBinaryStatic(&oI2C3.xSemaphoreBufferPort);
            configASSERT(oI2C3.xSemaphorePort);
            xSemaphoreGive(oI2C3.xSemaphorePort);
        }

        if (oI2C3.xSemaphoreComplete == NULL)
        {
            oI2C3.xSemaphoreComplete = xSemaphoreCreateBinaryStatic(&oI2C3.xSemaphoreBuffer);
            configASSERT(oI2C3.xSemaphoreComplete);
        }
  /* USER CODE END I2C3_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(SENSORS_I2C_SCL_GPIO_Port, SENSORS_I2C_SCL_Pin);

    HAL_GPIO_DeInit(SENSORS_I2C_SDA_GPIO_Port, SENSORS_I2C_SDA_Pin);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
  else if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C2_MspDeInit 0 */

  /* USER CODE END I2C2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();

    /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB14     ------> I2C2_SDA
    */
    HAL_GPIO_DeInit(LED_I2C_SCL_GPIO_Port, LED_I2C_SCL_Pin);

    HAL_GPIO_DeInit(LED_I2C_SDA_GPIO_Port, LED_I2C_SDA_Pin);

    /* I2C2 DMA DeInit */
    HAL_DMA_DeInit(i2cHandle->hdmatx);

    /* I2C2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
  /* USER CODE BEGIN I2C2_MspDeInit 1 */

  /* USER CODE END I2C2_MspDeInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspDeInit 0 */

  /* USER CODE END I2C3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C3_CLK_DISABLE();

    /**I2C3 GPIO Configuration
    PC0     ------> I2C3_SCL
    PC1     ------> I2C3_SDA
    */
    HAL_GPIO_DeInit(TOUCH_I2C_SCL_GPIO_Port, TOUCH_I2C_SCL_Pin);

    HAL_GPIO_DeInit(TOUCH_I2C_SDA_GPIO_Port, TOUCH_I2C_SDA_Pin);

  /* USER CODE BEGIN I2C3_MspDeInit 1 */

  /* USER CODE END I2C3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/*! brief Reset I2C for LEDs (channel 2) when they get wedged

    Adapted from code found at:
      https://electronics.stackexchange.com/questions/351972/hal-i2c-hangs-cannot-be-solved-with-standard-routine-use-to-unlock-i2c

    Assumed that caller holds oI2C2.xSemaphorePort
 */
static void I2C2_ClearBusyFlagErratum(I2C_HandleTypeDef *instance)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    int timeout = 100;
    int timeout_cnt = 0;

    // Ensure that we're being called for channel 2
    if (instance->Instance != I2C2) return;

    // 1. Clear PE bit.
    instance->Instance->CR1 &= ~(0x0001);

    //  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
    GPIO_InitStruct.Mode         = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Alternate    = GPIO_AF4_I2C2;
    GPIO_InitStruct.Pull         = GPIO_PULLUP;
    GPIO_InitStruct.Speed        = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin          = LED_I2C_SCL_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, LED_I2C_SCL_Pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin          = LED_I2C_SDA_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, LED_I2C_SDA_Pin, GPIO_PIN_SET);

    // 3. Check SCL and SDA High level in GPIOx_IDR.
    while (GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOB, LED_I2C_SCL_Pin))
    {
        timeout_cnt++;
        if(timeout_cnt>timeout)
            return;
    }

    while (GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOB, LED_I2C_SDA_Pin))
    {
        //Move clock to release I2C
        HAL_GPIO_WritePin(GPIOB, LED_I2C_SCL_Pin, GPIO_PIN_RESET);
        asm("nop");
        HAL_GPIO_WritePin(GPIOB, LED_I2C_SCL_Pin, GPIO_PIN_SET);

        timeout_cnt++;
        if(timeout_cnt>timeout)
            return;
    }

    // 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, LED_I2C_SDA_Pin, GPIO_PIN_RESET);

    //  5. Check SDA Low level in GPIOx_IDR.
    while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(GPIOB, LED_I2C_SDA_Pin))
    {
        timeout_cnt++;
        if(timeout_cnt>timeout)
            return;
    }

    // 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, LED_I2C_SCL_Pin, GPIO_PIN_RESET);

    //  7. Check SCL Low level in GPIOx_IDR.
    while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(GPIOB, LED_I2C_SCL_Pin))
    {
        timeout_cnt++;
        if(timeout_cnt>timeout)
            return;
    }

    // 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, LED_I2C_SCL_Pin, GPIO_PIN_SET);

    // 9. Check SCL High level in GPIOx_IDR.
    while (GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOB, LED_I2C_SCL_Pin))
    {
        timeout_cnt++;
        if(timeout_cnt>timeout)
            return;
    }

    // 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, LED_I2C_SDA_Pin, GPIO_PIN_SET);

    // 11. Check SDA High level in GPIOx_IDR.
    while (GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIOB, LED_I2C_SDA_Pin))
    {
        timeout_cnt++;
        if(timeout_cnt>timeout)
            return;
    }

    // 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;

    GPIO_InitStruct.Pin = LED_I2C_SCL_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_I2C_SDA_Pin;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, LED_I2C_SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, LED_I2C_SDA_Pin, GPIO_PIN_SET);

    // 13. Set SWRST bit in I2Cx_CR1 register.
    instance->Instance->CR1 |= 0x8000;

    asm("nop");

    // 14. Clear SWRST bit in I2Cx_CR1 register.
    instance->Instance->CR1 &= ~0x8000;

    asm("nop");

    // 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register
    instance->Instance->CR1 |= 0x0001;

    // Call initialization function.
    HAL_I2C_Init(instance);
}

/**
 * @brief       HAL_I2C_MemTxCpltCallback()
 * @details     Callback for DMA complete transfer.
 * @public
 * @param[in]   phi2c: Handle on I2C port.
 */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    if (hi2c->Instance == I2C1)
    {
        // xSemaphoreGiveFromISR( oI2C1.xSemaphoreComplete, &xHigherPriorityTaskWoken );
        // portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    } else if (hi2c->Instance == I2C2) {
        xSemaphoreGiveFromISR( oI2C2.xSemaphoreComplete, &xHigherPriorityTaskWoken );
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

/**
 * @brief Log an I2C error
 * @details On a problem with I2C read/write, encode and log the error as
 *          an event. The encoded error will be:
 *          
 *              0xaarreeee
 *              
 *          where:
 *            aa   = i2 device address
 *            rr   = reserved, normally 0, or ff on an undefined HAL I2C error
 *            eeee = i2c error code -- see stm32l4xx_hal_i2c.h
 * 
 * @param phi2c handle to the HAL I2C instance 
 * @param devAddr i2c device address, expect 0x29: photosensor, 0x33: accelerometer
 */
static void logError(I2C_HandleTypeDef *phi2c, uint8_t devAddr)
{
    uint32_t encodedError = 0;
    uint32_t res = phi2c->ErrorCode;

    switch (res) {
    case HAL_I2C_ERROR_NONE:
        break;
    case HAL_I2C_ERROR_BERR:
    case HAL_I2C_ERROR_ARLO:
    case HAL_I2C_ERROR_AF:
    case HAL_I2C_ERROR_OVR:
    case HAL_I2C_ERROR_DMA:
    case HAL_I2C_ERROR_TIMEOUT:
    case HAL_I2C_ERROR_SIZE:
    case HAL_I2C_ERROR_DMA_PARAM:
    case HAL_I2C_ERROR_INVALID_PARAM:
        encodedError = (devAddr << 24) | res;
        log_deviceTelemetry(eI2CERROR, encodedError);
        break;
    default:
        log_deviceTelemetry(eI2CERROR, (devAddr << 24) | 0x00ff0000 | res);
        break;
    }
}

/**
 * @brief       Blocking wrapper for i2c read.
 * @details     Only used on i2c channel 1 (photosensor/accel).
 * @public
 * @param[in]   phi2c Handle to HAL I2C object.
 * @param[in]   u8DeviceAddr I2C Address.
 * @param[in]   u8Register I2C register
 * @param[in]   pu8Data Pointer to buffer to read into.
 * @param[in]   u16Length Number of bytes to read.
 * @return      true on success.
 */
bool HAL_I2C_Read(I2C_HandleTypeDef *phi2c, uint8_t u8DeviceAddr,
                  uint8_t u8Register, uint8_t *pu8Data, uint16_t u16Length)
{
    bool              retval = false;
    HAL_StatusTypeDef Result;

    if (phi2c == NULL || pu8Data == NULL) return retval;

    if (phi2c->Instance != I2C1) return retval;

    xSemaphoreTake(oI2C1.xSemaphorePort, portMAX_DELAY);

    // Start transmission
    Result = HAL_I2C_Mem_Read(phi2c, u8DeviceAddr, u8Register,
                              I2C_MEMADD_SIZE_8BIT, pu8Data, u16Length, 1000);

    if (Result == HAL_OK) {
        retval = true;
    } else {
        logError(phi2c, u8DeviceAddr);
    }

    xSemaphoreGive(oI2C1.xSemaphorePort);

    return retval;
}

/**
 * @brief       Blocking wrapper for i2c writes.
 * @details     Only used on i2c channel 1 (photosensor/accel).
 * @public
 * @param[in]   phi2c Handle to HAL I2C object.
 * @param[in]   u8DeviceAddr I2C Address.
 * @param[in]   u8Register IC register.
 * @param[in]   pu8Data Pointer to data to be written.
 * @param[in]   u16Length Number of bytes to write.
 * @return      true on success.
 */
bool HAL_I2C_Write(I2C_HandleTypeDef *phi2c, uint8_t u8DeviceAddr,
                   uint8_t u8Register, uint8_t *pu8Data, uint16_t u16Length)
{
    bool retval = false;
    HAL_StatusTypeDef Result;

    if (phi2c == NULL || pu8Data == NULL) return retval;

    if (phi2c->Instance != I2C1) return retval;

    xSemaphoreTake(oI2C1.xSemaphorePort, portMAX_DELAY);

    // Start transmission
    Result = HAL_I2C_Mem_Write(phi2c, u8DeviceAddr, u8Register,
                               I2C_MEMADD_SIZE_8BIT, pu8Data, u16Length, 1000);

    if (Result == HAL_OK) {
        retval = true;
    } else {
        logError(phi2c, u8DeviceAddr);
    }

    xSemaphoreGive(oI2C1.xSemaphorePort);

    return retval;
}

/**
 * @brief       Function to access st HAL I2C with DMA.
 * @public
 * @param[in]   phi2c: Handle on I2C port.
 * @param[in]   u8DeviceAddr: I2C Address.
 * @param[in]   u8Register: IC register
 * @param[in]   pu8Data: Handle on buffer to write.
 * @param[in]   u16Length: Number of byte to write.
 * @return      bool: true on success
 */
bool HAL_I2C_Write_DMA(I2C_HandleTypeDef *phi2c, uint8_t u8DeviceAddr,
                       uint8_t u8Register, uint8_t *pu8Data, uint16_t u16Length)
{
    bool              stuck = false;
    bool              retval = false;
    HAL_StatusTypeDef halResult;
    if (phi2c == NULL || pu8Data == NULL) return false;

    // only LEDs I2C channel is set up for DMA
    if (phi2c->Instance != I2C2) return false;

    // Lock the I2C Mutex
    if (xSemaphoreTake(oI2C2.xSemaphorePort, portMAX_DELAY) == pdFALSE) {
        log_Shell("%s: failed lock I2C2 in time: %" PRIu32, __func__, portMAX_DELAY);
        return false;
    }

    // in testing, we see the i2c device getting stuck
    // the symptom is that the DMA call below returns a timeout error
    // below is a retry loop, the intent is to reset and retry the DMA
    // one time after seeing any HAL_ERROR
    for (int i = 0; i < 2; i++) {
        // Start transmission
        halResult =
            HAL_I2C_Mem_Write_DMA(phi2c, u8DeviceAddr, u8Register,
                                  I2C_MEMADD_SIZE_8BIT, pu8Data, u16Length);

        if (halResult != HAL_OK) {
            retval = false;
            char *p, *extra = "";
            switch (halResult) {
            case HAL_OK:
                p = "HAL_OK";
                break;
            case HAL_ERROR:
                p = "HAL_ERROR";
                switch (phi2c->ErrorCode) {
                case HAL_I2C_ERROR_NONE:
                    extra = "HAL_I2C_ERROR_NONE";
                    break;
                case HAL_I2C_ERROR_BERR:
                    extra = "HAL_I2C_ERROR_BERR";
                    break;
                case HAL_I2C_ERROR_ARLO:
                    extra = "HAL_I2C_ERROR_ARLO";
                    break;
                case HAL_I2C_ERROR_AF:
                    extra = "HAL_I2C_ERROR_AF";
                    break;
                case HAL_I2C_ERROR_OVR:
                    extra = "HAL_I2C_ERROR_OVR";
                    break;
                case HAL_I2C_ERROR_DMA:
                    extra = "HAL_I2C_ERROR_DMA";
                    break;
                case HAL_I2C_ERROR_TIMEOUT:
                    extra = "HAL_I2C_ERROR_TIMEOUT";
                    break;
                case HAL_I2C_ERROR_SIZE:
                    extra = "HAL_I2C_ERROR_SIZE";
                    break;
                case HAL_I2C_ERROR_DMA_PARAM:
                    extra = "HAL_I2C_ERROR_DMA_PARAM";
                    break;
                case HAL_I2C_ERROR_INVALID_PARAM:
                    extra = "HAL_I2C_ERROR_INVALID_PARAM";
                    break;
                default:
                    extra = "unknown";
                    break;
                }
                stuck = true;
                break;
            case HAL_BUSY:
                p = "HAL_BUSY";
                stuck = true;   // Simon has a unit stuck in HAL_BUSY
                break;
            default:
                p = "duh... i don't know";
                break;
            }
            log_Shell("%s: HAL_I2C_Mem_Write_DMA returned: %s %s", __func__,
                      p, extra);
            if (stuck == true) {
                log_Shell("%s: calling I2C2_ClearBusyFlagErratum",
                          __func__);
                I2C2_ClearBusyFlagErratum(phi2c);
            }
        } else {
            // Wait transmission completion mutex
            if (xSemaphoreTake(oI2C2.xSemaphoreComplete, portMAX_DELAY) == pdFALSE) {
                log_Shell("%s: I2C2 timeout taking completion semaphore: %" PRIu32, __func__, portMAX_DELAY);
            }
            retval = true;
            break;
        }
    }
    // unlock the I2C mutex
    xSemaphoreGive(oI2C2.xSemaphorePort);

    return retval;
}

// never actually seen this called
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    // if (hi2c->Instance == I2C2) {
    log_Shell("%s: holy crap", __func__);
    // }
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

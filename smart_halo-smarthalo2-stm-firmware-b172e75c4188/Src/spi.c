/**
  ******************************************************************************
  * File Name          : SPI.c
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
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
#include "spi.h"

/* USER CODE BEGIN 0 */
#include "main.h"
#include "semphr.h"
#include <SystemUtilitiesTask.h>

typedef struct
{
    SemaphoreHandle_t 	xSemaphorePort;
    StaticSemaphore_t 	xSemaphoreBufferPort;
    SemaphoreHandle_t 	xSemaphoreComplete;
    StaticSemaphore_t 	xSemaphoreBuffer;
}oSPI_t, *poSPI_t;

oSPI_t oSPI2;
/* USER CODE END 0 */

SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;

/* SPI2 init function */
void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI2 GPIO Configuration
    PC3     ------> SPI2_MOSI
    PB13     ------> SPI2_SCK
    */
    GPIO_InitStruct.Pin = OLED_SDIN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(OLED_SDIN_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OLED_SCLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(OLED_SCLK_GPIO_Port, &GPIO_InitStruct);

    /* SPI2 DMA Init */
    /* SPI2_TX Init */
    hdma_spi2_tx.Instance = DMA1_Channel5;
    hdma_spi2_tx.Init.Request = DMA_REQUEST_1;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_spi2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(spiHandle,hdmatx,hdma_spi2_tx);

    /* SPI2 interrupt Init */
    HAL_NVIC_SetPriority(SPI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
  /* USER CODE BEGIN SPI2_MspInit 1 */
        //Create semaphore
        if (oSPI2.xSemaphorePort == NULL)
        {
            oSPI2.xSemaphorePort = xSemaphoreCreateBinaryStatic(&oSPI2.xSemaphoreBufferPort);
            configASSERT(oSPI2.xSemaphorePort);
            xSemaphoreGive(oSPI2.xSemaphorePort);
        }

        if (oSPI2.xSemaphoreComplete == NULL)
        {
            oSPI2.xSemaphoreComplete = xSemaphoreCreateBinaryStatic(&oSPI2.xSemaphoreBuffer);
            configASSERT(oSPI2.xSemaphoreComplete);
        }
  /* USER CODE END SPI2_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PC3     ------> SPI2_MOSI
    PB13     ------> SPI2_SCK
    */
    HAL_GPIO_DeInit(OLED_SDIN_GPIO_Port, OLED_SDIN_Pin);

    HAL_GPIO_DeInit(OLED_SCLK_GPIO_Port, OLED_SCLK_Pin);

    /* SPI2 DMA DeInit */
    HAL_DMA_DeInit(spiHandle->hdmatx);

    /* SPI2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (hspi->Instance == SPI2)
    {
        xSemaphoreGiveFromISR( oSPI2.xSemaphoreComplete, &xHigherPriorityTaskWoken );
        //xEventGroupSetBitsFromISR(*oSPI2.pEvent, SPI_EVENT_COMPLETED, &xHigherPriorityTaskWoken );
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


/**
 * @brief       HAL_SPI_Write()
 * @details     Function to access st HAL SPI blocking.
 * @public
 * @param[in]   pspi: Handle on spi port.
 * @param[in]   pu8Data: Handle on buffer to write.
 * @param[in]   u16Length: Number of byte to write.
 * @param[out]  pbResponse: Handle on to return the success state.
 * @return      bool: true if success, false otherwise.
 */

bool HAL_SPI_Write_DMA(SPI_HandleTypeDef *pspi, GPIO_TypeDef * pPort, uint32_t u32Pin, uint8_t * pu8Data, uint16_t u16Length, bool *pbResponse)
{
    HAL_StatusTypeDef res;

    if (pspi == NULL || pu8Data == NULL || pbResponse == NULL) return false;

    if (pspi->Instance == SPI2)
    {
        // Lock the SPI Mutex
        if ( xSemaphoreTake(oSPI2.xSemaphorePort, 10000/portTICK_RATE_MS ) == pdFALSE)
        {
            log_Shell("SPI timeout");
            // todo do something if a timeout occurs.
        }
        else
        {
            // Assert the CS
            HAL_GPIO_WritePin(pPort, u32Pin, GPIO_PIN_RESET);

            // Start transmission
            res = HAL_SPI_Transmit_DMA(pspi, pu8Data, u16Length);

            if (res != HAL_OK)
            {
                log_Shell("%s: HAL_SPI_Transmit_DMA returned 0x%02x", __func__, res);
                *pbResponse = false;
            }
            else
            {
                *pbResponse = true;
                // Wait transmission completion mutex
                if ( xSemaphoreTake( oSPI2.xSemaphoreComplete, 100/portTICK_RATE_MS  ) == pdFALSE) {log_Shell("oups... spi EVENT_SPI_TX"); }
            }

            // Deassert the CS
            HAL_GPIO_WritePin(pPort, u32Pin, GPIO_PIN_SET);
        }
        // unlock the SPI mutex
        xSemaphoreGive(oSPI2.xSemaphorePort);
    }
    return true;
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

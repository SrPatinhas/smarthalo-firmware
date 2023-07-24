/**
  ******************************************************************************
  * @file    HW_Init.cpp
  * @author  MCD Application Team
  * @version V1.0.0RC1
  * @date    19-June-2017
  * @brief   This file implements the hardware configuration for the GUI library
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "HW_Init.hpp"

/* USER CODE BEGIN user includes */

/* USER CODE END user includes */

/** @addtogroup HARDWARE CONFIGURATION
* @{
*/

/** @defgroup HARDWARE CONFIGURATION_Private_Variables
* @{
*/

LTDC_HandleTypeDef            hltdc;

DMA2D_HandleTypeDef           hdma2d;

SDRAM_HandleTypeDef hsdram1;

#define REFRESH_COUNT        1835

#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

static FMC_SDRAM_CommandTypeDef Command;

/**
  * @brief  Initialize the LCD Controller.
  * @param  LayerIndex : layer Index.
  * @retval None
  */

void MX_LCD_Init(void)
{
    LTDC_LayerCfgTypeDef pLayerCfg;

    /* De-Initialize LTDC */
    HAL_LTDC_DeInit(&hltdc);
    /* Configure LTDC */

    hltdc.Instance = LTDC;
    hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    hltdc.Init.HorizontalSync = 40;
    hltdc.Init.VerticalSync = 9;
    hltdc.Init.AccumulatedHBP = 53;
    hltdc.Init.AccumulatedVBP = 11;
    hltdc.Init.AccumulatedActiveW = 533;
    hltdc.Init.AccumulatedActiveH = 283;
    hltdc.Init.TotalWidth = 565;
    hltdc.Init.TotalHeigh = 285;
    hltdc.Init.Backcolor.Blue = 0;
    hltdc.Init.Backcolor.Green = 0;
    hltdc.Init.Backcolor.Red = 0;
    if (HAL_LTDC_Init(&hltdc) != HAL_OK)
    {
        Error_Handler();
    }

    pLayerCfg.WindowX0 = 0;
    pLayerCfg.WindowX1 = 480;
    pLayerCfg.WindowY0 = 0;
    pLayerCfg.WindowY1 = 272;
    pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
    pLayerCfg.Alpha = 255;
    pLayerCfg.Alpha0 = 0;
    pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    pLayerCfg.FBStartAdress = 0xC0000000;
    pLayerCfg.ImageWidth = 480;
    pLayerCfg.ImageHeight = 272;
    pLayerCfg.Backcolor.Blue = 0;
    pLayerCfg.Backcolor.Green = 0;
    pLayerCfg.Backcolor.Red = 0;
    if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_LTDC_SetPitch(&hltdc, 480, 0);

}

/**
  * @brief  Initializes LCD IO.
  */
void MX_FMC_Init(void)
{
    /* FMC initialization function */
    FMC_SDRAM_TimingTypeDef SdramTiming;

    /** Perform the SDRAM1 memory initialization sequence
    */
    hsdram1.Instance = FMC_SDRAM_DEVICE;
    /* hsdram1.Init */
    hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
    hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
    hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
    hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
    hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
    /* SdramTiming */
    SdramTiming.LoadToActiveDelay = 2;
    SdramTiming.ExitSelfRefreshDelay = 7;
    SdramTiming.SelfRefreshTime = 4;
    SdramTiming.RowCycleDelay = 7;
    SdramTiming.WriteRecoveryTime = 3;
    SdramTiming.RPDelay = 2;
    SdramTiming.RCDDelay = 2;

    if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
    {
        Error_Handler();
    }

}

/**
  * @brief  Programs the SDRAM device.
  * @retval None
  */
void MX_SDRAM_InitEx(void)
{
    __IO uint32_t tmpmrd = 0;

    /* Step 1: Configure a clock configuration enable command */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget          =  FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 2: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 3: Configure a PALL (precharge all) command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 4: Configure an Auto Refresh command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 8;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 5: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          | \
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   | \
             SDRAM_MODEREG_CAS_LATENCY_3           | \
             SDRAM_MODEREG_OPERATING_MODE_STANDARD | \
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

    /* Step 6: Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(&hsdram1, REFRESH_COUNT);
}

/* DMA2D init function */
void MX_DMA2D_Init(void)
{
    /* Configure the DMA2D default mode */

    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_M2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset = 0;
    hdma2d.LayerCfg[1].InputOffset = 0;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0;
    if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
    {
        Error_Handler();
    }

}

/*  MSPInit/deInit Implementation */

void HAL_LTDC_MspInit(LTDC_HandleTypeDef* ltdcHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (ltdcHandle->Instance == LTDC)
    {
        /* USER CODE BEGIN LTDC_MspInit 0 */

        /* USER CODE END LTDC_MspInit 0 */
        /* Enable Peripheral clock */
        __HAL_RCC_LTDC_CLK_ENABLE();

        /**LTDC GPIO Configuration
        PE4     ------> LTDC_B0
        PJ13     ------> LTDC_B1
        PK7     ------> LTDC_DE
        PK6     ------> LTDC_B7
        PK5     ------> LTDC_B6
        PG12     ------> LTDC_B4
        PJ14     ------> LTDC_B2
        PI10     ------> LTDC_HSYNC
        PK4     ------> LTDC_B5
        PJ15     ------> LTDC_B3
        PI9     ------> LTDC_VSYNC
        PK1     ------> LTDC_G6
        PK2     ------> LTDC_G7
        PI15     ------> LTDC_R0
        PJ11     ------> LTDC_G4
        PK0     ------> LTDC_G5
        PI14     ------> LTDC_CLK
        PJ8     ------> LTDC_G1
        PJ10     ------> LTDC_G3
        PJ7     ------> LTDC_G0
        PJ9     ------> LTDC_G2
        PJ6     ------> LTDC_R7
        PJ4     ------> LTDC_R5
        PJ5     ------> LTDC_R6
        PJ3     ------> LTDC_R4
        PJ2     ------> LTDC_R3
        PJ0     ------> LTDC_R1
        PJ1     ------> LTDC_R2
        */
        GPIO_InitStruct.Pin = LCD_B0_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(LCD_B0_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_B1_Pin | LCD_B2_Pin | LCD_B3_Pin | LCD_G4_Pin
                              | LCD_G1_Pin | LCD_G3_Pin | LCD_G0_Pin | LCD_G2_Pin
                              | LCD_R7_Pin | LCD_R5_Pin | LCD_R6_Pin | LCD_R4_Pin
                              | LCD_R3_Pin | LCD_R1_Pin | LCD_R2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_DE_Pin | LCD_B7_Pin | LCD_B6_Pin | LCD_B5_Pin
                              | LCD_G6_Pin | LCD_G7_Pin | LCD_G5_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_B4_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(LCD_B4_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_HSYNC_Pin | LCD_VSYNC_Pin | LCD_R0_Pin | LCD_CLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(LTDC_IRQn);
        /* USER CODE BEGIN LTDC_MspInit 1 */

        /* USER CODE END LTDC_MspInit 1 */
    }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* ltdcHandle)
{
    if (ltdcHandle->Instance == LTDC)
    {
        /* USER CODE BEGIN LTDC_MspDeInit 0 */

        /* USER CODE END LTDC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LTDC_CLK_DISABLE();

        /**LTDC GPIO Configuration
        PE4     ------> LTDC_B0
        PJ13     ------> LTDC_B1
        PK7     ------> LTDC_DE
        PK6     ------> LTDC_B7
        PK5     ------> LTDC_B6
        PG12     ------> LTDC_B4
        PJ14     ------> LTDC_B2
        PI10     ------> LTDC_HSYNC
        PK4     ------> LTDC_B5
        PJ15     ------> LTDC_B3
        PI9     ------> LTDC_VSYNC
        PK1     ------> LTDC_G6
        PK2     ------> LTDC_G7
        PI15     ------> LTDC_R0
        PJ11     ------> LTDC_G4
        PK0     ------> LTDC_G5
        PI14     ------> LTDC_CLK
        PJ8     ------> LTDC_G1
        PJ10     ------> LTDC_G3
        PJ7     ------> LTDC_G0
        PJ9     ------> LTDC_G2
        PJ6     ------> LTDC_R7
        PJ4     ------> LTDC_R5
        PJ5     ------> LTDC_R6
        PJ3     ------> LTDC_R4
        PJ2     ------> LTDC_R3
        PJ0     ------> LTDC_R1
        PJ1     ------> LTDC_R2
        */
        HAL_GPIO_DeInit(LCD_B0_GPIO_Port, LCD_B0_Pin);

        HAL_GPIO_DeInit(GPIOJ, LCD_B1_Pin | LCD_B2_Pin | LCD_B3_Pin | LCD_G4_Pin
                        | LCD_G1_Pin | LCD_G3_Pin | LCD_G0_Pin | LCD_G2_Pin
                        | LCD_R7_Pin | LCD_R5_Pin | LCD_R6_Pin | LCD_R4_Pin
                        | LCD_R3_Pin | LCD_R1_Pin | LCD_R2_Pin);

        HAL_GPIO_DeInit(GPIOK, LCD_DE_Pin | LCD_B7_Pin | LCD_B6_Pin | LCD_B5_Pin
                        | LCD_G6_Pin | LCD_G7_Pin | LCD_G5_Pin);

        HAL_GPIO_DeInit(LCD_B4_GPIO_Port, LCD_B4_Pin);

        HAL_GPIO_DeInit(GPIOI, LCD_HSYNC_Pin | LCD_VSYNC_Pin | LCD_R0_Pin | LCD_CLK_Pin);

        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(LTDC_IRQn);

        /* USER CODE BEGIN LTDC_MspDeInit 1 */

        /* USER CODE END LTDC_MspDeInit 1 */
    }
}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void)
{
    /* USER CODE BEGIN FMC_MspInit 0 */

    /* USER CODE END FMC_MspInit 0 */
    GPIO_InitTypeDef GPIO_InitStruct;
    if (FMC_Initialized)
    {
        return;
    }
    FMC_Initialized = 1;
    /* Peripheral clock enable */
    __HAL_RCC_FMC_CLK_ENABLE();

    /** FMC GPIO Configuration
    PE1   ------> FMC_NBL1
    PE0   ------> FMC_NBL0
    PG15   ------> FMC_SDNCAS
    PD0   ------> FMC_D2
    PD1   ------> FMC_D3
    PF0   ------> FMC_A0
    PF1   ------> FMC_A1
    PF2   ------> FMC_A2
    PF3   ------> FMC_A3
    PG8   ------> FMC_SDCLK
    PF4   ------> FMC_A4
    PH5   ------> FMC_SDNWE
    PH3   ------> FMC_SDNE0
    PF5   ------> FMC_A5
    PD15   ------> FMC_D1
    PD10   ------> FMC_D15
    PC3   ------> FMC_SDCKE0
    PD14   ------> FMC_D0
    PD9   ------> FMC_D14
    PD8   ------> FMC_D13
    PF12   ------> FMC_A6
    PG1   ------> FMC_A11
    PF15   ------> FMC_A9
    PF13   ------> FMC_A7
    PG0   ------> FMC_A10
    PE8   ------> FMC_D5
    PG5   ------> FMC_BA1
    PG4   ------> FMC_BA0
    PF14   ------> FMC_A8
    PF11   ------> FMC_SDNRAS
    PE9   ------> FMC_D6
    PE11   ------> FMC_D8
    PE14   ------> FMC_D11
    PE7   ------> FMC_D4
    PE10   ------> FMC_D7
    PE12   ------> FMC_D9
    PE15   ------> FMC_D12
    PE13   ------> FMC_D10
    */
    GPIO_InitStruct.Pin = FMC_NBL1_Pin | FMC_NBL0_Pin | FMC_D5_Pin | FMC_D6_Pin
                          | FMC_D8_Pin | FMC_D11_Pin | FMC_D4_Pin | FMC_D7_Pin
                          | FMC_D9_Pin | FMC_D12_Pin | FMC_D10_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FMC_SDNCAS_Pin | FMC_SDCLK_Pin | FMC_A11_Pin | FMC_A10_Pin
                          | FMC_BA1_Pin | FMC_BA0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FMC_D2_Pin | FMC_D3_Pin | FMC_D1_Pin | FMC_D15_Pin
                          | FMC_D0_Pin | FMC_D14_Pin | FMC_D13_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FMC_A0_Pin | FMC_A1_Pin | FMC_A2_Pin | FMC_A3_Pin
                          | FMC_A4_Pin | FMC_A5_Pin | FMC_A6_Pin | FMC_A9_Pin
                          | FMC_A7_Pin | FMC_A8_Pin | FMC_SDNRAS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FMC_SDNME_Pin | FMC_SDNE0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FMC_SDCKE0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(FMC_SDCKE0_GPIO_Port, &GPIO_InitStruct);

    /* USER CODE BEGIN FMC_MspInit 1 */

    /* USER CODE END FMC_MspInit 1 */
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef* hsdram)
{
    /* USER CODE BEGIN SDRAM_MspInit 0 */

    /* USER CODE END SDRAM_MspInit 0 */
    HAL_FMC_MspInit();
    /* USER CODE BEGIN SDRAM_MspInit 1 */

    /* USER CODE END SDRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void)
{
    /* USER CODE BEGIN FMC_MspDeInit 0 */

    /* USER CODE END FMC_MspDeInit 0 */
    if (FMC_DeInitialized)
    {
        return;
    }
    FMC_DeInitialized = 1;
    /* Peripheral clock enable */
    __HAL_RCC_FMC_CLK_DISABLE();

    /** FMC GPIO Configuration
    PE1   ------> FMC_NBL1
    PE0   ------> FMC_NBL0
    PG15   ------> FMC_SDNCAS
    PD0   ------> FMC_D2
    PD1   ------> FMC_D3
    PF0   ------> FMC_A0
    PF1   ------> FMC_A1
    PF2   ------> FMC_A2
    PF3   ------> FMC_A3
    PG8   ------> FMC_SDCLK
    PF4   ------> FMC_A4
    PH5   ------> FMC_SDNWE
    PH3   ------> FMC_SDNE0
    PF5   ------> FMC_A5
    PD15   ------> FMC_D1
    PD10   ------> FMC_D15
    PC3   ------> FMC_SDCKE0
    PD14   ------> FMC_D0
    PD9   ------> FMC_D14
    PD8   ------> FMC_D13
    PF12   ------> FMC_A6
    PG1   ------> FMC_A11
    PF15   ------> FMC_A9
    PF13   ------> FMC_A7
    PG0   ------> FMC_A10
    PE8   ------> FMC_D5
    PG5   ------> FMC_BA1
    PG4   ------> FMC_BA0
    PF14   ------> FMC_A8
    PF11   ------> FMC_SDNRAS
    PE9   ------> FMC_D6
    PE11   ------> FMC_D8
    PE14   ------> FMC_D11
    PE7   ------> FMC_D4
    PE10   ------> FMC_D7
    PE12   ------> FMC_D9
    PE15   ------> FMC_D12
    PE13   ------> FMC_D10
    */
    HAL_GPIO_DeInit(GPIOE, FMC_NBL1_Pin | FMC_NBL0_Pin | FMC_D5_Pin | FMC_D6_Pin
                    | FMC_D8_Pin | FMC_D11_Pin | FMC_D4_Pin | FMC_D7_Pin
                    | FMC_D9_Pin | FMC_D12_Pin | FMC_D10_Pin);

    HAL_GPIO_DeInit(GPIOG, FMC_SDNCAS_Pin | FMC_SDCLK_Pin | FMC_A11_Pin | FMC_A10_Pin
                    | FMC_BA1_Pin | FMC_BA0_Pin);

    HAL_GPIO_DeInit(GPIOD, FMC_D2_Pin | FMC_D3_Pin | FMC_D1_Pin | FMC_D15_Pin
                    | FMC_D0_Pin | FMC_D14_Pin | FMC_D13_Pin);

    HAL_GPIO_DeInit(GPIOF, FMC_A0_Pin | FMC_A1_Pin | FMC_A2_Pin | FMC_A3_Pin
                    | FMC_A4_Pin | FMC_A5_Pin | FMC_A6_Pin | FMC_A9_Pin
                    | FMC_A7_Pin | FMC_A8_Pin | FMC_SDNRAS_Pin);

    HAL_GPIO_DeInit(GPIOH, FMC_SDNME_Pin | FMC_SDNE0_Pin);

    HAL_GPIO_DeInit(FMC_SDCKE0_GPIO_Port, FMC_SDCKE0_Pin);

    /* USER CODE BEGIN FMC_MspDeInit 1 */

    /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef* hsdram)
{
    /* USER CODE BEGIN SDRAM_MspDeInit 0 */

    /* USER CODE END SDRAM_MspDeInit 0 */
    HAL_FMC_MspDeInit();
    /* USER CODE BEGIN SDRAM_MspDeInit 1 */

    /* USER CODE END SDRAM_MspDeInit 1 */
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* dma2dHandle)
{
    if (dma2dHandle->Instance == DMA2D)
    {
        /* USER CODE BEGIN DMA2D_MspInit 0 */

        /* USER CODE END DMA2D_MspInit 0 */
        /* Enable Peripheral clock */
        __HAL_RCC_DMA2D_CLK_ENABLE();

        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(DMA2D_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(DMA2D_IRQn);
        /* USER CODE BEGIN DMA2D_MspInit 1 */

        /* USER CODE END DMA2D_MspInit 1 */
    }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* dma2dHandle)
{
    if (dma2dHandle->Instance == DMA2D)
    {
        /* USER CODE BEGIN DMA2D_MspDeInit 0 */

        /* USER CODE END DMA2D_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_DMA2D_CLK_DISABLE();

        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(DMA2D_IRQn);

        /* USER CODE BEGIN DMA2D_MspDeInit 1 */

        /* USER CODE END DMA2D_MspDeInit 1 */
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

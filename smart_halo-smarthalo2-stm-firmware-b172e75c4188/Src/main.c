/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "quadspi.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "touchsensing.h"
#include "tsc.h"
#include "usart.h"
#include "wwdg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <SystemUtilitiesTask.h>
#include <CommunicationTask.h>
#include <RideTask.h>
#include "GraphicsTask.h"
#include "SensorsTask.h"
#include "SoundTask.h"
#include "WatchdogTask.h"
#include "BootLoaderImport.h"
#include "PersonalityTask.h"
#include "NightLightTask.h"
#include "AlarmTask.h"
#include "BoardRev.h"
#include "device_telemetry.h"
#include "reboot.h"
#include "PiezoDriver.h"
#include "rtcUtils.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Magic things from the bootloader
extern volatile uint32_t    __boot_flags__;
extern volatile uint32_t    __boot_reason__;
mcb_t                       *MCB = (mcb_t *)MCB_MAGIC;
BootFlags_t                 *BF = (BootFlags_t *)&__boot_flags__;
eBootReason_t               *BR = (eBootReason_t *)&__boot_reason__;

static char *stackOverflowTaskName;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* N.B.
      Below the following modification is required after GPIO Init before DMA Init
      MX_GPIO_Init();
      HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, GPIO_PIN_SET);
      MX_DMA_Init();

      The BLE_EN_Pin must be set to properly enable the DMA.
      If this code is regenerated, make sure to add it back (and re-add the
      definition of SH2_MODS_COMPLETE)

      Also, the MX_IWDG_Init() below should be commented out.
      WatchdogTaskInit() below will take care of that initialization
      as needed.
  */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, GPIO_PIN_SET);
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_QUADSPI_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TSC_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TOUCHSENSING_Init();
  // MX_I2C3_Init();
  MX_TIM15_Init();
  MX_CRC_Init();
  // MX_IWDG_Init();
  MX_WWDG_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
#define SH2_MODS_COMPLETE
  /* USER CODE BEGIN 2 */
  /*
      Ensure that our SH2 modifications (described above) are present.

      If CubeMX regens the code outside these USER CODE BEGIN blocks, the

        #define SH2_MODS_COMPLETE

      will be removed, resulting in the the block below triggering a
      compilation failure.
  */
  #ifndef SH2_MODS_COMPLETE
  #error The SmartHalo2 modifications to the init code are missing!
  #endif

  init_BoardRev();
  uint8_t *p = getVersion_CommunicationTask();
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  printf("\nSmartHalo 2 Firmware: v%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
  printf("Board revision: %u\n", get_BoardRev());
  printf("Booting RTOS...\n");

  printf("Boot reason: %x\n", *BR);
  extern volatile uint32_t __update_crc__;

#ifndef GOLDEN
  // The bootloader does not know about WWDG -- this block should
  // trigger the downgrade to golden just like a IWDG event would
  // no point in a log_deviceTelemetry() call, it will be lost
  if (BF->ignoreWatchdog == 0 && (*BR & eWWDGRST)) {
    printf("ignoreWatchdog: %u, eWWDGRST: %u, address: %lx\n",
           BF->ignoreWatchdog, (*BR & eWWDGRST) ? true : false, __update_crc__);
    reboot_nosync();
  }
#endif

  saveResetType_SystemUtilities();
  eRebootReason_t reason = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_REBOOT);
  if (reason) {
    saveRebootReason_SystemUtilities(reason);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_REBOOT, 0);
  }

  char *name1 = (char *)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_LASTTASK1);
  char *name2 = (char *)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_LASTTASK2);
  if (name1) {
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_LASTTASK1, 0);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_LASTTASK2, 0);
    static char lasttask[LASTTASK_STRING_LEN];
    snprintf(lasttask, sizeof(lasttask), "%s,%s", name1, name2);
    log_deviceTelemetry(eLASTTASK, (uint32_t)lasttask);
    printf("found some last task data: %s, %s\n", name1, name2);
  }

  if ((*BR & eIWDGRST)) {
    log_deviceTelemetry(eIWDG, 0);
  }

  // Pick-up any messages left for us by a WWDG event
  if ((*BR & eWWDGRST)) {
    log_deviceTelemetry(eWWDG, __update_crc__);
    collectCrashDump_rtcUtils();
    log_deviceTelemetry(eCRASHD0, 0);
    log_deviceTelemetry(eCRASHD1, 0);
    log_deviceTelemetry(eCRASHD2, 0);
    log_deviceTelemetry(eCRASHD3, 0);
    printf("message to you Rudy: %lx\n", __update_crc__);
    // If the interrupt handler stuffed the task name pointer into __update_crc__
    // then maybe you want to print that instead
    // printf("message to you Rudy: %s\n", (char*)__update_crc__);
    __update_crc__ = 0;
  }

  // Pick-up anything left for us after a crash
  if (BF->crash) {
    uint32_t fault = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_FAULT);
    if (fault) {
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FAULT, 0);
    } else {
      // Unexpected path
      fault = BF->all;
    }
    printf("Logging a crash, fault type: %lx\n", fault);
    log_deviceTelemetry(eCRASH, fault);
    if (__update_crc__) {
      printf("We have an address: %lx\n", __update_crc__);
      log_deviceTelemetry(eCRASHADDR, __update_crc__);
    }
    collectCrashDump_rtcUtils();
    log_deviceTelemetry(eCRASHD0, 0);
    log_deviceTelemetry(eCRASHD1, 0);
    log_deviceTelemetry(eCRASHD2, 0);
    log_deviceTelemetry(eCRASHD3, 0);
    BF->crash = 0;
  }
  if (BF->softWD) {
    log_deviceTelemetry(eSOFTWD, HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_SOFTWD));
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_SOFTWD, 0);
    BF->softWD = 0;
  }
  stackOverflowTaskName = (char *)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_TASKNAME);
  if (stackOverflowTaskName) {
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_TASKNAME, 0);
    log_deviceTelemetry(eSTACKOVERFLOW, (uint32_t)stackOverflowTaskName);
  }
  log_deviceTelemetry(eBOOTREASON, *BR);

  init_SystemUtilities();
  init_GraphicsTask();
  init_SensorsTask();
  init_CommunicationTask();
  init_SoundTask();
  WatchdogTaskInit();

  //Application Layer
  init_PersonalityTask();
  init_NightLightTask();
#ifndef GOLDEN
  init_RideTask();
  init_AlarmTask();
#endif
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    SOFT_CRASH(eMAIN);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 30;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_I2C3
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  if (htim->Instance == TIM6) {
    updateTicks_PiezoDriver();
  }
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM16) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
        log_Shell("%s: no idea how we got here but it's thing", __func__);

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

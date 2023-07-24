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
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma.h"
#include "iwdg.h"
#include "quadspi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "qspiflash.h"
#include "Boot.h"
#include "FlashMem.h"
#include "ExtFlash.h"
#include "log.h"
#include "mcb.h"
#include "BootLoaderVersion.h"
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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void die()
{
    LOG ("All those moments will be lost in time, like tears in rain.\n");

    for (int i = 0; i < 5000000; i++)
        asm("nop");

    LOG("Time to die.\n");

#ifndef SPIN
    // clear the bootflags and halt the board
    BF->all = 0;
    BootResetBoard(1);
#else
    // Maybe we setup the watchdog to fire?
    // Maybe we just reboot?
    // Or we just hang until somebody toggles the reset with USB power
    for (;;) asm("nop");
#endif
}

/*! @brief  Trivial utility function to check that a buffer is full of 0xff
            (flash erase pattern)
*/
bool is_block_erased(uint8_t *buf, size_t len)
{
    while (len--) {
        if (*buf++ != 0xff) return false;
    }
    return true;
}

/*! @brief  Check to see if buf points at a firmware image

    The 64 bits of a FW image contain a stack pointer and an
    entry point (address to branch to on reset). This function
    simply validates that these are sane.
*/
bool CheckFirmwareMagic(void *buf)
{
    uint32_t        sp, ep;
    extern uint32_t _estack;

    if (buf == NULL) return false;

    sp = *((uint32_t *)buf);
    ep = *((uint32_t *)(buf + sizeof(uint32_t)));

    if ((uint32_t *)sp != &_estack || ep < UC_START_ADDR || ep > UC_END_ADDR) return false;

    return true;
}

/*! brief   Do all the checks and boot the FW

    This function only returns on failure.
*/
void doit(int idx)
{
    bool        bData = false;

    // Boot internal flash if we can
    if (CheckFirmwareMagic((void *)UC_START_ADDR)) {
        if (BootValidateCRC32(MCB_CRC(idx), MCB_LEN(idx), &bData)) {
            if (bData) {
                if (BootIsNormalBoot()) {
                    // if we are called with the backup IDX then this is last
                    // chance saloon... just boot
                    if (idx == MCB_EXT_BACKUP_IDX || bData) {
                        // but first... maybe start the watchdog
                        if (!BF->disableWatchdog) {
                                LOG("Starting watchdog...\n");
                                MX_IWDG_Init();
                        }
                        // never returns
                        BootJumpToUser();
                    } else {
                        LOG("BootIsNormalBoot returned bData false\n");
                    }
                } else {
                    LOG("BootIsNormalBoot returned false\n");
                }
            } else {
                ERR("crc check on internal flash failed\n");
            }
        } else {
            ERR("BootValidateCRC32 failed\n");
        }
    } else {
        ERR("CheckFirmwareMagic failed\n");
    }

    ERR("WTF!!\n");
}

// Lifted from the intertubes -- may not stay
// Useful for learning what the rcc module offers us
//
// Regular reboots are "software reset",
// Plugging in USB cable is "external reset pin reset"
// and watchdog is ... "independent watchdog reset"
#ifdef DEBUG
const char *reset_reason(eBootReason_t br)
{
    const char * reset_cause = "TBD";

    if (br & eLPWRRST) {
        reset_cause = "LOW_POWER_RESET";
    } else if (br & eWWDGRST) {
        reset_cause = "WINDOW_WATCHDOG_RESET";
    } else if (br & eIWDGRST) {
        reset_cause = "INDEPENDENT_WATCHDOG_RESET";
    } else if (br & eSFTRST) {
        reset_cause = "SOFTWARE_RESET"; // This reset is induced by calling the ARM CMSIS `NVIC_SystemReset()` function!
    } else if (br & ePINRST) {
        reset_cause = "EXTERNAL_RESET_PIN_RESET";
    } else if (br & eBORRST) {
        reset_cause = "BROWNOUT_RESET (BOR)";
    } else {
        reset_cause = "UNKNOWN";
    }

    return reset_cause;
}
#endif

/*! @brief Set __boot_reason__ with something for FW to pickup
*/
eBootReason_t getBootReason()
{
    eBootReason_t   ret = 0;
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
        ret |= eLPWRRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
        ret |= eWWDGRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        ret |= eIWDGRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        ret |= eSFTRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        ret |= ePINRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)) {
        ret |= eBORRST;
    }
    if (ret == 0) {
        ret = eUNKNOWN;
    }

    return ret;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    int         idx = MCB_INTERNAL_IDX;
#ifdef DEBUG
    const char  *reset_str;
#endif

    BootInit();

    /* USER CODE END 1 */


    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    // Cache the boot reason now:
    //   - in case something changes it later
    //   - pass it up to firmware
    *BR = getBootReason();

#ifdef DEBUG
    // Get the reset reason for printing (after the UART is init'ed)
    reset_str = reset_reason(*BR);
#endif

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_CRC_Init();
    MX_QUADSPI_Init();
    MX_USART1_UART_Init();
    // MX_IWDG_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */

    // NB the MX_IWDG_Init() call above gets re-created by CubeMX -- it should be commented out

// #ifdef DEBUG
    // If we're running a debugger, we don't want the watchdog spoiling things
    __HAL_DBGMCU_FREEZE_IWDG();
// #endif

    LOG("SmartHalo 2 Bootloader: v");
    printBootLoaderVersion(&MCB->version);
    LOG("\nS/N: %s", MCB->serial);
    LOG("\n\n");

    // Clear all the reset flags else they may remain set during future resets
    __HAL_RCC_CLEAR_RESET_FLAGS();

    // Debug/diagnostic output
#ifdef DEBUG
    extern volatile uint32_t __boot_flags__;
#endif
    extern volatile uint32_t __update_crc__, __update_len__;

    LOG("__boot_flags__:      %08p  %08x\n", &__boot_flags__, __boot_flags__);
    LOG("__update_crc__:      %08p  %08x\n", &__update_crc__, __update_crc__);
    LOG("__update_len__:      %08p  %08x\n", &__update_len__, __update_len__);

    LOG("\nreset_str: %s\n", reset_str);

    ExtFlashInit();

    if (McbFirstboot()) {
        __update_crc__ = __update_len__ = 0;
        if (ExtFlashCopyGoldenFW()) {
            McbUpdateGoldenCRC();
            BF->firstBoot = 1;
            BootResetBoard(0);
        }
        die();
    }

    if (*BR & eSFTRST) {
        if (BF->normalReboot) {
            // normal
            LOG("Normal reboot\n");
            if (BF->halt) {
                LOG("but we were told to halt\n");
                BootResetBoard(1);
            }
        } else {
            // crashed
            LOG("Reboot after firmware crash!!!\n\n");
        }
    }

    // we are done deciding if we're booting normally
    // or if firmware has asked for a halt so...
    // unconditionally reset the halt and normalReboot flags
    BF->halt = BF->normalReboot = 0;

    // Maybe reinstall the Golden FW
    //  a) we were asked to
    //  b) we got a watchdog reset
    if (BF->forceGolden || ((*BR & eIWDGRST) && BF->ignoreWatchdog == 0)) {
        if (BF->forceGolden) {
            LOG("Force Golden flag set, ");
        } else {
            LOG("Watchdog reset, ");
        }
        BF->forceGolden = 0;
        LOG("re-installing Golden firmware\n\n");

        idx = MCB_EXT_BACKUP_IDX;
        if (ExtFlashInstallImage(EXT_FLASH_BACKUP_START_OFFSET)) {
            McbUpdateInteralMetadata(idx);
            doit(idx);
        } else {
            ERR("Reinstall of golden firmware failed!!\n\n");
            die();
        }
    }

    if ((*BR & eIWDGRST) && BF->ignoreWatchdog) {
        LOG("Ignored watchdog reset\n");
    }

    // Check for an update, if there is one, install it and boot
    if (__update_len__ > UC_LEN) {
        // this situation typically happens after the battery has
        // completely died and these values in RAM are completely
        // random -- so ignore and clear
        LOG("Update length is bogus... ignoring\n");
        __update_crc__ = __update_len__ = 0;
    } else if (__update_crc__ && __update_len__) {
        LOG("Received update firmware\n");

        idx = MCB_EXT_UPDATE_IDX;
        if (ExtFlashInstallImage(EXT_FLASH_UPDATE_START_OFFSET)) {
            McbUpdateMCBWithUpdate(__update_crc__, __update_len__);
            __update_crc__ = __update_len__ = 0;
            doit(idx);
        }

        // update failed for some reason, clear flags so we don't try again
        LOG("Failed\n");
        __update_crc__ = __update_len__ = 0;
    }

    // We get here if there was no update, attempt to boot internal flash
    // This is actually the normal case
    doit(MCB_INTERNAL_IDX);

    // This is now badness...
    // If we get this far then we have to grab an image from the external flash
    LOG("Not good. We should have booted by now...\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    LOG("Reinstalling update firmware\n");

    idx = MCB_EXT_UPDATE_IDX;
    if (ExtFlashInstallImage(EXT_FLASH_UPDATE_START_OFFSET)) {
        McbUpdateInteralMetadata(idx);
        doit(idx);
    }

    LOG("Failed\n");

    LOG("Copying Golden firmware\n");

    idx = MCB_EXT_BACKUP_IDX;
    if (ExtFlashInstallImage(EXT_FLASH_BACKUP_START_OFFSET)) {
        McbUpdateInteralMetadata(idx);
        doit(idx);
    }

    ERR("Failed to find anything to boot!\n");
    die();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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

    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 15;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
    /** Configure the main internal regulator output voltage
    */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

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
void assert_failed(char *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

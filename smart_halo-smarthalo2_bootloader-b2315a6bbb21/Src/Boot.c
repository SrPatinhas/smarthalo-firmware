///
/// \file       Boot.c
/// \brief      Boot loader helper functions
///
/// \author     NOVO, Georg Nikodym
///

#include "Boot.h"
#include "crc.h"
#include "memory_map.h"
#include "log.h"
#include "gpio.h"
#include "qspiflash.h"
#include "usart.h"
#include "dma.h"

extern volatile uint32_t    __boot_flags__;     ///< see linker script
BootFlags_t                 *BF = (BootFlags_t *)&__boot_flags__;
extern volatile uint32_t    __boot_reason__;    ///< see linker script
eBootReason_t               *BR = (eBootReason_t *)&__boot_reason__;

/**
 * @brief       BootInit()
 * @details     Init the boot module
 * @public
 * @return      true if success, false otherwise.
 */
bool BootInit(void)
{
    // after power-on, the magic field will be unknown so
    // initialize the structure here
    if (BF->magic != BF_MAGIC) {
        LOG("Initializing BootFlags\n");
        BF->all = 0;    // clear BF
        BF->magic = BF_MAGIC;
    }
    return true;
}

/**
 * @brief       BootIsNormalBoot()
 * @details     XXX noop placeholder
 * @public
 * @return      true if success, false otherwise.
 */
bool BootIsNormalBoot(void)
{
    return true;
}

/**
 * @brief       BootValidateCRC32()
 * @details     Validate the CRC32.
 * @public
 * @param[in]   pbCRCGood: Handle on the variable to CRC state.
 * @return      true if success, false otherwise.
 */
bool BootValidateCRC32(uint32_t u32Expected, uint32_t u32Len, bool * pbCRCGood)
{
    uint32_t u32CalculatedChecksum  = 0;

    /*
        the caller likely got the length from the MCB -- if it was mashed/erased
        and we feed it to the CRC engine, we'll die
    */
    if (u32Len == 0 || u32Len == 0xffffffff) return false;

    LOG("Before CRC\n");

    // Calculate the ROM library checksum
    u32CalculatedChecksum = ~HAL_CRC_Calculate(&hcrc, (uint32_t*)UC_START_ADDR, u32Len);

    LOG("After Expected: %08lx, Calculated: %08lx, Len: %08lx - ", u32Expected, u32CalculatedChecksum, u32Len);

    // Compare the calculated checksum with the stored
    if (u32Expected != u32CalculatedChecksum) {
        LOG("CRC failed\n");
        *pbCRCGood = false;
    } else {
        LOG("CRC pass\n");
        *pbCRCGood = true;
    }
    return true;
}

/**
 * @brief       BootResetBoard()
 * @details     Soft reset OR halt the device. Halt can either be driven
                by the parameter or by the bootflags (assumed to be set
                by firmware)
 * @public
 * @param[in]   halt if true, put the device into a low-power state
 * @return      does not return on success, false otherwise.
 */
bool BootResetBoard(bool halt)
{
    // unconditionally reset halt and normalReboot flags
    BF->halt = BF->normalReboot = 0;

    if (halt) {

        LOG("\nBootloader Halt!\n");

        SPIFLASH_deep_sleep();

        // Make sure the GPIO pulldowns are configured correctly
        // else the power enable lines on things will float
        // (and then the Nordic will turn on again)

        MX_GPIO_Init();

        // This block of calls sets up the internal pulldowns
        // such that they survive in a standby/shutdown/etc
        HAL_PWREx_EnablePullUpPullDownConfig();
        // For BLE
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_C, BLE_EN_Pin);
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, BLE_TX_Pin);
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, BLE_RX_Pin);
        // for OLED
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_C, OLED_VDD_PWR_EN_Pin);
        HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_C, OLED_VCC_PWR_nEN_Pin);
        // for VLED
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_C, EN_VLED_Pin);
        // for piezo
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, VBZ_EN_Pin);
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_H, VBZ_LDO_EN_Pin);
        // for sensors
        HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_B, SENSORS_EN_Pin);

        extern QSPI_HandleTypeDef hqspi;
        HAL_QSPI_DeInit(&hqspi);

        // Seems needed by STOP2 mode
        HAL_GPIO_DeInit(BLE_RX_GPIO_Port, BLE_RX_Pin);
        HAL_GPIO_DeInit(BLE_TX_GPIO_Port, BLE_TX_Pin);
        HAL_GPIO_DeInit(BLE_EN_GPIO_Port, BLE_EN_Pin);

        HAL_GPIO_DeInit(EN_VLED_GPIO_Port, EN_VLED_Pin);
        HAL_GPIO_DeInit(OLED_VCC_PWR_nEN_GPIO_Port, OLED_VCC_PWR_nEN_Pin);
        HAL_GPIO_DeInit(OLED_VDD_PWR_EN_GPIO_Port, OLED_VDD_PWR_EN_Pin);
        HAL_GPIO_DeInit(VBZ_EN_GPIO_Port, VBZ_EN_Pin);
        HAL_GPIO_DeInit(VBZ_LDO_EN_GPIO_Port, VBZ_LDO_EN_Pin);
        HAL_GPIO_DeInit(SENSORS_EN_GPIO_Port, SENSORS_EN_Pin);

        __HAL_RCC_GPIOA_CLK_DISABLE();
        __HAL_RCC_GPIOB_CLK_DISABLE();
        __HAL_RCC_GPIOC_CLK_DISABLE();
        __HAL_RCC_GPIOH_CLK_DISABLE();
        __HAL_RCC_CRC_CLK_DISABLE();
        __HAL_RCC_ADC_CLK_DISABLE();
        __HAL_RCC_QSPI_CLK_DISABLE();
        __HAL_RCC_USART2_CLK_DISABLE();

        extern UART_HandleTypeDef huart2;
        extern UART_HandleTypeDef huart1;

        HAL_UART_MspDeInit(&huart1);
        HAL_UART_MspDeInit(&huart2);

        HAL_DeInit();

        // // Standby
        // //  HAL_PWREx_EnableSRAM2ContentRetention();
        // //  HAL_PWR_EnterSTANDBYMode();
        // // or
        // //  shutdown, we don't need to remember anything
        HAL_SuspendTick();
        HAL_DBGMCU_DisableDBGStopMode();            // 605uA without this
        HAL_DBGMCU_DisableDBGSleepMode();           // no numbers but super important
        HAL_DBGMCU_DisableDBGStandbyMode();         // no numbers but super important

        HAL_PWREx_DisableSRAM2ContentRetention();

        HAL_PWREx_DisableLowPowerRunMode();

        // HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);    // 358uA
        // HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);    // 354uA
        // HAL_PWREx_EnterSTOP2Mode(0);                       // 354uA
        // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); // 358uA
        HAL_PWREx_EnterSHUTDOWNMode();  // 123uA - rev1 / 250uA - rev4
        // HAL_PWR_EnterSTANDBYMode();

#if 1
        //
        // Should not get here because currently we don't have anything
        // that would wake us up from here (the USB reset reboots the
        // device completely, ie doesn't get here)
        //
        // So if we are here, it's because we woke up again either because
        // the above didn't actually halt the processor or because something
        // else triggered a wakeup
        HAL_ResumeTick();
        HAL_Init();

        // re-config enough stuff to allow console to work
        MX_GPIO_Init();
        // MX_DMA_Init();
        // MX_CRC_Init();
        // MX_QUADSPI_Init();
        memset(&huart1, 0, sizeof(huart1));
        MX_USART1_UART_Init();

        LOG("Returned from shutdown!  Pause...\n");
        for (int i = 0; i < 30000000; i++) asm("nop");

        // Pretend normal reboot, falls through to reset below
        BF->normalReboot = 1;
#endif
    } else {
        BF->normalReboot = 1;
    }

    HAL_NVIC_SystemReset();

    // Should not get here
    return false;
}

/**
 * @brief       BootJumpToUser()
 * @details     Perform a jump to user code.
 * @public
 * @return      nothing (for now?)
 */
void BootJumpToUser()
{
    // Magic here:
    //  - find the address to jump to in the second 32 bits
    //    of the image, assign to JumpAddress
    //  - create a function pointer, Jump, and assign it to JumpAddress
    uint32_t    JumpAddress = *(uint32_t *)(UC_START_ADDR + 4);
    void (*Jump)(void) = (void (*)(void))JumpAddress;

    LOG("Moment of truth... leaping at %p\n", (void *)JumpAddress);

    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = SysTick->LOAD = SysTick->VAL = 0;

    // Set the stack pointer to whatever the first 32 bits
    // of the image points at
    __set_MSP(*(uint32_t *)UC_START_ADDR);

    // dereference the function pointer
    Jump();
}

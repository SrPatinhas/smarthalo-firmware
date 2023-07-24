/**
 * @file       Power.c
 *
 * @brief      This file implements power management functions.
 *
 * @author     Georg Nikodym
 * @copyright  Copyright (c) 2020 SmartHalo Inc
 */

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "ShellComPort.h"
#include "Shell.h"
#include "SHTimers.h"
#include "HaloLedsDriver.h"
#include "BLEDriver.h"
#include "usart.h"
#include "BootLoaderImport.h"
#include "OLEDDriver.h"
#include "Power.h"
#include "reboot.h"
#include "GraphicsTask.h"
#include "PhotoSensor.h"
#include "WatchdogTask.h"
#include "device_telemetry.h"
#include "wwdg.h"
#include "CommunicationTask.h"

#define IDLE_TIMEOUT 10000

static TimerHandle_t IdleTimer;
static StaticTimer_t IdleTimerBuffer;
static StaticSemaphore_t canSleepSemaBuffer;
static SemaphoreHandle_t canSleepSemaphore;

bool powerState = true;     // needs to be visible in Shell.c

static void IdleTimerCB(TimerHandle_t timer);
static void stayAwakeFromISR_Power(void);

static void peripheralsOff(void);
static void peripheralsOn(void);
static bool isSleepAllowed(void);
static void sleepSTM(void);

/*! @brief flag to signal that we should stay awake */
static volatile bool IdleStayAwake;
/*! @brief the name of the last function that set IdleStayAwake flag */
static const char    *IdleStayAwakeBy;

/*! @brief  initialize power "driver"

            Sets up the idle timer that will trigger power management state
            changes.

            It is _not_ started -- that falls onto SystemUtiltiesTask
*/
void init_Power(void)
{
    if (IdleTimer == NULL) {
        IdleTimer = xTimerCreateStatic(
            "IdleTimer", IDLE_TIMEOUT, pdTRUE,
            NULL, IdleTimerCB, &IdleTimerBuffer);
        storeTimer(IdleTimer);
    }
    if (canSleepSemaphore == NULL) {
        canSleepSemaphore = xSemaphoreCreateCountingStatic(10, 0, &canSleepSemaBuffer);
    }
}

/*! @brief  Start the idle timer

            It is assumed that this will be called by SystemUtilitiesTask
            when it starts up.
*/
void start_Power(void)
{
    log_Shell("Starting Idle/Power Timer...");
    xTimerStart(IdleTimer, 200);
}

/*! @brief  handle idle timeout

            When the idle timer expires, do one of:
 
            1. nothing (power management is disabled)
            2. report on the last function to request that we stay awake and
               return (thereby staying awake)
            3. switch to a lower-power state

    @param[in]  timer  unused timer handle passed in by FreeRTOS
*/
static void IdleTimerCB(TimerHandle_t timer)
{
    if (isTestMode_SystemUtilities()) return;
    if (IdleStayAwake) {
        IdleStayAwake = false;
        log_Shell("idle timer expired, staying awake: %s", IdleStayAwakeBy);
        return;
    }
    log_Shell("idle timer expired, power off");
    setState_Power(false);
}

/*! @brief  Stop the MCU tick

            Not strictly necessary but stopping this tick saves
            around .3-.4mA
*/
static inline void SysTickStop(void)
{
    /* Disable SysTick Interrupt */
    SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);
}

/*! @brief  Start the MCU tick
*/
static inline void SysTickStart(void)
{
    /* Enable SysTick Interrupt */
    SysTick->CTRL  |= (SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);
}

static IRQn_Type IRQ2Toggle[] = {
    ADC1_IRQn,
    DMA1_Channel1_IRQn,
    DMA1_Channel4_IRQn,
    DMA1_Channel5_IRQn,
    DMA1_Channel6_IRQn,
    DMA1_Channel7_IRQn,
    DMA2_Channel6_IRQn,
    DMA2_Channel7_IRQn,
    EXTI15_10_IRQn,
    EXTI3_IRQn,
    EXTI3_IRQn,
    EXTI9_5_IRQn,
    I2C1_ER_IRQn,
    I2C1_EV_IRQn,
    I2C2_ER_IRQn,
    I2C2_EV_IRQn,
    QUADSPI_IRQn,
    SPI2_IRQn,
    TIM1_UP_TIM16_IRQn,
    TSC_IRQn,
    USART1_IRQn,
    // USART2_IRQn,     // don't touch BLE UART
};

static volatile EWakeupReasons_t wakeupReason;

/**
 * @brief       Register the fact that we have ISR activity that should prevent
 *              idle sleep.
 * @details     Called from an ISR (for either accelerometer or BLE UART wake-up).
 * @param[in]   reason enum identifying which ISR triggered the wakekup
 */
void wakeupFromISR_Power(EWakeupReasons_t reason)
{
    if (powerState == false) {
        wakeupReason |= reason;
    }
    stayAwakeFromISR_Power();
}

uint32_t lostTicks;
#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))

/*! @brief  Control power state

            Calling this directly is discouraged. Consider stayAwake_Power()
            instead.

            There are three main power states, in order of power consumption:

              - fully on
              - peripherals powered down
              - STM processor in sleep

            Callers pass in true for "fully on" and false for a lower power
            state.

            Since there are a number of tasks/functions that may need to
            prevent complete sleep (typically because they want a timer to
            expire), the lowest available power state will change.

            Lowest power state (sleep) is prevented by that otherr code calling:

              SLEEP_NOTALLOWED();

            later followed by:

              SLEEP_ALLOWED();

            The number of calls to SLEEP_ALLOWED() must equal the number of
            calls to SLEEP_NOTALLOWED().

            See README.md for a more complete description.

    @param[in]  newPowerState false for power save state
 */
void setState_Power(bool newPowerState)
{
    if (newPowerState) {
        if (powerState) return;
        peripheralsOn();
        powerState = true;
    } else {
        if (powerState) {
            peripheralsOff();
        }
        powerState = false;
        if (isSleepAllowed()) {
            sleepSTM();
            powerState = true;
        }
    }
}

/*! @brief  Sleep until interrupt

            Suspend everything, sleep the CPU until either
              - wakeup interrupt on BLE UART
              - accelerometer interrupt

            Then resume everything again.

            Obviously, this code is _very_ SH2 specific.
*/
static void sleepSTM(void)
{
    bool abortedSleep = false;

    wakeupReason = 0;

    // if watchdog is enabled, this will reboot one time
    disableWatchdog_WatchdogTask(); 

    uint32_t sleepTick = xTaskGetTickCount();

    log_deviceTelemetry(eSLEEP, 0);

    // TODO: this might be a good place to send a message
    //       to the Nordic but experimentally it doesn't seem
    //       to be needed

    log_Shell("%s: sleeping on tick: %ld\n\n", __func__, xTaskGetTickCount());
    // log_Shell("stopping timers...");
    stopTimers();
    // log_Shell("done");
    // No console printing beyond this point

    vTaskSuspendAll();  // stop freeRTOS

    // We've stopped the FreeRTOS schedule -- we're the only ones 
    // servicing WWDG now
    HAL_WWDG_Refresh(&hwwdg);

    // Put ADC into deep power down mode
    extern ADC_HandleTypeDef hadc1;
    HAL_ADCEx_EnterADCDeepPowerDownMode(&hadc1);

    // Stop BLE DMA
    HAL_UART_DMAStop(&BLE_UART);

    // Disable serial console
    HAL_UART_DMAStop(&huart1);
    HAL_UART_MspDeInit(&huart1);

    // You might not think so, but experiments show that
    // disabling these clocks and interrupts gives ~1mA of savings
    __HAL_RCC_TIM1_CLK_DISABLE();
    __HAL_RCC_TIM2_CLK_DISABLE();
    __HAL_RCC_TIM15_CLK_DISABLE();
    // __HAL_RCC_GPIOA_CLK_DISABLE();   // don't touch BLE UART
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOH_CLK_DISABLE();
    __HAL_RCC_CRC_CLK_DISABLE();
    __HAL_RCC_ADC_CLK_DISABLE();
    __HAL_RCC_QSPI_CLK_DISABLE();
    __HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_DMA1_CLK_DISABLE();
    __HAL_RCC_DMA2_CLK_DISABLE();
    // __HAL_RCC_USART2_CLK_DISABLE();  // don't touch BLE UART

    for (int i = 0; i < ARRAY_SIZE(IRQ2Toggle); i++) {
        HAL_NVIC_DisableIRQ(IRQ2Toggle[i]);
    }

    // enable the BLE UART wakeup interrupt
    // setup the BLE UART to wake us up if there's RX data
    UART_WakeUpTypeDef wakeupSelection = {
        .WakeUpEvent = UART_WAKEUP_ON_READDATA_NONEMPTY
    };
    HAL_UARTEx_EnableStopMode(&huart2);
    HAL_UARTEx_StopModeWakeUpSourceConfig(&huart2, wakeupSelection);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_WUF);
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

    // stop all the clocks
    HAL_SuspendTick();
    SysTickStop();

    HAL_NVIC_ClearPendingIRQ(EXTI2_IRQn);

    HAL_PWREx_EnableLowPowerRunMode();
    HAL_DBGMCU_DisableDBGStopMode();    // BLE UART will not wake-up without this

    // Ensure that if we're a little slow waking up, we don't get shot in the
    // face by WWDG
    HAL_WWDG_Refresh(&hwwdg);

    // Last chance check -- if we got an accelerometer interrupt between the
    // top of this function and here, then wakeupReason will be set and
    // we should not sleep
    if (wakeupReason) {
        abortedSleep = true;
    } else {
        // THIS IS WHERE WE STOP
        // Observe that rev1 and rev4 boards draws 3-4mA (Nordic/BLE UART still active)
        HAL_PWREx_EnterSTOP0Mode(PWR_STOPENTRY_WFI);
    }

    // We are waking up! The processor is woken by an interrupt. One of the ISRs
    // will have run first and set wakeupReason. On their return, we are here...

    HAL_PWREx_DisableLowPowerRunMode();
    SystemClock_Config();

    // Renable RCC peripheral clocks
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM15_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_CRC_CLK_ENABLE();
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    // __HAL_RCC_USART2_CLK_ENABLE();   // don't touch BLE UART

    // Now that all the clocks are running again... tickle the dog
    HAL_WWDG_Refresh(&hwwdg);

    // Restart ADC
    int adcret1 = HAL_ADC_Init(&hadc1);
    int adcret2 = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    adcRetrigger_SystemUtilities();

    // Restart the serial console
    MX_USART1_UART_Init();
    init_ShellComPort();

    // Renable IRQs
    for (int i = 0; i < ARRAY_SIZE(IRQ2Toggle); i++) {
        HAL_NVIC_EnableIRQ(IRQ2Toggle[i]);
    }

    SysTickStart();     // restart MCU tick
    HAL_ResumeTick();   // restart HAL tick (no clear why it's different)
    xTaskResumeAll();   // restart freeRTOS scheduler (also should keep the watchdog happy)
    
    if(getStandbyReason_CommunicationTask() == StandbyOff)
        restartDMA_BLEDriver();

    char *reason;
    if (wakeupReason == WUR_ACCEL) {
        reason = "ACCEL";
    } else if (wakeupReason == WUR_BLEWAKEUP) {
        reason = "BLE";
    } else if (wakeupReason == (WUR_ACCEL | WUR_BLEWAKEUP)) {
        reason = "ACC+BLE";
    } else {
        reason = "unknown";
    }
    uint32_t wakeTick = xTaskGetTickCount();
    lostTicks += (wakeTick - sleepTick);
    log_Shell("%s: wake-up on tick:  %ld, reason: %s\n"
              "%s: lost %ld ticks (%ld total)",
              __func__, wakeTick, reason,
              __func__, wakeTick - sleepTick, lostTicks);
    if (abortedSleep) {
        log_Shell("%s: ABORTED SLEEP!!!", __func__);
    }

    if (adcret1 != HAL_OK || adcret2 != HAL_OK) {
        log_Shell("%s: adcret1: %d, adcret2: %d", __func__, adcret1, adcret2);
    }

    // short delay to allow console output to finish before writing to flash
    vTaskDelay(5 / portTICK_RATE_MS);

    log_deviceTelemetry(eWAKEUP, wakeupReason);

    wakeupReason = 0;

    peripheralsOn();

    startTimers();
}

/*! @brief  UI needs things -> stay awake a little longer

            Called for any UI event that needs the device to not sleep.

            Calling this function is temporary -- the effect runs out.
            On average, it will be another (IDLE_TIMEOUT + IDLE_TIMEOUT/2)
            before sleeping will be considered.

            If we are in test mode, then do nothing.

    @param[in] calledFrom name of the calling function (for debugging log)
*/
void stayAwake_Power(const char *calledFrom)
{
    if (isTestMode_SystemUtilities()) {
        return;
    }
    IdleStayAwake = true;
    IdleStayAwakeBy = calledFrom;
    setState_Power(true);
}

/*! brief   accelerometer interrupts -> stay awake a little longer

            Similar in function to stayAwake_Power() except safe to
            call from an ISR (in this case the accelerometer).
*/
static void stayAwakeFromISR_Power(void)
{
    if (isTestMode_SystemUtilities()) {
        return;
    }
    IdleStayAwake = true;
    IdleStayAwakeBy = __func__;
}

/*! @brief  Turn off peripherals
*/
static void peripheralsOff(void)
{
    log_Shell("%s: turning off", __func__);

    sleep_PhotoSensor();
    disableOLED_GraphicsTask();
    sleep_HaloLedsDriver();
}

/*! @brief  Turn on peripherals
*/
static void peripheralsOn(void)
{
    log_Shell("%s: turning on", __func__);

    if (!init_HaloLedsDriver()) {
        log_Shell("%s: init_HaloLedsDriver returned false, trying again", __func__);
        sleep_HaloLedsDriver();
        vTaskDelay(5);
        init_HaloLedsDriver();
    }
    enableOLED_GraphicsTask();
    wake_PhotoSensor();
}

/*! @brief  Register/dereg a need to prevent full idle sleep

            Called with false, add to a counting semaphore.
            Called with true, subtract from that semaphore.

            A caller that needs to prevent full sleep calls
            with false and that "vote" is counted here. The
            caller need not worry about the state of the
            device.

            When that caller wants to withdraw their "vote",
            they call with true.

            Sleeping will be prevented as long as the semaphore
            value is non-zero.

            For convenience, wrapper macros SLEEP_NOTALLOWED()
            and SLEEP_ALLOWED() are provided in Power.h
*/
void sleepAllowed_Power(bool allowSleep, const char *func)
{
    log_Shell("%s: called from %s with %s", __func__, func,
              allowSleep ? "sleep if you want" : "dont sleep");

    if (!allowSleep) {
        xSemaphoreGive(canSleepSemaphore);
    } else {
        if (xSemaphoreTake(canSleepSemaphore, 50) == pdFALSE) {
            log_Shell("%s: failed to get semaphore -- logic problem", __func__);
        }
    }
}

/*! @brief  Determines if sleep allowed.

            Check the value of counting semaphore. If it is zero,
            then nobody has asked to prevent sleep.

    @return True if sleeping is allowed
*/
static bool isSleepAllowed(void)
{
    uint32_t dontSleepCount = uxSemaphoreGetCount(canSleepSemaphore);

    log_Shell("%s: dontSleepCount is %lu", __func__, dontSleepCount);

    return (dontSleepCount ? false : true);
}

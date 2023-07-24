/*!
    @file WatchdogTask.c

    Watchdog/keepalive task. Simply a low priority task that reloads
    the watchdog register repeatedly. If it gets starved, then the
    hardware watchdog fires and resets the device.

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019-2020 SmartHalo Inc
*/

#include <inttypes.h>
#include <SystemUtilitiesTask.h>
#include "iwdg.h"
#include "wwdg.h"
#include "BootLoaderImport.h"
#include "Power.h"
#include "reboot.h"
#include "WatchdogTask.h"
#include "rtc.h"
#include "rtcUtils.h"
#include "SHTaskUtils.h"
#include "device_telemetry.h"

#define STACK_SIZE      configMINIMAL_STACK_SIZE

static TaskHandle_t iwdgHandle = NULL;
static StaticTask_t iwdgTaskBuffer;
static StackType_t  iwdgStack[STACK_SIZE];

static TaskHandle_t shwdHandle = NULL;
static StaticTask_t shwdTaskBuffer;
static StackType_t  shwdStack[STACK_SIZE];

EventGroupHandle_t  shwdEventGroup;
StaticEventGroup_t  shwdEventGroupBuffer;
static uint32_t allBits;

static void WatchdogTask(void *pvParameters);
static void shwdTask(void *params);

volatile uint32_t wwdgCount;

void WatchdogTaskInit()
{
    if (BF->disableWatchdog == false && iwdgHandle == NULL) {
        MX_IWDG_Init();
        iwdgHandle =
            xTaskCreateStatic(WatchdogTask, TASKNAME_IWDG, STACK_SIZE, NULL,
                              PRIORITY_IDLE, iwdgStack, &iwdgTaskBuffer);
        configASSERT(iwdgHandle);
    }

    if (shwdHandle == NULL) {
        shwdHandle =
            xTaskCreateStatic(shwdTask, TASKNAME_SOFTWATCHDOG, STACK_SIZE, NULL,
                              PRIORITY_REAL_TIME, shwdStack, &shwdTaskBuffer);
        configASSERT(shwdHandle);
    }

    if (shwdEventGroup == NULL) {
        shwdEventGroup = xEventGroupCreateStatic(&shwdEventGroupBuffer);
        configASSERT(shwdEventGroup);
    }
}

/*  @brief  Task that resets the hardware watchdog register

    If this task gets delayed, starved because reasons the hardware
    watchdog timer expires and resets the device.
    On booting, the bootloader can detect a watchdog reset and will
    consult the ignoreWatchdog bit.
*/
static void WatchdogTask(void *pvParameters)
{
    int  count = 0;

    log_Shell("%s: starting", __func__);

    // reset the watchdog flag
    // if we get a watchdog reset now, the bootloader will
    // re-install the golden firmware
    BF->ignoreWatchdog = 0;

    SLEEP_NOTALLOWED();

    for (;;) {
        // after about 10 minutes, tell the bootloader not
        // to freak out on a watchdog
        if (BF->ignoreWatchdog == 0 && ++count > 300) {
            BF->ignoreWatchdog = 1;
            SLEEP_ALLOWED();
        }
        HAL_IWDG_Refresh(&hiwdg);
        vTaskDelay(2000);
    }
}

/*  @brief  Disables the watchdog

    If the watchdog is enabled, set the disableWatchdog flag and reboot.
*/
void disableWatchdog_WatchdogTask(void)
{
    if (!BF->disableWatchdog) {
        // Disabling the watchdog requires a reboot. It is assumed that
        // we are here because our caller has determined that we are idle,
        // so now seems like the perfect time
        log_Shell("%s: doing watchdog disable reboot", __func__);
        BF->disableWatchdog = 1;
        reboot(eRebootDisableWatchdog);
    }
}

static inline uint32_t eshwdTask2mask(ESHWD_Bits_t bit)
{
    return (1 << bit);
}

const char *eshwdTask2name(ESHWD_Bits_t bit)
{
    switch (bit) {
    case eSHWD_SystemUtilities:
        return TASKNAME_SYSTEMUTILITIES;
    case eSHWD_GraphicsTask:
        return TASKNAME_OLED;
    case eSHWD_HaloTask:
        return TASKNAME_HALO;
    case eSHWD_SensorsTask:
        return TASKNAME_SENSORS;
    case eSHWD_CommunicationTask:
        return TASKNAME_COMMUNICATION;
    case eSHWD_SoundTask:
        return TASKNAME_SOUND;
    case eSHWD_PersonalityTask:
        return TASKNAME_PERSONALITY;
    case eSHWD_NightLightTask:
        return TASKNAME_NIGHTLIGHT;
#ifndef GOLDEN
    case eSHWD_RideTask:
        return TASKNAME_RIDE;
    case eSHWD_AlarmTask:
        return TASKNAME_ALARM;
#endif
    default:
        return "Unknown bit";
    }
}

/*! @brief  software watchdog task
    @param  params ignored (part of FreeRTOS task api)
 */
static void shwdTask(void *params)
{
    EventBits_t eventBits;
    allBits = eshwdTask2mask(eSHWD_MAX) - 1;

    // We cannot simply call waitForFS_SystemUtilities() here because
    // we will hang on mutex (probably a priority-inversion)
    // This is a simple work-around
    // If we do not get a filesystem within 10 seconds, then we have bigger problems
    // fall through, start the watchdog and let it do its reset thing
    for (int i = 0; i < 20 && !isFSMounted_SystemUtilities(); i++) {
        vTaskDelay(500);
    }

    for (uint8_t counter = 0;; counter++) {
        eventBits = xEventGroupWaitBits(shwdEventGroup, allBits, pdTRUE, pdTRUE, 2000);

        // roughly every 20 seconds, collect all the task stack highwater marks
        if ((counter % 10 == 0)) {
            collectTaskStats_SHTaskUtils();
        }

        // Very low priority telemetry:
        // roughly every hour, create event logs for the task stacks
        // N.B. this "hour" is a CPU hour, this counter stops on sleeps
        if ((counter % 1800 == 0)) {
            logStack_SHTaskUtils();
        }

        // Normal case: all tasks have checked in
        if (eventBits == allBits) {
            vTaskDelay(2000);
            if (wwdgCount) {
                log_Shell("%s: skipped %" PRIu32 " WWDG events", __func__, wwdgCount);
                log_deviceTelemetry(eSKIPWWDG, wwdgCount);
                wwdgCount = 0;
            }
            continue;
        }

        // Abnormal case -- we will report and reboot

        if (wwdgCount) {
            log_Shell("%s: abnormal skipped %" PRIu32 " WWDG events", __func__, wwdgCount);
            log_deviceTelemetry(eSKIPWWDG, wwdgCount);
        }

        for (int bit = 0; bit < eSHWD_MAX; bit++) {
            if (!(eventBits & eshwdTask2mask(bit))) {
                log_Shell("%s: %s did not check in", __func__, eshwdTask2name(bit));
            }
        }

        store_deviceTelemetry();

#ifndef MEGABUG_HUNT
        log_Shell("%s: rebooting", __func__);
        BF->softWD = 1;
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_SOFTWD, allBits ^ eventBits);
        reboot_nosync();
#else
        log_Shell("%s: eventBits: 0x%08" PRIx32, __func__, eventBits);
        log_Shell("%s: mask: 0x%08" PRIx32, __func__, allBits);
#endif
    }
}

/**
 * @brief   Called by FreeRTOS when a stack overflow is detected
 * @details We cannot print here so...
 *          Hacky magic: convert taskname from a ram address to a
 *          flash address and store that pointer one of the RTC
 *          backup registers.
 * 
 * @param xTask         handle to the overflowing task
 * @param pcTaskName    the name of the task
 */
void vApplicationStackOverflowHook(xTaskHandle xTask, const char *pcTaskName)
{
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_TASKNAME,
                        (uint32_t)findConstTaskname_SHTaskUtils(xTask, pcTaskName));
}

/*! @brief  Keep alive for a task

            Each task needs to call this function at least once per interval
            to prevent the software watchdog from doing its thing.

            Periodically, we'll also collect the free stack info for each task.

    @param  task The taskID (as assigned in WatchdogTask.h)
*/
void shwd_KeepAlive(ESHWD_Bits_t task)
{
    if (task > 32) return;

    xEventGroupSetBits(shwdEventGroup, eshwdTask2mask(task));
}

/**
 * @brief   Called by WWDG interrupt handler just before a watchdog reset
 * @details Store the address of the interrupted code into __update_crc__ which
 *          will a) not be reset on reboot and b) the bootloader will not molest
 *          allowing for main() to pick it up. This address needs to be decoded
 *          post-mortem.
 *          
 *          Alternative code commented out (store the address of the name of the
 *          interrupted task). This is much easier to turn into an event log
 *          message that doesn't need any fancy decoding but in practice is not
 *          precise enough to aid in the identification of watchdog resets.
 * 
 * @param hwwdg unused
 */
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
    // buy some time -- See comment at bottom
    HAL_WWDG_Refresh(hwwdg);

    extern volatile uint32_t __update_crc__, wwdgReturnAddress;
    extern void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress,
                                         uint32_t pc, bool wwdg);

    // __update_crc__ = (uint32_t)findConstTaskname_SHTaskUtils(xTaskGetCurrentTaskHandle(), pcTaskGetName(NULL));
    __update_crc__ = wwdgReturnAddress;
    __asm volatile
    (
        // these first 4 instructions are idiomatic ARM to figure
        // out if we're in an interrupt/exception handler
        // and load %r0 with the appropriate stack pointer (there
        // are two, M and P, interrupts use the M)
        // I am _assuming_ that we want to know about the freertos
        // task that was running (I'm unconditionally using the P)
        // The rest just sets up that stack frame as an argument
        // to the call to prvGetRegistersFromStack()
        // which will handle printing.
        //
        // The reason I'm choosing to ignore the M stack is because
        // _most_ of the time it's simply the freertos scheduler and
        // not very useful for debugging (freertos schedules tasks
        // from its tick interrupt)
        //
        // " tst lr, #4                                            \n"
        // " ite eq                                                \n"
        // " mrseq r0, msp                                         \n"
        // " mrsne r0, psp                                         \n"
        " mrs r0, psp\n"        // pulFaultStackAddress
        " ldr r1, [r0, #24]\n"  // pc argument
        " mov r2, #1\n"         // wwdg = true
        " ldr r3, handler2_address_const\n"
        " bx r3\n"
        " handler2_address_const: .word prvGetRegistersFromStack\n"
    );
    // Not reached because prvGetRegistersFromStack should never return.
    // This function should not return. We reset the watchdog timer
    // above, so if we return here we will end up here again after
    // the watchdog timer gets down to 40 (where this EWI is triggered
    // again).
}


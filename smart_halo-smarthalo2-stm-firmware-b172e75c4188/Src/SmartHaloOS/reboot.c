/*!
    @file reboot.c

    Polite reboot function. Polite, meaning, graceful shutdown

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#include "reboot.h"
#include "spiffs.h"
#include "stm32l4xx_hal.h"
#include "BootLoaderImport.h"
#include "device_telemetry.h"
#include "rtc.h"
#include "rtcUtils.h"

#include <FreeRTOS.h>
#include <task.h>

#define GOLDEN_THRESHOLD    5000

extern spiffs rootFS;

static inline void maybeSetGoldenFlag(void)
{
    if (xTaskGetTickCount() < GOLDEN_THRESHOLD) {
        BF->forceGolden = 1;
    }
}

/*! @brief  Graceful reboot

    Before resetting the STM, do things to cleanly shut down.
    At this time, that is:
    * Store the reboot code in the RTC backup
    * Log the event
    * Unmount the filesystem
*/
void reboot(eRebootReason_t rebootreason)
{
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_REBOOT, rebootreason);
    log_deviceTelemetry(eREBOOT, rebootreason);
    vTaskDelay(100);

    SPIFFS_unmount(&rootFS);
    vTaskDelay(500);

    // Tell the bootloader this was on purpose
    BF->normalReboot = 1;

    maybeSetGoldenFlag();

    // XXX here is the place to do other stuff like:
    //     * kick the Nordic
    //     * shut off leds, OLED display, etc
    //     * ...
    HAL_NVIC_SystemReset();
}

/*! @brief   Immediate reboot

    Called from places where it may not be possible to unmount the filesystem.
*/
void reboot_nosync(void)
{
    maybeSetGoldenFlag();
    HAL_NVIC_SystemReset();
}

/*! @brief      Software crash / ungraceful reboot

    We "know" that we are doing to crash, so log it and reboot. Do not worry
    about unmounting the filestem. If we haven't even been alive for
    GOLDEN_THRESHOLD milliseconds, arrange for the golden FW to be 
    reinstalled.

    The high order 16 bits of the arg encode the calling source file and the
    low order 16 bits is the line number.
 
 * @param[IN]   arg encoded caller location
 */
void soft_crash(uint32_t arg)
{
    log_deviceTelemetry(eSOFTCRASH, arg);
    BF->crash = 1;
    maybeSetGoldenFlag();
    HAL_NVIC_SystemReset();
}

/**
 * @file        rtcUtils.c
 * @details     The RTC hardware has 32 backup registers that we can use to store
 *              data across reboots.
 * @author      Georg Nikodym <georg@smarthalo.bike>
 * @copyright   Copyright (c) 2021 SmartHalo Technologies
 */
 
#include <inttypes.h>
#include <stdio.h>

#include "rtc.h"
#include "rtcUtils.h"
#include "wwdg.h"

uint32_t crashDumpData[RTC_CRASHDUMP_ARRAY_SZ];

/**
 * @brief Collect the register dump stored by the hard fault handler
 * @details The hard fault handler backed the final stack frame
 *          (register values) in some of the RTC's backup registers.
 *          This function collects that data into an array for use
 *          by device_telemetry (event logging).
 *          
 *          It is expected that this function is called one time,
 *          early in main after the WWDG is started but before
 *          FreeRTOS has been given a chance to start the default
 *          thread that refreshes the watchdog -- so care must be
 *          taken here to avoid a watchdog reset.
 */
void collectCrashDump_rtcUtils(void)
{
    HAL_WWDG_Refresh(&hwwdg);

    for (int i = 0; i < RTC_NUM_CRASH_ELEMENTS; i++) {
        crashDumpData[i] = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_R0 + i);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_R0 + i, 0);
    }

    HAL_WWDG_Refresh(&hwwdg);

    // If you're lucky enough to have a serial console...
    printf("\nr0:  %08lx\n", crashDumpData[0]);
    printf("r1:  %08lx\n", crashDumpData[1]);
    printf("r2:  %08lx\n", crashDumpData[2]);
    printf("r3:  %08lx\n", crashDumpData[3]);
    printf("r12: %08lx\n", crashDumpData[4]);
    printf("lr:  %08lx\n", crashDumpData[5]);
    printf("pc:  %08lx\n", crashDumpData[6]);
    printf("psr: %08lx\n", crashDumpData[7]);
    printf ("BFAR = %08lx\n", crashDumpData[8]);
    printf ("CFSR = %08lx\n", crashDumpData[9]);
    printf ("HFSR = %08lx\n", crashDumpData[10]);
    printf ("DFSR = %08lx\n", crashDumpData[11]);
    printf ("AFSR = %08lx\n", crashDumpData[12]);
    printf ("SCB_SHCSR = %08lx\n", crashDumpData[13]);

    HAL_WWDG_Refresh(&hwwdg);
}

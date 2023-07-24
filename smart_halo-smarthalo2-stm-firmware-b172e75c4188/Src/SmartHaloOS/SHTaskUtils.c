/**
 * @file       SHTaskUtils.c
 *
 * @brief      Some SmartHaloOS task utilities
 *
 * @author     Georg Nikodym
 * @copyright  Copyright (c) 2020 SmartHalo Inc
 */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "SHTaskUtils.h"
#include "WatchdogTask.h"
#include "wwdg.h"
#include "Shell.h"
#include "device_telemetry.h"
#include "rtc.h"
#include "rtcUtils.h"

/**
 * @brief   A simple table of task "objects"
 * @details The name member must point to string constants because it is
 *          expected that they live in the .rodata section which remains
 *          in internal flash addresses -- this is done because FreeRTOS
 *          uses a RAM structure to store task names (which tends not
 *          to survive reboots)
 */
static struct {
    ESHWD_Bits_t bit;     ///< bits used by the software watchdog event group
    TaskHandle_t handle;  ///< FreeRTOS task handle
    UBaseType_t  hwMark;  ///< stack highwater mark
    char *       name;    ///< taskname
} taskTable[] = {
    { .bit = eSHWD_INVALID, .name = TASKNAME_TIMER },
    { .bit = eSHWD_INVALID, .name = TASKNAME_IDLE },
    { .bit = eSHWD_INVALID, .name = TASKNAME_DEFAULT },
    { .bit = eSHWD_INVALID, .name = TASKNAME_SOFTWATCHDOG },
    { .bit = eSHWD_INVALID, .name = TASKNAME_IWDG },
    { .bit = eSHWD_SystemUtilities, .name = TASKNAME_SYSTEMUTILITIES },
    { .bit = eSHWD_SensorsTask, .name = TASKNAME_SENSORS },
    { .bit = eSHWD_PersonalityTask, .name = TASKNAME_PERSONALITY },
    { .bit = eSHWD_CommunicationTask, .name = TASKNAME_COMMUNICATION },
    { .bit = eSHWD_RideTask, .name = TASKNAME_RIDE },
    { .bit = eSHWD_GraphicsTask, .name = TASKNAME_OLED },
    { .bit = eSHWD_SoundTask, .name = TASKNAME_SOUND },
    { .bit = eSHWD_AlarmTask, .name = TASKNAME_ALARM },
    { .bit = eSHWD_NightLightTask, .name = TASKNAME_NIGHTLIGHT },
    { .bit = eSHWD_HaloTask, .name = TASKNAME_HALO },
};
static const uint32_t taskTableSize = (sizeof(taskTable) / sizeof(taskTable[0]));

static const char *schedHistory[10];
static int schedHistoryIndex;
static volatile bool printing;

/**
 * @brief Dump the scheduling history array to the console
 */
static void printSchedHistory(void)
{
    printing = true;
    log_Shell("Scheduling history:");
    schedHistoryIndex--;
    if (schedHistoryIndex < 0) schedHistoryIndex = 9;
    for (int i = schedHistoryIndex; i >= 0; i--) {
        if (schedHistory[i]) {
            log_Shell("%2d: %s", i, schedHistory[i]);
        }
    }
    for (int i = 9; i > schedHistoryIndex; i--) {
        if (schedHistory[i]) {
            log_Shell("%2d: %s", i, schedHistory[i]);
        }
    }
    schedHistoryIndex++;
    if (schedHistoryIndex >= 10) schedHistoryIndex = 0;
    printing = false;
}

/**
 * @brief   Log the taskname being scheduled out
 * @details Called by traceTASK_SWITCHED_OUT() macro inside the FreeRTOS
 *          scheduler. Stores the taskname in a circular array
 *          of pointers to static const names.
 *          
 *          Logging is temporarily disabled by printSchedHistory() so
 *          that the array can be dumped without it changing. Since
 *          this is only used by the console shell we don't need to do
 *          anything more sophisticated (we can't really because we're
 *          in the scheduler!)
 * 
 * @param[in] taskhandle handle to the current task
 * @param[in] taskname the name of the current task
 */
void logSched_SHTaskUtils(xTaskHandle taskhandle, const char *taskname)
{
    if (printing == true) return;
    schedHistory[schedHistoryIndex] = findConstTaskname_SHTaskUtils(taskhandle, taskname);
    schedHistoryIndex++;
    if (schedHistoryIndex >= 10) schedHistoryIndex = 0;
}

/**
 * @brief   Saves a couple task names in preparation for a crash
 * @details Saves the most recent items logs in RTC registers. These will get
 *          picked up at boot time by main().
 */
void saveLastTasks_SHTaskUtils(void)
{
    int idx = schedHistoryIndex - 1;
    if (idx < 0) idx = 9;
    HAL_WWDG_Refresh(&hwwdg);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_LASTTASK1, (uint32_t)schedHistory[idx]);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_LASTTASK2, (uint32_t)schedHistory[--idx < 0 ? 9 : idx]);
}

/**
 * @brief   Indirectly called by FreeRTOS when a stack overflow is detected
 * @details Hacky magic: examine the task name (which is in .bss)
 *          and return a pointer to a string in the .rodata section (flash).
 *
 * @param taskname    the name of the task
 */
const char *findConstTaskname_SHTaskUtils(xTaskHandle taskhandle, const char *taskname)
{
    for (int i = 0; i < taskTableSize; i++) {
        if (taskTable[i].handle == taskhandle
            || (taskTable[i].handle == NULL && !strncmp(taskname, taskTable[i].name, 4))) {
            return taskTable[i].name;
        }
    }
    return "unknown";
}

/**
 * @brief   Print the task table
 * @details Called by the stack shell command
 */
void printTasks_SHTaskUtils(void)
{
    UBaseType_t nTasks = uxTaskGetNumberOfTasks();
    log_Shell("%" PRIu32 " running tasks", nTasks);

    if (nTasks != taskTableSize) {
        log_Shell("Task table size of %" PRIu32 " does not match nTasks!!!", taskTableSize);
    }

    for (int i = 0; i < taskTableSize; i++) {
        log_Shell("%24s %12" PRIu32, taskTable[i].name, taskTable[i].hwMark);
    }

    printSchedHistory();
}

/**
 * @brief   Collect the stack highwater marks for all tasks
 * @details Called periodically by the software watchdog, this function
 *          lazily fills the handle member of the taskTable and gathers
 *          stack highwater marks.
 */
void collectTaskStats_SHTaskUtils(void)
{
    for (int i = 0; i < taskTableSize; i++) {
        if (!taskTable[i].handle)
            taskTable[i].handle = xTaskGetHandle(taskTable[i].name);
        taskTable[i].hwMark = uxTaskGetStackHighWaterMark(taskTable[i].handle);
    }
}

/**
 * @brief   Add the stack highwater marks to the device event log
 * @details Called periodically by the software watchdog, this function
 *          creates an event log for each task, containing the stack
 *          highwater mark.
 */
void logStack_SHTaskUtils(void)
{
    for (int i = 0; i < taskTableSize; i++) {
        log_deviceTelemetry(eSTACKHW_0 + i, taskTable[i].hwMark);
    }
}
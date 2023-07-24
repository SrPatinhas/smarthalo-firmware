/*
    @file WatchdogTask.h

    API for the two watchdog tasks (one software, one hardware)

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019-2020 SmartHalo Inc
*/

#ifndef __WATCHDOG_TASK_H__
#define __WATCHDOG_TASK_H__

/**
 * @brief   Enumeration of bit shift values for the tasks monitored by the software watchdog
 *
 *          The software watchdog is implemented around a FreeRTOS event group. Each
 *          task checks in, setting a bit in the event group word. This is the
 *          secret decoder ring.
 */
typedef enum {
    eSHWD_SystemUtilities = 0,  ///< System utilities task (0x001)
    eSHWD_GraphicsTask,         ///< Graphics (oled) task (0x002)
    eSHWD_HaloTask,             ///< Halo task (0x004)
    eSHWD_SensorsTask,          ///< Sensors task (0x008)
    eSHWD_CommunicationTask,    ///< Communications task (0x010)
    eSHWD_SoundTask,            ///< Sound task (0x020)
    eSHWD_PersonalityTask,      ///< Personality task (0x040)
    eSHWD_NightLightTask,       ///< Night light task (0x080)
#ifndef GOLDEN
    eSHWD_RideTask,             ///< Ride task (0x100)
    eSHWD_AlarmTask,            ///< Alarm task (0x200)
#endif

    eSHWD_MAX,                  //< limit value, for iterator bounds checking
    eSHWD_INVALID               //< out of range value, must always be last
} ESHWD_Bits_t;

void WatchdogTaskInit(void);
void disableWatchdog_WatchdogTask(void);

void        shwd_KeepAlive(ESHWD_Bits_t task);
const char *eshwdTask2name(ESHWD_Bits_t bit);

#endif // __WATCHDOG_TASK_H__
/*
 * @file       SHTaskUtils.h
 *
 * @brief      Some SmartHaloOS task utilities
 *
 * @author     Georg Nikodym
 * @copyright  Copyright (c) 2020 SmartHalo Inc
 */

#ifndef __SHTASKUTILS_H__
#define __SHTASKUTILS_H__

#include <FreeRTOS.h>
#include <task.h>

#define TASKNAME_TIMER "Tmr Svc"                        // eSTACKHW_0
#define TASKNAME_IDLE "IDLE"                            // eSTACKHW_1
#define TASKNAME_DEFAULT "defaultTask"                  // eSTACKHW_2
#define TASKNAME_SOFTWATCHDOG "SoftWatchdog"            // eSTACKHW_3
#define TASKNAME_IWDG "Watchdog_IWDG"                   // eSTACKHW_4
#define TASKNAME_SYSTEMUTILITIES "systemUtilities"      // eSTACKHW_5
#define TASKNAME_SENSORS "Sensors_Task"                 // eSTACKHW_6
#define TASKNAME_PERSONALITY "Personality"              // eSTACKHW_7
#define TASKNAME_COMMUNICATION "CommunicationTa"        // eSTACKHW_8
#define TASKNAME_RIDE "Ride Task"                       // eSTACKHW_9
#define TASKNAME_OLED "oled_TASK"                       // eSTACKHW_A
#define TASKNAME_SOUND "sound_TASK"                     // eSTACKHW_B
#define TASKNAME_ALARM "Alarm"                          // eSTACKHW_C
#define TASKNAME_NIGHTLIGHT "Night Light"               // eSTACKHW_D
#define TASKNAME_HALO "HaloTask"                        // eSTACKHW_E

const char *findConstTaskname_SHTaskUtils(xTaskHandle taskhandle, const char *taskName);
void        printTasks_SHTaskUtils(void);
void        collectTaskStats_SHTaskUtils(void);
void        logStack_SHTaskUtils(void);
void        logSched_SHTaskUtils(xTaskHandle taskhandle, const char *taskname);
void        saveLastTasks_SHTaskUtils(void);

#endif // __SHTASKUTILS_H__

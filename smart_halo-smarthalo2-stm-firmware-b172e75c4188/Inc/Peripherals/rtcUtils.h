/**
 * @file        rtcUtils.h
 * @details
 * @author      Georg Nikodym <georg@smarthalo.bike>
 * @copyright   Copyright (c) 2021 SmartHalo Technologies
 */

#ifndef __RTCUTILS_H__
#define __RTCUTILS_H__

// The RTC backup registers are simply numbered.
// These our symbols names for each one that we
// are using.
#define RTC_BKP_SOFTWD      0x00
#define RTC_BKP_TASKNAME    0x01
#define RTC_BKP_FAULT       0x02

// Used by crash handler
#define RTC_BKP_R0          0x03
#define RTC_BKP_R1          0x04
#define RTC_BKP_R2          0x05
#define RTC_BKP_R3          0x06
#define RTC_BKP_R12         0x07
#define RTC_BKP_LR          0x08
#define RTC_BKP_PC          0x09
#define RTC_BKP_PSR         0x0a
#define RTC_BKP_BFAR        0x0b
#define RTC_BKP_CFSR        0x0c
#define RTC_BKP_HFSR        0x0d
#define RTC_BKP_DFSR        0x0e
#define RTC_BKP_AFSR        0x0f
#define RTC_BKP_SCB_SHCSR   0x10

#define RTC_BKP_LASTTASK1   0x11
#define RTC_BKP_LASTTASK2   0x12

#define RTC_BKP_REBOOT      0x13

#define RTC_NUM_CRASH_ELEMENTS  14
#define RTC_CRASHDUMP_ARRAY_SZ  16

/**
 * @brief   Fault handler type
 * @details The CubeMX generated code creates several different
 *          fault handling functions, all of which simply reset
 *          the device.
 *          
 *          In practice we _think_ we only ever see the hard fault
 *          handler run but there was really no way to distinguish. 
 *          The handlers have been modified to write the appropriate
 *          fault code into a backup register.
 */
enum {
    eFaultHard = 1,
    eFaultMemManage,
    eFaultBus,
    eFaultUsage
};

void collectCrashDump_rtcUtils(void);

extern uint32_t crashDumpData[RTC_CRASHDUMP_ARRAY_SZ];

#endif // __RTCUTILS_H__

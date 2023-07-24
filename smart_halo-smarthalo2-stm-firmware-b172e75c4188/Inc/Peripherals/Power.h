/**
 * @file       Power.h
 *
 * @brief      Power management header file
 *
 * @author     Georg Nikodym
 * @copyright  Copyright (c) 2020 SmartHalo Inc
 */

#ifndef _POWER_H_
#define _POWER_H_

/*! @brief  Wake-up reasons
*/
typedef enum {
    WUR_ACCEL = 1,
    WUR_BLEWAKEUP = 2,
} EWakeupReasons_t;

void init_Power(void);
void start_Power(void);

void setState_Power(bool newPowerState);
void stayAwake_Power(const char *calledFrom);
bool waitForFS_SystemUtilities(const char *filename);
void wakeupFromISR_Power(EWakeupReasons_t reason);
void sleepAllowed_Power(bool canSleep, const char *func);

#define SLEEP_ALLOWED()     do { sleepAllowed_Power(true, __func__); } while(0)
#define SLEEP_NOTALLOWED()  do { sleepAllowed_Power(false, __func__); } while(0)

#endif
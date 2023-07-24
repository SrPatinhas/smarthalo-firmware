/*!
    @file       SHTimers.c

    @detail     Timer utilities for SmartHaloOS

    @author     Georg Nikodym
    @copyright  Copyright (c) 2020 SmartHalo Inc
*/

#include "SHTimers.h"
#include "Shell.h"

#define NUM_TIMERS  20
#define DELAY       portMAX_DELAY

static TimerHandle_t timerTable[NUM_TIMERS];
static int numTimers;

/*! @brief  Register a timer

            Simple routine to start a timer handle in a table
            for later management.

    @param  timer a timer handle, as prepared by xTimerCreate*()
*/
void storeTimer(TimerHandle_t timer)
{
    // do not overflow the array, just ignore
    if (numTimers > NUM_TIMERS - 1) {
        return;
    }
    timerTable[numTimers] = timer;
    numTimers++;
}

/*! @brief  Stop all the timers

            Stop each registered timer. Called by power management prior to
            putting the MCU into a low-power sleep.
*/
void stopTimers(void)
{
    // log_Shell("%s: numTimers: %d", __func__, numTimers);
    for (int i = 0; i < numTimers; i++) {
        // log_Shell("%s: %s", __func__, pcTimerGetName(timerTable[i]));
        xTimerStop(timerTable[i], DELAY);
    }
}

/*! @brief  Start all the timers

            (Re)start each registered timer. Called by power management after
            waking the MCU from a low-power sleep.
*/
void startTimers(void)
{
    // log_Shell("%s: numTimers: %d", __func__, numTimers);
    for (int i = 0; i < numTimers; i++) {
        // log_Shell("%s: %s", __func__, pcTimerGetName(timerTable[i]));
        xTimerReset(timerTable[i], DELAY);
    }
}

/*! @brief  Print timer info

            Print the name and remaining time for each registered timer.
*/
void timeLeft_SHTimers(void)
{
    TickType_t now = xTaskGetTickCount();
    for (int i = 0; i < numTimers; i++) {
        log_Shell("%-18s: %10ld", pcTimerGetName(timerTable[i]),
                  xTimerIsTimerActive(timerTable[i])
                      ? xTimerGetExpiryTime(timerTable[i]) - now
                      : 0);
    }
}

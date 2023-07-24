/*!
    @file       SHTimers.h

    @detail     Header for SmartHaloOS timer utilities

    @author     Georg Nikodym
    @copyright  Copyright (c) 2020 SmartHalo Inc
*/

#ifndef __SHTIMERS_H__
#define __SHTIMERS_H__

#include <stdio.h>
#include "FreeRTOS.h"
#include "timers.h"

void storeTimer(TimerHandle_t timer);
void stopTimers(void);
void startTimers(void);
void timeLeft_SHTimers(void);

#endif // __SHTIMERS_H__
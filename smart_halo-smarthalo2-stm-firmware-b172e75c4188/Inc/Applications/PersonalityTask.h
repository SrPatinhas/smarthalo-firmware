/*
 * PersonalityTask.h
 *
 *  Created on: Sep 18, 2019
 *      Author: matt
 */

#ifndef APPLICATIONS_PERSONALITYTASK_H_
#define APPLICATIONS_PERSONALITYTASK_H_

#include <main.h>
#include <AnimationsLibrary.h>

struct ClockQuickPayload{
    uint8_t hour;
    HsvColour_t hourHsv;
    uint8_t minute;
    HsvColour_t minuteHsv;
    uint16_t duration;
    bool fade;
    bool intro;
    bool pulse;
    bool is24Hour;
}__attribute__((packed));

struct TouchSoundsPayload{
    uint8_t volume;
    bool enabled;
}__attribute__((packed));

struct LowBatteryPayload{
    uint8_t volume;
}__attribute__((packed));

struct SleepModePayload{
    uint8_t codeBitmap;
    uint8_t length;
}__attribute__((packed));

void init_PersonalityTask();

#endif /* APPLICATIONS_PERSONALITYTASK_H_ */

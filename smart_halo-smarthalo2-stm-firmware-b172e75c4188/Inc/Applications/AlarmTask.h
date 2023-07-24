/*
 * AlarmTask.h
 *
 *  Created on: 28 Oct 2019
 *      Author: Matt
 */

#ifndef APPLICATIONS_ALARMTASK_H_
#define APPLICATIONS_ALARMTASK_H_

#include <main.h>

typedef enum {
    AlarmDemoOff = 0,
    AlarmDemoArm = 1,
    AlarmDemoDisarm = 2,
    AlarmDemoWarn = 3,
    AlarmDemoSound = 4,
} AlarmDemoStates_e;

struct AlarmArmStatePayload{
    uint32_t seed;
    uint8_t isArmed;
}__attribute__((packed));

struct AlarmSettingsPayload{
    uint8_t code;
    uint8_t length;
    uint8_t mode;
    uint8_t severity;
    uint8_t allowStandby;
}__attribute__((packed));

struct AlarmDemoPayload {
    AlarmDemoStates_e state;
    uint8_t progress;
}__attribute__((packed));

void init_AlarmTask();

#endif /* APPLICATIONS_ALARMTASK_H_ */

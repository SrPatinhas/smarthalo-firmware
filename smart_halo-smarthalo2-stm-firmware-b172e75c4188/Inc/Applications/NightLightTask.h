/*
 * NightLightTask.h
 *
 *  Created on: 11 Oct 2019
 *      Author: Matt
 */

#ifndef APPLICATIONS_NIGHTLIGHTTASK_H_
#define APPLICATIONS_NIGHTLIGHTTASK_H_

#include <main.h>
#include <GraphicsTask.h>

struct NightLightSettingsPayload{
    NightLightModes_e nightLightMode;
    uint8_t nightLightPercentage;
    bool isBlinkingLocked;
    bool silenceModeRocker;
}__attribute__((packed));

struct NightLightNotifyPayload{
    uint8_t type;
    uint8_t setting;
    uint8_t state;
    uint8_t touch;
}__attribute__((packed));

struct NightLightStatePayload{
    bool isOn;
    bool isMovementRequired;
}__attribute__((packed));

void init_NightLightTask();

#endif /* APPLICATIONS_NIGHTLIGHTTASK_H_ */

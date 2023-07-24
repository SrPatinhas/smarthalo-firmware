/*
 * AnimationsLibrary.h
 *
 *  Created on: Jun 5, 2019
 *      Author: Nzo
 */

#ifndef ANIMATIONSLIBRARY_H_
#define ANIMATIONSLIBRARY_H_

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "semphr.h"

#include "cmsis_os.h"

#include "GraphicsTask.h"
#include "CommunicationTask.h"

#define LED_COUNT 24

typedef struct
{
	uint8_t startLED;
	uint8_t endLED;
	float startFraction;
	float endFraction;
	uint8_t ledGap;
	int16_t hueSweep;
	int8_t direction;
} LEDAnimationData_t;

struct AverageSpeedAnimationData{
    uint32_t averageMetersPerHour;
    uint32_t metersPerHour;
    HsvColour_t lowerHsv;
    HsvColour_t averageHsv;
    HsvColour_t higherHsv;
    bool gap;
    bool off;
};

struct FractionalData{
    uint8_t percentage;
    HsvColour_t colour;
};

struct fireData{
    HsvColour_t colour;
};

struct PointerAnimationData{
    uint16_t heading;
    int16_t headingGap;
    HsvColour_t colour;
    bool standby;
    bool intro;
    bool off;
};

struct SpeedometerAnimationData{
    uint8_t percentage;
    int8_t gap;
    bool off;
    bool intro;
};

struct AngleAnimationData{
    HsvColour_t colour;
    int16_t angle;
    uint8_t progress;
    uint16_t width;
    bool background;
    HsvColour_t backgroundColour;
    uint8_t repeat;
    int8_t gap;
    bool complete;
    bool backgroundReady;
    bool sameTurn;
};

struct AnglesAnimationData{
    HsvColour_t nextColour;
    int16_t nextAngle;
    uint8_t nextProgress;
    uint16_t nextWidth;
    int8_t nextGap;
    struct AngleAnimationData angleData;
};

struct ProgressAnimationData{
    HsvColour_t colour1;
    HsvColour_t colour2;
    uint16_t cycle;
    uint8_t progress;
    bool lowPower;
    int8_t gap;
    bool off;
    bool intro;
};

struct CustomCircleAnimationData{
    bool flash;
    uint8_t style;
    HsvColour_t mainColour;
    HsvColour_t otherColour;
    int16_t angles[MAX_ANGLES];
    uint8_t count;
    uint16_t width;
    bool ready;
    bool off;
};

struct ClockAnimationData{
    uint8_t hour;
    HsvColour_t hourHsv;
    uint8_t minute;
    HsvColour_t minuteHsv;
    uint16_t duration;
    bool fade;
    bool intro;
    bool pulse;
    bool is24Hour;
};

struct SpiralAnimationData{
    HsvColour_t colour;
    int16_t tail;
    bool clockwise;
    uint8_t speed;
    uint8_t rotations;
};

struct StateOfChargeAnimationData{
    uint8_t stateOfCharge;
    bool charging;
    bool showIdle;
};

void setMaxBrightness_AnimationsLibrary(uint8_t brightness);
uint8_t getMaxBrightness_AnimationsLibrary();
int logo_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount);
int disconnect_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount);
int pointer_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct PointerAnimationData * data);
uint16_t pointerStandbyLocation_AnimationsLibrary(uint16_t animationClockCount, uint16_t pointerHeading);
float speedometerIntroTicks_AnimationLibrary();
int speedometer_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct SpeedometerAnimationData * data);
int averageSpeed_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AverageSpeedAnimationData * data);
int fractionalData_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, struct FractionalData data, bool fractionalDataOff);
int fire_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, HsvColour_t colour, bool fireOff);
int nightLightIntro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount);
int nightLight_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, uint8_t * nightLEDPercentage, const struct FrontLightAnimationData * data);
int nightLightOutro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount);
int progressIntro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct ProgressAnimationData * data);
int progress_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct ProgressAnimationData * data);
int customCircle_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct CustomCircleAnimationData *circleData);
int battery_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct StateOfChargeAnimationData * data);
int angleTwoBackground_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AnglesAnimationData * data);
int angleBackground_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AngleAnimationData * data);
int angleTwo_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AnglesAnimationData * data);
int angle_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AngleAnimationData * data);
int angleIntro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount);
int spiral_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct SpiralAnimationData * data);
int swipe_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, bool isRight, bool connectionState);
void taps_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, uint8_t code, uint8_t length, bool connectionState, eStandbyReason_t standbyReason, bool isShadow);
int alarmStateChange_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, uint8_t alarmProgress, uint8_t * frontLEDPercentage);
int alarmSettingChange_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, bool alarmEnabled);
int clock_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct ClockAnimationData * clockSettings);
void ledsTest_AnimationsLibrary(uint8_t * leds, uint8_t stage, uint8_t brightness, uint8_t offLED);

#endif /* ANIMATIONSLIBRARY_H_ */

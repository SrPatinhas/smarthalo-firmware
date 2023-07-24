/*
 * RideTask.h
 *
 *  Created on: 19 Dec 2019
 *      Author: Matt
 */

#ifndef APPLICATIONS_RIDETASK_H_
#define APPLICATIONS_RIDETASK_H_

#include <main.h>
#include <AnimationsLibrary.h>

struct PointerPayload{
    HsvColour_t hsv;
    int16_t angle;
    uint32_t distance;
    uint8_t text[30];
}__attribute__((packed));

struct PointerStandbyPayload{
    HsvColour_t hsv;
    uint32_t distance;
    uint8_t text[30];
}__attribute__((packed));

struct SpeedometerPayload{
    uint8_t percentage;
    uint8_t speed;
    uint8_t unitType;
}__attribute__((packed));

struct NavigationPayload{
    uint32_t distance;
    uint16_t angle;
    HsvColour_t haloColour;
    uint8_t text[30];
}__attribute__((packed));

struct CarouselNotifyPayload{
    uint8_t type;
    uint8_t position;
}__attribute__((packed));

struct CarouselDistancePayload{
    uint32_t distance;
    HsvColour_t haloColour;
}__attribute__((packed));

struct CarouselTimePayload{
    uint16_t seconds;
    HsvColour_t haloColour;
}__attribute__((packed));

struct CarouselCaloriesPayload{
    uint16_t calories;
    HsvColour_t haloColour;
}__attribute__((packed));

struct CarouselCarbonDioxidePayload{
    uint16_t grams;
    HsvColour_t haloColour;
}__attribute__((packed));

struct NavAnglePayload{
    HsvColour_t hsv;
    int16_t angle;
    uint8_t progress;
    HsvColour_t nextHsv;
    int16_t nextAngle;
    uint8_t nextProgress;
    uint32_t distance;
    uint32_t nextDistance;
    uint8_t names[38];
}__attribute__((packed));

struct NavLegacyPayload{
    HsvColour_t hsv;
    int8_t direction;
    uint8_t progress;
    uint32_t distance;
    uint8_t names[60];
}__attribute__((packed));

struct DisplayNotificationPayload{
    HsvColour_t colour;
    HsvColour_t otherColour;
    int16_t firstFade;
    int16_t firstDuration;
    int16_t secondFade;
    int16_t secondDuration;
    uint8_t repeat;
    uint8_t type;
    uint8_t text[30];
}__attribute__((packed));

struct ProgressPayload{
    HsvColour_t colour1;
    HsvColour_t colour2;
    uint16_t cycle;
    uint8_t progress;
    bool isProgress;
    uint8_t text[30];
}__attribute__((packed));

struct RoundaboutPayload{
    bool exitNow;
    uint8_t style;
    HsvColour_t exitColour;
    HsvColour_t wrongExitColour;
    int16_t exits[MAX_ANGLES];
}__attribute__((packed));

struct RoundaboutOLEDPayload{
    uint32_t distance;
    uint8_t text[30];
}__attribute__((packed));

struct ClockPayload{
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

struct AverageSpeedPayload{
    uint32_t averageMetersPerHour;
    uint32_t metersPerHour;
    HsvColour_t lowerHsv;
    HsvColour_t averageHsv;
    HsvColour_t higherHsv;
}__attribute__((packed));

enum CarouselSwipeState{
    navigationCarousel = 0,
    goalCarousel = 1,
    distanceCarousel = 2,
    timeCarousel = 3,
    speedCarousel = 4,
    caloriesCarousel = 5,
    clockCarousel = 6,
    avgSpeedCarousel = 7,
    carbonDioxideCarousel = 8,
    batteryCarousel = 9,
};

typedef union {
    struct {
        uint32_t    navigation:1;   ///show navigation card
        uint32_t    goal:1;         //show goal card
        uint32_t    distance:1;     //show ride distance card
        uint32_t    time:1;         //show ride duration card
        uint32_t    speedometer:1;  //show speedometer card
        uint32_t    calories:1;     //show ride calories card
        uint32_t    clock:1;        //show clock card
        uint32_t    averageSpeed:1; //show ride average speed card
        uint32_t    carbonDioxide:1;//show ride saved cardon dioxide card
        uint32_t    battery:1;      //show battery card
        uint32_t    reserved:6;     //reserved for future carousel cards
        uint32_t    action:16;      //reserved for action carousel cards
    };
    struct {
        uint16_t rideBits;
        uint16_t actionBits;
    };
    uint32_t    all;
} CarouselMask_t;

struct CarouselMetricPayload {
    uint8_t position;
    CarouselMask_t mask;
    union {
        struct NavigationPayload navigationPayload;
        struct ProgressPayload progressPayload;
        struct CarouselDistancePayload distancePayload;
        struct CarouselTimePayload timePayload;
        struct SpeedometerPayload speedometerPayload;
        struct CarouselCaloriesPayload caloriesPayload;
        struct ClockPayload clockPayload;
        struct AverageSpeedPayload averageSpeedPayload;
        struct CarouselCarbonDioxidePayload carbonDioxidePayload;
    };
}__attribute__((packed));

struct CarouselPositionPayload {
    uint8_t position;
    CarouselMask_t mask;
}__attribute__((packed));

void init_RideTask();

#endif /* APPLICATIONS_RIDETASK_H_ */

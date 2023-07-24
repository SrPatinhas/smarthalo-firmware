/*
 * RideTask.c
 *
 *  Created on: 19 Dec 2019
 *      Author: Matt
 */

#include "main.h"
#include <RideTask.h>
#include <CommunicationTask.h>
#include <GraphicsTask.h>
#include <SystemUtilitiesTask.h>
#include <SensorsTask.h>
#include "SoundTask.h"
#include "OLEDLibrary.h"
#include "AnimationsLibrary.h"
#include "math.h"
#include "assets.h"
#include "WatchdogTask.h"
#include "SHTaskUtils.h"

#define STACK_SIZE (configMINIMAL_STACK_SIZE + 256)
#define TASK_PRIORITY   PRIORITY_BELOW_NORMAL
#define MAX_SUBTEXT 30
#define MINIMUM_NAV_TURN_DATA 15
#define MINIMUM_NAV_LEGACY_TURN_DATA 8
#define INTRO_TEXT_START_BYTE 1
#define RIDE_TO_CALIBRATION_FIRST_LINE 22
#define RIDE_TO_CALIBRATION_SECOND_LINE 42
#define BASE_STRING_Y_POSITION 58
#define BASE_STRING_Y_POSITION_NOTIFY 55
#define PERCENTAGE_Y_POSITION 21
#define PERCENTAGE_X_POSITION 114
#define MAX_PROGRESS_TEXT 30
#define UNITS_Y_POSITION 45
#define DISTANCE_UNITS_Y_POSITION 45
#define DISTANCE_UNITS_ONE_CHAR_X_POSITION 114
#define DISTANCE_UNITS_METRIC_TWO_CHAR_X_POSITION 107
#define DISTANCE_UNITS_IMP_TWO_CHAR_X_POSITION 116
#define INTRO_MILLISECOND_DURATION 3000
#define NOTIFY_BASE_MILLISECOND_DURATION 700
#define NOTIFY_REPEAT_MILLISECOND_DURATION 500
#define FEET_PER_MILE 5280
#define METERS_PER_KM 1000

#define DISTANCE_UNITS_FLAG 0x80000000
#define DISTANCE_VALUE_BITS 0x7FFFFFFF

#define RIDE_OLED_CAROUSEL_PRIORITY 1
#define RIDE_OLED_PRIORITY 4
#define RIDE_OLED_HIGH_PRIORITY 5

#define LEGACY_POINTER_PAYLOAD_SIZE 5
#define LEGACY_POINTER_STANDBY_PAYLOAD_SIZE 3
#define LEGACY_NOTIFY_LENGTH 15
#define CAROUSEL_DATA_PAYLOAD_START 6

#define DELAY_CAROUSEL_DISPLAY_MS 1000
#define MAX_INTERRUPT_COUNT 50
#define DISMISS_CAROUSEL_THRESHOLD 15

#define QUICK_TOUCH_ICONS_START 64

enum SpeedometerUnitTypes{
    km_per_hour = 0,
    mile_per_hour = 1,
    meters_per_second = 2,
    feet_per_second = 3
};

enum RoundaboutStyles{
    clockwise = 0,
    counterclockwise = 1,
    unspecified = 2,
};

typedef enum{
    notifyCall = 0,
    notifySMS = 1,
    notifyThunder = 32,
    notifyHorn = 64,
    notifyClock = 65,
    notifyHome = 66,
    notifyWork = 67,
    notifyStop = 68,
    notifyQuestion = 69,
    notifyAsTheCrowFlies = 70,
    notifyTurnByTurn = 71,
} NofityTypes;

static TaskHandle_t selfHandle = NULL;
static StaticTask_t xTaskBuffer;
static StackType_t RideTaskStack[STACK_SIZE];

const static uint8_t navigationTitle[] =
{ 0x00, 0xFF, 0x01, 0x00, 0x00, 0xC0, 0xFF, 0x0F, 0x00, 0x00, 0xF0, 0xFF, 0x3F, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x03, 0x00, 0xFC, 0xFF, 0xFF, 0x07, 0x00, 0xFE, 0xFF, 0xFF, 0x1F, 0x00, 0xFE, 0x81, 0xFF, 0x7F, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x7F, 0x00, 0xFE, 0xFF, 0x03, 0x7F, 0x00, 0xFE, 0xFF, 0x07, 0x7F, 0x00, 0xFE, 0xFF, 0x0F, 0x7F, 0x00, 0xFE, 0xFF, 0x0F, 0x7F, 0x00, 0xFE, 0xFF, 0x07, 0x7F, 0x00, 0xFE, 0xFF, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFE, 0x81, 0xFF, 0x7F, 0x00, 0xFE, 0xFF, 0xFF, 0x1F, 0x00, 0xFC, 0xFF, 0xFF, 0x07, 0x00, 0xFC, 0xFF, 0xFF, 0x03, 0x00, 0xF8, 0xFF, 0xFF, 0x00, 0x00, 0xF0, 0xFF, 0x3F, 0x00, 0x00, 0xC0, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00 };
const static uint8_t goalTitle[] =
{ 0xFE, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x82, 0x0F, 0x7E, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0x7E, 0xF0, 0x41, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00 };
const static uint8_t distanceTitle[] =
{ 0x00, 0x08, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x7F, 0x00, 0x80, 0xFF, 0x00, 0xC0, 0xFF, 0x01, 0xE0, 0xFF, 0x03, 0xF0, 0xFF, 0x07, 0xF8, 0xFF, 0x0F, 0xFC, 0xFF, 0x1F, 0xFE, 0xFF, 0x3F, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0x7F, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xC0, 0xFF, 0x01, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0x7F, 0xFE, 0xFF, 0x3F, 0xFC, 0xFF, 0x1F, 0xF8, 0xFF, 0x0F, 0xF0, 0xFF, 0x07, 0xE0, 0xFF, 0x03, 0xC0, 0xFF, 0x01, 0x80, 0xFF, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x08, 0x00 };
const static uint8_t timeTitle[] =
{ 0x00, 0x80, 0xFF, 0x03, 0x00, 0x00, 0xE0, 0xFF, 0x0F, 0x00, 0x00, 0xF8, 0xFF, 0x3F, 0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0x80, 0xFF, 0xFF, 0xFF, 0x03, 0x80, 0xFF, 0xFF, 0xFF, 0x03, 0xC0, 0xFF, 0xFF, 0xE7, 0x07, 0xC2, 0xFF, 0xFF, 0xE3, 0x07, 0xE7, 0xFF, 0xFF, 0xF1, 0x0F, 0xE7, 0xFF, 0xFF, 0xF8, 0x0F, 0xE7, 0xFF, 0x7F, 0xFC, 0x0F, 0xE7, 0xFF, 0x3F, 0xFE, 0x0F, 0xE7, 0xFF, 0x07, 0xFF, 0x0F, 0xE7, 0x03, 0x80, 0xFF, 0x0F, 0xE7, 0x03, 0xC0, 0xFF, 0x0F, 0xE7, 0xFF, 0xE7, 0xFF, 0x0F, 0xE7, 0xFF, 0xFF, 0xFF, 0x0F, 0xE7, 0xFF, 0xFF, 0xFF, 0x0F, 0xE7, 0xFF, 0xFF, 0xFF, 0x0F, 0xE7, 0xFF, 0xFF, 0xFF, 0x0F, 0xC2, 0xFF, 0xFF, 0xFF, 0x07, 0xC0, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0xFF, 0xFF, 0xFF, 0x03, 0x80, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0x00, 0xF8, 0xFF, 0x3F, 0x00, 0x00, 0xE0, 0xFF, 0x0F, 0x00, 0x00, 0x80, 0xFF, 0x03, 0x00 };
const static uint8_t speedTitle[] =
{ 0x00, 0x00, 0x7C, 0x00, 0x00, 0x80, 0x7F, 0x00, 0x00, 0xE0, 0x7F, 0x00, 0x00, 0xF8, 0x7F, 0x00, 0x00, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0xC0, 0xFF, 0x7F, 0x00, 0xE0, 0xFF, 0x7F, 0x00, 0xF0, 0xFF, 0x7F, 0x00, 0xF0, 0xFF, 0x7F, 0x00, 0xF8, 0xFF, 0x7F, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0xFF, 0xFF, 0x1F, 0x00, 0xFF, 0xFF, 0x07, 0x00, 0xFF, 0xFF, 0x07, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0x3F, 0x70, 0x00, 0xFE, 0x1F, 0x78, 0x00, 0xFE, 0x0F, 0x7C, 0x00, 0xFC, 0x07, 0x7E, 0x00, 0xFC, 0x03, 0x7F, 0x00, 0xF8, 0x83, 0x7F, 0x00, 0xF8, 0xC3, 0x7F, 0x00, 0xF0, 0xE7, 0x7F, 0x00, 0xF0, 0xFF, 0x7F, 0x00, 0xE0, 0xFF, 0x7F, 0x00, 0xC0, 0xFF, 0x7F, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0x7F, 0x00, 0x00, 0xF8, 0x7F, 0x00, 0x00, 0xE0, 0x7F, 0x00, 0x00, 0x80, 0x7F, 0x00, 0x00, 0x00, 0x7C, 0x00 };
const static uint8_t caloriesTitle[] =
{ 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x00, 0xFF, 0x3F, 0x00, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0xE0, 0xFF, 0x7F, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0x00, 0x00, 0xFC, 0xFF, 0xF7, 0x01, 0x00, 0xFF, 0xFF, 0xC0, 0x01, 0x80, 0xFF, 0x3F, 0x80, 0x01, 0xC0, 0xFF, 0x0F, 0x00, 0x00, 0xF0, 0xFF, 0x07, 0x00, 0x00, 0xFC, 0xFF, 0x03, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x81, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0x00, 0xE0, 0xFF, 0xFF, 0xFF, 0x00, 0xC0, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFC, 0xFF, 0x3F, 0x00, 0x00, 0xF0, 0xFF, 0x1F, 0x00, 0x00, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0xF8, 0x03, 0x00 };
const static uint8_t clockTitle[] =
{ 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0x01, 0x00, 0x00, 0x3E, 0xC0, 0x07, 0x00, 0x80, 0x07, 0x00, 0x1E, 0x00, 0xC0, 0xC1, 0x1F, 0x38, 0x00, 0xE0, 0xF0, 0xFF, 0x70, 0x00, 0x70, 0xFC, 0xFF, 0xE3, 0x00, 0x38, 0xFE, 0xFF, 0xC7, 0x01, 0x18, 0xFF, 0xFF, 0x8F, 0x01, 0x8C, 0xFF, 0xFF, 0x1F, 0x03, 0xCC, 0xFF, 0xFF, 0x39, 0x03, 0xC6, 0xFF, 0xFF, 0x38, 0x06, 0xE6, 0xFF, 0x7F, 0x7C, 0x06, 0xE6, 0xFF, 0x3F, 0x7E, 0x06, 0xF3, 0xFF, 0x1F, 0xFF, 0x0C, 0xF3, 0xFF, 0x8F, 0xFF, 0x0C, 0xF3, 0xFF, 0xC1, 0xFF, 0x0C, 0xF3, 0x00, 0xE0, 0xFF, 0x0C, 0xF3, 0x00, 0xF0, 0xFF, 0x0C, 0xF3, 0xFF, 0xF9, 0xFF, 0x0C, 0xF3, 0xFF, 0xFF, 0xFF, 0x0C, 0xF3, 0xFF, 0xFF, 0xFF, 0x0C, 0xE6, 0xFF, 0xFF, 0x7F, 0x06, 0xE6, 0xFF, 0xFF, 0x7F, 0x06, 0xC6, 0xFF, 0xFF, 0x3F, 0x06, 0xCC, 0xFF, 0xFF, 0x3F, 0x03, 0x8C, 0xFF, 0xFF, 0x1F, 0x03, 0x18, 0xFF, 0xFF, 0x8F, 0x01, 0x38, 0xFE, 0xFF, 0xC7, 0x01, 0x70, 0xFC, 0xFF, 0xE3, 0x00, 0xE0, 0xF0, 0xFF, 0x70, 0x00, 0xC0, 0xC1, 0x3F, 0x38, 0x00, 0x80, 0x07, 0x00, 0x1E, 0x00, 0x00, 0x3E, 0xC0, 0x07, 0x00, 0x00, 0xF8, 0xFF, 0x01, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00 };
const static uint8_t avgSpeedTitle[] =
{ 0x00, 0xFC, 0xFF, 0x01, 0x00, 0xFC, 0xFF, 0x01, 0x00, 0xFC, 0xFF, 0x01, 0x00, 0xFC, 0xFF, 0x01, 0x00, 0xFC, 0xFF, 0x01, 0x00, 0xFC, 0xFF, 0x01, 0x00, 0xFC, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x01 };
const static uint8_t carbonDioxideTitle[] =
{ 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x01, 0x00, 0x00, 0xF0, 0xFF, 0x03, 0x00, 0x00, 0xFE, 0xFF, 0x07, 0x00, 0x00, 0xFF, 0xF7, 0x07, 0x00, 0xC0, 0xFF, 0xEF, 0x0F, 0x00, 0xF0, 0xEF, 0xDF, 0x0F, 0x00, 0xFC, 0xDF, 0xBF, 0x0F, 0x00, 0xFE, 0xBF, 0x7F, 0x0F, 0x00, 0xFF, 0x00, 0x00, 0xFC, 0x03, 0xFE, 0xBF, 0x7F, 0x0F, 0x00, 0xFC, 0xDF, 0xBF, 0x0F, 0x00, 0xF0, 0xEF, 0xDF, 0x0F, 0x00, 0xC0, 0xFF, 0xEF, 0x0F, 0x00, 0x00, 0xFF, 0xF7, 0x07, 0x00, 0x00, 0xFE, 0xFF, 0x07, 0x00, 0x00, 0xF0, 0xFF, 0x03, 0x00, 0x00, 0xC0, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00 };
const static uint8_t batteryTitle[] =
{ 0xFE, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0x1F, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0xF3, 0xFF, 0xFF, 0x19, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0xFF, 0xFF, 0xFF, 0x1F, 0xFE, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x07, 0x00, 0x00, 0xFC, 0x07, 0x00};
const static uint8_t snail[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x0E, 0x00, 0x00, 0xFC, 0x3F, 0x00, 0x0F, 0x00, 0x00, 0x1F, 0xF8, 0x80, 0x0D, 0x00, 0xC0, 0x03, 0xC0, 0xC1, 0x0C, 0x00, 0xE0, 0x00, 0x00, 0x63, 0x0C, 0x00, 0x30, 0x00, 0x00, 0x36, 0x0C, 0x00, 0x18, 0x00, 0x00, 0x1C, 0x0C, 0x00, 0x0C, 0x00, 0x00, 0x18, 0x0C, 0x00, 0x06, 0x80, 0x01, 0x30, 0x0C, 0x00, 0x06, 0xE0, 0x07, 0x30, 0x0C, 0x00, 0x03, 0x70, 0x1E, 0x60, 0x0C, 0x00, 0x03, 0x18, 0x18, 0x60, 0x0C, 0x80, 0x01, 0x0C, 0x30, 0x60, 0x0C, 0x80, 0x01, 0x06, 0x60, 0x60, 0x0C, 0x80, 0x01, 0x03, 0x63, 0x60, 0x0C, 0xC0, 0x00, 0xC3, 0x60, 0x60, 0x0C, 0xC0, 0x80, 0x61, 0x20, 0x60, 0x0C, 0xC0, 0x80, 0x61, 0x30, 0x30, 0x0C, 0xC0, 0xC0, 0x60, 0x18, 0x30, 0x0C, 0xC0, 0xC0, 0xC0, 0x19, 0x18, 0x0C, 0xC0, 0xC0, 0x80, 0x0F, 0x18, 0x0C, 0xC0, 0x80, 0x01, 0x06, 0x0C, 0x0C, 0xC0, 0x80, 0x01, 0x00, 0x0C, 0x0C, 0x80, 0x01, 0x03, 0x00, 0x06, 0x0C, 0x80, 0x01, 0x06, 0x00, 0x03, 0x0C, 0x80, 0x01, 0x0C, 0x80, 0x01, 0x0C, 0x00, 0x03, 0x18, 0xC0, 0x00, 0x0C, 0x00, 0x03, 0xF0, 0x70, 0x08, 0x0C, 0x00, 0x06, 0xE0, 0x3F, 0x0C, 0x0C, 0x00, 0x06, 0x00, 0x0F, 0x0C, 0x0C, 0x00, 0x0C, 0x00, 0x00, 0x06, 0x0C, 0x00, 0x18, 0x00, 0x00, 0x06, 0x0C, 0x00, 0x30, 0x00, 0x00, 0x06, 0x0C, 0x00, 0xE0, 0x00, 0x00, 0x03, 0x0C, 0x00, 0xC0, 0x07, 0x80, 0x01, 0x0C, 0x00, 0x00, 0x3F, 0xE0, 0x00, 0x0C, 0x00, 0x00, 0xF8, 0x7F, 0x00, 0x0C, 0x00, 0x00, 0xC0, 0x5F, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x60, 0x00, 0x0C, 0x00, 0xC0, 0xFF, 0x3F, 0x00, 0x0C, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x06, 0x02, 0x30, 0x00, 0x00, 0x00, 0x06, 0xFD, 0x1F, 0x00, 0x00, 0x00, 0x03, 0x02, 0x0C, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x00, 0x80, 0x01, 0x00, 0x0C, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x70, 0x00, 0x82, 0x0F, 0xE0, 0xFF, 0x3F, 0x00, 0x7D, 0x1C, 0xF0, 0xFF, 0x0F, 0x00, 0x02, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x07, 0x00, 0x00, 0x00 };
const static uint8_t turnByTurn[] =
{ 0xF8, 0xFF, 0xFF, 0x0F, 0xFE, 0xFF, 0xFF, 0x3F, 0xFE, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0x1F, 0x00, 0x7C, 0xFF, 0x07, 0x00, 0x7C, 0xFF, 0x03, 0x00, 0x7C, 0xFF, 0x01, 0x00, 0x7C, 0xFF, 0x81, 0xFF, 0x7F, 0xFF, 0xC0, 0xFF, 0x7F, 0xFF, 0xE0, 0xFF, 0x7F, 0xFF, 0xE0, 0xFF, 0x7F, 0xFF, 0xE0, 0xFF, 0x7F, 0xFF, 0xE0, 0xFF, 0x7F, 0x1F, 0x00, 0xFE, 0x7F, 0x1F, 0x00, 0xFE, 0x7F, 0x3F, 0x00, 0xFF, 0x7F, 0x7F, 0x80, 0xFF, 0x7F, 0xFF, 0xC0, 0xFF, 0x7F, 0xFF, 0xE1, 0xFF, 0x7F, 0xFF, 0xF3, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x80, 0x01, 0x40, 0x01, 0x80, 0x01, 0x40, 0x01, 0xC0, 0x01, 0x40, 0x01, 0xC0, 0x01, 0x40, 0x01, 0xE0, 0x01, 0x40, 0x01, 0xE0, 0x01, 0x40, 0x01, 0xF0, 0x01, 0x40, 0x01, 0xF0, 0x01, 0x40, 0x01, 0xF8, 0x01, 0x40, 0x01, 0xF8, 0xFF, 0x43, 0x01, 0xFC, 0xFF, 0x43, 0x01, 0xFC, 0xFF, 0x40, 0x01, 0xFE, 0x3F, 0x40, 0x01, 0xFE, 0x0F, 0x40, 0x01, 0xFF, 0x03, 0x40, 0x01, 0xFF, 0x00, 0x40, 0x81, 0x3F, 0x00, 0x40, 0x81, 0x0F, 0x00, 0x40, 0xC1, 0x03, 0x00, 0x40, 0xC1, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x20, 0x06, 0x00, 0x00, 0x30, 0xF8, 0xFF, 0xFF, 0x0F };
const static uint8_t asTheCrowFlies[] =
{ 0xF8, 0xFF, 0xFF, 0x0F, 0x06, 0x00, 0x00, 0x30, 0x02, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0xE0, 0xFF, 0x43, 0x01, 0xF8, 0xFF, 0x43, 0x01, 0xFC, 0xFF, 0x43, 0x01, 0xFE, 0xFF, 0x43, 0x01, 0x7E, 0x00, 0x40, 0x01, 0x3F, 0x00, 0x40, 0x01, 0x1F, 0x00, 0x40, 0x01, 0x1F, 0x00, 0x40, 0x01, 0x1F, 0x00, 0x40, 0x01, 0x1F, 0x00, 0x40, 0xE1, 0xFF, 0x01, 0x40, 0xE1, 0xFF, 0x01, 0x40, 0xC1, 0xFF, 0x00, 0x40, 0x81, 0x7F, 0x00, 0x40, 0x01, 0x3F, 0x00, 0x40, 0x01, 0x1E, 0x00, 0x40, 0x01, 0x0C, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0x7F, 0xFE, 0x7F, 0xFF, 0x7F, 0xFE, 0x7F, 0xFF, 0x3F, 0xFE, 0x7F, 0xFF, 0x3F, 0xFE, 0x7F, 0xFF, 0x1F, 0xFE, 0x7F, 0xFF, 0x1F, 0xFE, 0x7F, 0xFF, 0x0F, 0xFE, 0x7F, 0xFF, 0x0F, 0xFE, 0x7F, 0xFF, 0x07, 0xFE, 0x7F, 0xFF, 0x07, 0x00, 0x7C, 0xFF, 0x03, 0x00, 0x7C, 0xFF, 0x03, 0x00, 0x7F, 0xFF, 0x01, 0xC0, 0x7F, 0xFF, 0x01, 0xF0, 0x7F, 0xFF, 0x00, 0xFC, 0x7F, 0xFF, 0x00, 0xFF, 0x7F, 0x7F, 0xC0, 0xFF, 0x7F, 0x7F, 0xF0, 0xFF, 0x7F, 0x3F, 0xFC, 0xFF, 0x7F, 0x3F, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFE, 0xFF, 0xFF, 0x3F, 0xFE, 0xFF, 0xFF, 0x3F, 0xF8, 0xFF, 0xFF, 0x0F };
const static uint8_t homeIcon[] =
{ 0x00, 0xE0, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0xF8, 0x03, 0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0xFE, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0xE0, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xE0, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFF, 0xFF, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0xF8, 0x03, 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0xE0, 0x00, 0x00 };
const static uint8_t workIcon[] =
{ 0x80, 0xFF, 0xFF, 0x0F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xDE, 0xFF, 0xFF, 0x1F, 0xDE, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDF, 0xFF, 0xFF, 0x1F, 0xDE, 0xFF, 0xFF, 0x1F, 0xDE, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0xC0, 0xFF, 0xFF, 0x1F, 0x80, 0xFF, 0xFF, 0x0F };
const static uint8_t stopIcon[] =
{ 0x00, 0xFF, 0x3F, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0xC0, 0xFF, 0xFF, 0x00, 0xE0, 0xFF, 0xFF, 0x01, 0xF0, 0xFF, 0xFF, 0x03, 0xF8, 0xFF, 0xFF, 0x07, 0xFC, 0xFF, 0xFF, 0x0F, 0xFE, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFE, 0xFF, 0xFF, 0x1F, 0xFC, 0xFF, 0xFF, 0x0F, 0xF8, 0xFF, 0xFF, 0x07, 0xF0, 0xFF, 0xFF, 0x03, 0xE0, 0xFF, 0xFF, 0x01, 0xC0, 0xFF, 0xFF, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0x3F, 0x00};
const static uint8_t hornIcon[] =
{ 0x80, 0xFF, 0x01, 0xE0, 0xFF, 0x07, 0xF0, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0xF8, 0xFF, 0x1F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xF0, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0xE0, 0xFF, 0x07, 0x80, 0xFF, 0x01, 0x80, 0xFF, 0x01, 0x80, 0xFF, 0x01, 0x80, 0xFF, 0x01, 0x80, 0xFF, 0x01, 0x80, 0xFF, 0x01, 0xC0, 0xFF, 0x03, 0xC0, 0xFF, 0x03, 0xC0, 0xFF, 0x03, 0xC0, 0xFF, 0x03, 0xC0, 0xFF, 0x03, 0xC0, 0xFF, 0x03, 0xE0, 0xFF, 0x07, 0xE0, 0xFF, 0x07, 0xE0, 0xFF, 0x07, 0xF0, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0x3F, 0xFE, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const static uint8_t questionMark[] =
{ 0x00, 0x1E, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x00, 0x00, 0x00, 0xE0, 0x1F, 0x00, 0x00, 0x00, 0xF0, 0x1F, 0x00, 0x00, 0x00, 0xF0, 0x1F, 0x00, 0x00, 0x00, 0xF0, 0x1F, 0x00, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0x00, 0xFE, 0x07, 0x80, 0x83, 0x1F, 0xFE, 0x03, 0xC0, 0xC7, 0x3F, 0xFF, 0x01, 0xF0, 0xC7, 0x3F, 0xFF, 0x01, 0xF8, 0xC7, 0x3F, 0xFF, 0x01, 0xFE, 0xC7, 0x3F, 0xFF, 0x01, 0xFF, 0xC7, 0x3F, 0xFF, 0x81, 0xFF, 0xC7, 0x3F, 0xFF, 0xC3, 0xFF, 0xC7, 0x3F, 0xFF, 0xE7, 0xFF, 0xC7, 0x3F, 0xFE, 0xFF, 0xFF, 0x87, 0x1F, 0xFE, 0xFF, 0xFF, 0x03, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0x00, 0xFC, 0xFF, 0x3F, 0x00, 0x00, 0xF8, 0xFF, 0x0F, 0x00, 0x00, 0xF8, 0xFF, 0x07, 0x00, 0x00, 0xF0, 0xFF, 0x03, 0x00, 0x00, 0xF0, 0xFF, 0x01, 0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00 };

static struct OLEDAsset navigationTitleAsset = {.width=24,.height=36,.asset=navigationTitle};
static struct OLEDLayout navigationTitleLayout = {.location[0]={.x=51,.y=2}};
static struct OLEDAsset goalTitleAsset = {.width=29,.height=32,.asset=goalTitle};
static struct OLEDLayout goalTitleLayout = {.location[0]={.x=49,.y=6}};
static struct OLEDAsset distanceTitleAsset = {.width=58,.height=23,.asset=distanceTitle};
static struct OLEDLayout distanceTitleLayout = {.location[0]={.x=34,.y=10}};
static struct OLEDAsset timeTitleAsset = {.width=32,.height=36,.asset=timeTitle};
static struct OLEDLayout timeTitleLayout = {.location[0]={.x=48,.y=2}};
static struct OLEDAsset speedTitleAsset = {.width=46,.height=25,.asset=speedTitle};
static struct OLEDLayout speedTitleLayout = {.location[0]={.x=40,.y=11}};
static struct OLEDAsset snailAsset = {.width=54,.height=44,.asset=snail};
static struct OLEDLayout snailLayout = {.location[0]={.x=37,.y=0}};
static struct OLEDAsset caloriesTitleAsset = {.width=26,.height=33,.asset=caloriesTitle};
static struct OLEDLayout caloriesTitleLayout = {.location[0]={.x=50,.y=6}};
static struct OLEDAsset clockTitleAsset = {.width=36,.height=36,.asset=clockTitle};
static struct OLEDLayout clockTitleLayout = {.location[0]={.x=46,.y=4}};
static struct OLEDAsset avgSpeedTitleAsset = {.width=27,.height=25,.asset=avgSpeedTitle};
static struct OLEDLayout avgSpeedTitleLayout = {.location[0]={.x=50,.y=12}};
static struct OLEDAsset carbonDioxideTitleAsset = {.width=19,.height=34,.asset=carbonDioxideTitle};
static struct OLEDLayout carbonDioxideTitleLayout = {.location[0]={.x=53,.y=6}};
static struct OLEDAsset batteryTitleAsset = {.width=55,.height=29,.asset=batteryTitle};
static struct OLEDLayout batteryTitleLayout = {.location[0]={.x=35,.y=8}};

static struct OLEDAsset destinationAsset = {.width=26,.height=39,.asset=destination};
static struct OLEDLayout destinationLayout = {.location[0]={.x=51,.y=2}};
static struct OLEDAsset counterClockwiseAsset = {.width=38,.height=39,.asset=counterClockwiseIcon};
static struct OLEDLayout counterClockwiseLayout = {.location[0]={.x=46,.y=1}};
static struct OLEDAsset clockwiseAsset = {.width=38,.height=39,.asset=clockwiseIcon};
static struct OLEDLayout clockwiseLayout = {.location[0]={.x=44,.y=1}};
static struct OLEDAsset navIntroAsset = {.width=71,.height=38,.asset=navIntro};
static struct OLEDLayout navIntroLayout = {.location[0]={.x=28,.y=5}};

static struct OLEDAsset callAsset = {.width=34,.height=38,.asset=callIcon};
static struct OLEDLayout callLayout = {.location[0]={.x=47,.y=3}};
static struct OLEDAsset msgAsset = {.width=42,.height=41,.asset=msgIcon};
static struct OLEDLayout msgLayout = {.location[0]={.x=43,.y=1}};
static struct OLEDAsset thunderAsset = {.width=38,.height=41,.asset=thunderIcon};
static struct OLEDLayout thunderLayout = {.location[0]={.x=45,.y=1}};
static struct OLEDAsset homeAsset = {.width=30,.height=32,.asset=homeIcon};
static struct OLEDLayout homeLayout = {.location[0]={.x=49,.y=6}};
static struct OLEDAsset workAsset = {.width=34,.height=29,.asset=workIcon};
static struct OLEDLayout workLayout = {.location[0]={.x=47,.y=9}};
static struct OLEDAsset stopAsset = {.width=31,.height=30,.asset=stopIcon};
static struct OLEDLayout stopLayout = {.location[0]={.x=48,.y=8}};
static struct OLEDAsset hornAsset = {.width=45,.height=24,.asset=hornIcon};
static struct OLEDLayout hornLayout = {.location[0]={.x=41,.y=13}};
static struct OLEDAsset questionAsset = {.width=29,.height=38,.asset=questionMark};
static struct OLEDLayout questionLayout = {.location[0]={.x=49,.y=5}};

static struct OLEDAsset borderTopAsset = {.width=120,.height=3,.asset=borderTop};
static struct OLEDAsset borderBottomAsset = {.width=120,.height=3,.asset=borderBottom};
static struct OLEDAsset borderSideAsset = {.width=1,.height=55,.asset=borderSide};
static struct OLEDLayout borderLayout = {.location[0]={.x=5,.y=2},.location[1]={.x=5,.y=58},.location[2]={.x=4,.y=5},.location[3]={.x=125,.y=5}};
static struct OLEDAsset asTheCrowFliesAsset = {.width = 116, .height = 31, .asset = asTheCrowFlies};
static struct OLEDLayout stateLayout = {.location[0] = {.x = 7,.y = 6}};
static struct OLEDAsset turnByTurnAsset = {.width = 116, .height = 31, .asset = turnByTurn};


static uint8_t drawingBoard[1024] = {0x00};
static uint8_t secondImg[1024] = {0};
static uint8_t navigationText[MAX_SUBTEXT];
static uint32_t navigationDistance;
static uint32_t navigationDistanceGoal;
static uint32_t navigationDistanceGap = 0;
static bool isMetric = true;
static bool displayNavigation = false;
static bool isDestination = false;
static uint32_t speedometerSpeed;
static uint32_t speedometerSpeedGap = 0;
static uint32_t speedometerSpeedGoal;
static uint8_t speedometerUnitType;
static bool displaySpeedometer;
static struct ProgressAnimationData progressData;
static uint32_t progress;
static uint32_t progressGap = 0;
static uint32_t progressGoal;
static uint8_t progressText[MAX_PROGRESS_TEXT];
static bool displayProgress;
static CarouselMask_t carouselMask;
static uint8_t roundaboutStyle = unspecified;
static struct ClockAnimationData clockData = {0};
static struct AverageSpeedAnimationData averageSpeedData;
static struct NavigationPayload navigationData;
static struct CarouselDistancePayload distanceData = {0,{194,148,255}}; // default colour
static struct CarouselTimePayload timeData = {0, {143,219,198}}; // default colour
static struct CarouselCaloriesPayload caloriesData = {0, {237,255,255}}; // default colour
static struct CarouselCarbonDioxidePayload carbonDioxideData = {0, {97,105,255}}; // default colour
static bool isPointerCalled = false;
static bool isPointerStandby = false;
static bool isAuthenticated = false;

static bool isNewNotification = false;
static bool notifyWithOLED = false;
static struct AngleAnimationData notificationAngle;
static uint8_t notificationType;
static uint8_t notificationText[MAX_SUBTEXT];
static TickType_t notificationCompleteTick = 0;

static bool isNewTurnData = false;
static struct AnglesAnimationData turnAngles;
static uint8_t carouselTimerCounter = 0;
static bool carouselMetricShowing = false;
static int8_t carouselPosition = 0;
static bool shouldUpdateCarouselPosition;
static oledDirections_e carouselDirection;
static TimerHandle_t CarouselTimer;
static StaticTimer_t CarouselTimerBuffer;
static struct SpeedometerAnimationData speedometerData = {0};
static uint8_t interruptCount = 0;
static TickType_t lastInterruptTick = 0;

const uint8_t calibrate1[4][30] = {"Ride to","Roulez pour","Zum Kalibrieren fahren","Rueda para"};
const uint8_t calibrate2[4][30] = {"calibrate","calibrer","fahren","calibrar"};
const uint8_t carousel_title_time[4][30] = {"TIME","TEMPS","ZEIT","TIEMPO"};
const uint8_t carousel_label_elapsed[4][30] = {"elapsed",{233,'c','o','u','l',233},"verstrichen","transcurrido"};
const uint8_t carousel_title_distance[4][30] = {"DISTANCE","DISTANCE","DISTANZ","DISTANCIA"};
const uint8_t carousel_title_speed[4][30] = {"SPEED","VITESSE","GESCHW.","VELOCIDAD"};
const uint8_t carousel_title_clock[4][30] = {"CLOCK","HORLOGE","UHR","RELOJ"};
const uint8_t carousel_title_calories[4][30] = {"CALORIES","CALORIES","KALORIEN",{'C','A','L','O','R',206,'A','S'}};
const uint8_t carousel_label_calories[4][30] = {"calories","calories","kalorien",{'c','a','l','o','r',237,'a','s'}};
const uint8_t carousel_title_avg_speed[4][30] = {"AVERAGE SPEED","VITESSE MOY.",{216,'-','G','E','S','C','H','W','.'},"VELOCIDAD PROM."};
const uint8_t carousel_title_battery[4][30] = {"BATTERY","BATTERIE","AKKU",{'B','A','T','E','R',206,'A'}};
const uint8_t carousel_title_goal[4][30] = {"GOAL","OBJECTIF","ZIEL","META"};
const uint8_t carousel_title_navigation[4][30] = {"NAVIGATION","NAVIGATION","NAVIGATION",{'N','A','V','E','G','A','C','I',211,'N'}};
const uint8_t units_meters[4][30] = {"meters",{'m',232,'t','r','e','s'},"Meter","metros"};
const uint8_t units_meters_short[4][30] = {"m","m","m","m"};
const uint8_t units_km[4][30] = {"km","km","km","km"};
const uint8_t units_miles[4][30] = {"mi","mi","mi","mi"};
const uint8_t units_miles_full[4][30] = {"miles","miles","meilen","millas"};
const uint8_t units_speed_metric[4][30] = {"km/h","km/h","km/h","km/h"};
const uint8_t units_avg_speed_metric[4][30] = {"km/h avg","km/h moy.",{248,' ','k','m','/','h'},"km/h prom."};
const uint8_t units_speed_imperial[4][30] = {"mph","mph","mph","mph"};
const uint8_t units_avg_speed_imperial[4][30] = {"mph","mph moy.",{248,' ','m','p','h'},"mph prom."};
const uint8_t units_grams[4][30] = {"g","g","g","g"};
const uint8_t units_kilograms[4][30] = {"kg","kg","kg","kg"};
const uint8_t units_feet[4][30] = {"ft","ft","ft","pie"};
const uint8_t units_feet_full[4][30] = {"feet","pieds",{'F','\xfc',223,'e'},"pies"};

// Get the sum of bits in a 2 byte number
static uint8_t bitCount(uint16_t bytes){
    uint16_t bits = (bytes&0x5555) + ((bytes>>1)&0x5555); //count 1's
    bits = (bits&0x3333) + ((bits>>2)&0x3333); //count 2's
    bits = (bits&0x0F0F) + ((bits>>4)&0x0F0F); // count 4's
    bits = (bits&0x00FF) + ((bits>>8)&0x00FF); // count 8's
    return bits;
}

static void prepareNumericAnimation(uint32_t * value, uint32_t * goal, uint32_t * gap, uint32_t newValue, bool * showing, bool reset){
    if(reset){
        *value = 0;
        *showing = false;
    }
    if(*showing){
        *value = *goal;
    }
    *goal = newValue;
    if(!*value)
        *value = *goal;
    *gap = abs(*value - *goal);
    *showing = true;
}

static void prepareNavForOLED(const uint8_t * text, uint32_t distance){
    bool reset = strcmp((char *)text,(char *)navigationText) || isMetric != !(distance&DISTANCE_UNITS_FLAG);
    prepareNumericAnimation(&navigationDistance,
            &navigationDistanceGoal,
            &navigationDistanceGap,
            distance&DISTANCE_VALUE_BITS,
            &displayNavigation,
            reset);
    isMetric = !(distance&DISTANCE_UNITS_FLAG);
    memcpy(navigationText,text,MAX_SUBTEXT);
}

static void prepareSpeedometerForOLED(uint8_t speed, uint8_t type){
    bool reset = speedometerUnitType != type;
    prepareNumericAnimation(&speedometerSpeed,
            &speedometerSpeedGoal,
            &speedometerSpeedGap,
            speed,
            &displaySpeedometer,
            reset);
    speedometerUnitType = type;
}

static void prepareProgressForOLED(uint8_t curProgress){
    bool reset = !displayProgress;
    prepareNumericAnimation(&progress,
            &progressGoal,
            &progressGap,
            curProgress,
            &displayProgress,
            reset);
}

static void notifyCarousel(uint8_t position){
    struct CarouselNotifyPayload carouselNotify;
    carouselNotify.type = BLE_NOTIFY_CAROUSEL;
    carouselNotify.position = position;

    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_NOTIFY, sizeof(struct CarouselNotifyPayload), &carouselNotify);
}

static uint8_t checkCarouselPositionLimits(int8_t carouselPosition){
    uint8_t count = bitCount(carouselMask.rideBits);
    if(carouselPosition > count - 1) return 0;
    if(carouselPosition < 0) return count-1;
    return carouselPosition;
}

//The difference between clamp position and check position is
//check position is about allowing the loop and clamp keeps it in bounds
static uint8_t clampCarouselPositionLimits(int8_t carouselPosition, CarouselMask_t mask){
    uint8_t count = bitCount(mask.rideBits);
    if(carouselPosition > count - 1) return count - 1;
    if(carouselPosition < 0) return 0;
    return carouselPosition;
}

static uint8_t getRealCarouselPosition(uint8_t carouselPosition, CarouselMask_t mask){
    uint8_t extraPositions = 0;
    for(int i=0;i<=carouselPosition+extraPositions;i++){
        if(((mask.rideBits>>i)&0x1) == 0)
            extraPositions++;
    }
    return extraPositions + carouselPosition;
}

static void dismissCarouselHalo(uint8_t position, CarouselMask_t mask){
    uint8_t realPosition = getRealCarouselPosition(position, mask);
    if(carouselMetricShowing){
        switch(realPosition){
            case navigationCarousel:
                anglesOff_GraphicsTask();
                break;
            case batteryCarousel:
                displayStateOfCharge_SystemUtilities(false);
                break;
            case goalCarousel:
                progressOff_GraphicsTask();
                break;
            case speedCarousel:
                speedometerOff_GraphicsTask();
                break;
            case avgSpeedCarousel:
                averageSpeedOff_GraphicsTask();
                break;
            case clockCarousel:
                clockOff_GraphicsTask();
                break;
            case distanceCarousel:
            case timeCarousel:
            case carbonDioxideCarousel:
            case caloriesCarousel:
                if(!isPointerCalled)
                    pointerOff_GraphicsTask();
                break;
        }
    }
    if(interruptCount < DISMISS_CAROUSEL_THRESHOLD && isPointerCalled)
        pointerOff_GraphicsTask();
}

static void showNavIntro(const uint8_t * text){
    memset(drawingBoard,0,sizeof(drawingBoard));
    alignPixellari16TextCentered_OLEDLibrary(drawingBoard, text, BASE_STRING_Y_POSITION);
    appendAssetToDisplay_OLEDLibrary(drawingBoard,
            navIntroAsset,
            navIntroLayout.location[0]);
    oled_cmd_t oledCMD = {
            .statusBarPosition = -1,
            .direction = oled_no_animation,
            .animation = false,
            .u8ImageCategory = oled_reactions,
            .u8ImageType = 0
    };
    setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_HIGH_PRIORITY, INTRO_MILLISECOND_DURATION);
    startAnimation_GraphicsTask(HaloAnimation_display_angle_intro, NULL);
}

static void silenceNavigation(){
    displayNavigation = false;
    navigationDistance = 0;
    roundaboutStyle = unspecified;
    isDestination = false;
    if(notificationCompleteTick < xTaskGetTickCount())
        anglesOff_GraphicsTask();
    circleOff_GraphicsTask();
    setPriorityImageTime_GraphicsTask(RIDE_OLED_HIGH_PRIORITY,0);
}

static void silenceSpeedometer(){
    if(displaySpeedometer){
        speedometerOff_GraphicsTask();
        speedometerSpeed = 0;
        displaySpeedometer = false;
        setPriorityImageTime_GraphicsTask(RIDE_OLED_CAROUSEL_PRIORITY,0);
    }
}

static void silenceProgress(){
    if(carouselMask.goal){
        if(carouselPosition > 0)
            carouselPosition--;
        clampCarouselPositionLimits(carouselPosition, carouselMask);
    }
    carouselMask.goal = false;
    updateStatusWidth_OLEDLibrary(bitCount(carouselMask.rideBits));
    progressOff_GraphicsTask();
    progress = 0;
    displayProgress = false;
}

static void pointerSetupOLED(const uint8_t * text, uint32_t distance){
    prepareNavForOLED(text, distance);
    if(!carouselMask.navigation){
        carouselMask.navigation = true;
        carouselPosition++;
    }
    clampCarouselPositionLimits(carouselPosition, carouselMask);
    updateStatusWidth_OLEDLibrary(bitCount(carouselMask.rideBits));
}

static void pointerSetupHalo(HsvColour_t colour, int16_t angle){
    isPointerCalled = true;
    dismissCarouselHalo(carouselPosition, carouselMask);
    if(interruptCount < DISMISS_CAROUSEL_THRESHOLD)
        return;
    pointerStateUpdate_GraphicsTask(isPointerStandby, false);
    struct PointerAnimationData pointerData = {
            .colour = colour,
            .heading = angle};
    startAnimation_GraphicsTask(HaloAnimation_display_pointer,(uint8_t *)&pointerData);
}

static void pointerShow(uint16_t length, const void * payload){
    if(length < LEGACY_POINTER_PAYLOAD_SIZE){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct PointerPayload * pointerPayload = payload;
    if(length > LEGACY_POINTER_PAYLOAD_SIZE)
        pointerSetupOLED(pointerPayload->text, __REV(pointerPayload->distance));
    isPointerStandby = false;
    pointerSetupHalo(pointerPayload->hsv, __REV16(pointerPayload->angle));
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void pointerStandby(uint16_t length, const void * payload){
    if(length < LEGACY_POINTER_STANDBY_PAYLOAD_SIZE){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct PointerStandbyPayload * pointerPayload = payload;
    if(length > LEGACY_POINTER_STANDBY_PAYLOAD_SIZE)
        pointerSetupOLED(pointerPayload->text, __REV(pointerPayload->distance));
    isPointerStandby = true;
    pointerSetupHalo(pointerPayload->hsv, 0);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void silencePointerNavigation(){
    if(!turnAngles.angleData.progress)
        silenceNavigation();
    pointerOff_GraphicsTask();
    isPointerCalled = false;
    isPointerStandby = false;
}

static void pointerOff(uint16_t length, const void * payload){
    silencePointerNavigation();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void angleTurn(uint16_t length, const void * payload){
    const struct NavAnglePayload * anglePayload = payload;
    isDestination = false;
    turnAngles.angleData.colour = anglePayload->hsv;
    turnAngles.angleData.angle = __REV16(anglePayload->angle);
    turnAngles.angleData.sameTurn = anglePayload->progress&0x80;
    turnAngles.angleData.progress = anglePayload->progress&0x7f;
    turnAngles.angleData.width = 105;
    turnAngles.angleData.background = true;
    turnAngles.angleData.backgroundColour.v = anglePayload->hsv.v;
    turnAngles.angleData.backgroundColour.s = 0;
    if(anglePayload->nextProgress){
        turnAngles.nextColour = anglePayload->nextHsv;
        turnAngles.nextAngle = __REV16(anglePayload->nextAngle);
        turnAngles.nextProgress = anglePayload->nextProgress;
        turnAngles.nextWidth = 45;
    }else{
        turnAngles.nextProgress = 0;
    }
    isNewTurnData = true;
    if(length >= MINIMUM_NAV_TURN_DATA){
        prepareNavForOLED(anglePayload->names, __REV(anglePayload->distance));
    }
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

typedef enum {
    NavDirection_destination = 0,
    NavDirection_straight = 1,
    NavDirection_slightRight = 2,
    NavDirection_right = 3,
    NavDirection_hardRight = 4,
    NavDirection_slightLeft = 5,
    NavDirection_left = 6,
    NavDirection_hardLeft = 7,
    NavDirection_uTurn = 8,
} LegacyNavDirections;

static void legacyNav(uint16_t length, const void * payload){
    const struct NavLegacyPayload * navData = payload;
    turnAngles.angleData.colour = navData->hsv,
    turnAngles.angleData.progress = navData->progress,
    turnAngles.angleData.background = true,
    turnAngles.angleData.backgroundColour = navData->hsv,
    turnAngles.angleData.backgroundColour.s = 0,
    turnAngles.angleData.progress = navData->progress,
    isDestination = false;
    switch(navData->direction){
        case NavDirection_destination:
            isDestination = true;
            turnAngles.angleData.width = 360;
            turnAngles.angleData.angle = 0;
            break;
        case NavDirection_straight:
            turnAngles.angleData.width = 180;
            turnAngles.angleData.angle = 0;
            break;
        case NavDirection_slightRight:
            turnAngles.angleData.width = 90;
            turnAngles.angleData.angle = 30;
            break;
        case NavDirection_right:
            turnAngles.angleData.width = 180;
            turnAngles.angleData.angle = 90;
            break;
        case NavDirection_hardRight:
            turnAngles.angleData.width = 90;
            turnAngles.angleData.angle = 120;
            break;
        case NavDirection_slightLeft:
            turnAngles.angleData.width = 90;
            turnAngles.angleData.angle = -30;
            break;
        case NavDirection_left:
            turnAngles.angleData.width = 180;
            turnAngles.angleData.angle = -90;
            break;
        case NavDirection_hardLeft:
            turnAngles.angleData.width = 90;
            turnAngles.angleData.angle = -120;
            break;
        case NavDirection_uTurn:
            turnAngles.angleData.width = 180;
            turnAngles.angleData.angle = 180;
            break;
        default:
            turnAngles.angleData.width = 180;
            turnAngles.angleData.angle = 0;
            break;
    }
    turnAngles.nextProgress = 0;
    isNewTurnData = true;
    if(length >= MINIMUM_NAV_LEGACY_TURN_DATA){
        prepareNavForOLED(navData->names, __REV(navData->distance));
    }
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void turnByTurnIntro(uint16_t length, const uint8_t * payload){
    uint8_t text[30] = {0};
    if(length > INTRO_TEXT_START_BYTE){
        memcpy(text, &payload[INTRO_TEXT_START_BYTE],length-INTRO_TEXT_START_BYTE);
    }
    showNavIntro(text);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void navOff(uint16_t length, const void * payload){
    silenceNavigation();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void notifyOff(uint16_t length, const void * payload){
    notificationCompleteTick = 0;
    if(!isNewTurnData)
        anglesOff_GraphicsTask();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void reroute(uint16_t length, const void * payload){
    anglesOff_GraphicsTask();
    struct SpiralAnimationData spiral = {
        .colour.h = 0,
        .colour.s = 0,
        .colour.v = 255,
        .clockwise = true,
        .tail = 60,
        .speed = 3,
        .rotations = 6,
    };
    startAnimation_GraphicsTask(HaloAnimation_display_spiral,(uint8_t *) &spiral);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void addBorder(uint8_t * screen){
    appendAssetToDisplay_OLEDLibrary(screen,
            borderTopAsset,
            borderLayout.location[0]);
    appendAssetToDisplay_OLEDLibrary(screen,
            borderBottomAsset,
            borderLayout.location[1]);
    appendAssetToDisplay_OLEDLibrary(screen,
            borderSideAsset,
            borderLayout.location[2]);
    appendAssetToDisplay_OLEDLibrary(screen,
            borderSideAsset,
            borderLayout.location[3]);
}

static void showNotification(){
    interruptCount = MAX_INTERRUPT_COUNT;
    uint16_t notifyDuration = NOTIFY_BASE_MILLISECOND_DURATION+NOTIFY_REPEAT_MILLISECOND_DURATION*notificationAngle.repeat;
    notificationCompleteTick = xTaskGetTickCount() + notifyDuration*2;
    startAnimation_GraphicsTask(HaloAnimation_display_angle,(uint8_t *) &notificationAngle);
    if(notifyWithOLED){
        memset(drawingBoard,0,sizeof(drawingBoard));
        oled_cmd_t oledCMD = {
                .statusBarPosition = -1,
                .direction = oled_no_animation,
                .animation = false,
                .u8ImageCategory = oled_reactions,
                .u8ImageType = 0
        };
        setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_HIGH_PRIORITY, notifyDuration);
        switch(notificationType){
            case notifyCall:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        callAsset,
                        callLayout.location[0]);
                break;
            case notifySMS:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        msgAsset,
                        msgLayout.location[0]);
                break;
            case notifyThunder:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        thunderAsset,
                        thunderLayout.location[0]);
                break;
            case notifyHorn:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        hornAsset,
                        hornLayout.location[0]);
                break;
            case notifyClock:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        clockTitleAsset,
                        clockTitleLayout.location[0]);
                break;
            case notifyWork:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        workAsset,
                        workLayout.location[0]);
                break;
            case notifyHome:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        homeAsset,
                        homeLayout.location[0]);
                break;
            case notifyStop:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        stopAsset,
                        stopLayout.location[0]);
                break;
            case notifyQuestion:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        questionAsset,
                        questionLayout.location[0]);
                break;
        }
        if(notificationType == notifyAsTheCrowFlies || notificationType == notifyTurnByTurn){
            oledCMD.animation = true;
            oledCMD.animationTypeMask = rockerAnimation;
            addBorder(drawingBoard);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, notificationText, BASE_STRING_Y_POSITION_NOTIFY);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    notificationType == notifyAsTheCrowFlies ? turnByTurnAsset : asTheCrowFliesAsset,
                            stateLayout.location[0]);
            memset(secondImg,0,sizeof(secondImg));
            addBorder(secondImg);
            appendAssetToDisplay_OLEDLibrary(secondImg,
                    notificationType == notifyAsTheCrowFlies ? asTheCrowFliesAsset : turnByTurnAsset,
                            stateLayout.location[0]);
            alignPixellari16TextCentered_OLEDLibrary(secondImg, notificationText, BASE_STRING_Y_POSITION_NOTIFY);
            setPriorityAnimatedImageWithCmd_GraphicsTask(drawingBoard,
                    secondImg,
                    &oledCMD,
                    RIDE_OLED_HIGH_PRIORITY,
                    NOTIFY_BASE_MILLISECOND_DURATION*4,
                    NOTIFY_BASE_MILLISECOND_DURATION/2,
                    NOTIFY_BASE_MILLISECOND_DURATION/2,
                    notificationType == notifyAsTheCrowFlies ? oled_right : oled_left);

        }else{
            //All values above are assigned to quick touch icons.
            if(notificationType >= QUICK_TOUCH_ICONS_START)
                addBorder(drawingBoard);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, notificationText, BASE_STRING_Y_POSITION_NOTIFY);
            setPriorityImageWithCmd_GraphicsTask(drawingBoard,
                    &oledCMD,
                    RIDE_OLED_HIGH_PRIORITY,
                    notifyDuration);
        }
    }
    isNewNotification = false;
}

static void notify(uint16_t length, const void * payload){
    const struct DisplayNotificationPayload * notifyData = payload;

    notificationAngle.colour = notifyData->colour;
    notificationAngle.angle = 0;
    notificationAngle.background = false;
    notificationAngle.width = 360;
    notificationAngle.progress = 100;
    notificationAngle.repeat = notifyData->repeat;

    anglesOff_GraphicsTask();
    isNewNotification = true;
    notifyWithOLED = length > LEGACY_NOTIFY_LENGTH;
    if(length > LEGACY_NOTIFY_LENGTH){
        notificationType = notifyData->type;
        memcpy(&notificationText,notifyData->text,length-16);
    }

    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void speedometerIntro(uint16_t length, const void * payload){
    startAnimation_GraphicsTask(HaloAnimation_display_speedometerIntro, NULL);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void speedometer(uint16_t length, const void * payload){
    if(length < 1){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct SpeedometerPayload * payloadData = payload;
    speedometerData.percentage = payloadData->percentage;
    if(length >= 2){
        prepareSpeedometerForOLED(payloadData->speed, payloadData->unitType);
    }
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void speedometerOff(uint16_t length, const void * payload){
    silenceSpeedometer();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void progressIntro(uint16_t length, const void * payload){
    if(length < 8){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    startAnimation_GraphicsTask(HaloAnimation_display_progressIntro,NULL);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void progressShow(uint16_t length, const void * payload){
    if(length < 10){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct ProgressPayload * data = payload;
    prepareProgressForOLED(data->progress);
    progressData.progress = data->progress;
    progressData.colour1 = data->colour1;
    progressData.colour2 = data->colour2;
    progressData.cycle = __REV16(data->cycle);
    progressData.lowPower = data->isProgress;
    if(length > 10)
        memcpy(progressText,data->text,length-10);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
    if(!carouselMask.goal){
        carouselPosition++;
        carouselPosition = clampCarouselPositionLimits(carouselPosition, carouselMask);
        updateStatusWidth_OLEDLibrary(bitCount(carouselMask.rideBits));
    }
    carouselMask.goal = true;
}

static void roundabout(uint16_t length, const void * payload){
    if(length < 10){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    isDestination = false;
    const struct RoundaboutPayload * roundaboutData = payload;
    uint8_t count = (length - 8) / 2;
    struct CustomCircleAnimationData circleData = {
            .mainColour = roundaboutData->exitColour,
            .otherColour = roundaboutData->wrongExitColour,
            .style = roundaboutData->style,
            .flash = roundaboutData->exitNow,
            .count = count,
            .width = count >= 5 ? 15 : 30,
    };
    for(int i=0;i<circleData.count;i++){
        circleData.angles[i] = __REV16(roundaboutData->exits[i]);
    }
    roundaboutStyle = roundaboutData->style;
    displayNavigation = true;
    anglesOff_GraphicsTask();
    startAnimation_GraphicsTask(HaloAnimation_display_customCircle,(uint8_t *) &circleData);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void roundaboutOLED(uint16_t length, const void * payload){
    if(length < 5){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    isDestination = false;
    const struct RoundaboutOLEDPayload * roundaboutData = payload;
    prepareNavForOLED(roundaboutData->text, __REV(roundaboutData->distance));

    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void displayCurrentTitle(oled_cmd_t oledCMD){
    uint8_t realPosition = getRealCarouselPosition(carouselPosition, carouselMask);
    notifyCarousel(realPosition);
    Localization_E locale = getLocale_SystemUtilities();
    memset(drawingBoard,0,sizeof(drawingBoard));
    switch(realPosition){
        case navigationCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_navigation[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    navigationTitleAsset,
                    navigationTitleLayout.location[0]);
            break;
        case goalCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_goal[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    goalTitleAsset,
                    goalTitleLayout.location[0]);
            break;
        case distanceCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_distance[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    distanceTitleAsset,
                    distanceTitleLayout.location[0]);
            break;
        case timeCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_time[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    timeTitleAsset,
                    timeTitleLayout.location[0]);
            break;
        case speedCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_speed[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    speedTitleAsset,
                    speedTitleLayout.location[0]);
            break;
        case caloriesCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_calories[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    caloriesTitleAsset,
                    caloriesTitleLayout.location[0]);
            break;
        case clockCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_clock[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    clockTitleAsset,
                    clockTitleLayout.location[0]);
            break;
        case avgSpeedCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_avg_speed[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    avgSpeedTitleAsset,
                    avgSpeedTitleLayout.location[0]);
            break;
        case carbonDioxideCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t *) "SPARED CO2", BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    carbonDioxideTitleAsset,
                    carbonDioxideTitleLayout.location[0]);
            break;
        case batteryCarousel:
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_title_battery[locale], BASE_STRING_Y_POSITION);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    batteryTitleAsset,
                    batteryTitleLayout.location[0]);
            break;
    }
    setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
    carouselMetricShowing = false;
    xTimerReset(CarouselTimer,100);
}

static void updateMask(CarouselMask_t newMask){
    uint8_t currentRealPosition = getRealCarouselPosition(carouselPosition, carouselMask);
    uint8_t newRelevantBits = bitCount(newMask.rideBits&(0xffff>>(16-currentRealPosition)));
    uint8_t oldRelevantBits = bitCount(carouselMask.rideBits&(0xffff>>(16-currentRealPosition)));
    carouselPosition = carouselPosition + (newRelevantBits - oldRelevantBits);
    carouselPosition = clampCarouselPositionLimits(carouselPosition, newMask);
    uint8_t newRealPosition = getRealCarouselPosition(carouselPosition, newMask);
    if(currentRealPosition != newRealPosition)
        dismissCarouselHalo(carouselPosition, carouselMask);
    carouselMask.all = newMask.all;
    uint8_t completeBits = bitCount(newMask.rideBits);
    updateStatusWidth_OLEDLibrary(completeBits);
}

static void updateCarouselPosition(uint16_t length, const void * payload){
    const struct CarouselPositionPayload * positionPayload = payload;
    if(length == 5)
        updateMask(positionPayload->mask);

    if(length < 1 || !((carouselMask.rideBits >> positionPayload->position)&0x1)){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }

    //Count the disabled cards before the absolute position to reach the relative position
    uint8_t relativeCount = 0;
    for(int i = 0;i<positionPayload->position;i++){
        if(!((carouselMask.rideBits>>i)&0x1))
            relativeCount++;
    }

    dismissCarouselHalo(carouselPosition, carouselMask);
    carouselPosition = positionPayload->position - relativeCount;
    oled_cmd_t oledCMD = {
           .statusBarPosition = carouselPosition,
           .direction = oled_no_animation,
           .animation = false,
    };
    displayCurrentTitle(oledCMD);

    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void updateCarouselMetricData(uint16_t length, const void * payload){
    const struct CarouselMetricPayload * metricPayload = payload;
    updateMask(metricPayload->mask);

    if(length < CAROUSEL_DATA_PAYLOAD_START){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
        return;
    }
    switch(metricPayload->position){
    	case navigationCarousel:
            memcpy(&navigationData,&metricPayload->navigationPayload,length-5);
            navigationData.distance = __REV(navigationData.distance);
            navigationData.angle = __REV16(navigationData.angle);
    		break;
    	case goalCarousel:;
    	    prepareProgressForOLED(metricPayload->progressPayload.progress);
    	    progressData.progress = metricPayload->progressPayload.progress;
            progressData.colour1 = metricPayload->progressPayload.colour1;
            progressData.colour2 = metricPayload->progressPayload.colour2;
            progressData.cycle = __REV16(metricPayload->progressPayload.cycle);
            progressData.lowPower = metricPayload->progressPayload.isProgress;
    	    if(length >= 15)
    	        memcpy(progressText,&metricPayload->progressPayload.text,length-15);
    	    break;
        case distanceCarousel:
            memcpy(&distanceData,&metricPayload->distancePayload,length-5);
        	distanceData.distance = __REV(distanceData.distance);
        	break;
        case timeCarousel:
            memcpy(&timeData,&metricPayload->timePayload,length-5);
            timeData.seconds = __REV16(timeData.seconds);
            break;
        case speedCarousel:;
            speedometerData.percentage = metricPayload->speedometerPayload.percentage;
            if(length >= 7){
                prepareSpeedometerForOLED(metricPayload->speedometerPayload.speed, metricPayload->speedometerPayload.unitType);
            }
            break;
        case caloriesCarousel:;
            memcpy(&caloriesData,&metricPayload->caloriesPayload,length-5);
            caloriesData.calories = __REV16(caloriesData.calories);
            break;
        case clockCarousel:;
            clockData.duration = -1;
            clockData.hour = metricPayload->clockPayload.hour;
            clockData.hourHsv = metricPayload->clockPayload.hourHsv;
            clockData.minute = metricPayload->clockPayload.minute;
            clockData.minuteHsv = metricPayload->clockPayload.minuteHsv;
            clockData.fade = metricPayload->clockPayload.fade;
            clockData.intro = metricPayload->clockPayload.intro;
            clockData.is24Hour = metricPayload->clockPayload.is24Hour;
            clockData.pulse = metricPayload->clockPayload.pulse;
            break;
        case avgSpeedCarousel:;
            averageSpeedData.averageMetersPerHour = __REV(metricPayload->averageSpeedPayload.averageMetersPerHour);
            averageSpeedData.metersPerHour = __REV(metricPayload->averageSpeedPayload.metersPerHour);
            averageSpeedData.averageHsv = metricPayload->averageSpeedPayload.averageHsv;
            averageSpeedData.higherHsv = metricPayload->averageSpeedPayload.higherHsv;
            averageSpeedData.lowerHsv = metricPayload->averageSpeedPayload.lowerHsv;
            break;
        case carbonDioxideCarousel:
            memcpy(&carbonDioxideData,&metricPayload->carbonDioxidePayload,length-5);
            carbonDioxideData.grams = __REV16(carbonDioxideData.grams);
            break;
        default:
            break;
    }

    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void progressOff(uint16_t length, const void * payload){
    silenceProgress();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void turnOffAnimations(uint16_t length, const void * payload){
    if(displayNavigation) silenceNavigation();
    if(displaySpeedometer) silenceSpeedometer();
    if(displayProgress) silenceProgress();
    animOff_GraphicsTask();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void showStateOfCharge(uint16_t length, const void * payload){
    displayStateOfCharge_SystemUtilities(false);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void authenticationStateUpdate(bool authenticationState){
    if(!authenticationState){
        if(displayNavigation) silenceNavigation();
        if(displaySpeedometer) silenceSpeedometer();
        if(displayProgress) silenceProgress();
        if(isPointerCalled) silencePointerNavigation();
        setPriorityImageTime_GraphicsTask(RIDE_OLED_CAROUSEL_PRIORITY, 0);
        if(carouselPosition == batteryCarousel && carouselMetricShowing)
            displayStateOfCharge_SystemUtilities(false);
        clockOff_GraphicsTask();
        dismissCarouselHalo(carouselPosition, carouselMask);
        carouselMetricShowing = false;
    }
    isAuthenticated = authenticationState;
}

static void accInterrupt(){
    // If authenticated track whether the device had interrupts
    if(!isAuthenticated) return;
    lastInterruptTick = xTaskGetTickCount();
    interruptCount++;
    if(interruptCount > MAX_INTERRUPT_COUNT)
        interruptCount = MAX_INTERRUPT_COUNT;
}

static void swipeData(touch_swipe_state_t swipeState){
    if(!isAuthenticated) return;
    if(displayNavigation && !isPointerCalled) return;
    interruptCount = MAX_INTERRUPT_COUNT;
    dismissCarouselHalo(carouselPosition, carouselMask);
    if(swipeState.direction == oled_right) carouselPosition--;
    else if(swipeState.direction == oled_left) carouselPosition++;
    carouselPosition = checkCarouselPositionLimits(carouselPosition);
    shouldUpdateCarouselPosition = true;
    carouselDirection = swipeState.direction;
}

static void CarouselTimerCB(TimerHandle_t timer){
    if(interruptCount < DISMISS_CAROUSEL_THRESHOLD){
        dismissCarouselHalo(carouselPosition, carouselMask);
        carouselMetricShowing = false;
        return; //only show the carousel if there has been movement
    }
    if(!isAuthenticated) return;
    uint8_t realPosition = getRealCarouselPosition(carouselPosition, carouselMask);
    carouselTimerCounter++;
    if(!carouselMetricShowing || !carouselMask.navigation || carouselTimerCounter%2)
        notifyCarousel(realPosition);
    carouselMetricShowing = true;
    if(displayNavigation && !isPointerCalled) return;
    if(isPointerCalled){
        pointerStateUpdate_GraphicsTask(isPointerStandby, false);
        return;
    }
    switch(realPosition){
        case navigationCarousel:
            if(xTaskGetTickCount() < notificationCompleteTick)
                break;
            struct AngleAnimationData angle = {
                .colour = navigationData.haloColour,
                .angle = navigationData.angle,
                .width = 105,
                .background = false,
            };
            bool isMetric = !(navigationData.distance&DISTANCE_UNITS_FLAG);
            //Leave the halo as a single LED until the user is at least 125 meters away
            int16_t x = navigationData.distance&DISTANCE_VALUE_BITS;
            x /= isMetric ? 1 : 3;
            angle.progress = x < 125 && x > 75 ? 250 - 2*x : 10;
            startAnimation_GraphicsTask(HaloAnimation_display_angle,(uint8_t *) &angle);
            break;
        case goalCarousel:
            startAnimation_GraphicsTask(HaloAnimation_display_progress,(uint8_t*)&progressData);
            break;
        case avgSpeedCarousel:
            startAnimation_GraphicsTask(HaloAnimation_display_averageSpeed,(uint8_t *) &averageSpeedData);
            break;
        case distanceCarousel: ;
            uint32_t distance = distanceData.distance&DISTANCE_VALUE_BITS;
            uint16_t divisor = (distanceData.distance&DISTANCE_UNITS_FLAG) ? FEET_PER_MILE : METERS_PER_KM;
            struct PointerAnimationData distancePointerData = {
                    .colour = distanceData.haloColour,
                    .heading = 180+(distance%divisor)*360/divisor};
            pointerStateUpdate_GraphicsTask(false, false);
            startAnimation_GraphicsTask(HaloAnimation_display_pointer,(uint8_t *)&distancePointerData);
            break;
        case timeCarousel: ;
            struct PointerAnimationData timePointerData = {
                    .colour = timeData.haloColour,
                    .heading = (timeData.seconds%60)*360/60};
            pointerStateUpdate_GraphicsTask(false, false);
            startAnimation_GraphicsTask(HaloAnimation_display_pointer,(uint8_t *)&timePointerData);
            break;
        case caloriesCarousel:;
            struct PointerAnimationData caloriesPointerData = {
                    .colour = caloriesData.haloColour,
                    .heading = 180+(caloriesData.calories%100)*360/100};
            pointerStateUpdate_GraphicsTask(false, false);
            startAnimation_GraphicsTask(HaloAnimation_display_pointer,(uint8_t *)&caloriesPointerData);
            break;
        case carbonDioxideCarousel: ;
            struct PointerAnimationData carbonDioxidePointerData = {
                    .colour = carbonDioxideData.haloColour,
                    .heading = 180+(carbonDioxideData.grams%1000)*360/1000};
            pointerStateUpdate_GraphicsTask(false, false);
            startAnimation_GraphicsTask(HaloAnimation_display_pointer,(uint8_t *)&carbonDioxidePointerData);
            break;
        case speedCarousel:
            startAnimation_GraphicsTask(HaloAnimation_display_speedometer,(uint8_t*)&speedometerData.percentage);
            break;
        case clockCarousel:
            startAnimation_GraphicsTask(HaloAnimation_display_clockLowPriority,(uint8_t *) &clockData);
            break;
        case batteryCarousel:
            setPriorityImageTime_GraphicsTask(RIDE_OLED_CAROUSEL_PRIORITY, 0);
            displayStateOfCharge_SystemUtilities(true);
            break;
    }
}

static void getMetricString(uint8_t *strMetric, uint32_t metric, bool allowZeroFirst, bool isMetric, bool onlyOneDecimal){
    memset(strMetric,'0',4);
    if(isMetric){
        if(metric < 1000 && !allowZeroFirst){
            itoa(round(metric/10.f)*10,(char *)strMetric,10);
        }else if(metric < 9995 && !onlyOneDecimal){
            uint16_t tenth = round(metric / 10.f);
            uint8_t wholeNumber = tenth/100;
            uint8_t fraction = tenth%100;
            sprintf((char *)strMetric,"%d.%02d", wholeNumber, fraction);
        }else if(metric < 100000){
            uint16_t hundredth = round(metric / 100.f);
            uint8_t wholeNumber = hundredth/10;
            uint8_t fraction = hundredth%10;
            sprintf((char *)strMetric,"%d.%1d", wholeNumber, fraction);
        }else{
            uint16_t wholeNumber = round(metric / 1000.f);
            itoa(wholeNumber,(char *)strMetric,10);
        }
    }else{
        if((metric < FEET_PER_MILE && !allowZeroFirst) || metric < FEET_PER_MILE/10){
            itoa(round(metric/10.f)*10,(char *)strMetric,10);
        }else if(metric < 9.95f*FEET_PER_MILE && !onlyOneDecimal){
            uint16_t tenth = round(100*metric / (float) FEET_PER_MILE);
            uint8_t wholeNumber = tenth/100;
            uint8_t fraction = tenth%100;
            sprintf((char *)strMetric,"%d.%02d", wholeNumber, fraction);
        }else if(metric < 100*FEET_PER_MILE){
            uint16_t hundredth = round(10*metric / (float) FEET_PER_MILE);
            uint8_t wholeNumber = hundredth/10;
            uint8_t fraction = hundredth%10;
            sprintf((char *)strMetric,"%d.%1d", wholeNumber, fraction);
        }else{
            uint16_t wholeNumber = round(metric / (float) FEET_PER_MILE);
            itoa(wholeNumber,(char *)strMetric,10);
        }
    }
}

static void rideTask(void *pvParameters) {
    assignFunction_CommunicationTask(COM_UI, UI_POINTER, pointerShow);
    assignFunction_CommunicationTask(COM_UI, UI_POINTER_STANDBY, pointerStandby);
    assignFunction_CommunicationTask(COM_UI, UI_POINTER_TURNOFF, pointerOff);
    assignFunction_CommunicationTask(COM_UI, UI_NAV_ANGLE, angleTurn);
    assignFunction_CommunicationTask(COM_UI, UI_NAV, legacyNav);
    assignFunction_CommunicationTask(COM_UI, UI_NAV_OFF, navOff);
    assignFunction_CommunicationTask(COM_UI, UI_TURN_BY_TURN_INTRO, turnByTurnIntro);
    assignFunction_CommunicationTask(COM_UI, UI_NAV_REROUTE, reroute);
    assignFunction_CommunicationTask(COM_UI, UI_NOTIF, notify);
    assignFunction_CommunicationTask(COM_UI, UI_SPEEDOMETER_INTRO, speedometerIntro);
    assignFunction_CommunicationTask(COM_UI, UI_SPEEDOMETER, speedometer);
    assignFunction_CommunicationTask(COM_UI, UI_SPEEDOMETER_OFF, speedometerOff);
    assignFunction_CommunicationTask(COM_UI, UI_FITNESS_INTRO, progressIntro);
    assignFunction_CommunicationTask(COM_UI, UI_PROGRESS, progressShow);
    assignFunction_CommunicationTask(COM_UI, UI_PROGRESS_OFF, progressOff);
    assignFunction_CommunicationTask(COM_UI, UI_ROUNDABOUT, roundabout);
    assignFunction_CommunicationTask(COM_UI, UI_ROUNDABOUT_OLED, roundaboutOLED);
    assignFunction_CommunicationTask(COM_UI, UI_CAROUSEL, updateCarouselMetricData);
    assignFunction_CommunicationTask(COM_UI, UI_CAROUSEL_POSITION, updateCarouselPosition);
    assignFunction_CommunicationTask(COM_UI, UI_NOTIF_OFF, notifyOff);
    assignFunction_CommunicationTask(COM_UI, UI_ANIM_OFF, turnOffAnimations);
    assignFunction_CommunicationTask(COM_UI, UI_SHOW_STATE_OF_CHARGE, showStateOfCharge);

    subscribeToConnectionState_CommunicationTask(authenticationStateUpdate);
    subscribeToSwipes_SensorsTask(swipeData);
    subscribeToAccInterrupt_SensorsTask(accInterrupt);

    carouselMask.rideBits = 1020;
    updateStatusWidth_OLEDLibrary(bitCount(carouselMask.rideBits));
    xTimerStart(CarouselTimer,200);


    while (1) {
        uint16_t delay = 100;
        if(speedometerSpeedGap){
            delay = 1000/speedometerSpeedGap;
            if(speedometerSpeed > speedometerSpeedGoal)
                speedometerSpeed--;
            else if(speedometerSpeed < speedometerSpeedGoal)
                speedometerSpeed++;
            else
                speedometerSpeedGap = 0;
        }
        if(navigationDistanceGap){
            delay = 1000/navigationDistanceGap;
            if(!delay)
                delay = 1;
            if(navigationDistance > navigationDistanceGoal)
                navigationDistance = delay < 5 ? navigationDistance-3 : navigationDistance-1;
            else if(navigationDistance < navigationDistanceGoal)
                navigationDistance = delay < 5 ? navigationDistance+3 : navigationDistance+1;
            else
                navigationDistanceGap = 0;
        }
        if(progressGap){
            delay = 1000/progressGap;
            if(progress > progressGoal)
                progress--;
            else if(progress < progressGoal)
                progress++;
            else
                progressGap = 0;
        }

        oled_cmd_t oledCMD = {
                .statusBarPosition = -1,
                .direction = oled_no_animation,
                .animation = false,
                .u8ImageCategory = oled_reactions,
                .u8ImageType = 0
        };
        uint8_t realPosition = getRealCarouselPosition(carouselPosition, carouselMask);
        TickType_t now = xTaskGetTickCount();
        Localization_E locale = getLocale_SystemUtilities();
        if(isNewNotification || notificationCompleteTick > now){
            if(isNewNotification)
                showNotification();
        }else if((displayNavigation && !isPointerCalled)
        		|| (displayNavigation
        		        && carouselMask.navigation
        		        && isPointerCalled
        		        && realPosition == navigationCarousel
        		        && interruptCount >= DISMISS_CAROUSEL_THRESHOLD)){
            if(isNewTurnData){
                if(turnAngles.nextProgress)
                    startAnimation_GraphicsTask(HaloAnimation_display_two_angles, (uint8_t *) &turnAngles);
                else
                    startAnimation_GraphicsTask(HaloAnimation_display_angle,(uint8_t *) &turnAngles.angleData);
                isNewTurnData = false;
            }
            memset(drawingBoard,0,sizeof(drawingBoard));
            if(!isPointerStandby)
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, navigationText, BASE_STRING_Y_POSITION);
            if(!navigationDistance){
                if(isDestination){
                    appendAssetToDisplay_OLEDLibrary(drawingBoard,
                            destinationAsset,
                            destinationLayout.location[0]);
                }else if(roundaboutStyle == clockwise || roundaboutStyle == counterclockwise){
                    if(roundaboutStyle == clockwise){
                        appendAssetToDisplay_OLEDLibrary(drawingBoard,
                                clockwiseAsset,
                                clockwiseLayout.location[0]);
                    }else{
                        appendAssetToDisplay_OLEDLibrary(drawingBoard,
                                counterClockwiseAsset,
                                counterClockwiseLayout.location[0]);
                    }
                }
            }else if(isPointerCalled && isPointerStandby){
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, calibrate1[locale], RIDE_TO_CALIBRATION_FIRST_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, calibrate2[locale], RIDE_TO_CALIBRATION_SECOND_LINE);
            }else{
                uint8_t distance[5];
                if((isMetric && navigationDistance < 994) || (!isMetric && navigationDistance < 525)){
                    uint16_t roundedDistance = navigationDistance > 10 ? round(navigationDistance/10.f)*10 : navigationDistance;
                    itoa(roundedDistance,(char *)distance,10);
                    alignRubik52Centered_OLEDLibrary(drawingBoard, distance ,UNITS_Y_POSITION);
                    if(isMetric){
                        appendPixellari16CharacterToDisplay_OLEDLibrary(drawingBoard, *units_meters_short[locale], DISTANCE_UNITS_ONE_CHAR_X_POSITION, DISTANCE_UNITS_Y_POSITION);
                    }else{
                        appendPixellari16CharactersToDisplay_OLEDLibrary(drawingBoard, units_feet[locale], 2, DISTANCE_UNITS_IMP_TWO_CHAR_X_POSITION,DISTANCE_UNITS_Y_POSITION);
                    }
                }else{
                    float fDistance;
                    if(isMetric){
                        if(navigationDistance < 999999)
                            fDistance = (navigationDistance/100)/10.f;
                        else
                            fDistance = (navigationDistance/100000)/10.f;
                    }else{
                        fDistance = navigationDistance/((float) FEET_PER_MILE);
                    }
                    uint16_t wholeNumber = floor(fDistance);
                    uint8_t offset = wholeNumber < 10 ? 1
                            : wholeNumber < 100 ? 2
                                    : wholeNumber < METERS_PER_KM ? 3
                                            : wholeNumber < 10000 ? 4
                                                    : 5;
                    itoa(wholeNumber,(char *)&distance[0],10);
                    if(offset < 2){
                        uint8_t fraction = round((fDistance - wholeNumber)*10.f);
                        itoa(fraction,(char *)&distance[offset+1],10);
                        distance[offset] = '.';
                    }
                    switch(offset){
                        case 1:
                            alignRubik52Centered_OLEDLibrary(drawingBoard, distance, UNITS_Y_POSITION);
                            break;
                        case 2:
                            alignRubik52Centered_OLEDLibrary(drawingBoard, distance, UNITS_Y_POSITION);
                            break;
                        case 3:
                            alignRubik43Centered_OLEDLibrary(drawingBoard, distance, UNITS_Y_POSITION);
                            break;
                        case 4: //Case is only for large miles count
                        case 5://Case is only for even larger miles count
                            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, distance ,2*UNITS_Y_POSITION/3);
                            break;
                    }
                    if(isMetric){
                        if(navigationDistance < 999999){
                            appendPixellari16CharactersToDisplay_OLEDLibrary(drawingBoard, units_km[locale], 2, DISTANCE_UNITS_METRIC_TWO_CHAR_X_POSITION,DISTANCE_UNITS_Y_POSITION);
                        }else{
                            appendPixellari16CharactersToDisplay_OLEDLibrary(drawingBoard, (uint8_t *) "Mm", 2, DISTANCE_UNITS_METRIC_TWO_CHAR_X_POSITION-3,DISTANCE_UNITS_Y_POSITION);
                        }
                    }else{
                        appendPixellari16CharactersToDisplay_OLEDLibrary(drawingBoard, (uint8_t *) "mi", 2, DISTANCE_UNITS_IMP_TWO_CHAR_X_POSITION-5,DISTANCE_UNITS_Y_POSITION);
                    }
                }
            }
            setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, carouselMask.navigation ? RIDE_OLED_CAROUSEL_PRIORITY : RIDE_OLED_HIGH_PRIORITY, 3000);
        //Check if the request for new data has had at least the delay to provide it
        }else if(interruptCount >= DISMISS_CAROUSEL_THRESHOLD){
            if(shouldUpdateCarouselPosition){
                oled_cmd_t oledCMD = {
                        .statusBarPosition = carouselPosition,
                        .direction = carouselDirection,
                        .animation = true,
                        .animationTime = 200,
                        .animationTypeMask = slidingAnimation,
                };
                displayCurrentTitle(oledCMD);
                shouldUpdateCarouselPosition = false;
            }else if(carouselMetricShowing && carouselMask.navigation && realPosition == navigationCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, navigationData.text, BASE_STRING_Y_POSITION);
                uint8_t distance[4];
                bool isMetric = !(navigationData.distance&DISTANCE_UNITS_FLAG);
                uint32_t metric = navigationData.distance&DISTANCE_VALUE_BITS;
                getMetricString(distance, metric, !isMetric, isMetric, true);
                alignRubik52Centered_OLEDLibrary(drawingBoard, distance ,UNITS_Y_POSITION);
                if((isMetric && metric < 994) || (!isMetric && metric < 525)){
                    if(isMetric){
                        appendPixellari16CharacterToDisplay_OLEDLibrary(drawingBoard, *units_meters_short[locale], DISTANCE_UNITS_ONE_CHAR_X_POSITION, UNITS_Y_POSITION);
                    }else{
                        appendPixellari16CharactersToDisplay_OLEDLibrary(drawingBoard, units_feet[locale], 2, DISTANCE_UNITS_IMP_TWO_CHAR_X_POSITION,UNITS_Y_POSITION);
                    }
                }else{
                    if(isMetric){
                        appendPixellari16CharactersToDisplay_OLEDLibrary(drawingBoard, units_km[locale], 2, DISTANCE_UNITS_METRIC_TWO_CHAR_X_POSITION,UNITS_Y_POSITION);
                    }else{
                        appendPixellari16CharactersToDisplay_OLEDLibrary(drawingBoard, units_miles[locale], 2, DISTANCE_UNITS_IMP_TWO_CHAR_X_POSITION-5,UNITS_Y_POSITION);
                    }
                }
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(displayProgress && carouselMetricShowing && carouselMask.goal && realPosition == goalCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, progressText, BASE_STRING_Y_POSITION);
                uint8_t progressPercent[4];
                itoa(progress,(char *)progressPercent,10);
                alignRubik52Centered_OLEDLibrary(drawingBoard, progressPercent ,UNITS_Y_POSITION);
                appendPixellari16CharacterToDisplay_OLEDLibrary(drawingBoard, '%', PERCENTAGE_X_POSITION, PERCENTAGE_Y_POSITION);
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(displaySpeedometer && carouselMetricShowing && realPosition == speedCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                char * units = "";
                switch(speedometerUnitType){
                    default:
                    case km_per_hour:
                        units = " km/h";
                        break;
                    case mile_per_hour:
                        units = "mph";
                        break;
                    case meters_per_second:
                        units = "m/s";
                        break;
                    case feet_per_second:
                        units = "ft/s";
                        break;
                }
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t *)units, BASE_STRING_Y_POSITION);
                if(speedometerSpeed){
                    uint8_t speed[3];
                    itoa(speedometerSpeed,(char *)speed,10);
                    alignRubik64Centered_OLEDLibrary(drawingBoard, speed ,UNITS_Y_POSITION);
                }else{
                    appendAssetToDisplay_OLEDLibrary(drawingBoard,
                            snailAsset,
                            snailLayout.location[0]);
                }
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(carouselMetricShowing && realPosition == distanceCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                uint8_t distance[4];
                bool isMetric = !(distanceData.distance&DISTANCE_UNITS_FLAG);
                uint32_t metric = distanceData.distance&DISTANCE_VALUE_BITS;
                getMetricString(distance, metric, !isMetric, isMetric, false);
                alignRubik52Centered_OLEDLibrary(drawingBoard, distance ,UNITS_Y_POSITION);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard,
                        (isMetric ? (metric < METERS_PER_KM ? units_meters[locale] : (uint8_t*) "km") : (metric < FEET_PER_MILE/10 ? units_feet_full[locale] : units_miles_full[locale])),
                        BASE_STRING_Y_POSITION);
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(carouselMetricShowing && realPosition == timeCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                uint8_t largeTime, smallTime;
                if(timeData.seconds < 3600){
                    largeTime = timeData.seconds/60;
                    smallTime = timeData.seconds%60;
                }else{
                    largeTime = timeData.seconds/3600;
                    smallTime = (timeData.seconds/60)%60;
                }
                uint8_t time[5] = "00:00";
                itoa(largeTime,(char *)(largeTime < 10 ? &time[1] : &time[0]),10);
                itoa(smallTime,(char *)(smallTime < 10 ? &time[4] : &time[3]),10);
                time[2] = ':';
                alignRubik43Centered_OLEDLibrary(drawingBoard, time ,UNITS_Y_POSITION);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_label_elapsed[locale], BASE_STRING_Y_POSITION);
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(carouselMetricShowing && realPosition == caloriesCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                uint8_t calories[5] = "00000";
                itoa(caloriesData.calories,(char *)calories,10);
                if(caloriesData.calories < 1000)
                    alignRubik52Centered_OLEDLibrary(drawingBoard, calories ,UNITS_Y_POSITION);
                else
                    alignRubik43Centered_OLEDLibrary(drawingBoard, calories ,UNITS_Y_POSITION);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, carousel_label_calories[locale], BASE_STRING_Y_POSITION);
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(carouselMetricShowing && realPosition == clockCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                uint8_t time[5] = "00000";
                uint8_t hour = clockData.hour;
                int8_t offset;
                if(clockData.is24Hour){
                    offset = 0;
                    itoa(hour,(char *)(hour < 10 ? &time[1+offset] : &time[0]),10);
                }else{
                    hour = hour%12 == 0 ? 12 : hour%12;
                    offset = hour < 10 ? -1 : 0;
                    itoa(hour,(char *)(&time[0]),10);
                    alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t*)((clockData.hour) >= 12 ? "PM" : "AM"), BASE_STRING_Y_POSITION);
                }
                time[2+offset] = ':';
                itoa(clockData.minute,(char *)(clockData.minute < 10 ? &time[4+offset] : &time[3+offset]),10);
                alignRubik43Centered_OLEDLibrary(drawingBoard, time ,UNITS_Y_POSITION);
                oled_cmd_t oledCMD = {
                        .statusBarPosition = -1,
                        .direction = oled_no_animation,
                        .animation = false,
                        .u8ImageCategory = oled_reactions,
                        .u8ImageType = 0
                };
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(carouselMetricShowing && realPosition == avgSpeedCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                uint8_t avgSpeed[4];
                bool isMetric =  !(averageSpeedData.averageMetersPerHour&DISTANCE_UNITS_FLAG);
                getMetricString(avgSpeed,averageSpeedData.averageMetersPerHour&DISTANCE_VALUE_BITS, true, isMetric, false);
                alignRubik52Centered_OLEDLibrary(drawingBoard, avgSpeed ,UNITS_Y_POSITION);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t*)(isMetric ? units_avg_speed_metric[locale] : units_avg_speed_imperial[locale]), BASE_STRING_Y_POSITION);
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }else if(carouselMetricShowing && realPosition == carbonDioxideCarousel){
                memset(drawingBoard,0,sizeof(drawingBoard));
                uint8_t carbonDioxide[4];
                getMetricString(carbonDioxide, carbonDioxideData.grams, false, true, false);
                if(carbonDioxideData.grams < 1000){
                    alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t*)"grams CO2", BASE_STRING_Y_POSITION);
                }else{
                    alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t*)" kg CO2", BASE_STRING_Y_POSITION);
                }
                alignRubik52Centered_OLEDLibrary(drawingBoard, carbonDioxide ,UNITS_Y_POSITION);
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, RIDE_OLED_CAROUSEL_PRIORITY, 3000);
            }
        }else if(carouselMetricShowing && interruptCount < DISMISS_CAROUSEL_THRESHOLD){
            //Turn off everything because it's stopped moving for a while.
            setPriorityImageTime_GraphicsTask(RIDE_OLED_CAROUSEL_PRIORITY, 0);
            setPriorityImageTime_GraphicsTask(RIDE_OLED_HIGH_PRIORITY, 0);
        }

        if(xTaskGetTickCount() - lastInterruptTick > 3000){
            if(interruptCount < DISMISS_CAROUSEL_THRESHOLD){
                interruptCount = 0;
            }else
                interruptCount -= 5;
            lastInterruptTick = xTaskGetTickCount();
        }
        vTaskDelay(delay);
#ifndef GOLDEN
        shwd_KeepAlive(eSHWD_RideTask);
#endif
    }
}

void init_RideTask(){
    if (selfHandle == NULL) {
        selfHandle = xTaskCreateStatic(rideTask, TASKNAME_RIDE,
                STACK_SIZE, NULL, TASK_PRIORITY, RideTaskStack, &xTaskBuffer);
        configASSERT(selfHandle);
    }

    if (CarouselTimer == NULL) {
        CarouselTimer = xTimerCreateStatic("CarouselTimer", DELAY_CAROUSEL_DISPLAY_MS / portTICK_PERIOD_MS,
                pdTRUE, NULL, CarouselTimerCB, &CarouselTimerBuffer);
    }
}

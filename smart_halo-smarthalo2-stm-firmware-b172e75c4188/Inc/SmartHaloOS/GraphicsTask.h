// ------------------------------------------------------------------------------------------------
/*!@file    GraphicsTask.h

 */
// ------------------------------------------------------------------------------------------------


#ifndef __GRAPHICTASK_H_
#define __GRAPHICTASK_H_

#include "main.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define MAX_ANGLES  10
#define MAX_LED_BRIGHTNESS 255
#define MIN_LED_BRIGHTNESS 120

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

typedef enum {
	oled_no_animation = 0,
	oled_left = 1,
	oled_right = 2,
	oled_down = 3,
	oled_up = 4,
} oledDirections_e;

typedef enum {
	oled_reactions = 0,
	oled_tests = 255,
} oledCategory_e;

typedef enum {
    oled_reaction_connected = 0,
    oled_reaction_startup = 1,
    oled_reaction_connectEn = 2,
    oled_reaction_connectFr = 3,
    oled_reaction_connectDe = 4,
    oled_reaction_connectSp = 5,
    oled_reaction_wake = 6,
    oled_reaction_bye = 7,
    oled_reaction_unpaired = 8,
    oled_reaction_lowBattery = 9,
    oled_reaction_onboardingWelcome = 10,
    oled_reaction_onboardingTap = 11,
    oled_reaction_onboardingRight = 12,
    oled_reaction_onboardingLeft = 13,
    oled_reaction_onboardingExplain = 14,
} oledReactionType_e;

typedef enum {
	oled_test_touch_short_tap = 0,
	oled_test_touch_long_tap = 1,
	oled_test_touch_left_swipe = 2,
	oled_test_touch_right_swipe = 3,
	oled_test_touch_complete = 4,
	oled_test_display = 5,
} oledTestType_e;

typedef struct {
	oledDirections_e direction;
	uint8_t percentage;
	bool secondary;
} touch_swipe_state_t;

typedef enum {
    HaloAnimation_display_stateOfCharge,
    HaloAnimation_display_speedometerIntro,
    HaloAnimation_display_speedometer,
    HaloAnimation_display_averageSpeed,
    HaloAnimation_display_fractionalData,
    HaloAnimation_display_fire,
    HaloAnimation_display_pointer,
    HaloAnimation_display_nightLight,
    HaloAnimation_display_progressIntro,
    HaloAnimation_display_progress,
    HaloAnimation_display_customCircle,
    HaloAnimation_display_clockLowPriority,
    HaloAnimation_display_angle,
    HaloAnimation_display_angle_intro,
    HaloAnimation_display_two_angles,
    HaloAnimation_display_spiral,
    HaloAnimation_display_clock,
    HaloAnimation_display_shadow_taps,
    HaloAnimation_display_taps,
    HaloAnimation_display_swipe,
    HaloAnimation_display_logo,
    HaloAnimation_display_disconnect,
    HaloAnimation_display_alarm_state,
    HaloAnimation_display_alarm_progress,
} HaloAnimation_function_e;

typedef enum {
    NightLightMode_persistent = 0,
    NightLightMode_blink = 1,
    NightLightMode_strobe = 2,
} NightLightModes_e;

struct FrontLightAnimationData{
    uint8_t percentage;
    bool isBlinking;
    int8_t gap;
};

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RgbColour_t;

typedef struct
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
} HsvColour_t;

enum AnimationType{
	rockerAnimation = 0x1,
	slidingAnimation = 0x2,
};

typedef struct {
	uint32_t 	animationTime; // in ms to do full transistion	*** Future Use
	uint8_t 	u8ImageCategory;
	uint8_t 	u8ImageType;
	bool 		animation;
	oledDirections_e 	direction;
	int8_t 	statusBarPosition; // -1 to 9
	uint8_t animationTypeMask;
} oled_cmd_t, *poled_cmd_t;

// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================

//uint32_t oled_stack();

void init_GraphicsTask(void);
void enableOLED_GraphicsTask(void);
void disableOLED_GraphicsTask(void);
bool oledOFF_GraphicsTask(bool * result);
bool setImage_GraphicsTask(poled_cmd_t poled_cmd);
void setPriorityImageTime_GraphicsTask(uint8_t priority, uint16_t additionalMsDuration);
void setPriorityAnimatedImageWithCmd_GraphicsTask(uint8_t * img, uint8_t * secondImg, poled_cmd_t poled_cmd, uint8_t priority, uint16_t msDuration, uint16_t msDelay, uint16_t msAnimationDuration, oledDirections_e direction);
void setPriorityImageWithCmd_GraphicsTask(const uint8_t * img, poled_cmd_t poled_cmd, uint8_t priority, uint16_t msDuration);
bool setCarouselImage_GraphicsTask(touch_swipe_state_t  * swipeState, bool * result);
bool setContrast_GraphicsTask(uint8_t constrast, bool * result);
bool setBrightness_GraphicsTask(uint8_t brightness, bool * result);
bool editDebugImage_GraphicsTask(uint16_t location, uint8_t value);
void unlockAnimations_GraphicsTask(const char * const taskName);
void lockAnimations_GraphicsTask(const char * const taskName);
bool startAnimationWithLock_GraphicsTask(HaloAnimation_function_e type, uint8_t * payload, const char * const taskName);
bool startAnimation_GraphicsTask(HaloAnimation_function_e type, uint8_t * u8Payload);
void setFrontLightLocked_GraphicsTask(bool isLocked);
bool getFrontLightLocked_GraphicsTask();
void startLEDTest_GraphicsTask(uint8_t stage, uint8_t brightness, uint8_t offLED);
void nightLightStateUpdate_GraphicsTask(struct FrontLightAnimationData data);
void pointerStateUpdate_GraphicsTask(bool standby, bool intro);
void pointerStateUpdateWithLock_GraphicsTask(bool standby, bool intro, const char * const taskName);
void pointerOff_GraphicsTask();
void speedometerOff_GraphicsTask();
void averageSpeedOff_GraphicsTask();
void fractionalDataOff_GraphicsTask();
void fireOff_GraphicsTask();
void progressOff_GraphicsTask();
void circleOff_GraphicsTask();
void anglesOff_GraphicsTask();
void clockOff_GraphicsTask();
void animOff_GraphicsTask();
void testNightLight_GraphicsTask(uint8_t percentage, uint8_t mode);
bool testHaloResponseByTurningOff_GraphicsTask();
bool testOLEDByTurningOff_GraphicsTask();

#endif  /* OLED_TASK_H_ */

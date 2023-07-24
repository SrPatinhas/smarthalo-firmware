// ------------------------------------------------------------------------------------------------
/*!@file    SensorsTask.h

 */
// ------------------------------------------------------------------------------------------------


#ifndef SENSORS_TASK_H_
#define SENSORS_TASK_H_

#include "main.h"
#include "GraphicsTask.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define SWIPE_START_REQUIREMENT 90
#define SWIPE_DISTANCE 25
#define LONG_TAP_TIME_MS 240
#define DISMISS_TIME_MS 720
#define EXTRA_SWIPE_DISMISS_TIME_MS 360
#define CALIBRATION_TIMEOUT_MS 650
#define CALIBRATION_REQUIRED_MS 5000
#define TOUCH_SENSOR_FILE "touchCalibration"
#define TOUCH_SENSOR_SWIPE_FILE "swipeCalibration"
#define TOUCH_SENSOR_SWIPE_AREA_FILE "swipeAreaCalibration"

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

typedef enum
{
    TOUCH_TEST_DISABLED             = 0,
    TOUCH_TEST_SHORT                = 1,
    TOUCH_TEST_LONG                 = 2,
    TOUCH_TEST_LEFT                 = 3,
    TOUCH_TEST_RIGHT                = 4,
    TOUCH_TEST_COMPLETE             = 5,
} eTouchTest_t;

typedef enum
{
    touch_idle             = 0,
    touch_debounce         = 1,
    touch_settled          = 2,
} TouchInteractionState_e;

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================

typedef struct {
    TouchInteractionState_e interactionState;
    int16_t initialPositionState;
    int16_t firstPosition;
    int16_t currentPosition;
    uint8_t positionCount;
    TickType_t downTick;
    TickType_t upTick;
    uint8_t code;
    uint8_t codeLength;
    bool released;
    bool nowLong;
    bool swiped;
} touchState_t;

// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
void init_SensorsTask();
void unlockTaps_SensorsTask(const char * const taskName);
void lockTaps_SensorsTask(void (*function)(uint8_t code, uint8_t len, bool adjusted),const char * const taskName);
void subscribeToTaps_SensorsTask(void (*function)(uint8_t code, uint8_t len, bool adjusted));
void unsubscribeToTaps_SensorsTask(void (*function)(uint8_t code, uint8_t len, bool adjusted));
void subscribeToRelease_SensorsTask(void (*function)(bool isReleased));
void unsubscribeToRelease_SensorsTask(void (*function)(bool isReleased));
void unlockSwipes_SensorsTask(const char * const taskName);
void lockSwipes_SensorsTask(void (*function)(touch_swipe_state_t swipeState),const char * const taskName);
void subscribeToSwipes_SensorsTask(void (*function)(touch_swipe_state_t swipeState));
void unsubscribeToSwipes_SensorsTask(void (*function)(touch_swipe_state_t swipeState));
void subscribeToAccInterrupt_SensorsTask(void (*function)());
void unsubscribeToAccInterrupt_SensorsTask(void (*function)());
void subscribeToAccData_SensorsTask(void (*function)(float * x, float * y, float * z));
void unsubscribeToAccData_SensorsTask(void (*function)(float * x, float * y, float * z));
void subscribeToLightSensorMagnitude_SensorsTask(void (*function)(uint16_t * brightnessMagnitude));
void unsubscribeToLightSensorMagnitude_SensorsTask(void (*function)(uint16_t * brightnessMagnitude));
void touchSensorResetThreshold_SensorTask();
void getAccData_SensorsTask(float * x, float * y, float * z, bool * response);
void getMagData_SensorsTask(float * x, float * y, float * z, bool * response);
void getTempData_SensorsTask(float * temp, bool * response);
void getPhotoData_SensorsTask(uint16_t * pCh1, uint16_t * pCh2, bool * response);
void sendSwipe_SensorsTask(bool isRight);
void sendTaps_SensorsTask(uint8_t code, uint8_t length);
void sendRelease_SensorsTask(bool isReleased);
void send_touch_test_update_SensorsTask();
bool testECompass_SensorsTask();
bool testLight_SensorsTask();

#endif  /* SENSORS_TASK_H_ */

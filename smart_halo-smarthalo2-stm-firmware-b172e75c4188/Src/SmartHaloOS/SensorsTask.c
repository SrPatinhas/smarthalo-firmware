// ------------------------------------------------------------------------------------------------
/*!@file    SensorsTask.c
 */
// ------------------------------------------------------------------------------------------------
#include <CommunicationTask.h>
#include <ECompassDriver.h>
#include <GraphicsTask.h>
#include <PhotoSensor.h>
#include <SystemUtilitiesTask.h>
#include "SensorsTask.h"
#include "SubscriptionHelper.h"
#include "main.h"
#include "iwdg.h"
#include "tsc.h"
#include "touchsensing.h"
#include "tsl_user.h"
#include "WatchdogTask.h"
#include "SHTaskUtils.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define QUEUE_LENGTH 4
#define STACK_SIZE configMINIMAL_STACK_SIZE + 300
#define TASK_PRIORITY   PRIORITY_NORMAL

#define POLLING_MS 100
#define MOVEMENT_TIMEOUT 15*1000/POLLING_MS
#define SUBSCRIPTION_LIMIT 5
#define LIGHTSENSOR_REFRESH_MS 250

#define TOUCH_CALIBRATION 39

// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE ENUM DEFINITION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================
#define _ENABLE_SENSORS_VDD() HAL_GPIO_WritePin(SENSORS_EN_GPIO_Port, SENSORS_EN_Pin, GPIO_PIN_SET)
#define _DISABLE_SENSORS_VDD() HAL_GPIO_WritePin(SENSORS_EN_GPIO_Port, SENSORS_EN_Pin, GPIO_PIN_RESET)
#define _READ_AX_SENSOR_INT1() HAL_GPIO_ReadPin(INT_1_XL_GPIO_Port, INT_1_XL_Pin)
#define _SET_AX_INT1_DOWN() HAL_GPIO_WritePin(INT_1_XL_GPIO_Port, INT_1_XL_Pin, GPIO_PIN_RESET)

// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================
static void SensorTask(void *pvParameters);

static TaskHandle_t selfHandle = NULL;
static StaticTask_t xTaskBuffer;
static StackType_t SensorsStack[STACK_SIZE];

static bool isAuthenticated = false;
static bool interrupted = false;
static bool notifying = true;

static char lockingTapsName[30] = {0};
static char lockingSwipesName[30] = {0};
void (*tapLockingSubscription)(uint8_t code, uint8_t len, bool adjusted);
void (*swipeLockingSubscription)(touch_swipe_state_t swipeState);
//Setting up to 5 subscribers for now
void (*tapSubscriptions[SUBSCRIPTION_LIMIT])(uint8_t code, uint8_t len, bool adjusted);
uint8_t tapSubscribers = 0;
void (*releaseSubscriptions[SUBSCRIPTION_LIMIT])(bool isReleased);
uint8_t releaseSubscribers = 0;
void (*swipeSubscriptions[SUBSCRIPTION_LIMIT])(touch_swipe_state_t swipeState);
uint8_t swipeSubscribers = 0;
void (*accInterruptSubscriptions[SUBSCRIPTION_LIMIT])();
uint8_t accInterruptSubscribers = 0;
void (*accRawDataStreamSubscriptions[SUBSCRIPTION_LIMIT])(float * pX, float * pY, float * pZ);
uint8_t accRawDataStreamSubscribers = 0;
void (*lightSensorMagnitudeSubscriptions[SUBSCRIPTION_LIMIT])(uint16_t * averageBrightness);
uint8_t lightSensorMagnitudeSubscribers = 0;

uint8_t rawDataCount = 0;

extern TSL_LinRotData_T MyLinRots_Data[TSLPRM_TOTAL_ALL_LINROTS];
touchState_t touchState;
touch_swipe_state_t swipeState;
int16_t positionDelta;
bool touchComplete = false;
bool tapStarted = false;
uint8_t touchTestState = TOUCH_TEST_DISABLED;
uint16_t touchTestTime = 0;
TickType_t lastCalibrationStart = 0;
bool touchCalibrating = false;

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/**
 * @brief Set up sensors task
 */
void init_SensorsTask() {
    if (selfHandle == NULL) {
        selfHandle = xTaskCreateStatic(SensorTask, TASKNAME_SENSORS,
                STACK_SIZE, NULL, TASK_PRIORITY, SensorsStack, &xTaskBuffer);
        configASSERT(selfHandle);
    }
}

/**
 * @brief Unlock tap subscriptions to all subscribers
 */
void unlockTaps_SensorsTask(const char * const taskName){
    if(lockingTapsName[0]==0 || !strcmp(lockingTapsName,taskName)){
        memset(lockingTapsName,0,sizeof(lockingTapsName));
        tapLockingSubscription = NULL;
    }
}

/**
 * @brief Lock tap subscriptions to only one subscriber
 */
void lockTaps_SensorsTask(void (*function)(uint8_t code, uint8_t len, bool adjusted),const char * const taskName){
    if(lockingTapsName[0]==0 || !strcmp(lockingTapsName,taskName)){
        tapLockingSubscription = function;
        strcpy(lockingTapsName, taskName);
    }
}

/**
 * @brief Subscribe to touch taps
 */

void subscribeToTaps_SensorsTask(void (*function)(uint8_t code, uint8_t len, bool adjusted)) {
    addSubscription((void*) tapSubscriptions,
            &tapSubscribers,
            SUBSCRIPTION_LIMIT,
            "Tap",
            function);
}

/**
 * @brief Unsubscribe to touch taps
 */
void unsubscribeToTaps_SensorsTask(void (*function)(uint8_t code, uint8_t len, bool adjusted)) {
    removeSubscription((void*) tapSubscriptions,
            &tapSubscribers,
            "Tap",
            function);
}

/**
 * @brief Subscribe to touch taps
 */
void subscribeToRelease_SensorsTask(void (*function)(bool isReleased)) {
    addSubscription((void*) releaseSubscriptions,
            &releaseSubscribers,
            SUBSCRIPTION_LIMIT,
            "Release",
            function);
}

/**
 * @brief Unsubscribe to touch taps
 */
void unsubscribeToRelease_SensorsTask(void (*function)(bool isReleased)) {
    removeSubscription((void*) releaseSubscriptions,
            &releaseSubscribers,
            "Release",
            function);
}

/**
 * @brief Unlock swipe subscriptions to all subscribers
 */
void unlockSwipes_SensorsTask(const char * const taskName){
    if(lockingSwipesName[0]==0 || !strcmp(lockingSwipesName,taskName)){
        memset(lockingSwipesName,0,sizeof(lockingSwipesName));
        swipeLockingSubscription = NULL;
    }
}

/**
 * @brief Lock swipe subscriptions to only one subscriber
 */
void lockSwipes_SensorsTask(void (*function)(touch_swipe_state_t swipeState),const char * const taskName){
    if(lockingSwipesName[0]==0 || !strcmp(lockingSwipesName,taskName)){
        swipeLockingSubscription = function;
        strcpy(lockingSwipesName, taskName);
    }
}

/**
 * @brief Subscribe to touch swipe
 */
void subscribeToSwipes_SensorsTask(void (*function)(touch_swipe_state_t swipeState)) {
    addSubscription((void*) swipeSubscriptions,
            &swipeSubscribers,
            SUBSCRIPTION_LIMIT,
            "Swipe",
            function);
}

/**
 * @brief Unsubscribe to touch swipe
 */
void unsubscribeToSwipes_SensorsTask(void (*function)(touch_swipe_state_t swipeState)) {
    removeSubscription((void*) swipeSubscriptions,
            &swipeSubscribers,
            "Swipe",
            function);
}

/**
 * @brief Subscribe to accelerometer interrupts
 */
void subscribeToAccInterrupt_SensorsTask(void (*function)()) {
    addSubscription((void*) accInterruptSubscriptions,
            &accInterruptSubscribers,
            SUBSCRIPTION_LIMIT,
            "Acc Interrupt",
            function);
}

/**
 * @brief Unsubscribe to accelerometer interrupts
 */
void unsubscribeToAccInterrupt_SensorsTask(void (*function)()) {
    removeSubscription((void*) accInterruptSubscriptions,
            &accInterruptSubscribers,
            "Acc Interrupt",
            function);
}

/**
 * @brief Subscribe to accelerometer raw data
 */
void subscribeToAccData_SensorsTask(void (*function)(float *pX, float *pY, float *pZ)) {
    addSubscription((void*) accRawDataStreamSubscriptions,
            &accRawDataStreamSubscribers,
            SUBSCRIPTION_LIMIT,
            "Acc Raw Data",
            function);
}

/**
 * @brief Unsubscribe to accelerometer raw data
 */
void unsubscribeToAccData_SensorsTask(void (*function)(float *pX, float *pY, float *pZ)) {
    removeSubscription((void*) accRawDataStreamSubscriptions,
            &accRawDataStreamSubscribers,
            "Acc Raw Data",
            function);
}

void touchSensorResetThreshold_SensorTask(){
    tsl_user_ConfigureMaxThreshold(TOUCH_CALIBRATION);
}

static void touchSensorRecalibrate_SensorTask(bool release){
    if(release){
        tsl_user_ConfigureMaxThreshold(255);
        lastCalibrationStart = xTaskGetTickCount();
    }else{
        touchSensorResetThreshold_SensorTask();
    }
    touchCalibrating = release;
}

/**
 * @brief Subscribe to light sensor magnitude value
 */
void subscribeToLightSensorMagnitude_SensorsTask(void (*function)(uint16_t *brightnessMagnitude)) {
    addSubscription((void*) lightSensorMagnitudeSubscriptions,
            &lightSensorMagnitudeSubscribers,
            SUBSCRIPTION_LIMIT,
            "Light Sensor Magnitude",
            function);
}

/**
 * @brief Unsubscribe to light sensor magnitude value
 */
void unsubscribeToLightSensorMagnitude_SensorsTask(void (*function)(uint16_t *brightnessMagnitude)) {
    removeSubscription((void*) lightSensorMagnitudeSubscriptions,
            &lightSensorMagnitudeSubscribers,
            "Light Sensor Magnitude",
            function);
}

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================
static void sendMovementNotify(bool movement) {
    uint8_t ble_msg[2];

    ble_msg[0] = BLE_NOTIFY_MOVEMENT;
    ble_msg[1] = movement;

    sendData_CommunicationTask(BLE_TYPE_MSG,
            BLE_TX_COMMAND_BLE_NOTIFY,
            2,
            ble_msg);
}

static void updateTapCode(uint8_t code, uint8_t length, bool adjusted) {
    if(tapLockingSubscription == NULL){
        for (int i = 0; i < tapSubscribers; i++) {
            tapSubscriptions[i](code, length, adjusted);
        }
    }else{
        tapLockingSubscription(code,length,adjusted);
    }
}

static void updateReleaseState(bool isReleased){
    for (int i = 0; i < releaseSubscribers; i++){
        releaseSubscriptions[i](isReleased);
    }
}

static void updateSwipeState(touch_swipe_state_t state) {
    if(swipeLockingSubscription == NULL){
        for (int i = 0; i < swipeSubscribers; i++) {
            swipeSubscriptions[i](state);
        }
    }else{
        swipeLockingSubscription(state);
    }
}

static void sendTapCode(uint8_t code, uint8_t length) {
    if(tapLockingSubscription != NULL) return;

    uint8_t msgLength = 3;
    uint8_t ble_msg[msgLength];
    ble_msg[0] = BLE_NOTIFY_TOUCH;
    ble_msg[1] = code;
    ble_msg[2] = length;

    sendData_CommunicationTask(BLE_TYPE_MSG,
            BLE_TX_COMMAND_BLE_NOTIFY,
            msgLength,
            ble_msg);
}

/**
 * @brief send test swipe
 */
void sendSwipe_SensorsTask(bool isRight){
    touch_swipe_state_t swipeState;
    swipeState.direction = isRight ? oled_right : oled_left;
    updateSwipeState(swipeState);
}

/**
 * @brief send test taps
 */
void sendTaps_SensorsTask(uint8_t code, uint8_t length){
    updateTapCode(code, length, false);
    sendTapCode(code, length);
}

/**
 * @brief send test release
 */
void sendRelease_SensorsTask(bool isReleased){
    updateReleaseState(isReleased);
}

/**
 * @brief Update the OLED with next stage of touch test
 */
void send_touch_test_update_SensorsTask() {
    oled_cmd_t oledCMD;
    oledCMD.statusBarPosition = -1;
    oledCMD.direction =
            touchTestState < TOUCH_TEST_LEFT ? oled_no_animation :
                    touchTestState == TOUCH_TEST_LEFT ? oled_left : oled_right;
    oledCMD.animation = oledCMD.direction;
    oledCMD.animationTime = 300;
    oledCMD.animationTypeMask = slidingAnimation;
    oledCMD.u8ImageCategory = oled_tests;
    oledCMD.u8ImageType = touchTestState;
    touchTestTime = touchTestState ? touchTestTime : 0;
    touchTestState =
            touchTestState < TOUCH_TEST_COMPLETE - 1 ?
                    touchTestState + 1 : TOUCH_TEST_DISABLED;
    setImage_GraphicsTask(&oledCMD);
}

static void dismiss_tap_code() {
    sendTapCode(touchState.code, touchState.codeLength);
    if (touchTestState != TOUCH_TEST_DISABLED) {
        if (touchState.code == 0
                && touchState.codeLength == 1
                && touchTestState == TOUCH_TEST_SHORT) {
            send_touch_test_update_SensorsTask();
        } else if (touchState.code == 1
                && touchState.codeLength == 1
                && touchTestState == TOUCH_TEST_LONG) {
            send_touch_test_update_SensorsTask();
        }
    }
    touchState.code = 0;
    touchState.codeLength = 0;
    updateTapCode(touchState.code, touchState.codeLength, touchState.swiped);
}

static void analyzeRelease() {
    touchComplete = false;
    touchState.upTick = xTaskGetTickCount();
    touchState.released = true;
    updateReleaseState(touchState.released);
    touchState.firstPosition = -1;
    touchState.nowLong = false;
    tapStarted = false;
    touchState.positionCount = 0;
}

static void setupNewTap() {
    touchState.initialPositionState = MyLinRots_Data[0].RawPosition;
    if (touchState.released) {
        touchState.downTick = xTaskGetTickCount();
    }
    touchState.released = false;
    updateReleaseState(touchState.released);
}

static void performSwipe() {
    if (touchTestState != TOUCH_TEST_DISABLED) {
        if (swipeState.direction == oled_left
                && touchTestState == TOUCH_TEST_LEFT) {
            send_touch_test_update_SensorsTask();
        } else if (swipeState.direction == oled_right
                && touchTestState == TOUCH_TEST_RIGHT) {
            send_touch_test_update_SensorsTask();
        }
    }
    updateSwipeState(swipeState);
    touchComplete = true;
    if(tapStarted){
        touchState.swiped = true;
        touchState.codeLength--;
        touchState.code = touchState.code & (0xff>>(8-touchState.codeLength));
    }
    if(touchState.codeLength)
        dismiss_tap_code();
    touchState.swiped = false;
}

static void analyzeTouch() {
    TickType_t now = xTaskGetTickCount();
    if (touchState.initialPositionState != MyLinRots_Data[0].RawPosition) {
        //Settle tap over three checks
        if(touchState.interactionState != touch_settled)
            touchState.interactionState++;
        if(touchState.firstPosition == -1){
           touchState.firstPosition = MyLinRots_Data[0].RawPosition;
           touchState.interactionState = touch_idle;
        }
        //Touch is considered settled and in the middle or at least once tap happened already
        if((touchState.interactionState == touch_settled || touchState.codeLength)
                && ((touchState.firstPosition > SWIPE_START_REQUIREMENT && touchState.firstPosition < 255 - SWIPE_START_REQUIREMENT)
                        || !isAuthenticated)
                && !touchComplete){
            if (!tapStarted) {
                touchState.codeLength++;
                updateTapCode(touchState.code, touchState.codeLength, touchState.swiped);
                tapStarted = true;
            }
            if (!touchState.nowLong && (now - touchState.downTick) > LONG_TAP_TIME_MS
                    && !swipeState.secondary) {
                touchState.code = touchState.code
                        | (1 << (touchState.codeLength - 1));
                touchState.nowLong = true;
                updateTapCode(touchState.code, touchState.codeLength, touchState.swiped);
            }
            //Tap is registered don't check for swipes anymore
            if(touchState.interactionState != touch_settled)
                touchState.interactionState = touch_settled;
        }
        touchState.currentPosition = MyLinRots_Data[0].RawPosition;
        touchState.initialPositionState = -1;
    }

    if((now - touchState.downTick) > CALIBRATION_REQUIRED_MS){
        touchSensorRecalibrate_SensorTask(true);
    }
}

static void analyzeIdleState() {
    if (touchTestState != TOUCH_TEST_DISABLED) {
        touchTestTime++;
        if (touchTestTime > 3000) {
            bool bResult;
            oledOFF_GraphicsTask(&bResult);
            if (bResult)
                send_touch_test_update_SensorsTask();
        } else if (touchState.codeLength != 0
                && (xTaskGetTickCount() - touchState.upTick) > DISMISS_TIME_MS) {
            dismiss_tap_code();
        }
    } else if (touchState.codeLength) {
        if ((xTaskGetTickCount() - touchState.upTick) > DISMISS_TIME_MS) {
            dismiss_tap_code();
        }
    }
}

static int16_t getMovementDistance(touchState_t state) {
    int16_t positionChange = 0;
    if (state.firstPosition != -1) {
        positionChange = state.currentPosition - state.firstPosition;
    }
    return positionChange;
}

static void authenticationStateUpdate(bool authenticationState){
    isAuthenticated = authenticationState;
}

static void SensorTask(void *pvParameters) {
    bool bResponse = false;

    waitForFS_SystemUtilities("SensorsTask");

    //  TickType_t xLastDisplayTime;
    _ENABLE_SENSORS_VDD();
    vTaskDelay(10);
    _SET_AX_INT1_DOWN();
    // Init the eCompassModule
    init_ECompassDriver();
    // Init the light sensor module.
    init_PhotoSensor(LTR329ALS01_GAIN_1X, &bResponse);

    tsl_user_ConfigureMaxThreshold(TOUCH_CALIBRATION);

    touchState.codeLength = 0;
    touchState.code = 0;
    touchState.downTick = xTaskGetTickCount();
    touchState.upTick = xTaskGetTickCount();
    touchState.released = true;
    touchState.initialPositionState = -1;
    touchState.firstPosition = -1;

    // Delay to allow setup to finish
    vTaskDelay(50);

    static uint16_t ticks_since_interrupt = 0;

    TickType_t lastSensorTime, lastCalibrationTime, lastLightSensorTime;
    lastSensorTime = lastCalibrationTime = lastCalibrationStart = lastLightSensorTime = xTaskGetTickCount();

    subscribeToConnectionState_CommunicationTask(authenticationStateUpdate);

    while (1) {
        tsl_user_status_t status = TSL_USER_STATUS_BUSY;
        TickType_t now = xTaskGetTickCount();
        for (int i = lastCalibrationTime; i <= now; i++) {
            TSL_tim_ProcessIT();
        }
        status         = tsl_user_Exec();
        now = xTaskGetTickCount();
        if (status != TSL_USER_STATUS_BUSY) {
            if(MyLinRots_Data[0].StateId == TSL_STATEID_RELEASE){
                if((now - lastCalibrationStart) > CALIBRATION_TIMEOUT_MS
                        && touchCalibrating)
                    touchSensorRecalibrate_SensorTask(false);
            }
            if (MyLinRots_Data[0].Change) {
                switch (MyLinRots_Data[0].StateId) {
                    case TSL_STATEID_RELEASE:
                    case TSL_STATEID_PROX: {
                        if (touchState.released) {
                            break;
                        }
                        analyzeRelease();
                        break;
                    }
                    case TSL_STATEID_DETECT: {
                        setupNewTap();
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }else if(!touchComplete){
                if (MyLinRots_Data[0].StateId == TSL_STATEID_DETECT
                        && !touchState.released && !touchComplete) {
                    analyzeTouch();
                    //Checking for swiping
                    positionDelta = getMovementDistance(touchState);
                    if (isAuthenticated && abs(positionDelta) > SWIPE_DISTANCE){
                        //Check if the swipe is left to right or right to left.
                        //Left to left and right to right should be ignored
                        //Middle locations are acceptable if touch has not settled
                        if((touchState.firstPosition < SWIPE_START_REQUIREMENT
                                || (touchState.interactionState == touch_debounce
                                        && touchState.firstPosition > SWIPE_START_REQUIREMENT
                                        && touchState.firstPosition < 255-SWIPE_START_REQUIREMENT))
                                && touchState.currentPosition > touchState.firstPosition)
                            swipeState.direction = oled_right;
                        else if((touchState.firstPosition > 255-SWIPE_START_REQUIREMENT
                                || (touchState.interactionState == touch_debounce
                                        && touchState.firstPosition > SWIPE_START_REQUIREMENT
                                        && touchState.firstPosition < 255-SWIPE_START_REQUIREMENT))
                                && touchState.currentPosition < touchState.firstPosition
                                && touchState.currentPosition != 0) //Zero indicates it's wrapped to the other side
                            swipeState.direction = oled_left;
                        else
                            swipeState.direction = oled_no_animation;
                        if(swipeState.direction != oled_no_animation)
                            performSwipe();
                    }
                }else if((MyLinRots_Data[0].StateId == TSL_STATEID_PROX
                        || MyLinRots_Data[0].StateId == TSL_STATEID_RELEASE)){
                    analyzeIdleState();
                }
                if(MyLinRots_Data[0].StateId >= TSL_STATEID_PROX && MyLinRots_Data[0].StateId <= TSL_STATEID_DETECT){
                    if((now - touchState.downTick) > CALIBRATION_REQUIRED_MS)
                        touchSensorRecalibrate_SensorTask(true);
                }
            }
        }

        if(lightSensorMagnitudeSubscribers && ((now - lastLightSensorTime) > LIGHTSENSOR_REFRESH_MS)){
            uint16_t brightnessMagnitude, channel1Brightness, channel2Brightness;
            bool unusedResult;
            lastLightSensorTime = xTaskGetTickCount();
            if(readData_PhotoSensor(&channel1Brightness,&channel2Brightness,&unusedResult)){
                brightnessMagnitude = ((channel1Brightness^2) + (channel2Brightness^2))^(1/2);
                for (int i = 0; i < lightSensorMagnitudeSubscribers; i++){
                    lightSensorMagnitudeSubscriptions[i](&brightnessMagnitude);
                }
            }
        }

        if (_READ_AX_SENSOR_INT1()) {
            if (!interrupted) {
                sendMovementNotify(true);
            }
            interrupted = true;
            notifying = true;
            for (int i = 0; i < accInterruptSubscribers; i++) {
                accInterruptSubscriptions[i]();
            }
        }

        if ((now - lastSensorTime) > POLLING_MS) {
            lastSensorTime = xTaskGetTickCount();
            if (notifying) {
                ticks_since_interrupt++;
                if ((ticks_since_interrupt % (MOVEMENT_TIMEOUT) == 0)) {
                    ticks_since_interrupt = 0;

                    sendMovementNotify(interrupted);
                    if (!interrupted) {
                        notifying = false;
                    }
                    interrupted = false;
                }
            }

            if (accRawDataStreamSubscribers) {
                rawDataCount++;
                if (rawDataCount % 2) {
                    float x, y, z;
                    bool response;
                    getAccData_SensorsTask(&x, &y, &z, &response);
                    if (response) {
                        for (int i = 0; i < accRawDataStreamSubscribers; i++) {
                            accRawDataStreamSubscriptions[i](&x, &y, &z);
                        }
                    }
                }
            }
        }

        lastCalibrationTime = xTaskGetTickCount();
        vTaskDelay(10);
        shwd_KeepAlive(eSHWD_SensorsTask);
    }
}

/**
 * @brief OS function to get accelerometer data
 */
void getAccData_SensorsTask(float * x, float * y, float * z, bool * response){
    getXLData_ECompassDriver(x, y, z, response);
}

/**
 * @brief OS function to get magnetic data
 */
void getMagData_SensorsTask(float * x, float * y, float * z, bool * response){
    getMAGData_ECompassDriver(x, y, z, response);
}

/**
 * @brief OS function to get temperature data
 */
void getTempData_SensorsTask(float * temp, bool * response){
    getTemperatureData_ECompassDriver(temp, response);
}

/**
 * @brief OS function to get photo data
 */
void getPhotoData_SensorsTask(uint16_t * ch1, uint16_t * ch2, bool * response){
    readData_PhotoSensor(ch1, ch2, response);
}

/**
 * @brief Communication test with the e compass
 */
bool testECompass_SensorsTask(){
    float x,y,z;
    bool newData;
    bool response = getXLData_ECompassDriver(&x, &y, &z, &newData);
    if(newData)
        log_Shell("New Data on the accelerometer!");

    return response;
}

/**
 * @brief Communication test with the photo sensor
 */
bool testLight_SensorsTask(){
    uint16_t ch1, ch2;
    bool response;
    readData_PhotoSensor(&ch1, &ch2, &response);

    return response;
}

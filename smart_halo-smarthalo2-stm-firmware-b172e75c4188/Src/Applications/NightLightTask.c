/*
 * NightLightTask.c
 *
 *  Created on: 11 Oct 2019
 *      Author: Matt
 */

#include <CommunicationTask.h>
#include <SystemUtilitiesTask.h>
#include "NightLightTask.h"
#include <GraphicsTask.h>
#include <AnimationsLibrary.h>
#include "SensorsTask.h"
#include "OLEDLibrary.h"
#include "assets.h"
#include "WatchdogTask.h"
#include "SHTaskUtils.h"

#include "math.h"

#define TASK_PRIORITY   PRIORITY_NORMAL
#define STACK_SIZE      configMINIMAL_STACK_SIZE + 0x300

#define MOVEMENT_THRESHOLD 30
#define MAX_TAP_LENGTH 8

#define NIGHT_LIGHT_OLED_PRIORITY 2
#define NIGHT_LIGHT_OLED_HIGH_PRIORITY 3
#define NIGHT_LIGHT_OLED_DURATION 760
#define BASE_STRING_Y_POSITION 55

#define MAX_INTERRUPT_COUNT 50
#define IDLE_INTERRUPT_COUNT 15

/* Static task --------------- */
static TaskHandle_t selfHandle = NULL;
static StaticTask_t xTaskBuffer;
static StackType_t NightLightStack[STACK_SIZE];

/* Queue with jobs ------------ */
static struct FrontLightAnimationData settings = {.isBlinking = false, .percentage = 100};
static bool movementRequired = true;

static bool wasOnDuringNewConnection = false;
static bool isOn = false;
static bool isShowing = false;

static bool externalRequired = false;
static bool tapUpdate = false;
static uint8_t tapCode = 0;
static uint8_t tapLength = 0;
static char finishedCode[MAX_TAP_LENGTH];
static uint8_t finishedCodeLength = 0;
static bool isConnected = false;
static TickType_t xLastShowingTime;
static const char *blinkPermissionFile = "BlinkPermission";
static bool isBlinkingLocked = true;

static uint8_t interruptCount = 0;
static TickType_t lastInterruptTick = 0;

static struct OLEDAsset borderTopAsset = {.width=120,.height=3,.asset=borderTop};
static struct OLEDAsset borderBottomAsset = {.width=120,.height=3,.asset=borderBottom};
static struct OLEDAsset borderSideAsset = {.width=1,.height=55,.asset=borderSide};
static struct OLEDLayout borderLayout = {.location[0]={.x=5,.y=2},.location[1]={.x=5,.y=58},.location[2]={.x=4,.y=5},.location[3]={.x=125,.y=5}};
static struct OLEDAsset onAsset = {.width = 116, .height = 31, .asset = lightOn};
static struct OLEDLayout stateLayout = {.location[0] = {.x = 7,.y = 6}};
static struct OLEDAsset offAsset = {.width = 116, .height = 31, .asset = lightOff};
static struct OLEDAsset persistentoffAsset = {.width = 116, .height = 31, .asset = lightPersistent};
static struct OLEDAsset blinkAsset = {.width = 116, .height = 31, .asset = lightBlink};

const uint8_t light_state[4][30] = {"Light","Phare","Licht","Luz"};
const uint8_t light_mode[4][30] = {"Light mode","Mode du phare","Lichtmodus","Modo de luz"};

static uint8_t oledImg[1024] = {0};
static uint8_t secondImg[1024] = {0};

static void prvNightLightTask(void *pvParameters);

/**
 * @brief Initialize the Night Light task
 */
void init_NightLightTask() {
    if (selfHandle == NULL) {
        selfHandle = xTaskCreateStatic(prvNightLightTask, TASKNAME_NIGHTLIGHT, STACK_SIZE, NULL, TASK_PRIORITY, NightLightStack, &xTaskBuffer);
        configASSERT(selfHandle);
    }
}

static void updateExternalRequired(uint16_t length, uint8_t * payload){
    externalRequired = payload[0];
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void updateIsOn(bool onState){
    isOn = onState;
    setLightSetting_SystemUtilities(onState);
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

static void showLightStateOled(bool isOn){
    oled_cmd_t oledCMD = {
            .statusBarPosition = -1,
            .direction = isOn ? oled_right : oled_left,
            .animation = true,
            .u8ImageCategory = oled_reactions,
            .u8ImageType = 0,
            .animationTypeMask = rockerAnimation,
    };
    Localization_E locale = getLocale_SystemUtilities();
    memset(oledImg,0,sizeof(oledImg));
    addBorder(oledImg);
    appendAssetToDisplay_OLEDLibrary(oledImg,
            isOn ? offAsset : onAsset,
                    stateLayout.location[0]);
    alignPixellari16TextCentered_OLEDLibrary(oledImg, light_state[locale],BASE_STRING_Y_POSITION);
    memset(secondImg,0,sizeof(secondImg));
    addBorder(secondImg);
    appendAssetToDisplay_OLEDLibrary(secondImg,
            isOn ? onAsset : offAsset,
                    stateLayout.location[0]);
    alignPixellari16TextCentered_OLEDLibrary(secondImg, light_state[locale],BASE_STRING_Y_POSITION);
    setPriorityAnimatedImageWithCmd_GraphicsTask(oledImg,
            secondImg,
            &oledCMD,
            NIGHT_LIGHT_OLED_PRIORITY,
            NIGHT_LIGHT_OLED_DURATION*4,
            NIGHT_LIGHT_OLED_DURATION/2,
            NIGHT_LIGHT_OLED_DURATION/2,
            isOn ? oled_right : oled_left);
}

static void showLightModeOled(bool isBlinking){
    oled_cmd_t oledCMD = {
            .statusBarPosition = -1,
            .direction = isBlinking ? oled_right : oled_left,
            .animation = true,
            .u8ImageCategory = oled_reactions,
            .u8ImageType = 0,
            .animationTypeMask = rockerAnimation,
    };
    Localization_E locale = getLocale_SystemUtilities();
    memset(oledImg,0,sizeof(oledImg));
    addBorder(oledImg);
    appendAssetToDisplay_OLEDLibrary(oledImg,
            isBlinking ? persistentoffAsset : blinkAsset,
                    stateLayout.location[0]);
    alignPixellari16TextCentered_OLEDLibrary(oledImg, light_mode[locale] ,BASE_STRING_Y_POSITION);
    memset(secondImg,0,sizeof(secondImg));
    addBorder(secondImg);
    appendAssetToDisplay_OLEDLibrary(secondImg,
            isBlinking ? blinkAsset : persistentoffAsset,
                    stateLayout.location[0]);
    alignPixellari16TextCentered_OLEDLibrary(secondImg, light_mode[locale],BASE_STRING_Y_POSITION);
    setPriorityAnimatedImageWithCmd_GraphicsTask(oledImg,
            secondImg,
            &oledCMD,
            NIGHT_LIGHT_OLED_PRIORITY,
            NIGHT_LIGHT_OLED_DURATION*4,
            NIGHT_LIGHT_OLED_DURATION/2,
            NIGHT_LIGHT_OLED_DURATION/2,
            isBlinking ? oled_right : oled_left);
}

static void updateLightShowing(bool showingState){
    showLightStateOled(showingState);
    isShowing = showingState;
    setLightState_SystemUtilities(showingState);
}

static void updateMovementState(bool movementState){
    movementRequired = movementState;
}

static void updateLightSettings(uint16_t length, const void * payload){
    const struct NightLightSettingsPayload * newSettings = payload;
    if(length == 3){
        isBlinkingLocked = newSettings->isBlinkingLocked;
        writeFile_SystemUtilities(blinkPermissionFile, &isBlinkingLocked, 1);
    }
    bool newBlinkingState = isBlinkingLocked ? 0 : newSettings->nightLightMode;
    if(newBlinkingState != settings.isBlinking && (length != 4 || !newSettings->silenceModeRocker))
        showLightModeOled(newBlinkingState);
    settings.isBlinking = newBlinkingState;
    //Lock the minimum at 10% because it becomes basically off at a lower point
    settings.percentage = newSettings->nightLightPercentage < 10 ? 10 : newSettings->nightLightPercentage;
    nightLightStateUpdate_GraphicsTask(settings);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void notifyNightLight(bool isTouch){
    struct NightLightNotifyPayload notifyConditions;
    notifyConditions.type = BLE_NOTIFY_NIGHTLIGHT;
    notifyConditions.setting = isOn;
    notifyConditions.state = isShowing;
    notifyConditions.touch = isTouch;

    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_NOTIFY, sizeof(struct NightLightNotifyPayload), &notifyConditions);
}

static void updateNightLightState(uint16_t length, const void * payload){
    if(wasOnDuringNewConnection){
        wasOnDuringNewConnection = false;
    }else{
        const struct NightLightStatePayload * state = payload;
        updateMovementState(state->isMovementRequired);
        updateIsOn(state->isOn);
        if(isOn && !isShowing && !movementRequired){
            xLastShowingTime = xTaskGetTickCount();
            interruptCount = MAX_INTERRUPT_COUNT;
            startAnimation_GraphicsTask(HaloAnimation_display_nightLight, (uint8_t *) &settings);
            updateLightShowing(true);

        }else if(!isOn && isShowing){
            struct FrontLightAnimationData offData = {
                    .isBlinking = settings.isBlinking,
                    .percentage = 0,
            };
            nightLightStateUpdate_GraphicsTask(offData);
            updateLightShowing(false);
        }
    }
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
    notifyNightLight(false);
}

static void connectionStateUpdate(bool connectionState){
    if(!connectionState){
        if(isOn){
            updateMovementState(false);
            updateIsOn(false);
            if(isShowing)
                updateLightShowing(false);
            struct FrontLightAnimationData newData = {
                    .isBlinking = settings.isBlinking,
                    .percentage = 0,
            };
            nightLightStateUpdate_GraphicsTask(newData);
            setPriorityImageTime_GraphicsTask(NIGHT_LIGHT_OLED_PRIORITY,0);
        }
        externalRequired = false;
    }else if(isOn){
        wasOnDuringNewConnection = true;
    }
    isConnected = connectionState;
}

static void finishCode(uint8_t code, uint8_t len){
    for(uint8_t i=0; i<len && i<MAX_TAP_LENGTH; i++){
        finishedCode[i] = code >> i & 0x1 ? 'L' : 'S';
    }
    finishedCodeLength = len;
}

static void accInterrupt(){
    lastInterruptTick = xTaskGetTickCount();
    if(interruptCount < MAX_INTERRUPT_COUNT)
        interruptCount++;
}

static void tapData(uint8_t code, uint8_t len, bool adjusted){
    if(len < tapLength){
        if(adjusted)
            finishCode(code, len);
        else
            finishCode(tapCode, tapLength);
    }
    tapUpdate = true;
    tapCode = code;
    tapLength = len;
}

static void prvNightLightTask(void *pvParameters) {
    subscribeToAccInterrupt_SensorsTask(accInterrupt);
    subscribeToTaps_SensorsTask(tapData);
    subscribeToConnectionState_CommunicationTask(connectionStateUpdate);
    assignFunction_CommunicationTask(COM_UI, UI_FRONTLIGHT, updateNightLightState);
    assignFunction_CommunicationTask(COM_UI, UI_FRONTLIGHT_SETTINGS, updateLightSettings);
    assignFunction_CommunicationTask(COM_UI, UI_FRONT_LIGHT_TOGGLE_MODE, updateExternalRequired);

    bool existingBlinkingLock;
    if(readFile_SystemUtilities(blinkPermissionFile, &existingBlinkingLock, 1)){
        isBlinkingLocked = existingBlinkingLock;
    }

    xLastShowingTime = xTaskGetTickCount();
    while (1) {
        TickType_t now = xTaskGetTickCount();
        if(finishedCodeLength && !getFrontLightLocked_GraphicsTask() && getStandbyReason_CommunicationTask() == StandbyOff){
            if(!strcmp(finishedCode,"SS") && !externalRequired){
                if(isOn){
                    struct FrontLightAnimationData offData = {
                            .isBlinking = settings.isBlinking,
                            .percentage = 0,
                    };
                    nightLightStateUpdate_GraphicsTask(offData);
                    updateLightShowing(false);
                }else{
                    startAnimation_GraphicsTask(HaloAnimation_display_nightLight, (uint8_t *) &settings);
                    xLastShowingTime = now;
                    updateLightShowing(true);
                    interruptCount = MAX_INTERRUPT_COUNT;
                }
                updateIsOn(!isOn);
                //This delay gives the touch time to respond before sending the light state
                vTaskDelay(100);
                notifyNightLight(true);
            }else if(!strcmp(finishedCode,"L") && !isConnected){
                settings.isBlinking = isBlinkingLocked ? 0 : !settings.isBlinking;
                struct FrontLightAnimationData newData = {
                        .isBlinking = settings.isBlinking,
                        .percentage = isShowing ? settings.percentage : 0,
                };
                nightLightStateUpdate_GraphicsTask(newData);
                showLightModeOled(settings.isBlinking);
            }
            finishedCodeLength = 0;
            memset(finishedCode,0,MAX_TAP_LENGTH);
        }

        if(now - lastInterruptTick > 3000){
            if(interruptCount < IDLE_INTERRUPT_COUNT){
                interruptCount = 0;
            }else
                interruptCount -= 5;
            lastInterruptTick = now;
        }

        if(!getFrontLightLocked_GraphicsTask()){
            if(interruptCount < IDLE_INTERRUPT_COUNT && isOn && isShowing){
                struct FrontLightAnimationData offData = {
                        .isBlinking = settings.isBlinking,
                        .percentage = 0,
                };
                nightLightStateUpdate_GraphicsTask(offData);
                updateLightShowing(false);
                updateMovementState(true);
                notifyNightLight(false);
            }else if(interruptCount >= IDLE_INTERRUPT_COUNT && isOn && !isShowing){
                startAnimation_GraphicsTask(HaloAnimation_display_nightLight, (uint8_t *) &settings);
                updateLightShowing(true);
                updateMovementState(false);
                notifyNightLight(true);
            }
        }else{
            isShowing = false;
            interruptCount = 0;
        }
        vTaskDelay(100);
        shwd_KeepAlive(eSHWD_NightLightTask);
    }
}

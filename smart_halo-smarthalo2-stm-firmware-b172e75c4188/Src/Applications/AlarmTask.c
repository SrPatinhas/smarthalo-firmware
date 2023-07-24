/*
 * AlarmTask.c
 *
 *  Created on: 28 Oct 2019
 *      Author: Matt
 */

#include <CommunicationTask.h>
#include <SystemUtilitiesTask.h>
#include "AlarmTask.h"
#include "GraphicsTask.h"
#include "SensorsTask.h"
#include "SoundTask.h"
#include "BootLoaderImport.h"
#include "OLEDLibrary.h"
#include "assets.h"
#include "Power.h"
#include "WatchdogTask.h"
#include "SHTaskUtils.h"

#include "math.h"

#define TASK_PRIORITY   PRIORITY_NORMAL
#define STACK_SIZE      configMINIMAL_STACK_SIZE + 0x300

#define MOVEMENT_THRESHOLD 0.05f
#define MAX_TAP_LENGTH 8
#define VIGILANT_PROGRESS_INCREMENT_MODIFIER 6
#define VIGILANT_PROGRESS_DECAY_MODIFIER 6
#define LENIENT_PROGRESS_INCREMENT 6
#define LENIENT_PROGRESS_DECAY 12
#define MILLISECONDS_PER_ALARM_DECAY 3000
#define MILLISECONDS_TO_STREAM_ACC 10000
#define ALARM_OLED_ENGAGE_DURATION 1000
#define ALARM_OLED_PANIC_DURATION 15000
#define ALARM_OLED_PROGRESS_DURATION 4000
#define ADDITIONAL_MILLISECONDS_TO_STOP_ALARM 12000
#define ALARM_OLED_PRIORITY 4
#define ALARM_OLED_HIGH_PRIORITY 5
#define HPF_ALPHA 0.5f

#define INSTRUCTION_LINE_1_Y 32
#define INSTRUCTION_LINE_2_Y 48

#define BASE_STRING_Y_POSITION 55

/* Static task --------------- */
static TaskHandle_t selfHandle = NULL;
static StaticTask_t xTaskBuffer;
static StackType_t AlarmStack[STACK_SIZE];

struct AlarmConfig{
    uint32_t seed;
    struct AlarmSettingsPayload settings;
};


/* Queue with jobs ------------ */
static struct AlarmArmStatePayload state = {0, 0};
static struct AlarmConfig config = {0, {0, 0, 1, 1, 0}};

static char alarmCode[MAX_TAP_LENGTH];
static const char * alarmFile = "AlarmConfig";
static const char * armedFile = "ArmedMode";

static uint8_t triggerCount = 0;
static TickType_t accInterruptTick = 0;
static float previousX, previousY, previousZ, X, Y, Z, hpfX, hpfY, hpfZ;
static bool accStreamStarted = false;


static bool tapUpdate = false;
static uint8_t tapCode = 0;
static uint8_t tapLength = 0;
static char finishedCode[MAX_TAP_LENGTH];
static uint8_t finishedCodeLength = 0;
static bool isConnected = false;
static TickType_t xLastMovementTime;
static int8_t alarmProgress = 0;
static bool alarmTriggered = false;
static bool isReadyToArm = false;
static bool alarmDontSleep = false;

static uint8_t armedHand[] =
{ 0x80, 0x01, 0x00, 0xC0, 0x03, 0x00, 0xC0, 0x0F, 0x00, 0x80, 0x1F, 0x00, 0x00, 0x3F, 0x00, 0x00, 0xFF, 0x00, 0xFE, 0xFF, 0x01, 0xFF, 0xFF, 0x03, 0xFF, 0xFF, 0x07, 0xFE, 0xFF, 0x0F, 0xC0, 0xFF, 0x0F, 0x80, 0xFF, 0x0F, 0x80, 0xFF, 0x0F, 0x80, 0xFF, 0x0F, 0x00, 0xFF, 0x07, 0x00, 0xFF, 0x07, 0x00, 0xFE, 0x03, 0x00, 0xFC, 0x00 };

static struct OLEDAsset borderTopAsset = {.width=120,.height=3,.asset=borderTop};
static struct OLEDAsset borderBottomAsset = {.width=120,.height=3,.asset=borderBottom};
static struct OLEDAsset borderSideAsset = {.width=1,.height=55,.asset=borderSide};
static struct OLEDLayout borderLayout = {.location[0]={.x=5,.y=2},.location[1]={.x=5,.y=58},.location[2]={.x=4,.y=5},.location[3]={.x=125,.y=5}};
static struct OLEDAsset engageAsset = {.width = 116, .height = 31, .asset = engage};
static struct OLEDAsset disengageAsset = {.width = 116, .height = 31, .asset = disengage};
static struct OLEDLayout alarmStateLayout = {.location[0] = {.x = 7,.y = 6}};
static struct OLEDAsset eyes1Asset = {.width=78,.height=19,.asset=eyes1};
static struct OLEDAsset mouth1Asset = {.width=62,.height=12,.asset=mouth1};
static struct OLEDLayout warning1Layout = {.location[0]={.x=25,.y=6},.location[1]={.x=33,.y=39}};
static struct OLEDAsset eyes2Asset = {.width=76,.height=22,.asset=eyes2};
static struct OLEDAsset mouth2Asset = {.width=62,.height=16,.asset=mouth2};
static struct OLEDLayout warning2Layout = {.location[0]={.x=26,.y=3},.location[1]={.x=33,.y=37}};
static struct OLEDAsset eyes3Asset = {.width=76,.height=24,.asset=eyes3};
static struct OLEDAsset mouth3Asset = {.width=60,.height=27,.asset=mouth3};
static struct OLEDLayout panicLayout = {.location[0]={.x=26,.y=1},.location[1]={.x=34,.y=32}};
static struct OLEDAsset armedAsset = {.width=18,.height=20,.asset=armedHand};
static struct OLEDLayout armedLayout = {.location[0]={.x=108,.y=42}};

static bool armedMode = false;
const uint8_t alarm_state[4][30] = {"Manual Alarm"," Alarme Manuelle","Manueller Alarm","Alarma Manual"};
const uint8_t alarm_explain1[4][30] = {"Tapcode"," Tapcode","Tippe ouf Code",{'T','o','q','u','e',' ','C',243,'d','i','g','o'}};
const uint8_t alarm_explain2[4][30] = {"to disengage",{'p','o','u','r',' ','d',233,'s','e','n','g','a','g','e','r'},{'s','i','c','h',' ','l',246,'s','e','n'},"desengancharse"};

static uint8_t drawingBoard[1024] = {0xFF};
static uint8_t secondImg[1024] = {0};

typedef enum {
    AlarmOLEDLocked,
    AlarmOLEDUnlocked,
    AlarmOLEDWarning1,
    AlarmOLEDWarning2,
    AlarmOLEDPanic,
    AlarmOLEDClear
} AlarmOLEDStates;
static void prvAlarmTask(void *pvParameters);

/**
 * @brief Initialize the Night Light task
 */
void init_AlarmTask()
{
    if (selfHandle == NULL) {
        selfHandle =
            xTaskCreateStatic(prvAlarmTask, TASKNAME_ALARM, STACK_SIZE, NULL,
                              TASK_PRIORITY, AlarmStack, &xTaskBuffer);
        configASSERT(selfHandle);
    }
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

/**
 * @brief       updateOLEDReactionState()
 * @details     Function to set up a reaction state on the OLED.
 * @private
 */
static void updateOLEDReactionState(uint8_t reaction, uint8_t state){
    oled_cmd_t oledCMD = {
            .statusBarPosition = -1,
            .direction = oled_no_animation,
            .animation = false,
            .u8ImageCategory = oled_reactions,
            .u8ImageType = reaction,
    };
    uint16_t duration = ALARM_OLED_PROGRESS_DURATION;
    Localization_E locale = getLocale_SystemUtilities();
    switch(reaction){
        case AlarmOLEDLocked:
        case AlarmOLEDUnlocked:
        	oledCMD.animation = true;
        	oledCMD.direction = (reaction == AlarmOLEDLocked ? oled_right : oled_left);
        	oledCMD.animationTypeMask = rockerAnimation;
            duration = ALARM_OLED_ENGAGE_DURATION;
            memset(drawingBoard,0,sizeof(drawingBoard));
            addBorder(drawingBoard);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    reaction == AlarmOLEDUnlocked ? engageAsset : disengageAsset,
                    alarmStateLayout.location[0]);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, alarm_state[locale],BASE_STRING_Y_POSITION);
            setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, ALARM_OLED_HIGH_PRIORITY, duration/2);
            memset(secondImg,0,sizeof(secondImg));
            addBorder(secondImg);
            appendAssetToDisplay_OLEDLibrary(secondImg,
                    reaction == AlarmOLEDUnlocked ? disengageAsset : engageAsset,
                    alarmStateLayout.location[0]);
            alignPixellari16TextCentered_OLEDLibrary(secondImg, alarm_state[locale],BASE_STRING_Y_POSITION);
            setPriorityAnimatedImageWithCmd_GraphicsTask(drawingBoard,
                    secondImg,
                    &oledCMD,
                    ALARM_OLED_HIGH_PRIORITY,
                    ALARM_OLED_ENGAGE_DURATION*2,
                    ALARM_OLED_ENGAGE_DURATION/2,
                    ALARM_OLED_ENGAGE_DURATION/2,
                    reaction == AlarmOLEDLocked ? oled_right : oled_left);
            if(isConnected){
            	//Were connected means it's entering armed mode and will warn that it needs to be exited manually
            	memset(drawingBoard,0,sizeof(drawingBoard));
            	alignPixellari16TextCentered_OLEDLibrary(drawingBoard, alarm_explain1[locale],INSTRUCTION_LINE_1_Y);
            	alignPixellari16TextCentered_OLEDLibrary(drawingBoard, alarm_explain2[locale],INSTRUCTION_LINE_2_Y);
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, ALARM_OLED_PRIORITY, ALARM_OLED_ENGAGE_DURATION*4);
            }
            return;
        default:
        case AlarmOLEDClear:
            memset(drawingBoard,0,sizeof(drawingBoard));
            break;
        case AlarmOLEDWarning1:
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    eyes1Asset,
                    warning1Layout.location[0]);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    mouth1Asset,
                    warning1Layout.location[1]);
            break;
        case AlarmOLEDWarning2:
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    eyes2Asset,
                    warning2Layout.location[0]);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    mouth2Asset,
                    warning2Layout.location[1]);
            break;
        case AlarmOLEDPanic:
            duration = ALARM_OLED_PANIC_DURATION;
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    eyes3Asset,
                    panicLayout.location[0]);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    mouth3Asset,
                    panicLayout.location[1]);
            break;
    }
    if(armedMode){
    	switch(reaction){
    		case AlarmOLEDWarning1:
    		case AlarmOLEDWarning2:
    		case AlarmOLEDPanic:
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        armedAsset,
                        armedLayout.location[0]);
                break;
    		default:
    			break;
    	}
    }

    setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, ALARM_OLED_PRIORITY, duration);
}

static void alarmDemo(uint16_t length, const void * payload){
    if(length < 1){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct AlarmDemoPayload * demoPayload = payload;
    uint8_t localAlarmProgress;
    switch(demoPayload->state){
        case AlarmDemoOff:
            updateOLEDReactionState(AlarmOLEDUnlocked,0);
            alarmProgress = 0;
            localAlarmProgress = alarmProgress;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
            soundStop_SoundTask();
            break;
        case AlarmDemoArm:{
            updateOLEDReactionState(AlarmOLEDLocked,0);
            uint8_t isDemoArmed = 1;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_state, &isDemoArmed);
            alarmProgress = 0;
            uint8_t localAlarmProgress = alarmProgress;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
            break;
        }
        case AlarmDemoDisarm:{
            updateOLEDReactionState(AlarmOLEDUnlocked,0);
            uint8_t isDemoArmed = 0;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_state, &isDemoArmed);
            alarmProgress = 0;
            uint8_t localAlarmProgress = alarmProgress;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
            break;
        }
        case AlarmDemoWarn:
            if(length < 2){
                genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
                return;
            }
            alarmProgress = demoPayload->progress;
            localAlarmProgress = alarmProgress;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
            break;
        case AlarmDemoSound:
            alarmProgress = 100;
            localAlarmProgress = alarmProgress;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
            break;
        default:
            genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
            return;
    }
    if(alarmProgress == 100){
        updateOLEDReactionState(AlarmOLEDPanic,0);
        if(!alarmTriggered){
            triggerCount++;
            alarmTriggered = true;
        }
        uint8_t freqCount = 3;
        uint16_t frequencies[3] = {2800,3600,0};
        uint16_t durations[3] = {100,100,0};
        bool sweeps[3] = {true, true, false};
        uint8_t repeats = 75;
        uint8_t volume = 100;
        compileSound_SoundTask(frequencies, durations, sweeps, volume, freqCount, repeats);
    }else if(alarmProgress > 25){
        updateOLEDReactionState(alarmProgress > 25 ? AlarmOLEDWarning2 : AlarmOLEDWarning1,0);
        uint8_t freqCount = 3;
        uint16_t frequencies[3] = {90,75,0};
        uint16_t durations[3] = {500,200,0};
        bool sweeps[3] = {true, false, false};
        uint8_t repeats = 0;
        uint8_t volume = alarmProgress;
        compileSound_SoundTask(frequencies, durations, sweeps, volume, freqCount, repeats);
    }

    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void compileCode(char * newCode, uint8_t code, uint8_t length){
    for(uint8_t i=0; i<length && i<MAX_TAP_LENGTH; i++){
        newCode[i] = code >> i & 0x1 ? 'L' : 'S';
    }
}

static void getAlarmSeed(){
    uint8_t payload[5] = {eCOM_RETURN_STATUS_OK, 1,2,3,4};
    responseWithPayload_CommunicationTask(5, payload);
}

static void updateIsArmed(bool armedState){
    isReadyToArm = armedState;
    alarmProgress = 0;
    state.isArmed = false;
    BF->armedBoot = false;
    setFrontLightLocked_GraphicsTask(!isConnected && state.isArmed);
    uint8_t curState = armedState ? 1 : 0;
    updateOLEDReactionState(armedState ? AlarmOLEDLocked : AlarmOLEDUnlocked,0);
    startAnimation_GraphicsTask(HaloAnimation_display_alarm_state, &curState);
    uint8_t localAlarmProgress = alarmProgress;
    startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
}

static void updateAlarmConfig(uint16_t length, const void * payload){
    const struct AlarmConfig * newConfig = payload;
    bool isArmedAllowed = config.settings.allowStandby;
    memcpy(&config, newConfig, sizeof(struct AlarmConfig));
    //In case the payload isn't large enough preserve previous setting
    if(length < 9)
        config.settings.allowStandby = isArmedAllowed;
    config.settings.severity = config.settings.severity ? 1 : 0;
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);

    compileCode(alarmCode, config.settings.code, config.settings.length);
    writeFile_SystemUtilities(alarmFile, &config.settings, sizeof(struct AlarmSettingsPayload));
    state.isArmed = config.settings.mode;
    setFrontLightLocked_GraphicsTask(!isConnected && state.isArmed);
}

static void updateAlarmState(uint16_t length, const void * payload){
    const struct AlarmArmStatePayload * newState = payload;
    memcpy(&state, newState, sizeof(struct AlarmArmStatePayload));
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void sendReport(uint16_t length, const void * payload){
    uint8_t report[5] = {eCOM_RETURN_STATUS_OK, state.isArmed, triggerCount, config.settings.severity, config.settings.mode};
    responseWithPayload_CommunicationTask(5, report);
    triggerCount = 0;
}

static void clearAlarm(){
    setFrontLightLocked_GraphicsTask(false);
    BF->armedBoot = false;
    alarmTriggered = false;
    alarmProgress = 0;
    if (alarmDontSleep) {
        SLEEP_ALLOWED();
        alarmDontSleep = false;
    }
    uint8_t localAlarmProgress = alarmProgress;
    startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
    soundStop_SoundTask();
    setPriorityImageTime_GraphicsTask(ALARM_OLED_PRIORITY,0);
}

static void connectionStateUpdate(bool connectionsState){
    if(connectionsState){
        clearAlarm();
    }else if(state.isArmed && (getStandbyReason_CommunicationTask() != StandbySleep)){
        isReadyToArm = true;
        state.isArmed = false;
        setFrontLightLocked_GraphicsTask(!isConnected && state.isArmed);
        xLastMovementTime = xTaskGetTickCount();
    }else if(getStandbyReason_CommunicationTask() == StandbySleep){
        isReadyToArm = false;
        state.isArmed = false;
    }
    isConnected = connectionsState;
}

static void pairedStateUpdate(bool paired){
    if(!paired){
        memset(&config.settings, 0, sizeof(struct AlarmSettingsPayload));
        writeFile_SystemUtilities(alarmFile, &config.settings, sizeof(struct AlarmSettingsPayload));
        BF->armedBoot = false;
        state.isArmed = false;
        setFrontLightLocked_GraphicsTask(!isConnected && state.isArmed);
        compileCode(alarmCode, config.settings.code, config.settings.length);
    }
}

static void finishCode(uint8_t code, uint8_t len){
    compileCode(finishedCode, code, len);
    finishedCodeLength = len;
}

static void accRawData(float * pX, float * pY, float * pZ){
    if(accStreamStarted){
        previousX = X;
        previousY = Y;
        previousZ = Z;
        //Move to micro g's
        X = (*pX)/-1000;
        Y = (*pY)/-1000;
        Z = (*pZ)/-1000;
    }
}

static void triggerAlarm(){
    updateOLEDReactionState(AlarmOLEDPanic,0);
    if(!alarmTriggered){
        triggerCount++;
        alarmTriggered = true;
    }
    uint8_t freqCount = 3;
    uint16_t frequencies[3] = {2800,3600,0};
    uint16_t durations[3] = {100,100,0};
    bool sweeps[3] = {true, true, false};
    uint8_t repeats = 75;
    uint8_t volume = 100;
    compileSound_SoundTask(frequencies, durations, sweeps, volume, freqCount, repeats);

}

static void checkAccelerometerData(){
    hpfX = HPF_ALPHA*(hpfX + X - previousX);
    hpfY = HPF_ALPHA*(hpfY + Y - previousY);
    hpfZ = HPF_ALPHA*(hpfZ + Z - previousZ);
    float vector = sqrt(hpfX*hpfX + hpfY*hpfY + hpfZ*hpfZ);
    if(vector > MOVEMENT_THRESHOLD && (xTaskGetTickCount() - xLastMovementTime  > 1000)){
        xLastMovementTime = xTaskGetTickCount();
        if(!isConnected && state.isArmed){
            if (!alarmDontSleep) {
                SLEEP_NOTALLOWED();
                alarmDontSleep = true;
            }
            alarmProgress += LENIENT_PROGRESS_INCREMENT + VIGILANT_PROGRESS_INCREMENT_MODIFIER*config.settings.severity;
            alarmProgress = alarmProgress > 100 ? 100 : alarmProgress;
            uint8_t localAlarmProgress = alarmProgress;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
        }
        if(alarmProgress == 100){
            triggerAlarm();
        }else if(alarmProgress > 25){
            updateOLEDReactionState(AlarmOLEDWarning2,0);
            uint8_t freqCount = 3;
            uint16_t frequencies[3] = {90,75,0};
            uint16_t durations[3] = {500,200,0};
            bool sweeps[3] = {true, false, false};
            uint8_t repeats = 0;
            uint8_t volume = alarmProgress;
            compileSound_SoundTask(frequencies, durations, sweeps, volume, freqCount, repeats);
        }else if(alarmProgress > 0){
            updateOLEDReactionState(AlarmOLEDWarning1,0);
        }
    }
}

static void accInterrupt(){
    if(isConnected) return;
    if(!accStreamStarted)
        subscribeToAccData_SensorsTask(accRawData);
    accInterruptTick = xTaskGetTickCount();
    accStreamStarted = true;
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

/**
 * @brief       Conditions for an instant alarm on boot
 * @details     This function encodes the policy for the immediate triggering
 *              of the alarm after boot.
 * 
 * @return      Returns true when the alarm should be triggered.
 */
static bool shouldAlarmOnBoot(void)
{
    if (getResetType_SystemUtilities() == eResetType_Normal &&
        getRebootReason_SystemUtilities() != eRebootDisableWatchdog) {
        return true;
    }
    return false;
}

static void prvAlarmTask(void *pvParameters) {
    subscribeToAccInterrupt_SensorsTask(accInterrupt);
    subscribeToTaps_SensorsTask(tapData);
    subscribeToConnectionState_CommunicationTask(connectionStateUpdate);
    subscribeToPairedState_CommunicationTask(pairedStateUpdate);
    assignFunction_CommunicationTask(COM_ALARM, ALARM_GETSEED, getAlarmSeed);
    assignFunction_CommunicationTask(COM_ALARM, ALARM_ARM, updateAlarmState);
    assignFunction_CommunicationTask(COM_ALARM, ALARM_SETCONFIG, updateAlarmConfig);
    assignFunction_CommunicationTask(COM_ALARM, ALARM_REPORT, sendReport);
    assignFunction_CommunicationTask(COM_UI, UI_DEMO, alarmDemo);

    struct AlarmSettingsPayload existingSettings;
    if(readFile_SystemUtilities(alarmFile, &existingSettings, sizeof(struct AlarmSettingsPayload))){
        memcpy(&config.settings, &existingSettings, 4);
        compileCode(alarmCode, config.settings.code, config.settings.length);
    }

    bool savedArmedMode;
    if(readFile_SystemUtilities(armedFile, &savedArmedMode, 1))
        armedMode = savedArmedMode;

    if(BF->armedBoot){
        if(shouldAlarmOnBoot()){
            alarmProgress = 100;
            uint8_t localAlarmProgress = alarmProgress;
            startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
            SLEEP_NOTALLOWED();
            alarmDontSleep = true;
            triggerAlarm();
        }else if(armedMode){
            BF->armedBoot = true;
        }else{
            isReadyToArm = false;
        }
        state.isArmed = true;
        setFrontLightLocked_GraphicsTask(!isConnected && state.isArmed);
        xLastMovementTime = xTaskGetTickCount();
    }

    while (1) {
        TickType_t now = xTaskGetTickCount();
        if(isReadyToArm && !state.isArmed){
            if(!getIsPlugged_SystemUtilities()){
                isReadyToArm = false;
                state.isArmed = true;
                BF->armedBoot = true;
                setFrontLightLocked_GraphicsTask(!isConnected && state.isArmed);
            }else{
                xLastMovementTime = xTaskGetTickCount();
            }
        }

        if(finishedCodeLength){
            if(config.settings.length == finishedCodeLength
                    && !strcmp(finishedCode, alarmCode)
                    && !getIsPlugged_SystemUtilities()
                    && ((isConnected && config.settings.allowStandby) || !isConnected)){
                eStandbyReason_t currentReason = getStandbyReason_CommunicationTask();
                if(currentReason != StandbySleep){
                    bool isEngaged = false;
                    if(isConnected) //always arming while connected
                    	isEngaged = true;
                    else if(!isReadyToArm) //toggle armed state
                    	isEngaged = !state.isArmed;

                    updateIsArmed(isEngaged);
                    if(!isEngaged){
                        clearAlarm();
                    }
                    xLastMovementTime = xTaskGetTickCount();
                }
                if(isConnected && currentReason == StandbyOff){
                    toggleStandby_CommunicationTask(StandbyArmed);
                    armedMode = true;
                    writeFile_SystemUtilities(armedFile, &armedMode, 1);
                }else if(armedMode && currentReason == StandbyArmed){
                    toggleStandby_CommunicationTask(StandbyOff);
                    armedMode = false;
                    writeFile_SystemUtilities(armedFile, &armedMode, 1);
                }
            }
            finishedCodeLength = 0;
            memset(finishedCode,0,MAX_TAP_LENGTH);
        }

        if(!isConnected && now - xLastMovementTime > MILLISECONDS_PER_ALARM_DECAY + (alarmProgress/100) * ADDITIONAL_MILLISECONDS_TO_STOP_ALARM){
            if(state.isArmed && alarmProgress){
                alarmProgress -= LENIENT_PROGRESS_DECAY - VIGILANT_PROGRESS_DECAY_MODIFIER*config.settings.severity;
                xLastMovementTime = xTaskGetTickCount();
                if(alarmProgress <= 0){
                    alarmProgress = 0;
                    alarmTriggered = false;
                    uint8_t localAlarmProgress = alarmProgress;
                    startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &localAlarmProgress);
                    if (alarmDontSleep) {
                        SLEEP_ALLOWED();
                        alarmDontSleep = false;
                    }
                }else if(alarmTriggered && alarmProgress < 100){
                    uint8_t stopAlarmProgress = 0;
                    startAnimation_GraphicsTask(HaloAnimation_display_alarm_progress, &stopAlarmProgress);
                }
            }
        }

        if(accStreamStarted && now - accInterruptTick > MILLISECONDS_TO_STREAM_ACC){
            accStreamStarted = false;
            unsubscribeToAccData_SensorsTask(accRawData);
        }

        if(accStreamStarted)
            checkAccelerometerData();

        //When the battery is really low, disable armed boot to avoid alarm depleting the battery during charge.
        //Don't need to disarm the alarm since that will happen when it runs out battery anyway.
        if(BF->armedBoot && getStateOfCharge_SystemUtilities() <= 10){
            BF->armedBoot = false;
        }

        vTaskDelay(isConnected ? 500 : 100);
#ifndef GOLDEN
        shwd_KeepAlive(eSHWD_AlarmTask);
#endif
    }
}

/*!@file    GraphicsTask.c
 */

#include <AnimationsLibrary.h>
#include <CommunicationTask.h>
#include <GraphicsTask.h>
#include <SensorsTask.h>
#include <SystemUtilitiesTask.h>
#include "OLEDLibrary.h"
#include "event_groups.h"
#include "OLEDDriver.h"
#include "HaloLedsDriver.h"
#include "FrontLEDDriver.h"
#include "FSUtil.h"
#include "SensorsTask.h"
#include "SHTimers.h"
#include "tsl_user.h"
#include "Power.h"
#include "WatchdogTask.h"
#include "BoardRev.h"
#include "SHTaskUtils.h"

#define STACK_SIZE      configMINIMAL_STACK_SIZE + 200
#define TASK_PRIORITY   PRIORITY_ABOVE_NORMAL

#define GRAPHIC_EVENT_NEW_IMAGE     0x00000001
#define GRAPHIC_EVENT_ENABLE        0x00000002
#define GRAPHIC_EVENT_DISABLE       0x00000004
#define GRAPHIC_EVENT_DISPLAYOFF    0x00000008
#define GRAPHIC_EVENT_CONTRAST      0x00000010
#define GRAPHIC_EVENT_BRIGHTNESS    0x00000020
#define GRAPHIC_EVENT_ANIM_TIMER    0x00000040

#define OLED_PRIORITY_LEVELS  10
#define OLED_BLE_OVERRIDE_PRIORITY 9

#define MAX_DAYLIGHT_BRIGHTNESS  30 // value above which it's so bright outside that the haloleds should just be at max brightness
#define FADE_TIMER_TIMEOUT_MS 10 //smooth to the eye
#define BRIGHTNESS_FADE_INCREMENT 2
#define BRIGHTNESS_FADE_DECREMENT 1

struct DisplayDataType {
    uint8_t          display[1024];
    bool             enabled;
    TickType_t       endTick;
    int8_t           position;
    bool             isAnimated;
    uint8_t          secondDisplay[1024];
    TickType_t       animationStartTick;
    TickType_t       animationEndTick;
    oledDirections_e animationDirection;
    uint8_t animationTypeMask;
};

typedef struct {
    // task data
    TaskHandle_t selfHandle;
    StaticTask_t taskBuffer;
    StackType_t  stack[STACK_SIZE];

    // event group data
    EventGroupHandle_t eventGroup;
    StaticEventGroup_t eventGroupBuffer;

    uint8_t oOLEDBuffer[SIZEOF_BUFFER_OLED];

    bool            isOLEDAnimation;
    uint8_t 		animationTypeMask;
    TickType_t      oledAnimationEndTick;
    TickType_t      oledAnimationStartTick;
    uint8_t *       secondStateImage;
    oledDirections_e direction;
    uint8_t *        newImageAdd;
    uint8_t *        oldImageptr;
    uint8_t *        imagePtr;

    uint8_t oledContrast;
    uint8_t oledBrightness;

    oled_cmd_t oLedCommand;
} oGraphicTask_t;

static oGraphicTask_t oGraphicTask;

struct GraphicsClocks {
    uint16_t intro;
    uint16_t disconnect;
    uint16_t battery;
    uint16_t pointer;
    uint16_t speedometer;
    uint16_t averageSpeed;
    uint16_t fractional;
    uint16_t fire;
    uint16_t nightLightIntro;
    uint16_t nightLight;
    uint16_t nightLightOutro;
    uint16_t alarmSetting;
    uint16_t alarmState;
    uint16_t shadowTaps;
    uint16_t taps;
    uint16_t swipe;
    uint16_t clock;
    uint16_t clockLowPriority;
    uint16_t progress;
    uint16_t circle;
    uint16_t angleIntro;
    uint16_t angle;
    uint16_t spiral;
    uint16_t animation;
};

typedef struct {
    TaskHandle_t  taskHandle;
    StaticTask_t  taskBuffer;
    StackType_t   stack[STACK_SIZE];
    TimerHandle_t taskTimer;
    StaticTimer_t taskTimerBuffer;
} haloState_t;

static haloState_t HaloState;

static TimerHandle_t fadeTimerRx;
static StaticTimer_t fadeTimerBufferRx;

static uint8_t targetBrightness  = MAX_LED_BRIGHTNESS;

static struct GraphicsClocks clocks;

uint8_t leds[72];

uint8_t     frontLEDPercentage = 0;
static bool frontLEDEnabled    = false;

static char lockingName[30] = {0};
static struct StateOfChargeAnimationData socData;
static struct PointerAnimationData pointerData;
static struct SpeedometerAnimationData speedometerData;
static struct AverageSpeedAnimationData averageSpeedData;
static struct FractionalData fractionalData;
static bool fractionalDataOff = false;
static HsvColour_t fireColour;
static bool fireOff = false;
static struct FrontLightAnimationData frontLightData;
static bool frontLightLocked;
static bool alarmEngaged = false;
static uint8_t alarmProgress = 0;
static uint8_t tapCode = 0;
static uint8_t tapLength = 0;
static uint8_t tapShadowCode = 0;
static uint8_t tapShadowLength = 0;
static bool isSwipeRight;
static struct ClockAnimationData clockData;
static struct ProgressAnimationData progressData;
static struct CustomCircleAnimationData circleData;
static struct AnglesAnimationData anglesData;
static struct SpiralAnimationData spiralData;
static uint8_t spiralCount = 0;
static uint8_t ledsTestStage = 0;
static uint8_t ledBrightness = 255;
static uint8_t ledsTestOffLED = 255;
static bool isAutoBrightness = true; 

static bool connectionState = false;
// buffer to store images read from the filesystem
static uint8_t imgBuf[SIZEOF_BUFFER_OLED];
static uint8_t oledBuf[SIZEOF_BUFFER_OLED];
static uint8_t previousOledBuf[SIZEOF_BUFFER_OLED];
static uint8_t secondaryBuf[SIZEOF_BUFFER_OLED];
static struct DisplayDataType displayQueue[OLED_PRIORITY_LEVELS];

static void GraphicsTask(void *pvParameters);
static void HaloAnimTimerCB(TimerHandle_t xTimer);
static void setConnectionState(bool isConnected);
static void updateHaloAnimations();
static void HaloTask(void *params);
static void brightnessMagnitudeUpdate(uint16_t * brightnessMagnitude);
static void prvTimerCallbackFadeTimeout(TimerHandle_t xTimer);

/**
 * @brief Initialize all display tasks and timer
 * @details Set up the a task and timer for Halo animations and a separate task
 *          for graphics/OLED
 */
void init_GraphicsTask(void)
{
    if (HaloState.taskHandle == NULL) {
        HaloState.taskHandle = xTaskCreateStatic(
                HaloTask, TASKNAME_HALO, STACK_SIZE, NULL,
                PRIORITY_BELOW_NORMAL, HaloState.stack, &HaloState.taskBuffer);
    }
    
    if (fadeTimerRx == NULL) {
        fadeTimerRx = xTimerCreateStatic("LedFadeTimeoutTimer", FADE_TIMER_TIMEOUT_MS / portTICK_PERIOD_MS,
        pdTRUE, (void *) 0, prvTimerCallbackFadeTimeout, &fadeTimerBufferRx);
        storeTimer(fadeTimerRx);
        configASSERT(fadeTimerRx);
    }

    if (HaloState.taskTimer == NULL) {
        HaloState.taskTimer =
                xTimerCreateStatic("HaloAnimTimer", 10 / portTICK_PERIOD_MS, pdTRUE, NULL,
                        HaloAnimTimerCB, &HaloState.taskTimerBuffer);
        storeTimer(HaloState.taskTimer);
    }

    oGraphicTask.eventGroup =
            xEventGroupCreateStatic(&oGraphicTask.eventGroupBuffer);
    configASSERT(oGraphicTask.eventGroup);

    if (oGraphicTask.selfHandle == NULL) {
        oGraphicTask.selfHandle = xTaskCreateStatic(
                GraphicsTask, TASKNAME_OLED, STACK_SIZE, NULL, TASK_PRIORITY,
                oGraphicTask.stack, &oGraphicTask.taskBuffer);
        configASSERT(oGraphicTask.selfHandle);
    }

    subscribeToConnectionState_CommunicationTask(setConnectionState);
    subscribeToLightSensorMagnitude_SensorsTask(brightnessMagnitudeUpdate);
}

uint32_t graphicTaskCBGiveCount;
uint32_t graphicTaskCBTakeCount;
uint32_t graphicTaskCBOvertimeCount;
uint32_t graphicTaskCBTotalTime;
uint32_t graphicTaskCBTotalOvertime;
uint32_t graphicTaskCBMinTime;
uint32_t graphicTaskCBMaxTime;
uint32_t graphicTaskCBFirstTick;    // tick of first CB
uint32_t graphicTaskCBFirstClock;   // clocks.animation of first CB
uint32_t graphicTaskCBAnimClock;    // independent tracking of clocks.animation
uint32_t graphicTaskBacklog;        // number of timer CBs we are behind

/**
 * @brief timer callback function
 * @details The controlling timer is set up to call this function every 10ms. 
 *          Since work that this function needs may block and require more than
 *          10ms to run, simply signal the HaloTask. The FreeRTOS notification
 *          API will keep count.
 * 
 * @param timer handle to the timer that fired
 */
static void HaloAnimTimerCB(TimerHandle_t timer)
{
    graphicTaskCBGiveCount++;
    xTaskNotifyGive(HaloState.taskHandle);
}

void HaloTask(void *params)
{
    uint32_t overCount = 0;

    for (;;) {
        graphicTaskBacklog = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        graphicTaskCBTakeCount++;

        uint32_t now, tm;
        uint32_t lastAClock = clocks.animation;
        now = xTaskGetTickCount();
        if (graphicTaskCBFirstClock == 0 && clocks.animation != 0) {
            graphicTaskCBFirstTick = now;
            graphicTaskCBFirstClock = clocks.animation;
        }

        updateHaloAnimations();

        tm = xTaskGetTickCount() - now;
        if (lastAClock == 0xffff && clocks.animation == 0) {
            graphicTaskCBAnimClock++;
        } else {
            graphicTaskCBAnimClock += clocks.animation - lastAClock;
        }
        if (graphicTaskCBMinTime == 0) {
            graphicTaskCBMinTime = tm;
            graphicTaskCBMaxTime = tm;
        }
        if (tm < graphicTaskCBMinTime) {
            graphicTaskCBMinTime = tm;
        } else if (tm > graphicTaskCBMaxTime) {
            graphicTaskCBMaxTime = tm;
        }
        graphicTaskCBTotalTime += tm;
        if (tm > 10) {
            graphicTaskCBTotalOvertime += tm;
            graphicTaskCBOvertimeCount++;
        }
        if (graphicTaskBacklog > 10) {
            if (overCount % 100 == 0)
                log_Shell("%s: we may be falling behind by: %ld", __func__, graphicTaskBacklog);
            overCount++;
        }
        shwd_KeepAlive(eSHWD_HaloTask);
    }
}

/**
 * @brief Enable the OLED
 * @details Turn on the OLED display (complete power-up, config sequence)
 */
void enableOLED_GraphicsTask(void)
{
    log_Shell("%s: enabling OLED", __func__);
    xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_ENABLE);
}

/**
 * @brief Disable the OLED
 * @details Switch the OLED display off completely (used by power management)
 */
void disableOLED_GraphicsTask(void)
{
    xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_DISABLE);
}

/**
 * @brief Turn the OLED off
 * @details Set the bit for the task to turn off the OLED
 * @return      bool: true if success, false otherwise.
 */
bool oledOFF_GraphicsTask(bool * result){
    if (result == NULL) return false;
    *result = true;
    xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_DISPLAYOFF);
    return true;
}

/**
 * @brief Set the OLED contrast
 * @details Set the bit for the task to change the OLED contrast
 * @return      bool: true if success, false otherwise.
 */
bool setContrast_GraphicsTask(uint8_t constrast, bool * result){
    if (result == NULL) return false;
    oGraphicTask.oledContrast = constrast;
    *result = true;
    xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_CONTRAST);
    return true;
}
/**
 * @brief Set the OLED Brightness
 * @details Set the bit for the task to change the OLED brightness
 * @return      bool: true if success, false otherwise.
 */
bool setBrightness_GraphicsTask(uint8_t brightness, bool * result){
    if (result == NULL) return false;
    oGraphicTask.oledBrightness = brightness;
    *result = true;
    xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_BRIGHTNESS);
    return true;
}

/**
 * @brief Set the OLED Image
 * @details Set the bit for the OLED image to update, it will update over the animation specified in the command
 * @return      bool: true if success, false otherwise.
 */
bool setImage_GraphicsTask(poled_cmd_t poled_cmd){
    if (poled_cmd == NULL) return false;

    setPriorityImageWithCmd_GraphicsTask(getOLEDImage_OLEDLibrary(poled_cmd), poled_cmd, OLED_PRIORITY_LEVELS-1, 30000);

    return true;
}

static void touchDisableTemporaryRev4(bool disable){
    if(get_BoardRev() > 4) return;
    if(disable){
        tsl_user_ConfigureMaxThreshold(255);
    }else{
        touchSensorResetThreshold_SensorTask();
    }
}

/**
 * @brief Set the OLED Image
 * @details Set the bit for the OLED image to update, it will update over the animation specified in the command
 *  It will also be removed if the duration expires or something of equal priority is added
 */
void setPriorityImageTime_GraphicsTask(uint8_t priority, uint16_t additionalMsDuration){
    displayQueue[priority].endTick = xTaskGetTickCount()+additionalMsDuration;
}

void setPriorityAnimatedImageWithCmd_GraphicsTask(uint8_t * img, uint8_t * secondImg, poled_cmd_t poled_cmd, uint8_t priority, uint16_t msDuration, uint16_t msDelay, uint16_t msAnimationDuration, oledDirections_e direction){
    displayQueue[priority].isAnimated = true;
    TickType_t curTick = xTaskGetTickCount();
    displayQueue[priority].animationStartTick = curTick + msDelay;
    displayQueue[priority].animationEndTick = curTick + msDelay + msAnimationDuration;
    displayQueue[priority].animationDirection = direction;
    memcpy(displayQueue[priority].secondDisplay, secondImg,SIZEOF_BUFFER_OLED);
    setPriorityImageWithCmd_GraphicsTask(img, poled_cmd, priority, msDuration);
}

void setPriorityImageWithCmd_GraphicsTask(const uint8_t * img, poled_cmd_t poled_cmd, uint8_t priority, uint16_t msDuration){
    if ((img == NULL) || (poled_cmd == NULL)) return;

    stayAwake_Power(__func__);
    memcpy(displayQueue[priority].display,img,SIZEOF_BUFFER_OLED);
    displayQueue[priority].isAnimated = poled_cmd->animation;
    displayQueue[priority].enabled = true;
    displayQueue[priority].position = poled_cmd->statusBarPosition;
    displayQueue[priority].endTick = xTaskGetTickCount() + msDuration;
    displayQueue[priority].animationTypeMask = poled_cmd->animationTypeMask;
    if(poled_cmd->animationTypeMask & slidingAnimation){
        displayQueue[priority].animationStartTick = xTaskGetTickCount();
        displayQueue[priority].animationDirection = poled_cmd->direction;
    }

    bool highestPriority = true;
    for(int i=OLED_PRIORITY_LEVELS-1; i>priority; i--){
        if(displayQueue[i].enabled){
            highestPriority = false;
            break;
        }
    }


    if (highestPriority){
        memcpy(previousOledBuf,oledBuf,SIZEOF_BUFFER_OLED);
        oGraphicTask.oldImageptr = previousOledBuf;
        memcpy(oledBuf,img,SIZEOF_BUFFER_OLED);
        oGraphicTask.newImageAdd = oledBuf;
        if(displayQueue[priority].isAnimated
                && (displayQueue[priority].animationEndTick > xTaskGetTickCount()
                        || (poled_cmd->animationTypeMask & slidingAnimation))){
            oGraphicTask.oledAnimationStartTick = displayQueue[priority].animationStartTick;
            oGraphicTask.oledAnimationEndTick = displayQueue[priority].animationEndTick;
            oGraphicTask.direction = displayQueue[priority].animationDirection;
            oGraphicTask.animationTypeMask = displayQueue[priority].animationTypeMask;
            oGraphicTask.isOLEDAnimation = true;
            memcpy(secondaryBuf,displayQueue[priority].secondDisplay,SIZEOF_BUFFER_OLED);
            oGraphicTask.secondStateImage = secondaryBuf;
            touchDisableTemporaryRev4(poled_cmd->animationTypeMask & rockerAnimation);
        }else{
            oGraphicTask.isOLEDAnimation = false;
            touchDisableTemporaryRev4(false);
        }

        if(oGraphicTask.newImageAdd){
            memcpy(&oGraphicTask.oLedCommand, poled_cmd, sizeof(oled_cmd_t));

            xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_NEW_IMAGE);
        }
    }
}

/**
 * @brief Edit the OLED image
 * @details This is a testing function to turn OLED pixels on and off individually
 */
bool editDebugImage_GraphicsTask(uint16_t location, uint8_t value){
    oGraphicTask.newImageAdd = updateDrawingBoard_OLEDLibrary(location, value);
    xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_NEW_IMAGE);
    return true;
}

/* NOTE : THIS IS A HARDWARE CRITICAL FEATURE DO NOT BREAK
   to protect the battery from cell degradation over extended periods of time, we make sure the 
   halo leds and the front led are never at max brightness at the same time */
static void prvTimerCallbackFadeTimeout(TimerHandle_t xTimer){ 
    uint16_t newBrightness; 
    uint8_t currentBrightness = getMaxBrightness_AnimationsLibrary();

    if(!isAutoBrightness){// disabled for test mode
        setMaxBrightness_AnimationsLibrary(255);
        return;
    }

    if((targetBrightness <= (currentBrightness + BRIGHTNESS_FADE_INCREMENT)) && 
                                      (targetBrightness >= (currentBrightness - BRIGHTNESS_FADE_DECREMENT))){
      setMaxBrightness_AnimationsLibrary(targetBrightness);
    }
    if(currentBrightness < targetBrightness){
      newBrightness = (currentBrightness + BRIGHTNESS_FADE_INCREMENT) < MAX_LED_BRIGHTNESS ? 
                                      (currentBrightness + BRIGHTNESS_FADE_INCREMENT) : MAX_LED_BRIGHTNESS;
      setMaxBrightness_AnimationsLibrary(newBrightness);
    }
    else if(currentBrightness > targetBrightness){ //no need for inferior limit clamping so long as MIN_LED_BRIGHTNESS is not < 0 + BRIGHTNESS_FADE_DECREMENT
      newBrightness = (currentBrightness - BRIGHTNESS_FADE_DECREMENT);
      setMaxBrightness_AnimationsLibrary(newBrightness);
    }
}

static void expOLEDShow(uint16_t messageLength, uint8_t * data){
    oled_cmd_t  oLEDCommand;
    oLEDCommand.animation = true;
    oLEDCommand.animationTime = (data[3] << 8) + data[4];
    oLEDCommand.animationTypeMask = slidingAnimation;
    oLEDCommand.direction = data[2];
    oLEDCommand.statusBarPosition = data[5];
    oLEDCommand.u8ImageType = data[1];
    oLEDCommand.u8ImageCategory = data[0];

    responseFromResult_CommunicationTask(setImage_GraphicsTask(&oLEDCommand));
}

static void expOLEDOff(uint16_t messageLength, uint8_t * data){
    bool result;
    oledOFF_GraphicsTask(&result);
    responseFromResult_CommunicationTask(result);
}

static void expOLEDContrast(uint16_t messageLength, uint8_t * data){
    bool result;
    setContrast_GraphicsTask(data[0], &result);
    responseFromResult_CommunicationTask(result);
}

static void expOLEDBrightness(uint16_t messageLength, uint8_t * data){
    bool bResult;
    setBrightness_GraphicsTask(data[0], &bResult);
    responseFromResult_CommunicationTask(bResult);
}

/*! @brief  Handle the EXP_OLED_SHOW_IMG command
 */
static void expOLEDShowImg(uint16_t messageLength, uint8_t *data)
{
    struct __attribute__ ((packed)) OLEDShowImgMsg {
        uint8_t     animationType;
        uint16_t    animationMs;
        char        name[0];
    } *msg = (struct OLEDShowImgMsg *)data;

    uint16_t    animationMs = __REV16(msg->animationMs);
    char        name[32];
    spiffs_file fh;
    int32_t     ret;

    strlcpy(name, msg->name, sizeof(name));

    log_Shell("%s: name: %s, animationType: %u, animationMs: %u", __func__, name, msg->animationType, animationMs);

    if ((fh = open(name, O_RDONLY, 0)) < 0) {
        log_Shell("%s: open failed with %d", __func__, fh);
        goto returnfail;
    }

    ret = read(fh, imgBuf, sizeof(imgBuf));

    close(fh);

    if (ret < sizeof(imgBuf)) {
        log_Shell("%s: read returned unexpected value: %ld", __func__, ret);
        goto returnfail;
    }

    // We only get animation time and direction from the BLE command
    oled_cmd_t cmd = {
            .animationTime = animationMs,
            .u8ImageCategory = 0,
            .u8ImageType = 0,
            .animation = true,
            .direction = msg->animationType,
            .statusBarPosition = -1
    };

    setPriorityImageWithCmd_GraphicsTask(imgBuf,&cmd,OLED_BLE_OVERRIDE_PRIORITY,10000);
    responseFromResult_CommunicationTask(true);
    return;

    returnfail:
    responseFromResult_CommunicationTask(false);
    return;
}

static void displayOnOLED(uint8_t * display, int8_t position){
    memcpy(previousOledBuf,oledBuf,SIZEOF_BUFFER_OLED);
    oGraphicTask.oldImageptr = previousOledBuf;
    memcpy(oledBuf,display,SIZEOF_BUFFER_OLED);
    oGraphicTask.newImageAdd = oledBuf;
    oled_cmd_t oledCmd = {0,0,0,false,oled_no_animation,position};
    if (oGraphicTask.newImageAdd)
    {
        memcpy(&oGraphicTask.oLedCommand, &oledCmd, sizeof(oled_cmd_t));

        xEventGroupSetBits(oGraphicTask.eventGroup, GRAPHIC_EVENT_NEW_IMAGE);
    }
}

static void GraphicsTask(void *pvParameters)
{

    EventBits_t EventBits;

    waitForFS_SystemUtilities("GraphicsTask");

    init_HaloLedsDriver();
    init_OLEDDriver();
    init_FrontLEDDriver();
    xTimerStart(HaloState.taskTimer, 200);
    xTimerStart(fadeTimerRx, 200);

    enableOLED_GraphicsTask();

    assignFunction_CommunicationTask(COM_EXP, EXP_OLED_SHOW, &expOLEDShow);
    assignFunction_CommunicationTask(COM_EXP, EXP_OLED_OFF, &expOLEDOff);
    assignFunction_CommunicationTask(COM_EXP, EXP_OLED_CONTRAST, &expOLEDContrast);
    assignFunction_CommunicationTask(COM_EXP, EXP_OLED_BRIGHTNESS, &expOLEDBrightness);
    assignFunction_CommunicationTask(COM_EXP, EXP_OLED_SHOW_IMG, expOLEDShowImg);

    while (1)
    {
        EventBits = xEventGroupWaitBits(
                oGraphicTask.eventGroup, /* The event group being tested. */
                0x000000FF, /* The bits within the event group to wait for. */
                pdTRUE,     /* event bits should be cleared before returning. */
                pdFALSE,    /* Don't wait for any bits, either bit will do. */
                100);       /* Wait a maximum of xms for a bit to be set. */

        shwd_KeepAlive(eSHWD_GraphicsTask);

        if( EventBits & GRAPHIC_EVENT_ENABLE )
        {
            setState_OLEDDriver(true);
            setDisplayOff_OLEDDriver();
        }
        if( EventBits & GRAPHIC_EVENT_DISABLE)
        {
            setState_OLEDDriver(false);
        }
        if (EventBits & GRAPHIC_EVENT_NEW_IMAGE)
        {
            //			uint32_t time = HAL_GetTick();

            static uint8_t display[1024];
            TickType_t curTick = xTaskGetTickCount();
            if(oGraphicTask.isOLEDAnimation && (oGraphicTask.animationTypeMask & rockerAnimation))
                getDisplayWithAnimation_OLEDLibrary(oGraphicTask.newImageAdd,
                        oGraphicTask.secondStateImage,
                        oGraphicTask.oledAnimationStartTick,
                        oGraphicTask.oledAnimationEndTick,
                        oGraphicTask.direction,
                        curTick,
                        display);
            else
                memcpy(display,oGraphicTask.newImageAdd,1024);

            static int progress = 0;
            bool isMoreFrames = getOLEDNewImageState_OLEDLibrary(oGraphicTask.oLedCommand,
                    oGraphicTask.oldImageptr,
                    display,
                    oGraphicTask.oOLEDBuffer,
                    curTick-oGraphicTask.oledAnimationStartTick);
            if(isMoreFrames || curTick < oGraphicTask.oledAnimationEndTick)
                xEventGroupSetBits( oGraphicTask.eventGroup, GRAPHIC_EVENT_NEW_IMAGE);
            else
                memcpy(oGraphicTask.oldImageptr, display, sizeof(display));
            progress = isMoreFrames ? progress + 1 : 0;
            display_OLEDDriver(oGraphicTask.oOLEDBuffer);

            //			if(!isMoreFrames)
            //			    consoleSend_Shell("Refresh speed %ld ms", HAL_GetTick() - time);
        }
        if (EventBits & GRAPHIC_EVENT_DISPLAYOFF)
        {
            setDisplayOff_OLEDDriver();
        }
        if (EventBits & GRAPHIC_EVENT_CONTRAST)
        {
            setConstrast_OLEDDriver(oGraphicTask.oledContrast);
        }
        if (EventBits & GRAPHIC_EVENT_BRIGHTNESS)
        {
            setBrightness_OLEDDriver(oGraphicTask.oledBrightness);
        }
        if (EventBits & GRAPHIC_EVENT_ANIM_TIMER)
        {
            updateHaloAnimations();
        }

        bool dismissable = false;
        for(int i=OLED_PRIORITY_LEVELS-1;i>=0;i--){
            if(displayQueue[i].enabled && dismissable){
                if(displayQueue[i].endTick < xTaskGetTickCount()){
                    displayQueue[i].enabled = false;
                }else{
                    if(displayQueue[i].isAnimated &&
                            (displayQueue[i].animationEndTick > xTaskGetTickCount()
                                    || displayQueue[i].animationDirection != oled_no_animation)){
                        touchDisableTemporaryRev4(displayQueue[i].animationTypeMask & rockerAnimation);
                        oGraphicTask.isOLEDAnimation = true;
                        oGraphicTask.oledAnimationStartTick = displayQueue[i].animationStartTick;
                        oGraphicTask.oledAnimationEndTick = displayQueue[i].animationEndTick;
                        oGraphicTask.direction = displayQueue[i].animationDirection;
                        oGraphicTask.animationTypeMask = displayQueue[i].animationTypeMask;
                        memcpy(secondaryBuf, &displayQueue[i].secondDisplay[0],1024);
                        oGraphicTask.secondStateImage = secondaryBuf;
                        log_Shell("New image");
                    }else{
                        touchDisableTemporaryRev4(false);
                        oGraphicTask.isOLEDAnimation = false;
                    }
                    displayOnOLED(displayQueue[i].display, displayQueue[i].position);
                    stayAwake_Power(__func__);
                    dismissable = false;
                    break;
                }
            }else if(displayQueue[i].enabled){
                if(displayQueue[i].endTick < xTaskGetTickCount()){
                    displayQueue[i].enabled = false;
                    dismissable = true;
                }else{
                    break;
                }
            }
        }
        if(dismissable){
            bool result;
            oledOFF_GraphicsTask(&result);
            touchDisableTemporaryRev4(false);
        }
    }
}

static void setConnectionState(bool isConnected){
    connectionState = isConnected;
    if(!isConnected){
        animOff_GraphicsTask();
    }
}

/* NOTE : THIS IS A HARDWARE CRITICAL FEATURE DO NOT BREAK
   to protect the battery from cell degradation over extended periods of time, we make sure the 
   halo leds and the front led are never at max brightness at the same time */
static void brightnessMagnitudeUpdate(uint16_t * brightnessMagnitude){
  	*brightnessMagnitude = (*brightnessMagnitude > MAX_DAYLIGHT_BRIGHTNESS) ? MAX_DAYLIGHT_BRIGHTNESS : *brightnessMagnitude;
  	uint16_t adjustedBrightnessMagnitude = (*brightnessMagnitude*MAX_LED_BRIGHTNESS)/MAX_DAYLIGHT_BRIGHTNESS ;
  	adjustedBrightnessMagnitude = adjustedBrightnessMagnitude < MIN_LED_BRIGHTNESS ? MIN_LED_BRIGHTNESS : 
  				        (adjustedBrightnessMagnitude > MAX_LED_BRIGHTNESS ? MAX_LED_BRIGHTNESS : adjustedBrightnessMagnitude); 
  	
  	//haloleds brightness
  	targetBrightness = adjustedBrightnessMagnitude; 
}

static uint16_t getClockValue(uint16_t * existingClock){
    return clocks.animation >= *existingClock
            ? clocks.animation - *existingClock
                    : 0xFFFF - *existingClock + clocks.animation;
}

static void setNewClockValue(uint16_t * newClock, bool reset){
    *newClock = reset || !*newClock ? clocks.animation ? clocks.animation : clocks.animation+1 : *newClock;
}

static void updateHaloAnimations(){
    if(clocks.battery && !isTestMode_SystemUtilities()){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.battery);
        if(battery_AnimationsLibrary(leds, clock, &socData)){
            clocks.battery = 0;
        }
        socData.charging = getIsChargingAnimationAllowed_SystemUtilities() && lockingName[0] == 0;
    }
    if(clocks.speedometer){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.speedometer);
        if(speedometerData.intro && clock > speedometerIntroTicks_AnimationLibrary() && speedometerData.gap == -99){
            speedometerData.gap = 0;
            speedometerData.percentage = 0;
            speedometerData.intro = false;
        }else if(speedometerData.intro && clock > speedometerIntroTicks_AnimationLibrary()){
            setNewClockValue(&clocks.speedometer, true);
            speedometerData.gap = -99;
            speedometerData.percentage = 1;
            clock = getClockValue(&clocks.speedometer);
        }
        if(speedometer_AnimationsLibrary(leds, clock, &speedometerData)){
            clocks.speedometer = 0;
            speedometerData.percentage = 0;
        }
    }
    if(clocks.averageSpeed){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.averageSpeed);
        if(averageSpeed_AnimationsLibrary(leds, clock, &averageSpeedData)){
            clocks.averageSpeed = 0;
            averageSpeedData.gap = 0;
            averageSpeedData.metersPerHour = 0;
        }
    }
    if(clocks.fractional){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.fractional);
        if(fractionalData_AnimationsLibrary(leds, clock, fractionalData, fractionalDataOff)){
            clocks.fractional = 0;
        }
    }
    if(clocks.fire){
        uint16_t clock = getClockValue(&clocks.fire);
        if((clock%10)==5){
            memset(leds,0,sizeof(leds));
        }
        if((clock%4)==0){
            if(fire_AnimationsLibrary(leds, clock, fireColour, fireOff)){
                clocks.fire = 0;
            }
        }
    }
    if(clocks.pointer){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.pointer);
        if(pointer_AnimationsLibrary(leds, clock, &pointerData)){
            clocks.pointer = 0;
        }
    }
    if(clocks.nightLightIntro){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.nightLightIntro);
        if(nightLightIntro_AnimationsLibrary(leds, clock)){
            clocks.nightLightIntro = 0;
            setNewClockValue(&clocks.nightLight, true);
        }
    }else if(clocks.nightLight){
        uint16_t clock = getClockValue(&clocks.nightLight);
        if(nightLight_AnimationsLibrary(leds, clock, &frontLEDPercentage, &frontLightData)){
            clocks.nightLight = 0;
            setNewClockValue(&clocks.nightLightOutro,true);
        }
    }else if(clocks.nightLightOutro){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.nightLightOutro);
        if(nightLightOutro_AnimationsLibrary(leds, clock)){
            clocks.nightLightOutro = 0;
        }
    }
    if(clocks.progress){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.progress);
        if(progressData.intro && progressIntro_AnimationsLibrary(leds,clock,&progressData)){
            progressData.intro = false;
            if(!progressData.progress)
                progressData.off = true;
        }else if(!progressData.intro && progress_AnimationsLibrary(leds,clock,&progressData)){
            clocks.progress = 0;
            progressData.progress = 0;
        }
    }
    if(clocks.clockLowPriority){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.clockLowPriority);
        if(clock_AnimationsLibrary(leds, clock, &clockData)){
            clocks.clockLowPriority = 0;
        }
    }
    if(clocks.circle){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.circle);
        if(!circleData.ready && customCircle_AnimationsLibrary(leds,clock, &circleData)){
            circleData.ready = true;
            setNewClockValue(&clocks.circle,true);
        }else if(circleData.ready && customCircle_AnimationsLibrary(leds,clock, &circleData)){
            clocks.circle = 0;
            circleData.ready = false;
        }
    }
    if(clocks.angle){
        //remove all LEDs for animations lower priority than angles
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.angle);
        if(anglesData.angleData.background || anglesData.angleData.background){
            if(anglesData.nextProgress){
                if(angleTwoBackground_AnimationsLibrary(leds,clock,&anglesData)){
                    anglesData.angleData.backgroundReady = true;
                    setNewClockValue(&clocks.angle, true);
                    anglesData.angleData.gap = anglesData.angleData.progress;
                    if(angleTwoBackground_AnimationsLibrary(leds,clock,&anglesData)){
                        clocks.angle = 0;
                    }
                }
            }else{
                if(!anglesData.angleData.backgroundReady && angleBackground_AnimationsLibrary(leds,clock,&anglesData.angleData)){
                    anglesData.angleData.backgroundReady = true;
                    setNewClockValue(&clocks.angle, true);
                    anglesData.angleData.gap = anglesData.angleData.progress;
                }else if(angleBackground_AnimationsLibrary(leds,clock,&anglesData.angleData)){
                    clocks.angle = 0;
                }
            }
        }else{
            if(anglesData.nextProgress){
                if(!anglesData.angleData.backgroundReady){
                    anglesData.angleData.backgroundReady = true;
                    anglesData.angleData.gap = anglesData.angleData.progress;
                }else if(angleTwo_AnimationsLibrary(leds,clock,&anglesData)){
                    anglesOff_GraphicsTask();
                }
            }else{
                if(!anglesData.angleData.backgroundReady){
                    anglesData.angleData.backgroundReady = true;
                    anglesData.angleData.gap = anglesData.angleData.progress;
                }else if(angle_AnimationsLibrary(leds,clock,&anglesData.angleData)){
                    anglesOff_GraphicsTask();
                }
            }
        }
    }
    if(clocks.spiral){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.spiral);
        if(spiral_AnimationsLibrary(leds, clock, &spiralData)){
            setNewClockValue(&clocks.spiral, true);
            spiralCount++;
            if(spiralCount == spiralData.rotations){
                memset(leds,0,sizeof(leds));
                spiralCount = 0;
                clocks.spiral = 0;
            }
        }
    }
    if(clocks.angleIntro){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.angleIntro);
        if(angleIntro_AnimationsLibrary(leds, clock)){
            clocks.angleIntro = 0;
        }
    }
    if(clocks.clock){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.clock);
        if(clock_AnimationsLibrary(leds, clock, &clockData)){
            clocks.clock = 0;
        }
    }
    if(clocks.alarmState){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.alarmState);
        if(alarmStateChange_AnimationsLibrary(leds,clock,alarmProgress,&frontLEDPercentage)){
            clocks.alarmState = 0;
        }
    }
    if(clocks.shadowTaps && !clocks.alarmSetting){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.shadowTaps);
        taps_AnimationsLibrary(leds, clock, tapShadowCode, tapShadowLength, connectionState, getStandbyReason_CommunicationTask(),true);
        if(tapShadowLength == 0){
            clocks.shadowTaps = 0;
        }
    }
    if(clocks.taps && !clocks.alarmSetting){
    	if(!clocks.shadowTaps)
    		memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.taps);
        taps_AnimationsLibrary(leds, clock, tapCode, tapLength, connectionState, getStandbyReason_CommunicationTask(),false);
        if(tapLength == 0){
            clocks.taps = 0;
        }
    }
    if(clocks.swipe){
        uint16_t clock = getClockValue(&clocks.swipe);
        if(swipe_AnimationsLibrary(leds,clock,isSwipeRight, connectionState)){
            clocks.swipe = 0;
            memset(leds,0,sizeof(leds));
        }
    }
    if(clocks.intro){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.intro);
        if(logo_AnimationsLibrary(leds, clock)){
            clocks.intro = 0;
        }
    }
    if(clocks.disconnect){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.disconnect);
        if(disconnect_AnimationsLibrary(leds, clock)){
            clocks.disconnect = 0;
        }
    }
    if(clocks.alarmSetting){
        memset(leds,0,sizeof(leds));
        uint16_t clock = getClockValue(&clocks.alarmSetting);
        if(alarmSettingChange_AnimationsLibrary(leds, clock, alarmEngaged)){
            clocks.alarmSetting = 0;
        }
    }
    if(ledsTestStage != 0){
        memset(leds, 0, sizeof(leds));
        ledsTest_AnimationsLibrary(leds, ledsTestStage, ledBrightness, ledsTestOffLED);
    }

    if(frontLEDPercentage != 0){
        setFrontLEDPercentage_FrontLEDDriver(frontLEDPercentage);
        frontLEDEnabled = true;
    }else if(frontLEDEnabled){
        setFrontLEDPercentage_FrontLEDDriver(0);
        frontLEDEnabled = false;
    }
    clocks.animation++;
    if(!update_HaloLedsDriver(leds)){
        log_Shell("Halo Led Write error");
    }
}

bool startAnimation(HaloAnimation_function_e type, uint8_t * payload){
    switch(type){
        case HaloAnimation_display_stateOfCharge: {
            setNewClockValue(&clocks.battery, false);
            struct StateOfChargeAnimationData * data = (struct StateOfChargeAnimationData *) payload;
            socData.stateOfCharge = data->stateOfCharge > 100 ? 100 : data->stateOfCharge < 0 ? 0 : data->stateOfCharge;
            socData.charging = data->charging;
            socData.showIdle = data->showIdle;
            break;
        }
        case HaloAnimation_display_pointer:{
            pointerData.off = false;
            struct PointerAnimationData * data = (struct PointerAnimationData *) payload;
            int16_t newHeading = data->heading;
            while(newHeading > 180)
                newHeading -= 360;
            while(newHeading < -180)
                newHeading += 360;
            if(clocks.pointer){
                int16_t clockwise = newHeading - pointerData.heading;
                int16_t counterClockwise = pointerData.heading - newHeading;
                pointerData.headingGap = abs(clockwise) < abs(counterClockwise) ? clockwise : counterClockwise;
            }
            pointerData.headingGap = clocks.pointer ? newHeading - pointerData.heading : 0;
            pointerData.colour = data->colour;
            if(pointerData.standby){
                pointerData.heading = pointerStandbyLocation_AnimationsLibrary(getClockValue(&clocks.pointer), pointerData.heading)%360;
            }else{
                pointerData.heading = newHeading;
            }
            setNewClockValue(&clocks.pointer, true);
            break;
        }
        case HaloAnimation_display_speedometerIntro:{
            setNewClockValue(&clocks.speedometer, true);
            speedometerData.intro = true;
            speedometerData.gap = 100;
            speedometerData.percentage = 100;
            speedometerData.off = false;
            break;
        }
        case HaloAnimation_display_speedometer:{
            setNewClockValue(&clocks.speedometer, true);
            struct SpeedometerAnimationData * state = (struct SpeedometerAnimationData *) payload;
            state->percentage = state->percentage > 100 ? 100 : state->percentage;
            speedometerData.gap = state->percentage - speedometerData.percentage;
            speedometerData.percentage = state->percentage;
            speedometerData.off = false;
            speedometerData.intro = false;
            break;
        }
        case HaloAnimation_display_averageSpeed:{
            setNewClockValue(&clocks.averageSpeed, true);
            struct AverageSpeedAnimationData * avgSpeedData = (struct AverageSpeedAnimationData *) payload;
            averageSpeedData.gap = avgSpeedData->metersPerHour - averageSpeedData.metersPerHour;
            averageSpeedData.averageHsv = avgSpeedData->averageHsv;
            averageSpeedData.higherHsv = avgSpeedData->higherHsv;
            averageSpeedData.lowerHsv = avgSpeedData->lowerHsv;
            averageSpeedData.averageMetersPerHour = avgSpeedData->averageMetersPerHour;
            averageSpeedData.metersPerHour = avgSpeedData->metersPerHour;
            averageSpeedData.off = false;
            break;
        }
        case HaloAnimation_display_fractionalData:{
            setNewClockValue(&clocks.fractional, true);
            memcpy(&fractionalData,payload,sizeof(struct FractionalData));
            fractionalDataOff = false;
            break;
        }
        case HaloAnimation_display_fire:{
            setNewClockValue(&clocks.fire, true);
            memcpy(&fireColour,payload,3);
            fireOff = false;
            break;
        }
        case HaloAnimation_display_nightLight:{
            if(frontLightLocked)
                break;
            setNewClockValue(&clocks.nightLightIntro, false);
            struct FrontLightAnimationData * lightData = (struct FrontLightAnimationData *) payload;
            frontLightData.gap = lightData->percentage - frontLightData.percentage;
            frontLightData.percentage = lightData->percentage;
            frontLightData.isBlinking = lightData->isBlinking;
            break;
        }
        case HaloAnimation_display_progressIntro:{
            setNewClockValue(&clocks.progress, true);
            struct ProgressAnimationData * state = (struct ProgressAnimationData *) payload;
            progressData.colour1 = state->colour1;
            progressData.colour2 = state->colour2;
            progressData.cycle = state->cycle;
            progressData.progress = state->progress;
            progressData.lowPower = state->lowPower;
            progressData.intro = true;
            progressData.off = false;
            break;
        }
        case HaloAnimation_display_progress:{
            setNewClockValue(&clocks.progress, false);
            struct ProgressAnimationData * state = (struct ProgressAnimationData *) payload;
            progressData.colour1 = state->colour1;
            progressData.colour2 = state->colour2;
            progressData.cycle = state->cycle;
            progressData.gap = state->progress - progressData.progress;
            progressData.progress = state->progress > 100 ? 100 : state->progress;
            progressData.lowPower = state->lowPower;
            progressData.off = false;
            progressData.intro = false;
            break;
        }
        case HaloAnimation_display_customCircle:{
            circleData.off = false;
            if(!clocks.circle) circleData.ready = false;
            setNewClockValue(&clocks.circle, false);
            struct CustomCircleAnimationData * data = (struct CustomCircleAnimationData *) payload;
            memcpy(&circleData,data,sizeof(struct CustomCircleAnimationData));
            break;
        }
        case HaloAnimation_display_logo: {
            setNewClockValue(&clocks.intro, false);
            break;
        }
        case HaloAnimation_display_disconnect: {
            setNewClockValue(&clocks.disconnect, false);
            break;
        }
        case HaloAnimation_display_shadow_taps:{
            uint8_t * taps = payload;
            tapShadowCode = taps[0];
            tapShadowLength = taps[1];
            setNewClockValue(&clocks.shadowTaps, false);
            break;
        }
        case HaloAnimation_display_taps:{
            uint8_t * taps = payload;
            tapCode = taps[0];
            tapLength = taps[1];
            setNewClockValue(&clocks.taps, false);
            break;
        }
        case HaloAnimation_display_swipe:{
            isSwipeRight = payload[0];
            setNewClockValue(&clocks.swipe, true);
            break;
        }
        case HaloAnimation_display_clockLowPriority:{
            memcpy(&clockData,payload,sizeof(struct ClockAnimationData));
            setNewClockValue(&clocks.clockLowPriority, false);
            break;
        }
        case HaloAnimation_display_angle_intro:{
            setNewClockValue(&clocks.angleIntro, false);
            break;
        }
        case HaloAnimation_display_angle:{
            struct AngleAnimationData * angle = (struct AngleAnimationData *)payload;
            int16_t newAngle = angle->angle;
            if((!angle->sameTurn && anglesData.angleData.angle != newAngle)
                    || clocks.angle == 0
                    || anglesData.nextProgress){
                bool adaptingSecondaryAngle = anglesData.nextProgress;
                anglesOff_GraphicsTask();
                anglesData.angleData.backgroundReady = adaptingSecondaryAngle;
            }
            anglesData.angleData.gap = angle->progress - anglesData.angleData.progress;
            anglesData.angleData.progress = angle->progress;
            anglesData.angleData.angle = newAngle;
            anglesData.angleData.width = angle->width;
            anglesData.angleData.colour = angle->colour;
            anglesData.angleData.background = angle->background;
            if(anglesData.angleData.background){
                anglesData.angleData.backgroundColour = angle->backgroundColour;
            }else{
                anglesData.angleData.repeat = angle->repeat;
                setNewClockValue(&clocks.angle, false);
            }
            setNewClockValue(&clocks.angle, true);
            break;
        }
        case HaloAnimation_display_two_angles:{
            struct AnglesAnimationData * angles = (struct AnglesAnimationData *)payload;
            int16_t newAngle = angles->angleData.angle;
            if((!angles->angleData.sameTurn && anglesData.angleData.angle != newAngle) || clocks.angle == 0)
                anglesOff_GraphicsTask();
            anglesData.angleData.angle = newAngle;
            anglesData.angleData.width = angles->angleData.width;
            anglesData.angleData.colour = angles->angleData.colour;
            int16_t newSecondAngle = angles->nextAngle;
            if(!angles->angleData.sameTurn && anglesData.nextAngle != newSecondAngle)
                anglesOff_GraphicsTask();
            anglesData.angleData.gap = angles->angleData.progress - anglesData.angleData.progress;
            anglesData.angleData.progress = angles->angleData.progress;
            anglesData.nextGap = angles->nextProgress - anglesData.nextProgress;
            anglesData.nextProgress = angles->nextProgress;
            anglesData.nextAngle = angles->nextAngle;
            anglesData.nextWidth = angles->nextWidth;
            anglesData.nextColour = angles->nextColour;
            anglesData.angleData.background = angles->angleData.background;
            if(anglesData.angleData.background){
                anglesData.angleData.backgroundColour = angles->angleData.backgroundColour;
            }
            setNewClockValue(&clocks.angle, true);
            break;
        }
        case HaloAnimation_display_spiral:{
            memcpy(&spiralData,payload,sizeof(struct SpiralAnimationData));
            setNewClockValue(&clocks.spiral, false);
            break;
        }
        case HaloAnimation_display_clock:{
            memcpy(&clockData,payload,sizeof(struct ClockAnimationData));
            setNewClockValue(&clocks.clock, false);
            break;
        }
        case HaloAnimation_display_alarm_state:{
            alarmEngaged = payload[0];
            setNewClockValue(&clocks.alarmSetting, false);
            break;
        }
        case HaloAnimation_display_alarm_progress:{
            alarmProgress = payload[0];
            setNewClockValue(&clocks.alarmState, true);
            break;
        }
        default:
            break;
    }

    return true;
}


/**
 * @brief Remove lock for other tasks
 * @details Send  the task name that is requesting it to make sure it's the one that
 *      initiated it.
 */
void unlockAnimations_GraphicsTask(const char * const taskName){
    if(lockingName[0]==0 || !strcmp(lockingName,taskName))
        memset(lockingName,0,sizeof(lockingName));
}

/**
 * @brief Lock for other tasks
 * @details Send  the task name that is requesting it to make sure it's the one that
 *      initiated it or wants to initiate it if it's unlocked.
 */
void lockAnimations_GraphicsTask(const char * const taskName){
    if(lockingName[0]==0 || !strcmp(lockingName,taskName)){
        strcpy(lockingName, taskName);
        animOff_GraphicsTask();
    }
}

/**
 * @brief Initiate an animation and lock out other tasks
 * @details Send a specific animation state with it's expected parameters to display it
 *      as well as the task name that is requesting it
 */
bool startAnimationWithLock_GraphicsTask(HaloAnimation_function_e type, uint8_t * payload, const char * const taskName){
    if(lockingName[0]==0 || !strcmp(lockingName,taskName)){
        //Only clear animations if this is a fresh lock
        if(lockingName[0]==0){
            animOff_GraphicsTask();
            strcpy(lockingName, taskName);
        }
        startAnimation(type, payload);
    }
    return true;
}

/**
 * @brief Initiate an animation
 * @details Send a specific animation state with it's expected parameters to display it
 */
bool startAnimation_GraphicsTask(HaloAnimation_function_e type, uint8_t * payload){
    if(lockingName[0]==0)
        startAnimation(type,payload);
    return true;
}

/**
 * @brief Set Front Light Lock
 * @details The intention is to give applications the right to instruct the  graphics task
 * that it would like to lock up the front light to prevent other applications from manipulating it
 * or to remove that lock to allow it to be manipulated again. Setting it to true will dismiss
 * attempts to make changes without effect, but it will be possible to query for the state so that
 * applications can be aware of whether they have access to it as well.
 */
void setFrontLightLocked_GraphicsTask(bool isLocked){
    frontLightLocked = isLocked;
    frontLightData.percentage = isLocked ? 0 : frontLightData.percentage;
    frontLEDPercentage = isLocked ? 0 : frontLEDPercentage;
    log_Shell("Light is %s", isLocked ? "locked" : "unlocked");
}

/**
 * @brief Get Front Light Lock
 * @details The intention is to give applications access to know if the front light is locked
 * so that they can accommodate this behaviour.
 */
bool getFrontLightLocked_GraphicsTask(){
    return frontLightLocked;
}


/**
 * @brief LED Test
 * @details The parameters determine what test to perform
 */
void startLEDTest_GraphicsTask(uint8_t stage, uint8_t brightness, uint8_t offLED){
    ledsTestStage = stage;
    ledBrightness = brightness;
    ledsTestOffLED = offLED;

    isAutoBrightness = !stage ? true : false; //makes sure autobrightness is disabled for test mode
    memset(leds, 0, sizeof(leds));
}

/**
 * @brief Night light state update
 * @details set night light state information, if the animation is off it will just turn on
 */
void nightLightStateUpdate_GraphicsTask(struct FrontLightAnimationData data){
    if(frontLightLocked)
        return;
    frontLightData.isBlinking = data.isBlinking;
    if(clocks.nightLightIntro || clocks.nightLight){
        frontLightData.gap = data.percentage - frontLightData.percentage;
        frontLightData.percentage = data.percentage;
        if(!clocks.nightLight)
            setNewClockValue(&clocks.nightLightIntro, true);
        else
            setNewClockValue(&clocks.nightLight, true);
    }
}

void pointerStateUpdate(bool standby, bool intro){
    pointerData.standby = standby;
    pointerData.intro = intro;
    pointerData.off = false;
    setNewClockValue(&clocks.pointer, false);
}

/**
 * @brief Update the state of the pointer use to start in standby and use the intro
 */
void pointerStateUpdate_GraphicsTask(bool standby, bool intro){
    if(lockingName[0]==0){
        pointerStateUpdate(standby, intro);
    }
}

/**
 * @brief Update the state of the pointer use to start in standby and use the intro with lock
 * @details The locking measure will make sure no other applications interrupt this sequence
 */
void pointerStateUpdateWithLock_GraphicsTask(bool standby, bool intro, const char * const taskName){
    if(lockingName[0]==0 || !strcmp(lockingName,taskName)){
        //Only clear animations if this is a fresh lock
        if(lockingName[0]==0){
            animOff_GraphicsTask();
            strcpy(lockingName, taskName);
        }
        pointerStateUpdate(standby, intro);
    }
}

/**
 * @brief Turns off the pointer animation
 */
void pointerOff_GraphicsTask(){
    if(clocks.pointer){
        pointerData.off = true;
        pointerData.headingGap = 0;
        setNewClockValue(&clocks.pointer, true);
    }
}

/**
 * @brief Turn off the speedometer
 */
void speedometerOff_GraphicsTask(){
    if(clocks.speedometer){
        speedometerData.off = true;
        setNewClockValue(&clocks.speedometer, true);
        speedometerData.intro = false;
    }
}

/**
 * @brief Turn off the average speed
 */
void averageSpeedOff_GraphicsTask(){
    if(clocks.averageSpeed){
        averageSpeedData.off = true;
        setNewClockValue(&clocks.averageSpeed, true);
    }
}

/**
 * @brief Turn off fractional data
 */
void fractionalDataOff_GraphicsTask(){
    if(clocks.fractional){
        fractionalDataOff = true;
        setNewClockValue(&clocks.fractional, true);
    }
}

/**
 * @brief Turn off fractional data
 */
void fireOff_GraphicsTask(){
    if(clocks.fire){
        fireOff = true;
    }
}

/**
 * @brief Turn off fitness
 */
void progressOff_GraphicsTask(){
    if(clocks.progress){
        progressData.off = true;
        setNewClockValue(&clocks.progress, true);
    }
}

/**
 * @brief turn off customized circle
 */
void circleOff_GraphicsTask(){
    circleData.off = true;
}

/**
 * @brief Turn angles off
 */
void anglesOff_GraphicsTask(){
    clocks.angle = 0;
    anglesData.angleData.progress = 0;
    anglesData.angleData.backgroundReady = false;
    anglesData.nextProgress = 0;
    clocks.spiral = 0;
    memset(leds,0,sizeof(leds));
}

//TODO determine the duration to set to make it fade from it's current clock value
void clockOff_GraphicsTask(){
    clockData.duration = 630;
}

void animOff_GraphicsTask(){
    if(clocks.pointer) pointerOff_GraphicsTask();
    if(clocks.angle) anglesOff_GraphicsTask();
    if(clocks.circle) circleOff_GraphicsTask();
    if(clocks.clock) clockOff_GraphicsTask();
    if(clocks.speedometer) speedometerOff_GraphicsTask();
    if(clocks.averageSpeed) averageSpeedOff_GraphicsTask();
    if(clocks.progress) progressOff_GraphicsTask();
    if(clocks.averageSpeed) averageSpeedOff_GraphicsTask();
    if(clocks.fractional) fractionalDataOff_GraphicsTask();
    if(clocks.fire) fireOff_GraphicsTask();
}

/**
 * @brief Test front light functional test
 * @details This should by no means be called outside of functional tests
 */
void testNightLight_GraphicsTask(uint8_t percentage, uint8_t mode){
    frontLightData.isBlinking = mode;
    frontLEDPercentage = frontLightData.percentage = percentage;
}

/**
 * @brief Test connectivity to the LED Driver
 * @details This will fail if it can't communication with either LED chips
 * @return bool: true if success, false otherwise.
 */
bool testHaloResponseByTurningOff_GraphicsTask(){
    uint8_t testLeds[72] = {0};
    return update_HaloLedsDriver(testLeds);
}

/**
 * @brief Test connectivity to the OLED
 * @details This will fail if it can't communication with the OLED
 * @return      bool: true if success, false otherwise.
 */
bool testOLEDByTurningOff_GraphicsTask(){
    return setDisplayOff_OLEDDriver();
}

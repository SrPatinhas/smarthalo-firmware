/*
 * PersonalityTask.c
 *
 *  Created on: Sep 18, 2019
 *      Author: matt
 */

#include <CommunicationTask.h>
#include <SystemUtilitiesTask.h>
#include "PersonalityTask.h"
#include "GraphicsTask.h"
#include "AnimationsLibrary.h"
#include "SensorsTask.h"
#include "SoundTask.h"
#include "OLEDLibrary.h"
#include "SHTimers.h"
#include "assets.h"
#include "WatchdogTask.h"
#include "SHTaskUtils.h"

#include "tim.h"
#include "math.h"

#define TASK_PRIORITY 	PRIORITY_NORMAL
#define STACK_SIZE      configMINIMAL_STACK_SIZE + 0x300

#define TIMER_IN_MILLISECONDS      50
#define STVZO_TIMER_IN_MILLISECONDS 3500u

#define STARTUP_DISPLAY_TIMEOUT    6*1000/TIMER_IN_MILLISECONDS //6 seconds * 20 50ms ticks
#define CONNECTION_DISPLAY_TIMEOUT 5*1000/TIMER_IN_MILLISECONDS //5 seconds * 20 50ms ticks
#define ONBOARDING_CHECK_TIMEOUT   3*1000/TIMER_IN_MILLISECONDS //3 seconds * 20 50ms ticks
#define SLEEP_OLED_HUD_MILLISECOND_TIMEOUT 6000
#define AWAKE_OLED_HUD_MILLISECOND_TIMEOUT 2000

#define MOVEMENT_THRESHOLD 0.02f
#define MAX_TAP_LENGTH             8

#define UNITS_Y_POSITION 45
#define BASE_STRING_Y_POSITION 58

#define ONBOARDING_FIRST_LINE 12
#define ONBOARDING_SECOND_LINE 28
#define ONBOARDING_THIRD_LINE 44
#define ONBOARDING_FOURTH_LINE 60
#define ONBOARDING_TOP_LINE 20
#define ONBOARDING_MIDDLE_LINE 36
#define ONBOARDING_BOTTOM_LINE 52
#define ONBOARDING_START_50MS_TICKS 1
#define ONBOARDING_WELCOME_50MS_TICKS 81
#define ONBOARDING_INSTRUCTION_50MS_TICKS 161
#define ONBOARDING_INTERACTION_TICK 162

#define CENTERED_TEXT_Y 40
#define BASE_TITLE_Y 61

#define STVZO_OLED_PRIORITY 0 //makes low voltage indicator for germany's stvzo laws lowest priority
#define PERSONALITY_OLED_PRIORITY 7
#define PERSONALITY_OLED_PRIORITY_LOW 6

#define LOW_VOLTAGE_INDICATOR 3500u //in millivolts
#define HPF_ALPHA 0.5f

struct SleepMode {
    uint8_t code;
    uint8_t length;
    bool engaged;
    char codeString[MAX_TAP_LENGTH];
    uint8_t instructionTick;
};

/* Static task --------------- */
static TaskHandle_t selfHandle = NULL;
static StaticTask_t xTaskBuffer;
static StackType_t PersonalityStack[STACK_SIZE];

/* Queue with jobs ------------ */
static TimerHandle_t xTimerRx;
static StaticTimer_t xTimerBufferRx;
static TimerHandle_t xTimerStvzoIndicator;
static StaticTimer_t xTimerBufferStvzoIndicator;

static const char * sleepConfigFile = "SleepConfig";
static const char * sleepFile = "SleepMode";

static uint16_t tickCount = 0;
static uint8_t wakeCount = 60;
static bool startUp = true;
static uint8_t startUpReaction = 2;
static bool onboarding = false;
static uint8_t onboardingCount = 0;
static bool onboardingTap = false;
static bool onboardingRight = false;
static bool onboardingLeft = false;
static bool onboardingCheck = false;
static uint8_t onboardingAnimationCount = 0;
static uint8_t onboardingSwipeCount = 0;
static uint8_t explainAnimationCount = 0;
static oledDirections_e onboardingExplainDirection = oled_no_animation;
static bool connectedNotify = false;
static bool timer_waiting = false;

static bool firstMove = false;
static uint16_t accTrackCount = 0;

static float previousX, previousY, previousZ, X, Y, Z, hpfX, hpfY, hpfZ;
static bool accStreamStarted, newAccData = false;
static uint8_t interruptCount = 0;

static uint8_t pairedState;

static uint8_t newTap = 0;
static bool tapUpdate = false;
static uint8_t tapCode = 0;
static uint8_t tapLength = 0;
static char finishedCode[MAX_TAP_LENGTH];
static uint8_t finishedCodeLength = 0;
static bool isAuthenticated = false;
static struct TouchSoundsPayload touchSettings = {0,false};
static bool isInMovement = false;

static struct SleepMode sleepMode;

static struct ClockAnimationData clockData = {0};
static bool displayClock = false;

static uint8_t drawingBoard[1024] = {0xFF};

const static uint8_t tap[] = { 0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C };
const static uint8_t leftArrow[] = { 0x18, 0x3C, 0x7E, 0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 };
const static uint8_t rightArrow[] = { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF, 0x7E, 0x3C, 0x18 };
const static uint8_t check[] =
    { 0x00, 0xE0, 0x07, 0x00, 0x00, 0xFC, 0x3F, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x80, 0x03, 0xC0, 0x01, 0xC0, 0x00, 0x00, 0x03, 0x60, 0x00, 0x00, 0x06, 0x30, 0x00, 0x00, 0x0C, 0x18, 0x00, 0x00, 0x18, 0x0C, 0x00, 0x02, 0x30, 0x0C, 0x00, 0x07, 0x30, 0x06, 0x00, 0x0E, 0x60, 0x06, 0x00, 0x1C, 0x60, 0x06, 0x00, 0x38, 0x60, 0x03, 0x00, 0x1C, 0xC0, 0x03, 0x00, 0x0E, 0xC0, 0x03, 0x00, 0x07, 0xC0, 0x03, 0x80, 0x03, 0xC0, 0x03, 0xC0, 0x01, 0xC0, 0x03, 0xE0, 0x00, 0xC0, 0x06, 0x70, 0x00, 0x60, 0x06, 0x38, 0x00, 0x60, 0x06, 0x1C, 0x00, 0x60, 0x0C, 0x08, 0x00, 0x30, 0x0C, 0x00, 0x00, 0x30, 0x18, 0x00, 0x00, 0x18, 0x30, 0x00, 0x00, 0x0C, 0x60, 0x00, 0x00, 0x06, 0xC0, 0x00, 0x00, 0x03, 0x80, 0x03, 0xC0, 0x01, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0xFC, 0x3F, 0x00, 0x00, 0xE0, 0x07, 0x00 };
const static uint8_t moon[] =
	{ 0x00, 0xC0, 0x1F, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0x03, 0x00, 0x80, 0xFF, 0xFF, 0x07, 0x00, 0xC0, 0xFF, 0xFF, 0x1F, 0x00, 0xE0, 0xFF, 0xFF, 0x1F, 0x00, 0xF0, 0xFF, 0xFF, 0x3F, 0x00, 0xF8, 0xFF, 0xFF, 0x7F, 0x00, 0xF8, 0xFF, 0xFF, 0x7F, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0x01, 0xFE, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0x00, 0xFF, 0xFF, 0x01, 0x1F, 0x00, 0xFC, 0xFF, 0x01, 0x07, 0x00, 0xF0, 0xFF, 0x03, 0x00, 0x00, 0xE0, 0xFF, 0x03, 0x00, 0x00, 0xC0, 0xFF, 0x03, 0x00, 0x00, 0xC0, 0xFF, 0x03, 0x00, 0x00, 0x80, 0xFF, 0x03, 0x00, 0x00, 0x80, 0xFF, 0x03, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00 };

static struct OLEDAsset tapAsset = {.width=7,.height=7,.asset=tap};
static struct OLEDLayout tapLayout = {.location[0]={.x=42,.y=5},.location[1]={.x=60,.y=49}};
static struct OLEDAsset leftSwipeAsset = {.width=17,.height=8,.asset=leftArrow};
static struct OLEDLayout leftSwipeLayout = {.location[0]={.x=42,.y=5},.location[1]={.x=55,.y=48}};
static struct OLEDAsset rightSwipeAsset = {.width=17,.height=8,.asset=rightArrow};
static struct OLEDLayout rightSwipeLayout = {.location[0]={.x=42,.y=5},.location[1]={.x=55,.y=48}};
static struct OLEDAsset checkAsset = {.width=32,.height=32,.asset=check};
static struct OLEDLayout checkLayout = {.location[0]={.x=48,.y=16}};
static struct OLEDAsset byeAsset = {.width=37,.height=20,.asset=bye};
static struct OLEDLayout byeLayout = {.location[0]={.x=45,.y=22}};

static struct OLEDAsset greetingAsset = {.width=125,.height=24,.asset=smartHaloText};
static struct OLEDLayout greetingLayout = {.location[0]={.x=2,.y=18}};
static struct OLEDAsset unpairedAsset = {.width = 18, .height = 18, .asset = xIcon};
static struct OLEDAsset connection1Asset = {.width = 18, .height = 29, .asset = connectedIcon};
static struct OLEDAsset connection2Asset = {.width = 18, .height = 29, .asset = connectedIcon2};
static struct OLEDAsset phoneAsset = {.width=33,.height=56,.asset=phoneIcon};
static struct OLEDAsset haloAsset = {.width=43,.height=53,.asset=haloIcon};
static struct OLEDLayout disconnectionLayout = {.location[0]={.x=12,.y=4},.location[1]={.x=54,.y=23},.location[2]={.x=78,.y=5}};
static struct OLEDLayout connectionLayout = {.location[0]={.x=12,.y=4},.location[1]={.x=54,.y=18},.location[2]={.x=78,.y=5}};

static struct OLEDAsset moonAsset = {.width=31,.height=34,.asset=moon};
static struct OLEDLayout sleepLayout = {.location[0]={.x=48,.y=7}};
static struct OLEDAsset wakeAsset = {.width=78,.height=47,.asset=bike};
static struct OLEDLayout wakeLayout = {.location[0]={.x=25,.y=0},.location[1]={.x=33,.y=60}};

static struct OLEDAsset lowBatteryAsset = {.width=59,.height=31,.asset=lowBatIcon};
static struct OLEDAsset lowBatteryEmptyAsset = {.width=59,.height=31,.asset=lowBatEIcon};
static struct OLEDLayout lowBatteryLayout = {.location[0]={.x=35,.y=16}};
static uint8_t lowBatteryCount = 0;
static uint8_t touchCount = 0;

static struct OLEDAsset touchAsset = {.width = 37,.height=47,.asset=touchIcon};
static struct OLEDLayout touchLayout = {.location[0]={.x=46,.y=8}};

const uint8_t welcome[4][30] = {"Let's ride!", {199,'a',' ','r','o','u','l','e','!'},"Los geht's!",{161,'R','o','d','e','m','o','s','!'}};
const uint8_t onboarding_intro1[4][30] = {"Welcome to","Bienvenue sur","Willkommen bei","\241Bienvenido a"};
const uint8_t onboarding_intro2[4][30] = {"SmartHalo 2!","SmartHalo 2!","SmartHalo 2!",{161,'S','m','a','r','t','H','a','l','o',' ','2','!'}};
const uint8_t onboarding_setup1[4][30] = {"Try these","Essayez ces","Probieren Sie","Intenta estos"};
const uint8_t onboarding_setup2[4][30] = {"gestures:","gestes:","diese Gesten aus:","gestos:"};
const uint8_t connect_instruction1[4][30] = {"Download the","T\351l\351chargez","Laden Sie die App","Descargue la"};
const uint8_t connect_instruction2[4][30] = {"SmartHalo App","l'appli SmartHalo","herunter und","app SmartHalo"};
const uint8_t connect_instruction3[4][30] = {"and connect","et connectez-vous","verbinden Sie sich","y con\351ctese"};
const uint8_t onboarding_instruction1_1[4][30] = {"You've got it!",{'T','r',232,'s',' ','b','i','e','n','!'},"Sie haben es raus!",{161,'Y','a',' ','l','o',' ','t','i','e','n','e','s','!'}};
const uint8_t onboarding_instruction2_1[4][30] = {"Now keep","Continuez","Wischen Sie","Sigue"};
const uint8_t onboarding_instruction3_1[4][30] = {"swiping.","de balayer.","jetzt weiter.","deslizando."};
const uint8_t onboarding_instruction1_2[4][30] = {"Use swipes to","Balayez pour","Wischen Sie, um","Desliza para ver"};
const uint8_t onboarding_instruction2_2[4][30] = {"view your ride","visualiser vos","Ihre Metriken",{'l','a','s',' ','m',233,'t','r','i','c','a','s',' ','d','e'}};
const uint8_t onboarding_instruction3_2[4][30] = {"metrics...",{'m',233,'t','r','i','q','u','e','s','.','.','.'},"anzuzeigen...","tu viaje..."};
const uint8_t onboarding_instruction1_3[4][30] = {"...and taps to","...et touchez pour","...und Tippen, um","...y toca para"};
const uint8_t onboarding_instruction2_3[4][30] = {"trigger Quick" ,{'d',233,'c','l','e','n','c','h','e','r',' ','l','e','s'},"Quick-Touch","activar atajos"};
const uint8_t onboarding_instruction3_3[4][30] = {"Touch shortcuts.","raccourcis tactiles.",{'a','u','s','z','u','l',246,'s','e','n','.'},"de Quick Touch"};
const uint8_t onboarding_instruction1_4[4][30] = {"(You can","(Vous pouvez les",{'(','S','i','e',' ','k',246,'n','n','e','n'},"(Atajos"};
const uint8_t onboarding_instruction2_4[4][30] = {"customize them","personnaliser","diese in der","desde"};
const uint8_t onboarding_instruction3_4[4][30] = {"in the app.)","dans l'appli.)","App anpassen.)","la app.)"};
const uint8_t onboarding_instruction1_5[4][30] = {"Tip: tap in the","Conseil: touchez au","In die Mitte tippen","Tip: pulsa en el"};
const uint8_t onboarding_instruction2_5[4][30] = {"middle and swipe","centre et balayez","und von den","medio y desliza"};
const uint8_t onboarding_instruction3_5[4][30] = {"from the edges.","depuis les bords.",{'R',228,'n','d','e','r','n',' ','w','i','s','c','h','e','n'},"desde los bordes."};
const uint8_t onboarding_instruction1_6[4][30] = {"All set! Happy",{'V','o','u','s',' ',234,'t','e','s',' ','p','r',234,'t','s',','},{'V','i','e','l',' ','S','p','a',223},{161,'L','i','s','t','o','!'}};
const uint8_t onboarding_instruction2_6[4][30] = {"biking!",{'b','o','n','n','e','s',' ','b','a','l','a','d','e','s','!'},"beim Radfahren!",{161,'F','e','l','i','z',' ','p','a','s','e','o','!'}};
const uint8_t sleep[4][30] = {"Sleep Mode","Mode Veille","Schlafmodus",{'M','o','d','o',' ','d','e',' ','s','u','e',241,'o'}};
const uint8_t awake[4][30] = {"Waking up...",{201,'v','e','i','l',' ','e','n',' ','c','o','u','r','s','.','.','.'},"Aufwachen...","Despertando..."};

typedef enum {
    OLEDWakePrep = 0,
    OLEDWakeIntro = 1,
    OLEDWakeDialog1 = 2,
    OLEDWakeDialog2 = 3,
    OLEDWakeClean = 4,
    OLEDWakeOutro = 5,
} OLEDWakeStates;

typedef enum {
    LowBatEmpty = 0,
    LowBatLow   = 1,
    LowBatDown  = 2,
    LowBatUp    = 3,
    LowBatStVZO = 4,
} LowBatStates;

static void prvPersonalityTask(void *pvParameters);
static void prvTimerCallbackRx(TimerHandle_t xTimer);
static void prvTimerCallbackStvzoIndicator(TimerHandle_t xTimer);
static void updateOLEDReactionState(uint8_t reaction, uint8_t state);
static void pairedStateUpdateOled(bool isPaired);

/**
 * @brief Initialize the Personality task
 */
void init_PersonalityTask()
{
    if (xTimerRx == NULL) {
        xTimerRx = xTimerCreateStatic(
            "PersonalityTimer", TIMER_IN_MILLISECONDS / portTICK_PERIOD_MS,
            pdTRUE, (void *)0, prvTimerCallbackRx, &xTimerBufferRx);
        configASSERT(xTimerRx);
        storeTimer(xTimerRx);
    }

    if (xTimerStvzoIndicator == NULL) {
        xTimerStvzoIndicator = xTimerCreateStatic("StvzoLowBatIndicatorTimer", STVZO_TIMER_IN_MILLISECONDS / portTICK_PERIOD_MS,
                pdFALSE, (void *) 0, prvTimerCallbackStvzoIndicator, &xTimerBufferStvzoIndicator);
        configASSERT(xTimerStvzoIndicator);
    }
    xTimerStart(xTimerStvzoIndicator, 200);

    if (selfHandle == NULL) {
        selfHandle = xTaskCreateStatic(prvPersonalityTask, TASKNAME_PERSONALITY, STACK_SIZE, NULL, TASK_PRIORITY, PersonalityStack, &xTaskBuffer);
        configASSERT(selfHandle);
    }
}

static void compileTapCode(char * newCode, uint8_t code, uint8_t length){
    for(uint8_t i=0; i<length && i<MAX_TAP_LENGTH; i++){
        newCode[i] = code >> i & 0x1 ? 'L' : 'S';
    }
}

static void showWake(){
    if(isTestMode_SystemUtilities() || onboarding)
        return;
    firstMove = false;
    startUp = false;
    connectedNotify = false;
    wakeCount = 1;
    updateOLEDReactionState(oled_reaction_wake,0);
}

static void showDisconnect(){
    startAnimation_GraphicsTask(HaloAnimation_display_disconnect,NULL);
    updateOLEDReactionState(oled_reaction_bye,0);
}

static void setSleepCode(uint16_t length, const void * payload){
    if(length < 2){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }

    struct SleepModePayload * sleepModePayload = (struct SleepModePayload *) payload;

    if(sleepModePayload->length > 4){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    sleepMode.code = sleepModePayload->codeBitmap;
    sleepMode.length = sleepModePayload->length;
    memset(sleepMode.codeString,0,MAX_TAP_LENGTH);
    compileTapCode(sleepMode.codeString,sleepMode.code,sleepMode.length);
    writeFile_SystemUtilities(sleepConfigFile, sleepModePayload, sizeof(struct SleepModePayload));

    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void logoAnimation(uint16_t u16MessageLength, const void * pu8Data){
    if(firstMove || wakeCount >= 54)
        showWake();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void clockAnimation(uint16_t length, const void * payload){
    if(length < 13){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct ClockQuickPayload * clockPayload = payload;
    clockData.duration = __REV16(clockPayload->duration);
    clockData.fade = clockPayload->fade;
    clockData.hour = clockPayload->hour;
    clockData.hourHsv = clockPayload->hourHsv;
    clockData.intro = clockPayload->intro;
    clockData.minute = clockPayload->minute;
    clockData.minuteHsv = clockPayload->minuteHsv;
    clockData.is24Hour = clockPayload->is24Hour;
    clockData.pulse = clockPayload->pulse;
    displayClock = true;
    if(!onboarding)
        startAnimation_GraphicsTask(HaloAnimation_display_clock,(uint8_t *) &clockData);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void clockAnimationOff(uint16_t u16MessageLength, const void * payload){
    clockOff_GraphicsTask();
    displayClock = false;
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void disconnectAnimation(uint16_t u16MessageLength, const void * pu8Data){
    showDisconnect();
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void finishCode(uint8_t code, uint8_t len){
    memset(finishedCode,0,sizeof(finishedCode));
    compileTapCode(finishedCode,code,len);
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
    newAccData = true;
    accStreamStarted = true;
}

static void checkAccelerometerData(){
    hpfX = HPF_ALPHA*(hpfX + X - previousX);
    hpfY = HPF_ALPHA*(hpfY + Y - previousY);
    hpfZ = HPF_ALPHA*(hpfZ + Z - previousZ);
    float vector = sqrt(hpfX*hpfX + hpfY*hpfY + hpfZ*hpfZ);
    if(vector > MOVEMENT_THRESHOLD){
        isInMovement = true;
        if(firstMove && wakeCount == 0) showWake();
    }
    newAccData = false;
}

static void accInterrupt(){
    if(!accTrackCount)
        subscribeToAccData_SensorsTask(&accRawData);
    accTrackCount = 1;
    interruptCount++;
}

static void clearOnboarding(){
    onboarding = onboardingTap = onboardingLeft = onboardingRight = false;
    onboardingCount = onboardingSwipeCount = onboardingAnimationCount = explainAnimationCount = 0;
    unlockAnimations_GraphicsTask(TASKNAME_PERSONALITY);
    unlockSounds_SoundTask(TASKNAME_PERSONALITY);
    unlockTaps_SensorsTask(TASKNAME_PERSONALITY);
    unlockSwipes_SensorsTask(TASKNAME_PERSONALITY);
    unlockStatusWidth_OLEDLibrary();
}

static void triggerCheck(oledDirections_e direction){
    onboardingCheck = true;
    onboardingAnimationCount = 0;
    oled_cmd_t cmd = {
            .animation = direction != oled_no_animation ? true : false,
            .animationTypeMask = direction != oled_no_animation ? slidingAnimation : 0,
            .direction = direction,
            .statusBarPosition = -1,
            .animationTime = 300
    };
    memset(drawingBoard,0,sizeof(drawingBoard));
    appendAssetToDisplay_OLEDLibrary(drawingBoard,checkAsset,checkLayout.location[0]);
    setPriorityImageWithCmd_GraphicsTask(drawingBoard, &cmd, PERSONALITY_OLED_PRIORITY, 4000);
    struct AngleAnimationData angle = {
               .colour = {.h=67,.s=255,.v=255},
               .angle = 0,
               .background = false,
               .width = 360,
               .progress = 100,
               .repeat = 4,
       };
    uint8_t tapData[2] = {0, 0};
    startAnimationWithLock_GraphicsTask(HaloAnimation_display_taps, tapData, TASKNAME_PERSONALITY);
    startAnimationWithLock_GraphicsTask(HaloAnimation_display_angle,(uint8_t *) &angle, TASKNAME_PERSONALITY);
}

static void tapData(uint8_t code, uint8_t len, bool adjusted){
    if(len < tapLength)
        if(adjusted)
            finishCode(code, len);
        else
            finishCode(tapCode, tapLength);
    else if(len != tapLength || code != tapCode )
        newTap = (code >> (len-1)) + 1;
    tapUpdate = true;
    tapCode = code;
    tapLength = len;
    if(!tapLength || adjusted){
        setPriorityImageTime_GraphicsTask(PERSONALITY_OLED_PRIORITY_LOW, 0);
        touchCount = 0;
    }
}

static void releaseState(bool isReleased){
    if(isReleased && getIsPaired_SystemUtilities()){
        static uint8_t touchDisplay[1024];
        memset(touchDisplay,0,sizeof(touchDisplay));
        appendAssetToDisplay_OLEDLibrary(touchDisplay,
                touchAsset,
                touchLayout.location[0]);
        oled_cmd_t oledCMD = {
                .statusBarPosition = -1,
                .direction = oled_no_animation,
                .animation = false,
                .u8ImageCategory = oled_reactions,
                .u8ImageType = 0
        };
        if(tapLength){
            if(onboardingTap && onboardingCount >= ONBOARDING_INTERACTION_TICK-1 && !tapCode){
                triggerCheck(oled_no_animation);
                onboardingTap = false;
            }else{
                setPriorityImageWithCmd_GraphicsTask(touchDisplay, &oledCMD, PERSONALITY_OLED_PRIORITY_LOW, 10000);
            }
        }
    }
}

static void swipeData(touch_swipe_state_t swipeState){
    setPriorityImageTime_GraphicsTask(PERSONALITY_OLED_PRIORITY_LOW, 0);
    clockOff_GraphicsTask();
    if(onboarding){
        if(!onboardingTap && !onboardingCheck){
            if(onboardingRight){
                if(swipeState.direction == oled_right){
                    triggerCheck(oled_right);
                    onboardingRight = false;
                }
            }else if(onboardingLeft){
                if(swipeState.direction == oled_left){
                    triggerCheck(oled_left);
                    onboardingLeft = false;
                }
            }else if((onboardingCount >= ONBOARDING_INTERACTION_TICK && swipeState.direction == oled_left)
                    || (onboardingCount >= ONBOARDING_INTERACTION_TICK+1 && swipeState.direction == oled_right)){
                onboardingAnimationCount = 0;
                explainAnimationCount = 0;
                onboardingExplainDirection = swipeState.direction;
                if(swipeState.direction == oled_left){
                    onboardingSwipeCount++;
                    onboardingCount++;
                }else{
                    onboardingSwipeCount--;
                    onboardingCount--;
                }
            }
        }
    }
}

/**
 * @brief       updateOLEDReactionState()
 * @details		Function to set up a reaction state on the OLED.
 * @private
 */
static void updateOLEDReactionState(uint8_t reaction, uint8_t state){
    if(getRawBatteryVoltage_SystemUtilities() < 3300)
        return;
    oled_cmd_t oledCMD = {
            .statusBarPosition = -1,
            .direction = oled_no_animation,
            .animation = false,
            .u8ImageCategory = oled_reactions,
            .u8ImageType = reaction
    };
    uint16_t duration = STARTUP_DISPLAY_TIMEOUT*TIMER_IN_MILLISECONDS;
    uint8_t priority = PERSONALITY_OLED_PRIORITY;
    Localization_E locale = getLocale_SystemUtilities();
    switch(reaction){
        default:
        case oled_reaction_startup:
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    greetingAsset,
                    greetingLayout.location[0]);
            break;
        case oled_reaction_connected:
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    phoneAsset,
                    connectionLayout.location[0]);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    state ? connection2Asset : connection1Asset,
                            connectionLayout.location[1]);
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    haloAsset,
                    connectionLayout.location[2]);
            break;
        case oled_reaction_unpaired:
            duration = 500;
            priority = PERSONALITY_OLED_PRIORITY_LOW;
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    phoneAsset,
                    disconnectionLayout.location[0]);
            if(state){
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        unpairedAsset,
                        disconnectionLayout.location[1]);
            }
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    haloAsset,
                    disconnectionLayout.location[2]);
            break;
        case oled_reaction_lowBattery:
            memset(drawingBoard,0,sizeof(drawingBoard));
            if(state==LowBatDown){
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, PERSONALITY_OLED_PRIORITY, duration);
                oledCMD.animation = true;
                oledCMD.direction = oled_down;
                oledCMD.animationTime = 300;
                oledCMD.animationTypeMask = slidingAnimation;
            }
            if(state == LowBatStVZO){
                duration = STVZO_TIMER_IN_MILLISECONDS * 2; //timer is shorter than animation to ensure no blinking in oled 
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        lowBatteryEmptyAsset,
                        lowBatteryLayout.location[0]);
                priority = STVZO_OLED_PRIORITY;
            }
            else{
                duration = 1000;
                appendAssetToDisplay_OLEDLibrary(drawingBoard,
                        state ? lowBatteryAsset : lowBatteryEmptyAsset,
                        lowBatteryLayout.location[0]);
            }
            if(state==LowBatUp){
                setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, PERSONALITY_OLED_PRIORITY, duration);
                oledCMD.animation = true;
                oledCMD.direction = oled_up;
                oledCMD.animationTime = 300;
                oledCMD.animationTypeMask = slidingAnimation;
                memset(drawingBoard,0,sizeof(drawingBoard));
            }
            break;
        case oled_reaction_wake:{
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    wakeAsset,
                    wakeLayout.location[0]);
            switch(state){
                case OLEDWakePrep:
                    memset(drawingBoard,0,sizeof(drawingBoard));
                    startAnimation_GraphicsTask(HaloAnimation_display_logo,NULL);
                    break;
                case OLEDWakeIntro:
                    oledCMD.direction = oled_right;
                    oledCMD.animation = true;
                    oledCMD.animationTime = 300;
                    oledCMD.animationTypeMask = slidingAnimation;
                    break;
                case OLEDWakeDialog1:
                    alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t *)welcome[locale], wakeLayout.location[1].y);
                    break;
                default:
                case OLEDWakeClean:
                    break;
                case OLEDWakeOutro:
                    memset(drawingBoard,0,sizeof(drawingBoard));
                    oledCMD.direction = oled_right;
                    oledCMD.animation = true;
                    oledCMD.animationTime = 300;
                    oledCMD.animationTypeMask = slidingAnimation;
                    duration = 1000;
                    break;
            }
            break;
        }
        case oled_reaction_connectEn:
            memset(drawingBoard,0,sizeof(drawingBoard));
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction1[0], ONBOARDING_TOP_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction2[0], ONBOARDING_MIDDLE_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction3[0], ONBOARDING_BOTTOM_LINE);
            break;
        case oled_reaction_connectFr:
            memset(drawingBoard,0,sizeof(drawingBoard));
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction1[1], ONBOARDING_TOP_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction2[1], ONBOARDING_MIDDLE_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction3[1], ONBOARDING_BOTTOM_LINE);
            break;
        case oled_reaction_connectDe:
            memset(drawingBoard,0,sizeof(drawingBoard));
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction1[2], ONBOARDING_TOP_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction2[2], ONBOARDING_MIDDLE_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction3[2], ONBOARDING_BOTTOM_LINE);
            break;
        case oled_reaction_connectSp:
            memset(drawingBoard,0,sizeof(drawingBoard));
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction1[3], ONBOARDING_TOP_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction2[3], ONBOARDING_MIDDLE_LINE);
            alignPixellari16TextCentered_OLEDLibrary(drawingBoard, connect_instruction3[3], ONBOARDING_BOTTOM_LINE);
            break;
        case oled_reaction_bye:
            duration = 1500;
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,
                    byeAsset,
                    byeLayout.location[0]);
            break;
        case oled_reaction_onboardingWelcome:
            memset(drawingBoard,0,sizeof(drawingBoard));
            //To override the halo
            if(state == 0){
                lockSounds_SoundTask(TASKNAME_PERSONALITY);
                lockTaps_SensorsTask(tapData,TASKNAME_PERSONALITY);
                lockSwipes_SensorsTask(swipeData,TASKNAME_PERSONALITY);
                startAnimationWithLock_GraphicsTask(HaloAnimation_display_logo,NULL,TASKNAME_PERSONALITY);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_intro1[locale], ONBOARDING_SECOND_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_intro2[locale], ONBOARDING_THIRD_LINE);
            }else{
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_setup1[locale], ONBOARDING_SECOND_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_setup2[locale], ONBOARDING_THIRD_LINE);
            }
            break;
        case oled_reaction_onboardingTap:
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,haloAsset,tapLayout.location[0]);
            if(state)
                appendAssetToDisplay_OLEDLibrary(drawingBoard, tapAsset, tapLayout.location[1]);
            break;
        case oled_reaction_onboardingRight:
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,haloAsset,rightSwipeLayout.location[0]);
            if(state)
                appendAssetToDisplay_OLEDLibrary(drawingBoard, rightSwipeAsset, rightSwipeLayout.location[1]);
            break;
        case oled_reaction_onboardingLeft:
            memset(drawingBoard,0,sizeof(drawingBoard));
            appendAssetToDisplay_OLEDLibrary(drawingBoard,haloAsset,leftSwipeLayout.location[0]);
            if(state)
                appendAssetToDisplay_OLEDLibrary(drawingBoard, leftSwipeAsset, leftSwipeLayout.location[1]);
            break;
        case oled_reaction_onboardingExplain:
            memset(drawingBoard,0,sizeof(drawingBoard));
            oledCMD.direction = explainAnimationCount < 100 ? onboardingExplainDirection : oled_no_animation;
            oledCMD.animationTime = 200;
            oledCMD.animation = true;
            oledCMD.animationTypeMask = slidingAnimation;
            oledCMD.statusBarPosition = state;
            if(state == 0){
                if(explainAnimationCount < 100 && onboardingExplainDirection == oled_right){
                    oledCMD.direction = oled_right;
                    oledCMD.animation = true;
                }else{
                    oledCMD.direction = oled_no_animation;
                    oledCMD.animation = false;
                }
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction1_1[locale], ONBOARDING_TOP_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction2_1[locale], ONBOARDING_MIDDLE_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction3_1[locale], ONBOARDING_BOTTOM_LINE);
            }else if(state == 1){
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction1_2[locale], ONBOARDING_TOP_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction2_2[locale], ONBOARDING_MIDDLE_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction3_2[locale], ONBOARDING_BOTTOM_LINE);
            }else if(state == 2){
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction1_3[locale], ONBOARDING_TOP_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction2_3[locale], ONBOARDING_MIDDLE_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction3_3[locale], ONBOARDING_BOTTOM_LINE);
            }else if(state == 3){
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction1_4[locale], ONBOARDING_TOP_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction2_4[locale], ONBOARDING_MIDDLE_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction3_4[locale], ONBOARDING_BOTTOM_LINE);
            }else if(state == 4){
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction1_5[locale], ONBOARDING_TOP_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction2_5[locale], ONBOARDING_MIDDLE_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction3_5[locale], ONBOARDING_BOTTOM_LINE);
            }else if(state == 5){
                oledCMD.statusBarPosition = -1;
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction1_6[locale], ONBOARDING_SECOND_LINE);
                alignPixellari16TextCentered_OLEDLibrary(drawingBoard, onboarding_instruction2_6[locale], ONBOARDING_THIRD_LINE);
                startAnimationWithLock_GraphicsTask(HaloAnimation_display_angle_intro, NULL, TASKNAME_PERSONALITY);
                duration = 3000; //show carousel after 3 seconds
                clearOnboarding();
            }
            break;
    }
    if(!isTestMode_SystemUtilities())
        setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, priority, duration);
}

static void initiateConnectionStateTimer(){
    startUp = false;
    connectedNotify = true;
    tickCount = 0;
}

static void authenticationStateUpdate(bool state){
    firstMove = state;
    wakeCount = state ? 0 : 60;
    if(isAuthenticated && !state && getStandbyReason_CommunicationTask() == StandbyOff){
        showDisconnect();
        clearOnboarding();
    }
    isAuthenticated = state;
    initiateConnectionStateTimer();
}

static void pairedStateUpdateOled(bool isPaired)
{
    if(isPaired && !wakeCount){
        updateOLEDReactionState(oled_reaction_connected,0);
    }
}

//low battery indicator used to pass the german stvzo certifications
static void stvzoLowBatteryIndicator(){
    if((getRawBatteryVoltage_SystemUtilities() <= LOW_VOLTAGE_INDICATOR) && !getIsChargingAnimationAllowed_SystemUtilities() 
                  && isInMovement){
        updateOLEDReactionState(oled_reaction_lowBattery,LowBatStVZO);
    }
        isInMovement = false;
        xTimerStart(xTimerStvzoIndicator, 200);
}

static void lowBattery(uint16_t length, const void * payload){
    HsvColour_t lowBatteryWarning = {.h = 253, .s =226, .v =255};
    struct AngleAnimationData lowBatteryDisplay = {
        .colour = lowBatteryWarning,
        .angle = 0,
        .background = false,
        .width = 360,
        .progress = 100,
        .repeat = 7
    };
    startAnimationWithLock_GraphicsTask(HaloAnimation_display_angle,(uint8_t *) &lowBatteryDisplay,TASKNAME_PERSONALITY);
    lowBatteryCount = 1;
    updateOLEDReactionState(oled_reaction_lowBattery,LowBatDown);
    const struct LowBatteryPayload * warningData = payload;
    uint8_t freqCount = 4;
    uint16_t frequencies[] = {493,349,138,0};
    uint16_t durations[] = {55,82,106,0};
    bool sweeps[] = {false,false,false,false};
    uint8_t repeats = 0;
    uint8_t volume = warningData->volume;
    compileSound_SoundTask(frequencies, durations, sweeps, volume, freqCount, repeats);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void touchSoundEnable(uint16_t length, const void * payload){
    if(length < 2){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct TouchSoundsPayload * settings = payload;
    touchSettings.volume = settings->volume > 100 ? 100 : settings->volume;
    touchSettings.enabled = settings->enabled;
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void shortTapSound()
{
    sound_cmd_t soundCmd ={
        .duration ={100},
        .singleEdge = true,
        .volume = touchSettings.volume,
        .nbr_seq = 1
    };
    if(onboarding)
        setSoundWithLock_SoundTask(&soundCmd, TASKNAME_PERSONALITY);
    else
        setSound_SoundTask(&soundCmd);
}

static void longTapSound()
{
    //Injecting two brief silence sounds around the tap sound
    //this seems to make the sound consistent every time.
    sound_cmd_t soundCmd ={
        .duration ={10,2,10},
        .singleEdge = false,
        .freq = {0,980,0},
        .volume = touchSettings.volume,
        .nbr_seq = 3
    };
    if(onboarding)
        setSoundWithLock_SoundTask(&soundCmd, TASKNAME_PERSONALITY);
    else
        setSound_SoundTask(&soundCmd);
}

static void startOnboarding(uint16_t length, const void * payload){
    onboarding = onboardingTap = onboardingLeft = onboardingRight = true;
    onboardingCount = onboardingSwipeCount = onboardingAnimationCount = explainAnimationCount = 0;
    updateStatusWidthLocked_OLEDLibrary(6);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

static void prvPersonalityTask(void *pvParameters) {
    xTimerStart(xTimerRx, 200);

    subscribeToAccInterrupt_SensorsTask(accInterrupt);
    subscribeToConnectionState_CommunicationTask(authenticationStateUpdate);
    subscribeToTaps_SensorsTask(tapData);
    subscribeToRelease_SensorsTask(releaseState);
    subscribeToSwipes_SensorsTask(swipeData);
    subscribeToPairedState_CommunicationTask(pairedStateUpdateOled);

    assignFunction_CommunicationTask(COM_UI, UI_LOGO, &logoAnimation);
    assignFunction_CommunicationTask(COM_UI, UI_DISCONNECT, &disconnectAnimation);
    assignFunction_CommunicationTask(COM_UI, UI_CLOCK, clockAnimation);
    assignFunction_CommunicationTask(COM_UI, UI_CLOCK_OFF, clockAnimationOff);
    assignFunction_CommunicationTask(COM_UI, UI_LOWBAT, lowBattery);
    assignFunction_CommunicationTask(COM_SOUND, SOUND_TAP_EN, &touchSoundEnable);
    assignFunction_CommunicationTask(COM_UI, UI_ONBOARDING, startOnboarding);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_SLEEP, setSleepCode);

    struct SleepModePayload savedSleepPayload;
    if(readFile_SystemUtilities(sleepConfigFile,&savedSleepPayload,sizeof(struct SleepModePayload))){
        memset(sleepMode.codeString,0,MAX_TAP_LENGTH);
        sleepMode.code = savedSleepPayload.codeBitmap;
        sleepMode.length = savedSleepPayload.length;
        compileTapCode(sleepMode.codeString,sleepMode.code,sleepMode.length);
    }else{
        log_Shell("No sleeping here!");
    }

    if(!readFile_SystemUtilities(pairedFile, &pairedState, sizeof(pairedState))){
        pairedState = false;
    }

    if(!pairedState){
        updateOLEDReactionState(oled_reaction_startup,0);
        startUpReaction = 1;
    }

    bool savedSleepMode;
    if(readFile_SystemUtilities(sleepFile, &savedSleepMode, 1))
        sleepMode.engaged = savedSleepMode;

    while (1) {
        if(displayClock){
            uint8_t clockDisplay[1024];
            memset(clockDisplay,0,sizeof(clockDisplay));
            uint8_t time[5] = "00000";
            uint8_t hour = clockData.hour&0x7f;
            int8_t offset;
            if(clockData.is24Hour){
                offset = 0;
                itoa(hour,(char *)(hour < 10 ? &time[1+offset] : &time[0]),10);
            }else{
                hour = hour%12 == 0 ? 12 : hour%12;
                offset = hour < 10 ? -1 : 0;
                itoa(hour,(char *)(&time[0]),10);
                alignPixellari16TextCentered_OLEDLibrary(clockDisplay, (uint8_t*)((clockData.hour) >= 12 ? "PM" : "AM"), BASE_STRING_Y_POSITION);
            }
            time[2+offset] = ':';
            itoa(clockData.minute,(char *)(clockData.minute < 10 ? &time[4+offset] : &time[3+offset]),10);
            alignRubik43Centered_OLEDLibrary(clockDisplay, time ,UNITS_Y_POSITION);
            oled_cmd_t oledCMD = {
                    .statusBarPosition = -1,
                    .direction = oled_no_animation,
                    .animation = false,
                    .u8ImageCategory = oled_reactions,
                    .u8ImageType = 0
            };
            setPriorityImageWithCmd_GraphicsTask(clockDisplay, &oledCMD, PERSONALITY_OLED_PRIORITY_LOW, clockData.duration);
            displayClock = false;
        }

        if(tapUpdate){
            if(tapLength <= 5 && tapUpdate){
                uint8_t tapData[2] = {tapCode, tapLength};
                if(onboarding && !onboardingCheck)
                    startAnimationWithLock_GraphicsTask(HaloAnimation_display_taps, tapData, TASKNAME_PERSONALITY);
                else
                    startAnimation_GraphicsTask(HaloAnimation_display_taps, tapData);
            }
            if(touchSettings.enabled && newTap && isAuthenticated){
                if(newTap == 1){
                    shortTapSound();
                }else{
                    longTapSound();
                }
                newTap = 0;
            }
            tapUpdate = false;
        }

        if(finishedCodeLength){
            if(sleepMode.length != 0 && !strcmp(finishedCode,sleepMode.codeString)){
                eStandbyReason_t currentReason = getStandbyReason_CommunicationTask();
                Localization_E locale = getLocale_SystemUtilities();
                if(sleepMode.engaged && currentReason == StandbySleep){
                    toggleStandby_CommunicationTask(StandbyOff);
                    sleepMode.engaged = false;
                    setPriorityImageTime_GraphicsTask(PERSONALITY_OLED_PRIORITY,0);
                    uint8_t tapData[2] = {0,0};
                    startAnimation_GraphicsTask(HaloAnimation_display_shadow_taps, tapData);
                    memset(drawingBoard,0,sizeof(drawingBoard));
                    alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t *)awake[locale], CENTERED_TEXT_Y);
                    oled_cmd_t oledCMD = {
                            .statusBarPosition = -1,
                            .direction = oled_no_animation,
                            .animation = false,
                            .u8ImageCategory = oled_reactions,
                            .u8ImageType = 0
                    };
                    setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, PERSONALITY_OLED_PRIORITY, AWAKE_OLED_HUD_MILLISECOND_TIMEOUT);
                }else if(currentReason == StandbyOff && isAuthenticated){
                    toggleStandby_CommunicationTask(StandbySleep);
                    sleepMode.engaged = true;
                    memset(drawingBoard,0,sizeof(drawingBoard));
                    appendAssetToDisplay_OLEDLibrary(drawingBoard, moonAsset, sleepLayout.location[0]);
                    alignPixellari16TextCentered_OLEDLibrary(drawingBoard, (uint8_t *)sleep[locale], BASE_TITLE_Y);
                    oled_cmd_t oledCMD = {
                            .statusBarPosition = -1,
                            .direction = oled_no_animation,
                            .animation = false,
                            .u8ImageCategory = oled_reactions,
                            .u8ImageType = 0
                    };
                    setPriorityImageWithCmd_GraphicsTask(drawingBoard, &oledCMD, PERSONALITY_OLED_PRIORITY, SLEEP_OLED_HUD_MILLISECOND_TIMEOUT);
                    sleepMode.instructionTick = 20*5 - 6*5;
                }
                writeFile_SystemUtilities(sleepFile, &sleepMode.engaged, 1);
            }
            finishedCodeLength = 0;
        }

        if(newAccData)
            checkAccelerometerData();

        vTaskDelay(30);
        shwd_KeepAlive(eSHWD_PersonalityTask);
    }
}

static void onboardingDemonstration(){
    if(onboardingCheck){
        onboardingCheck = onboardingAnimationCount < ONBOARDING_CHECK_TIMEOUT;
    }else{
        if(onboardingTap)
            updateOLEDReactionState(oled_reaction_onboardingTap,onboardingAnimationCount%20<10);
        else if(onboardingRight)
            updateOLEDReactionState(oled_reaction_onboardingRight,onboardingAnimationCount%20<10);
        else if(onboardingLeft)
            updateOLEDReactionState(oled_reaction_onboardingLeft,onboardingAnimationCount%20<10);
        else
            onboardingCount++;
    }
}

static void displayOLEDWake(){
   updateOLEDReactionState(oled_reaction_onboardingTap,1);
   uint8_t tapData[2] = {sleepMode.code, sleepMode.length};
   startAnimation_GraphicsTask(HaloAnimation_display_shadow_taps, tapData);
}

static void fifty_milliseconds_update()
{
    timer_waiting = false;

    if(startUp || connectedNotify){
        tickCount++;
        uint16_t timeout = startUp ? STARTUP_DISPLAY_TIMEOUT : CONNECTION_DISPLAY_TIMEOUT;

        if(tickCount == timeout){
            if(startUp && getIsPlugged_SystemUtilities() && !pairedState) {
                startUpReaction = startUpReaction == 5 ? 2 : startUpReaction + 1;
                updateOLEDReactionState(startUpReaction,0);
            }else{
                startUp = false;
                connectedNotify = false;
            }
            tickCount = 0;
        }
    }else if(lowBatteryCount){
        lowBatteryCount++;
        if(lowBatteryCount > 20 && lowBatteryCount < 90)
            updateOLEDReactionState(oled_reaction_lowBattery,lowBatteryCount%20<10);
        if(lowBatteryCount == 90){
            updateOLEDReactionState(oled_reaction_lowBattery,LowBatUp);
            HsvColour_t lowBatteryWarning = {.h = 253, .s =226, .v =255};
            struct AngleAnimationData lowBatteryDisplay = {
                .colour = lowBatteryWarning,
                .angle = 0,
                .background = false,
                .width = 360,
                .progress = 0,
                .repeat = 0
            };
            startAnimationWithLock_GraphicsTask(HaloAnimation_display_angle,(uint8_t *) &lowBatteryDisplay,TASKNAME_PERSONALITY);
        }
        if(lowBatteryCount>= 101){
            lowBatteryCount = 0;
            anglesOff_GraphicsTask();
            unlockAnimations_GraphicsTask(TASKNAME_PERSONALITY);
        }
    }

    if(!firstMove && wakeCount < 54){
        if(wakeCount == 53){
            updateOLEDReactionState(oled_reaction_wake,OLEDWakeOutro);
        }else if(wakeCount == 52){
            updateOLEDReactionState(oled_reaction_wake,OLEDWakeClean);
        }else if(wakeCount == 22){
            updateOLEDReactionState(oled_reaction_wake,OLEDWakeDialog1);
        }else if(wakeCount == 2){
            updateOLEDReactionState(oled_reaction_wake,OLEDWakeIntro);
        }else if(wakeCount == 1){
            updateOLEDReactionState(oled_reaction_wake,OLEDWakePrep);
        }
        wakeCount++;
    }

    if(onboarding){
        if(onboardingCount <= ONBOARDING_INTERACTION_TICK-2) onboardingCount++;
        if(onboardingCount == ONBOARDING_START_50MS_TICKS){
            updateOLEDReactionState(oled_reaction_onboardingWelcome,0);
        }else if(onboardingCount == ONBOARDING_WELCOME_50MS_TICKS){
            updateOLEDReactionState(oled_reaction_onboardingWelcome,1);
        }else if(onboardingCount == ONBOARDING_INSTRUCTION_50MS_TICKS){
            onboardingDemonstration();
        }else if(onboardingCount == ONBOARDING_INTERACTION_TICK+onboardingSwipeCount){
            if(explainAnimationCount < 100)
                explainAnimationCount++;
            if(explainAnimationCount == 1 || explainAnimationCount == 100)
                updateOLEDReactionState(oled_reaction_onboardingExplain,onboardingSwipeCount);
            if(explainAnimationCount == 100) //Resending every 5 seconds
                explainAnimationCount = 1;
        }
        if(onboardingCount >= ONBOARDING_INSTRUCTION_50MS_TICKS){
            onboardingAnimationCount++;
            if(onboardingAnimationCount == 240)
                onboardingAnimationCount = 0;
        }
    }

    if(tapLength){
        if(!getIsPaired_SystemUtilities()){
            updateOLEDReactionState(oled_reaction_unpaired,touchCount%20<10);
            touchCount++;
            //Prevent roll over which will cause a change in the frequency.
            if(touchCount == 240)
                touchCount = 0;
        }
    }

    if(accTrackCount){
        if(sleepMode.engaged){
            if(sleepMode.instructionTick == 0 && interruptCount > 15){
                displayOLEDWake();
                interruptCount =- 5;
                if(interruptCount < 15)
                    interruptCount = 0;
            }
            sleepMode.instructionTick++;
            if(sleepMode.instructionTick > 20*5){
                sleepMode.instructionTick = 0;
            }
        }
        accTrackCount++;
        if(accTrackCount == 20*10){
            accTrackCount = 0;
            sleepMode.instructionTick = 0;
            unsubscribeToAccData_SensorsTask(accRawData);
            accStreamStarted = false;
            setPriorityImageTime_GraphicsTask(PERSONALITY_OLED_PRIORITY_LOW,0);
            uint8_t tapData[2] = {0,0};
            startAnimation_GraphicsTask(HaloAnimation_display_shadow_taps, tapData);
        }
    }
}

static void prvTimerCallbackRx(TimerHandle_t xTimer) {
    static int timeout = 0;
    if (timer_waiting == false || timeout == 0) {
        timer_waiting = true;
        fifty_milliseconds_update();
        timeout = 20;
    } else {
        log_Shell("Skip %d ms", TIMER_IN_MILLISECONDS);
        timeout--;
    }
}

static void prvTimerCallbackStvzoIndicator(TimerHandle_t xTimer) {
    stvzoLowBatteryIndicator();
}

// ------------------------------------------------------------------------------------------------
/*!@file    SystemUtilitiesTask.c
 */
// ------------------------------------------------------------------------------------------------
/*!@file    systemUtilitiesTask.c
 */
// ------------------------------------------------------------------------------------------------
#include <inttypes.h>

#include <CommunicationTask.h>
#include <qspiflash.h>
#include <SoundTask.h>
#include <string.h>
#include <errno.h>
#include <GraphicsTask.h>
#include <AnimationsLibrary.h>
#include "OLEDLibrary.h"
#include "HaloLedsDriver.h"
#include "SensorsTask.h"
#include "BootLoaderImport.h"
#include "FirmwareUpdate.h"
#include "PhotoSensor.h"
#include "ShellComPort.h"
#include "Power.h"
#include "WatchdogTask.h"
#include "assets.h"

/* API ---------------- */
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"
#include "adc.h"
#include "usart.h"
#include "quadspi.h"
#include <spiffs.h>
#include <SystemUtilitiesTask.h>
#include "tim.h"
#include "math.h"
#include "crc.h"
#include "reboot.h"
#include "iwdg.h"
#include "FSUtil.h"
#include "SHftp.h"
#include "tsl_user.h"
#include "SHTimers.h"
#include <device_telemetry.h>
#include "SHTaskUtils.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define TASK_PRIORITY 	PRIORITY_NORMAL
#define STACK_SIZE      configMINIMAL_STACK_SIZE + 0x300
#define QUEUE_LENGTH    20

#define NUM_ADC_1 2
#define VREFINT 1.212f

#define COUNTER_250ms           5
#define COUNTER_2s              40
#define LOW_BATT_SHUTDOWN_LEVEL 20

#define FREE_HEAP_CRITICAL_MIN     10            // Enable check of Free Heap
#define PRINTF_CHAR_SIZE (CONSOLE_MAX_DATA_SIZE-10)

#define PB_DELAY		1000

#define LOG_PAGE_SIZE       256

#define CONFIG_HEADER_SIZE 9
static uint8_t PRODUCT[CONFIG_HEADER_SIZE] = "SmartHalo";
static uint8_t SERIAL[CONFIG_HEADER_SIZE] = "SH2-Halo2";
static uint16_t KEY = 0x1337;

#define TIMER_IN_MILLISECONDS 50

#define NUMBER_OF_IMAGES_TITLES 7

#define DELAY_BEFORE_VOLTAGE_SHUTDOWN (5*32)      // 5 50ms ticks, 32 times as a precaution
#define STARTUP_DISPLAY_TIMEOUT (3 * 20)          // 3 seconds * 20 50ms ticks
#define CONNECTION_DISPLAY_TIMEOUT (5 * 20)       // 3 seconds * 20 50ms ticks
#define STATE_OF_CHARGE_TIMEOUT (60 / 2)          // 60 seconds / 2s ticks
#define SHUTDOWN_TIMEOUT (8 * 60 * 60 * 4)        // 8 hours in 250ms ticks
#define FIRSTBOOT_SHUTDOWN_TIMEOUT (3 * 60 * 4)   // 3 minutes in 250ms ticks

#define SOC_ARRAY_SIZE 101
#define FRONT_LED_CURRENT_MA 325u
#define NUMBER_OF_LEDS 72.0f

#define MAX_CURRENT_ESTIMATE_GUESS 400
//Piecewise equations for determining the voltage drop due to current draw estimate
#define CURRENT_CONTINUITY_THRESHOLD 330
#define CURRENT_DRAW_M1 (298/330.f)
//Current Estimate per LED
#define LED_CURRENT_A0 1.57430001
#define LED_CURRENT_A1 -0.04652182
#define LED_CURRENT_A2 0.00037490

//Temperature Linear Voltage Drop Modifier
//This is linear because my empirical data was insufficient
//I have a feeling it is pretty inaccurate, but I can collect more data if needed
#define BASELINE_TEMPERATURE_OF_IMPACT 10
#define TEMPERATURE_VOLTAGE_MODIFIER 5
//Charging Estimate, limited charging data as well, but this approximation seems to align approximately
#define CHARGING_ESTIMATE 120
#define ESTIMATED_STATIC_CURRENT 30

#define MEDIUM_Y_POSITION 47
#define PERCENTAGE_Y_POSITION 11
#define PERCENTAGE_X_POSITION 114
#define STATE_OF_CHARGE_OLED_PRIORITY 0
#define STATE_OF_CHARGE_OLED_DURATION 3000

// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================

#define _READ_CHARGING() !HAL_GPIO_ReadPin(nCHG_GPIO_Port, nCHG_Pin)
#define _READ_PLUGGED() !HAL_GPIO_ReadPin(PRE_USB_RESET_GPIO_Port, PRE_USB_RESET_Pin)
#define _SET_PIN(Port, Pin, State) HAL_GPIO_WritePin(Port, Pin, State)
#define _READ_RAW_BAT_VOLTAGE() (1000*ADC_1_DATA[ADC_1_VBAT_AN_INDEX] * VCPU_AN * 2 / 4096)

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

typedef enum {
    ADC_1_VBAT_AN_INDEX = 1, /*1*/
    ADC_1_VREFINT_INDEX,
} io_self_adc_1_index_t;

typedef struct {
    uint8_t product[CONFIG_HEADER_SIZE];	// SmartHalo
    uint16_t keyNumber;
    uint8_t serial[CONFIG_HEADER_SIZE];	// SH2-Halo2
} product_config_t;

typedef struct {
    uint8_t 	status;
    uint8_t 	serial[8];
} __attribute__ ((packed)) SerialReply_t;

struct PairedState{
    bool paired;
};

struct LocalizationPayload{
    time_t time;
    char locale[2];
}__attribute__((packed));

// ================================================================================================
// ================================================================================================
//            PUBLIC VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================

const char        languageFile[]     = "Language";
const char        pairedFile[]       = "PairedState";
static const char bootCountFile[]    = "BootCount";
static const int  bootCountThreshold = 2;
static uint32_t   bootCount;

// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================

static struct PairedState pairedState;

/* Static task --------------- */
static TaskHandle_t selfHandle = NULL;
static StaticTask_t xTaskBuffer;
static StackType_t SysUtilStack[STACK_SIZE];

static product_config_t product_config = { 0 };

static volatile uint16_t ADC_1_DATA[NUM_ADC_1 + 2] = { 0xAAAA, 0, 0, 0xEEEE }; // DMA-Buffer
static volatile float VCPU_AN;

static volatile bool adc_ready = false;
static bool timer_waiting = false;

static SemaphoreHandle_t fs_Mutex = NULL;
static StaticSemaphore_t fs_Mutex_buffer;
static SemaphoreHandle_t fs_MountedMutex = NULL;
static StaticSemaphore_t fs_MountedMutexBuffer;
spiffs rootFS;
static bool fsIsMounted;
static u8_t spiffs_work_buf[LOG_PAGE_SIZE * 2];
static u8_t spiffs_fds[32 * 4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE + 32) * 4];

static Localization_E locale = english;
static const uint16_t thresholdsSOC[11] = {3570, 3609, 3655, 3680, 3724, 3767, 3829, 3899, 3968, 4034, 4113};
static int8_t temperature = 0;
static uint8_t  stateOfCharge = 100;
static uint16_t previousVoltage;
static eRebootReason_t needReboot = eRebootNot;
static uint8_t  rebootCountDown = 2;

static uint32_t shutdownCount = 0;
static uint32_t unpairedShutdownTimeout;
static bool isLightReady = false;
static bool isLightShowing = false;

static uint8_t pinTestCount = 0;
static uint16_t pinTestClock = 0;
static uint16_t pinTestPeriod = 0;
static uint16_t testPin;
static GPIO_TypeDef * testPort;

bool disableAutoPower;      // needs to be visible in Shell.c, HardwareTests.c

static uint8_t batteryState[] =
{ 0xFE, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0x1F, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0x03, 0x00, 0x00, 0x18, 0xFF, 0xFF, 0xFF, 0x1F, 0xFE, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x07, 0x00, 0x00, 0xFC, 0x07, 0x00};
static struct OLEDAsset batteryAsset = {.width=55,.height=29,.asset=batteryState};
static struct OLEDLayout batteryLayout = {.location[0]={.x=35,.y=8}};

static bool batteryFlash = false;
static uint8_t chargingProgress = 0;
static bool allowStateOfCharge = true;

extern const uint8_t leds[72];
extern const uint8_t frontLEDPercentage;

static uint8_t oledImage[1024] = {0};

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
static void fifty_milliseconds_update(void);
static void deviceHandleGetSerial(uint16_t length, uint8_t *data);
static void deviceHandleBootloader(uint16_t messageLength, uint8_t * data);
static void deviceHandleSetName(uint16_t messageLength, uint8_t * data);
static void deviceHandleSetShutdown(uint16_t u16Length, uint8_t *ignored);
static void deviceHandleInstallGolden(uint16_t u16Length, uint8_t *ignored);
static void authGetVersions(uint16_t messageLength, uint8_t * data);
static void prvSystemUtilitiesTask(void *pvParameters);
static void trackConnectionState(bool isConnected);
static void handleDeviceSetDate(uint16_t len, const void *payload);

static TimerHandle_t watchTimer;
static StaticTimer_t watchTimerBuffer;
static void watchCB(TimerHandle_t tt);

//static void tohex(unsigned char * in, size_t insz, char * out, size_t outsz);

void write_config();
bool read_config();
void my_spiffs_mount();

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/**
 * @brief Initialize the System Utilities
 * @details Setup the task and shell
 */
void init_SystemUtilities()
{
    if (selfHandle == NULL) {
        selfHandle = xTaskCreateStatic(prvSystemUtilitiesTask,
                                       TASKNAME_SYSTEMUTILITIES, STACK_SIZE, NULL,
                                       TASK_PRIORITY, SysUtilStack, &xTaskBuffer);
        configASSERT(selfHandle);
    }

    if (fs_MountedMutex == NULL) {
        fs_MountedMutex = xSemaphoreCreateMutexStatic(&fs_MountedMutexBuffer);
        xSemaphoreGive(fs_MountedMutex);
    }

    if (fs_Mutex == NULL) {
        fs_Mutex = xSemaphoreCreateMutexStatic(&fs_Mutex_buffer);
        configASSERT(fs_Mutex);
        xSemaphoreGive(fs_Mutex);
    }

    if (watchTimer == NULL) {
        watchTimer = xTimerCreateStatic("watchTimer", 1000, pdTRUE, NULL, watchCB, &watchTimerBuffer);
        storeTimer(watchTimer);
    }

    init_Shell();
    init_Power();

    subscribeToConnectionState_CommunicationTask(trackConnectionState);
}

void setLightSetting_SystemUtilities(bool isReady){
    isLightReady = isReady;
}

void setLightState_SystemUtilities(bool isShowing){
    isLightShowing = isShowing;
}

void displayStateOfCharge_SystemUtilities(bool showIdle){
    struct StateOfChargeAnimationData socData = {
            .stateOfCharge = stateOfCharge,
            .charging = _READ_CHARGING(),
            .showIdle = showIdle,
    };
    allowStateOfCharge = stateOfCharge != 100;
    startAnimation_GraphicsTask(HaloAnimation_display_stateOfCharge, (uint8_t *) &socData);
    if(_READ_CHARGING() && allowStateOfCharge){
        memset(oledImage, 0, sizeof(oledImage));
        for(int i = 0;i<44; i++){
            uint8_t traversePixels = i*4;
            if(i<44*chargingProgress/100){
                batteryState[16+traversePixels] = 0xF3;
                batteryState[17+traversePixels] = 0xFF;
                batteryState[18+traversePixels] = 0xFF;
                batteryState[19+traversePixels] = 0x19;
            }else{
                batteryState[16+traversePixels] = 0x03;
                batteryState[17+traversePixels] = 0x00;
                batteryState[18+traversePixels] = 0x00;
                batteryState[19+traversePixels] = 0x18;
            }
        }
        appendAssetToDisplay_OLEDLibrary(oledImage,batteryAsset,batteryLayout.location[0]);
        chargingProgress += 10;
        if(chargingProgress > 100)
            chargingProgress = 0;
    }else{
        memset(oledImage, 0, sizeof(oledImage));
        for(int i = 0;i<44; i++){
            uint8_t traversePixels = i*4;
            if(i<44*stateOfCharge/100 && !batteryFlash){
                batteryState[16+traversePixels] = 0xF3;
                batteryState[17+traversePixels] = 0xFF;
                batteryState[18+traversePixels] = 0xFF;
                batteryState[19+traversePixels] = 0x19;
            }else{
                batteryState[16+traversePixels] = 0x03;
                batteryState[17+traversePixels] = 0x00;
                batteryState[18+traversePixels] = 0x00;
                batteryState[19+traversePixels] = 0x18;
            }
        }
        appendAssetToDisplay_OLEDLibrary(oledImage,batteryAsset,batteryLayout.location[0]);
    }
    oled_cmd_t oledCMD = {
            .statusBarPosition = -1,
            .direction = oled_no_animation,
            .animation = false,
            .u8ImageCategory = oled_reactions,
            .u8ImageType = 0
    };
    setPriorityImageWithCmd_GraphicsTask(oledImage, &oledCMD, STATE_OF_CHARGE_OLED_PRIORITY, STATE_OF_CHARGE_OLED_DURATION);
    if(stateOfCharge <= 10)
        batteryFlash = !batteryFlash;
    else
        batteryFlash = false;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1) {
    if (adc_ready == false) {
        adc_ready = true;
        VCPU_AN = (VREFINT * 4096) / ADC_1_DATA[ADC_1_VREFINT_INDEX];
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc1)
{
    uint32_t errorCode = HAL_ADC_GetError(hadc1);

    printf("%s: Adc error: %lx, restarting\n", __func__, errorCode);
    adcRetrigger_SystemUtilities();
}

void adcRetrigger_SystemUtilities(void)
{
    HAL_StatusTypeDef   adcRet;
    extern ADC_HandleTypeDef hadc1;

    adc_ready = false;

    adcRet = HAL_ADC_Stop_DMA(&hadc1);
    if (adcRet != HAL_OK) {
        log_Shell("%s: HAL_ADC_Stop_DMA failed with %d\n", __func__, adcRet);
    }

    adcRet = HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_1_DATA[1], NUM_ADC_1);
    if (adcRet != HAL_OK) {
        log_Shell("%s: Start ADC Error: %d", __func__, adcRet);
    }
}

/**
 * @brief returns battery voltage
 * @details voltage is in millivolts
 */
float getRawBatteryVoltage_SystemUtilities()
{
    return _READ_RAW_BAT_VOLTAGE();
}

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================

static void pairedStateUpdate(bool paired){
    struct PairedState state;
    sleepAllowed_Power(paired, __func__);
    state.paired = paired;
    memcpy(&pairedState,&state,sizeof(struct PairedState));
    writeFile_SystemUtilities(pairedFile, &state, sizeof(struct PairedState));
    shutdownCount = 0;
    log_Shell("My pairing is %s", paired ? "beautiful!" : "a lie!");
}

void send_battery_level(uint8_t soc){
    uint8_t ble_msg[4];

    ble_msg[0] = BLE_NOTIFY_DEVICE;
    ble_msg[1] = soc;
    ble_msg[2] = (int8_t) temperature;
    ble_msg[3] = _READ_PLUGGED();

    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_NOTIFY, 4, ble_msg);
}

static void handleFirmwareUpdateReboot(uint16_t messageLength, uint8_t * data){
    if(stateOfCharge < 10 && !_READ_CHARGING()){
        deniedResponse_CommunicationTask(DENY_LOW_BATTERY);
        return;
    }
    if (handleInstall_FirmwareUpdate(messageLength, data)){
        needReboot = eRebootSystemUtilitiesDFU;
        BF->armedBoot = false;
    }
}

static void returnState(uint16_t messageLength, uint8_t * data){
    float returnedTemperature = 0;
    bool response = false;

    getTempData_SensorsTask(&returnedTemperature, &response);
    uint8_t ble_msg[10];
    ble_msg[0] = eCOM_RETURN_STATUS_OK;
    ble_msg[1] = stateOfCharge;
    ble_msg[2] = _READ_CHARGING(); // charging?
    ble_msg[3] = 0; // compass 1
    ble_msg[4] = 0; // compass 2
    ble_msg[5] = isLightReady; // front light setting
    ble_msg[6] = isLightShowing; // front light state
    ble_msg[7] = (int8_t)returnedTemperature; // temperature
    ble_msg[8] = _READ_PLUGGED(); // plugged in?
    ble_msg[9] = 0; // travel mode?

    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_RESPONSE, 10, ble_msg);
}

static void prvSystemUtilitiesTask(void *pvParameters) {
    log_Shell("\n\n");

    log_Shell("Welcome to SmartHalo\n");

    SPIFLASH_init();

    SPIFLASH_wake();
    vTaskDelay(1);
    uint32_t jedec_id = 0;

    SPIFLASH_read_jedec_id(&jedec_id);
    log_Shell("{\"test_flash\": {\"result\": \"%s\",\"value\": \"%06lX\" }}", (jedec_id == FLASH_ID) ? "OK" : "ERROR", jedec_id);

    my_spiffs_mount();

    start_Power();
    xTimerStart(watchTimer, 1000);

    log_Shell("Assigning functions...");

    assignFunction_CommunicationTask(COM_AUTH, AUTH_GETVERSIONS, &authGetVersions);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_BOOT, &deviceHandleBootloader);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_GETSTATE, &returnState);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_SETNAME, &deviceHandleSetName);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_SHUTDOWN, &deviceHandleSetShutdown);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_GETSERIAL, &deviceHandleGetSerial);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_STM_INSTALL_GOLDEN, &deviceHandleInstallGolden);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_STM_DFU_CRC, &handleCRC_FirmwareUpdate);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_STM_DFU_DATA, &handleData_FirmwareUpdate);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_STM_DFU_REBOOT, &handleFirmwareUpdateReboot);
    assignFunction_CommunicationTask(COM_DEVICE, DEVICE_SETDATE, handleDeviceSetDate);
    assignFunction_CommunicationTask(COM_EXP, EXP_FS_PUT, handleFsPut_SHftp);
    assignFunction_CommunicationTask(COM_EXP, EXP_FS_PUT_DATA, handleFsPutData_SHftp);
    assignFunction_CommunicationTask(COM_EXP, EXP_FS_DEL, handleFsDelete_SHftp);
    subscribeToPairedState_CommunicationTask(pairedStateUpdate);

    struct PairedState state;
    if(readFile_SystemUtilities(pairedFile, &state, sizeof(struct PairedState))){
        pairedState.paired = state.paired;
        if (pairedState.paired == false) {
            SLEEP_NOTALLOWED();
        }
    }

    Localization_E savedLocale;
    if(readFile_SystemUtilities(languageFile, &savedLocale, sizeof(Localization_E))){
        locale = savedLocale;
    }

    // if the file doesn't exist, it's ok because bootCount will be zero
    readFile_SystemUtilities(bootCountFile, &bootCount, sizeof(bootCount));
    if (BF->firstBoot) {
        BF->firstBoot = bootCount = 0;
    }
    bootCount++;
    // don't bother rewriting the file after we get to threshold
    if (bootCount <= bootCountThreshold) {
        writeFile_SystemUtilities(bootCountFile, &bootCount, sizeof(bootCount));
        unpairedShutdownTimeout = FIRSTBOOT_SHUTDOWN_TIMEOUT;
    } else {
        unpairedShutdownTimeout = SHUTDOWN_TIMEOUT;
    }

    log_Shell("Calibrating ADC");
    HAL_StatusTypeDef adcRet = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    if (adcRet != HAL_OK) {
        log_Shell("%s: ADC calibration returns: %d", __func__, adcRet);
    }
    log_Shell("Starting ADC DMA");
    adcRet = HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_1_DATA[1], NUM_ADC_1);
    if (adcRet != HAL_OK) {
        log_Shell("%s: Start ADC Error: %d", __func__, adcRet);
    }

    //Count down the percentage if we are not charging
    //Count up the percentage if we are charging
    stateOfCharge = _READ_CHARGING() ? 0 : 100;
    previousVoltage = 0;

    log_Shell("Running SystemUtilities main loop...");
    while (1) {
        task_Shell();
        uint8_t delay = pinTestClock ? 1 : 50;
        vTaskDelay(delay);
        if(pinTestClock){
            pinTestClock++;
            if(pinTestClock < pinTestPeriod/2){
                _SET_PIN(testPort, testPin, 1);
            }else if(pinTestClock < pinTestPeriod){
                _SET_PIN(testPort, testPin, 0);
            }else{
                pinTestClock = 1;
                pinTestCount++;
                if(pinTestCount == 10){
                    pinTestClock = 0;
                    pinTestCount = 0;
                }
            }
        } else {
            fifty_milliseconds_update();
        }
        shwd_KeepAlive(eSHWD_SystemUtilities);
    }
}

static void batteryShutdown(int batteryLevel)
{
    log_Shell("%s Voltage: %d", __func__, batteryLevel);
    log_Shell("Shutting down from low battery without charging");
    BF->halt = 1;   // signal the bootloader that we don't want to come back
    log_deviceTelemetry(eHALT, __LINE__);
    reboot(eRebootSystemUtilitiesLowBattery);            
}

/**
 * @brief Return the instantaneous battery state of charge
 * @details This is the "model". Consider several inputs including
 *          battery voltage, temperature, haloLED activity, etc to
 *          derive a percentage level for the battery
 *          If the state of charge is too low it will also shutdown
 *          the device to protect the battery.
 * 
 * @return floating point percentage battery state of charge
 */
static uint8_t getBatteryStateOfCharge(void)
{
    uint16_t batterymV = (int)_READ_RAW_BAT_VOLTAGE();

    if (!adc_ready) {
        log_Shell("%s: adc_ready is false", __func__);
        return stateOfCharge;
    }

    if (batterymV > 4500 || batterymV < 3000) {
        log_Shell("%s: batterymV is wacky: %u" PRIu32, __func__, batterymV);
        return stateOfCharge;
    }

    uint16_t totalAprxCurrent = 0;  //milliAmpere
    uint16_t frontLedAprxCurrent = 0; //milliAmpere
    float haloLedAprxCurrent = 0; //milliAmpere

    float returnedTemperature=0;
    bool response=0;

    //Approximate the frontLed current output
    frontLedAprxCurrent = FRONT_LED_CURRENT_MA * frontLEDPercentage / 100.0f ;

    //Approximate the HaloLed current output
    for (uint8_t i = 0; i < 72; i++){
        //So little current it's not worth even considering
        if(leds[i] < 75)
            continue;
        uint8_t intensity = leds[i];
        haloLedAprxCurrent += intensity*intensity*LED_CURRENT_A2 + intensity*LED_CURRENT_A1 + LED_CURRENT_A0;
    }

    //Approximate total current output
    totalAprxCurrent = frontLedAprxCurrent + haloLedAprxCurrent + ESTIMATED_STATIC_CURRENT;

    //If we exceed the max skips the measurement until it drops again
    if(totalAprxCurrent > MAX_CURRENT_ESTIMATE_GUESS)
        return stateOfCharge;

    float currentVoltageDrop = 0;
    if(totalAprxCurrent < CURRENT_CONTINUITY_THRESHOLD){
        currentVoltageDrop = totalAprxCurrent*CURRENT_DRAW_M1;
    }else{
        //Approximately 1 mV per 1 mA draw at this point
        currentVoltageDrop = totalAprxCurrent;
    }

    batterymV += currentVoltageDrop;

    //get temperature value
    getTempData_SensorsTask(&returnedTemperature, &response);
    if (response)//update only if new data
        temperature = returnedTemperature; //store statically
    //No change to the curve above BASELINE_TEMPERATURE_OF_IMPACT from existing data
    if(temperature < BASELINE_TEMPERATURE_OF_IMPACT)
        batterymV += (BASELINE_TEMPERATURE_OF_IMPACT - temperature)*TEMPERATURE_VOLTAGE_MODIFIER;

    //if plugged and charging
    if (_READ_CHARGING() && _READ_PLUGGED())
        batterymV -= CHARGING_ESTIMATE;

    //If the current draw is very low, assume the influence on the voltage is negligible
    if(previousVoltage == 0){
        previousVoltage = batterymV;
    }else if(abs(previousVoltage - batterymV) > 20){ //Jump is too large, likely unrealistic
        return stateOfCharge;
    }
    previousVoltage = batterymV;

    //Voltage suggests we are lower than 20% with compensation for current
    //and temperature without charging
    if (batterymV <= thresholdsSOC[0] && !(_READ_CHARGING()))
        batteryShutdown(batterymV);

    //While charging only count upwards towards 100%, when a threshold is passed it never drops unless we unplug
    if(_READ_CHARGING()){
        for(int8_t i = 9; i>=0; i--){
            //1/4 of the volts to the next threshold for hysteresis
            if(batterymV > thresholdsSOC[i] + (thresholdsSOC[i+1] - thresholdsSOC[i])/4 || stateOfCharge/10 == i+1){
                return (i+1)*10;
            }
        }
        return 0;
    }else{
        for(uint8_t i = 1; i<11; i++){
            //1/2 of the volts below the next threshold for hysteresis
            if(batterymV < thresholdsSOC[i] - (thresholdsSOC[i] - thresholdsSOC[i-1])/2 || stateOfCharge/10 == i){
                return (i)*10;
            }
        }
        return 100;
    }
}

/**
 * @brief   Update system state of charge
 * @details Scale and update the system view of battery state of charge
 *
 * @param batteryLevel Percentage of battery 
 */
static void updateStateOfCharge(float batteryLevel)
{
    static uint8_t send_count = 0;

    stateOfCharge = batteryLevel;
    send_count++;
    if(send_count == STATE_OF_CHARGE_TIMEOUT){
        send_battery_level(stateOfCharge);
        send_count = 0;
    }
}

static void fifty_milliseconds_update()
{
    static uint32_t counter = 0;
    uint8_t  batSOC;

    counter++;

    if(_READ_CHARGING() && counter%((7000/10)/50)==0 && allowStateOfCharge && !isTestMode_SystemUtilities()){
        displayStateOfCharge_SystemUtilities(false);
    }
    if (counter % COUNTER_2s == 0) {
        // Prime the ADC, so the conversion will be done now
        adcRetrigger_SystemUtilities();
        vTaskDelay(1);
        // Get current SoC only returns 0-100 in increments of 10 
        batSOC = getBatteryStateOfCharge();

        updateStateOfCharge(batSOC);
    }



    if(needReboot != eRebootNot)
        rebootCountDown--;
    if(!rebootCountDown){
        log_Shell("reboot counted down!");
        reboot(needReboot);
    }

    if (counter % COUNTER_250ms == 0) {
        if(!pairedState.paired && !isTestMode_SystemUtilities()) {
            shutdownCount++;
            if(shutdownCount > unpairedShutdownTimeout){
                BF->halt = 1;   // signal the bootloader that we don't want to come back
                needReboot = eRebootUnpaired;
                log_deviceTelemetry(eHALT, __LINE__);
            }
        }
    }

    timer_waiting = false;
}

void write_config()
{
    spiffs_file      fh;
    product_config_t config;
    int32_t          ret;

    memcpy(config.product, PRODUCT, CONFIG_HEADER_SIZE);
    memcpy(config.serial, SERIAL, CONFIG_HEADER_SIZE);
    config.keyNumber = KEY;

    fh = open("config-file", (SPIFFS_O_WRONLY | SPIFFS_O_TRUNC | SPIFFS_O_CREAT), 0666);
    if (fh < 0) {
        log_Shell("%s: open failed, returned %d", __func__, fh);
        return;
    }
    ret = write(fh, &config, sizeof(config));
    if (ret < 0) {
        log_Shell("%s: write failed, returned %ld", __func__, ret);
        close(fh);
        return;
    }
    ret = close(fh);
    if (ret < 0) {
        log_Shell("%s: close failed, returned %ld", __func__, ret);
        return;
    }
}

bool read_config()
{
    spiffs_file fh;
    int32_t     ret;

    fh = open("config-file", O_RDONLY, 0);
    if (fh < 0) {
        log_Shell("%s: open failed, returned %d", __func__, fh);
        return false;
    }
    ret = read(fh, &product_config, sizeof(product_config));
    if (ret < 0) {
        log_Shell("%s: read failed, returned %ld", __func__, ret);
        close(fh);
        return false;
    }
    ret = close(fh);
    if (ret < 0) {
        log_Shell("%s: close failed, returned %ld", __func__, ret);
        return false;
    }

    if (memcmp(product_config.product, PRODUCT, CONFIG_HEADER_SIZE) == 0 &&
            memcmp(product_config.serial, SERIAL, CONFIG_HEADER_SIZE) == 0) {
        return true;
    } else {
        memset(&product_config, 0, sizeof(product_config_t));
        return false;
    }
}

static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst)
{
    if (SPIFLASH_read(addr, size, dst) != HAL_OK) {
        return SPIFFS_ERR_INTERNAL;
    }
    return SPIFFS_OK;
}

static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
    if (SPIFLASH_write(addr, size, src) != HAL_OK) {
        return SPIFFS_ERR_INTERNAL;
    }
    return SPIFFS_OK;
}

static s32_t my_spiffs_erase(u32_t addr, u32_t size)
{
    // Reset the watchdog timer here because fsFormat will call
    // this repeatedly in a loop, holding a lock, which will
    // starve the low priority watchdog task
    HAL_IWDG_Refresh(&hiwdg);

    if (SPIFLASH_erase_SECTOR(addr) != HAL_OK) {
        return SPIFFS_ERR_INTERNAL;
    }
    return SPIFFS_OK;
}

void fsLock_SystemUtilities(spiffs *fs)
{
    if (fs_Mutex == NULL) {
        log_Shell("%s: mutex not initialized", __func__);
        return;
    }

    if (xSemaphoreTake(fs_Mutex, 5000) == pdFALSE) {
        log_Shell("%s: mutex timed out", __func__);
    }
}

void fsUnlock_SystemUtilities(spiffs *fs)
{
    if (fs_Mutex == NULL) {
        log_Shell("%s: mutex not initialized", __func__);
        return;
    }
    xSemaphoreGive(fs_Mutex);
}

bool setConductivityTest_SystemUtilities(char port, uint8_t pin, uint16_t period){
    testPin = 1 << pin;
    switch(port){
        case 'a':
            testPort = GPIOA;
            break;
        case 'b':
            testPort = GPIOB;
            break;
        case 'c':
            testPort = GPIOC;
            break;
        default:
            log_Shell("Bad Port!");
            return false;
    }
    pinTestPeriod = period;
    pinTestClock = 1;

    return true;
}
static spiffs_config cfg = {
        .hal_read_f = my_spiffs_read,
        .hal_write_f = my_spiffs_write,
        .hal_erase_f = my_spiffs_erase,
};

#define FSFLAGS_FILE    ".fs_flags"
#define FSFLAGS_METADATA "metadata: true\n"

void my_spiffs_mount() {
    spiffs_stat     statbuf;
    int             res;

    xSemaphoreTake(fs_MountedMutex, 200);

    res = SPIFFS_mount(&rootFS, &cfg, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), spiffs_cache_buf, sizeof(spiffs_cache_buf), 0);
    if (res < 0 || (stat(FSFLAGS_FILE, &statbuf) < 0)) {
        log_Shell("%s, (re)formatting flash", (res < 0) ? "FS mount failed" : "old format filesystem detected");
        fsFormat_SystemUtilities();
    }
    fsIsMounted = true;
    xSemaphoreGive(fs_MountedMutex);
}

/*! @brief  Wait until filesystem completely mounted and ready

            The mount procedure can, in some cases, include an erase and
            (re)format of the filesystem. This routine will block callers
            until that work is complete. 

    @param  identifying string, by convention, a filename the caller wants
            to read/write
 */
bool waitForFS_SystemUtilities(const char *name)
{
    const int delay = 10000;

    if (fsIsMounted) return true;

    log_Shell("%s: wait triggered for: %s", __func__, name);

    for (;;) {
        if (xSemaphoreTake(fs_MountedMutex, delay) == pdFALSE) {
            log_Shell("%s: timeout for FS mount, on behalf of %s",
                      __func__, name);
            return false;
        }
        if (fsIsMounted) break;
        xSemaphoreGive(fs_MountedMutex);
    }
    xSemaphoreGive(fs_MountedMutex);
    return true;
}

/*! @brief      Special getter for fs mounted state for the software watchdog
    @details    On a boot (like the first ever) where the external flash is being
                erased and the filesystem is being recreated, the software watchdog
                needs to wait, or it will reset the device before that process is
                complete. It cannot use waitForFS_SystemUtilities() above because
                of it's high priority (it hangs everybody else on the mutex).
                
                Future work could clean this up.
*/
bool isFSMounted_SystemUtilities(void)
{
    return fsIsMounted;
}

bool readFile_SystemUtilities(const char *filename, void *buf, size_t length)
{
    spiffs_file fh;
    int32_t     ret;

    if (!waitForFS_SystemUtilities(filename)) return false;

    // log_Shell("%s: filename: %s, buf: %p, length: %u", __func__, filename, buf, length);

    fh = open(filename, O_RDONLY, 0);
    if (fh < 0) {
        log_Shell("%s: open failed on %s, returned %d", __func__, filename, fh);
        return false;
    }

    ret = read(fh, buf, length);
    if (ret < 0) {
        log_Shell("%s: read failed, returned %ld", __func__, ret);
        close(fh);
        return false;
    }

    ret = close(fh);
    if (ret < 0) {
        log_Shell("%s: close failed, returned %ld", __func__, ret);
        return false;
    }

    return true;
}

static uint8_t existingData[2048];

bool writeFile_SystemUtilities(const char *filename, void *buf, size_t length)
{
    spiffs_file fh;
    spiffs_stat statbuf;
    int32_t     ret;
    bool        needWrite = true;

    if (!waitForFS_SystemUtilities(filename)) return false;

    if (length > sizeof(existingData)) return false;

    // log_Shell("%s: filename: %s, buf: %p, length: %u", __func__, filename, buf, length);

    fh = open(filename, (O_RDWR | O_CREAT), 0666);
    if (fh < 0) {
        log_Shell("%s: open on %s failed, returned %d", __func__, filename, fh);
        return false;
    }

    // if the length matches, then read&compare existing data
    // maybe we can skip this write
    ret = fstat(fh, &statbuf);
    if (statbuf.size == length) {
        ret = read(fh, existingData, length);
        if (ret < 0) {
            log_Shell("%s: read failed, returned %ld", __func__, ret);
            goto close_and_bail;
        }

        if (memcmp(existingData, buf, length) == 0) {
            needWrite = false;
        }
    }

    if (needWrite) {
        lseek(fh, 0, SEEK_SET);
        ret = write(fh, buf, length);
        if (ret < 0) {
            log_Shell("%s: write failed, returned %ld", __func__, ret);
            goto close_and_bail;
        }
    }

    ret = close(fh);
    if (ret < 0) {
        log_Shell("%s: close failed, returned %ld", __func__, ret);
        return false;
    }

    return true;

close_and_bail:
    close(fh);
    return false;
}

void fsFormat_SystemUtilities()
{
    spiffs_file     fh;
    int             res;

    log_Shell("unmounting...");
    SPIFFS_unmount(&rootFS);
    log_Shell("unmount done");
    log_Shell("formatting...");
    SPIFFS_format(&rootFS);

    log_Shell("remounting...");
    res = SPIFFS_mount(&rootFS, &cfg, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), spiffs_cache_buf, sizeof(spiffs_cache_buf), 0);
    if (res < 0) {
        log_Shell("FS remount after format failed, status %d", res);
        log_Shell("Bad Flash Hardware");
        // TBD should never happen but if it does, what to do here?
#ifdef MEGABUG_HUNT
        while(1);
#else
        SOFT_CRASH(eSYSTEMUTILITIES);
#endif
    }

    // Re-create .fs_flags file
    fh = open(FSFLAGS_FILE, O_RDWR | O_CREAT, 0);
    write(fh, FSFLAGS_METADATA, sizeof(FSFLAGS_METADATA));
    close(fh);
}

/**
 * @brief   Return the results from a firmware hardware test
 * @details Validates the mag/acc, photo sensor, OLED, Halo IC's and Flash are responding via i2c and SPI.
 */
uint8_t hardwareTests_SystemUtilities(){
    uint8_t tests = 0;
    tests |= !testECompass_SensorsTask() << TEST_BIT_ECOMPASS;
    tests |= !testLight_SensorsTask() << TEST_BIT_PHOTO;
    tests |= !testOLEDByTurningOff_GraphicsTask() << TEST_BIT_OLED;
    tests |= !testHaloResponseByTurningOff_GraphicsTask() << TEST_BIT_HALO;
    tests |= !testFlash_SystemUtilities() << TEST_BIT_FLASH;

    return tests;
}

/**
 * @brief Return the current state of charge
 */
uint8_t getStateOfCharge_SystemUtilities(void)
{
    return stateOfCharge;
}

bool getIsChargingAnimationAllowed_SystemUtilities(){
    return _READ_CHARGING() && allowStateOfCharge;
}

bool getIsPlugged_SystemUtilities(){
    return _READ_PLUGGED();
}

bool getIsPaired_SystemUtilities(){
    return pairedState.paired;
}

/**
 * @brief Test the Flash Connection
 * @return true if written data can be read back, false otherwise
 */
bool testFlash_SystemUtilities(){
    write_config();
    return read_config();
}

/**
 * @brief       AuthGetVersions()
 * @details		Function to get the version data.
 * @private
 * @param[in]	messageLength: Message Length
 * @param[in]	data: Handle on the buffer to send.
 */
static void authGetVersions(uint16_t messageLength, uint8_t * data){
    if (data == NULL)
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);

    uint8_t length = 1;
    uint8_t ble_msg[length];
    ble_msg[0] = BLE_GET_VERSIONS;

    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_COMMAND_SYNC, length, ble_msg);
}

static void deviceHandleBootloader(uint16_t messageLength, uint8_t * data){
    uint8_t length = 1;
    uint8_t ble_msg[length];
    ble_msg[0] = BLE_SET_BOOTLOADER;

    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_COMMAND_SYNC, length, ble_msg);
}

static void deviceHandleSetName(uint16_t messageLength, uint8_t * data){
    uint8_t length = messageLength+1;
    uint8_t ble_msg[length];
    ble_msg[0] = BLE_SET_NAME;
    for(int i=0; i<messageLength; i++){
        ble_msg[1+i] = data[i];
    }

    sendData_CommunicationTask(BLE_TYPE_MSG, BLE_COMMAND_SYNC, length, ble_msg);
}

/**
 * @brief       private function to handle DEVICE_GETSERIAL command
 * @details     constructs response packet and sends via BLE
 * @param[in]   data: pointer to the incoming command argument
 */
static void deviceHandleGetSerial(uint16_t length, uint8_t *data)
{
    // the only one we know how to do is the serial# in flash
    if (*data == 0) {
        log_Shell("Product serial requested");
        SerialReply_t response;
        response.status = eCOM_RETURN_STATUS_OK;
        memcpy(response.serial, MCB->serial, 8);
        sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_RESPONSE, sizeof(response), &response);
        return;
    }

    switch (*data) {
        case 1:
            log_Shell("PCBA serial requested");
            break;
        case 2:
            log_Shell("Lock serial requested");
            break;
        default:
            log_Shell("Unsupported command %d", data[1]);
            break;
    }
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_UNIMPLEMENTED);
}

static void deviceHandleSetShutdown(uint16_t u16Length, uint8_t *ignored)
{
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
    BF->halt = 1;	// signal the bootloader that we don't want to come back
    log_deviceTelemetry(eHALT, __LINE__);
    needReboot = true;
}

static void deviceHandleInstallGolden(uint16_t u16Length, uint8_t *ignored)
{
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
    BF->forceGolden = 1;	// signal the bootloader to reinstall the golden FW
    needReboot = true;
}

/*! brief   use connection state to disable test mode

            Any connection state change drops test mode (which among other
            things disables power management)
 */
static void trackConnectionState(bool isConnected)
{
    disableAutoPower = false;
}

/**
 * @brief This should return true while we are in test mode
 */
bool isTestMode_SystemUtilities(){
    return disableAutoPower;
}

/**
 * @brief      Called by test mode to enable full halt after a shorter time
 * @details    Test mode is used in manufacturing. After testing, the device
 *             can be halted (on the assumption that it will be put into a
 *             box for shipping).
 *             After this function returns, the device will halt in
 *             FIRSTBOOT_SHUTDOWN_TIMEOUT (3 minutes).
 *             If not actually in test mode, this function does nothing.
 */
void enableEarlyHalt_SystemUtilities(void)
{
    if (isTestMode_SystemUtilities()) {
        shutdownCount = 0;
        unpairedShutdownTimeout = FIRSTBOOT_SHUTDOWN_TIMEOUT;
    }
}

/**
 * @brief       Get the current locale settings
*  @details     This will allow any application that uses text to
*               decide if they should use available translations.
 */
Localization_E getLocale_SystemUtilities(){
    return locale;
}

/**
 * @brief       Set locale for translation's sake
 * @details     This locale is consumed by other parts of the code
 *              to decide which language to use in hardcoded text.
*               This setter is intended for testing purposes at this
*               time, but perhaps certain applications can have
*               control over this feature too in the future.
 */
void setLocale_SystemUtilities(Localization_E newLocale){
    locale = newLocale;
    writeFile_SystemUtilities(languageFile, &locale, sizeof(Localization_E));
}

/**
 * @brief       Handle the DEVICE_SETDATE command
 * @details     Convert the incoming time_t to time structure and
 *              use that to set the device RTC
 * 
 * @param[in]   len BLE message size, used to validate byte count
 * @param[in]   payload BLE message payload, expecting a LSB 64-bit time_t
 */
static void handleDeviceSetDate(uint16_t len, const void *payload)
{
    if(len != 8 && len != 10){
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
        return;
    }
    const struct LocalizationPayload * localizationPayload = payload;
    extern RTC_HandleTypeDef hrtc;

    time_t t = localizationPayload->time;
    struct tm tm;
    gmtime_r(&t, &tm);

    RTC_DateTypeDef sDate1 = {
        .Year = tm.tm_year - 100,
        .Month = tm.tm_mon + 1,
        .Date = tm.tm_mday
    };
    RTC_TimeTypeDef sTime1 = {
        .Hours = tm.tm_hour,
        .Minutes = tm.tm_min,
        .Seconds = tm.tm_sec
    };

    HAL_RTC_SetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate1, RTC_FORMAT_BIN);

    if(len == 10){
        if(localizationPayload->locale[0] == 'e' && localizationPayload->locale[1] == 'n')
            locale = english;
        else if(localizationPayload->locale[0] == 'f' && localizationPayload->locale[1] == 'r')
            locale = french;
        else if(localizationPayload->locale[0] == 'e' && localizationPayload->locale[1] == 's')
            locale = spanish;
        else if(localizationPayload->locale[0] == 'd' && localizationPayload->locale[1] == 'e')
            locale = german;
        else locale = english;
        writeFile_SystemUtilities(languageFile, &locale, sizeof(Localization_E));
    }

    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

/**
 * @brief       Callback for the watch timer
 * @details     If watch flag is set, print the RTC and the tick count 
 *              (assumed to be called once per second)
 * 
 * @param[in]   t ignored
 */
static void watchCB(TimerHandle_t t)
{
    extern bool watch;
    if (watch) {
        extern RTC_HandleTypeDef hrtc;

        RTC_TimeTypeDef sTime1;
        RTC_DateTypeDef sDate1;

        HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BIN);
        uint32_t now = xTaskGetTickCount();

        log_Shell("%02d:%02d:%02d -- %lu.%lu", sTime1.Hours, sTime1.Minutes,
                  sTime1.Seconds, now / 1000, now % 1000);
    }
}

static EResetType_t resetType;
static eRebootReason_t rebootReason;

/**
 * @brief       How did the previous life of the system end?
 * @description The previous life of the system could have ended with:
 *                a normal reboot
 *                a crash/watchdog
 * @return      reset type
 */
EResetType_t getResetType_SystemUtilities(void)
{
    return resetType;
}

/**
 * @brief       Figure out and save the reset type
 * @details     Called only once from main() before any of the
 *              boot information is rewritten/lost.
 *              Will automatically ignore subsequent calls
 */
void saveResetType_SystemUtilities(void)
{
    if (resetType) return;  // we were already called once -- ignore

    resetType = eResetType_Normal;

    // If our last reboot was under duress, consider it a crash
    if ((*BR & eIWDGRST) || (*BR & eWWDGRST) || BF->softWD || BF->crash) {
        resetType = eResetType_Crash;
    } 
}

/**
 * @brief       Get the saved reboot reason
 * @details     The reboot() function saves a reboot code that persists across
 *              the actual reboot and re-stored by saveRebootReason below.
 * @return      the stored reboot code
 */
eRebootReason_t getRebootReason_SystemUtilities(void)
{
    return rebootReason;
}

/**
 * @brief       Store the reboot code
 * @details     Called from main shortly after boot to store the reboot code
 * 
 * @param       reason the reboot code/reason
 */
void saveRebootReason_SystemUtilities(eRebootReason_t reason)
{
    rebootReason = reason;
}

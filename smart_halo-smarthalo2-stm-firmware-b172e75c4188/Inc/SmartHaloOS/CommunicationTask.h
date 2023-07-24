// ------------------------------------------------------------------------------------------------
/*!@file   CommunicationTask.h

 */
// ------------------------------------------------------------------------------------------------


#ifndef COMMUNICATION_TASK_H_
#define COMMUNICATION_TASK_H_

#include "main.h"
#include "BLEDriver.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

typedef enum {
    BLE_TYPE_MSG = 0,
    BLE_TYPE_ACK = 1,
    BLE_TYPE_ERR = 2
} BLE_TYPE_E;

typedef enum {
    BLE_RX_COMMAND_BLE = 0,
    BLE_TX_COMMAND_BLE_NOTIFY = 1,
    BLE_TX_COMMAND_BLE_RESPONSE = 2,
    BLE_COMMAND_SYNC = 3,
    BLE_CMD_MAX // must be last
} UART_COMMAND_E;

typedef enum {
    BLE_SET_BOOTLOADER = 0,
    BLE_CONNECTION_STATE = 1,
    BLE_SET_NAME = 2,
    BLE_GET_VERSIONS = 3,
    BLE_GET_ID = 4,
    BLE_PAIRED_STATE = 5,
} UART_SYNC_E;

typedef enum{
    BLE_NOTIFY_TOUCH = 0,
    BLE_NOTIFY_ALARM = 1,
    BLE_NOTIFY_MOVEMENT = 2,
    BLE_NOTIFY_NIGHTLIGHT = 3,
    BLE_NOTIFY_DEVICE = 4,
    BLE_NOTIFY_CAROUSEL = 5,
    BLE_NOTIFY_LOG = 0xf8,
} BLE_NOTIFY_TYPE_E;

typedef enum {
    eCOM_RETURN_STATUS_OK = 0,
    eCOM_RETURN_STATUS_FAIL = 1,
    eCOM_RETURN_STATUS_DENIED = 2,
    eCOM_RETURN_STATUS_UNIMPLEMENTED = 3,
    eCOM_RETURN_STATUS_UNNECESSARY = 4,
}eCOM_RETURN_STATUS_t,*peCOM_RETURN_STATUS_t;

typedef enum {
    DENY_UNAUTHENTICATED = 0,
    DENY_UNNECESSARY = 1,
    DENY_LOW_BATTERY = 2,
}eDenyReason_t;

typedef enum {
    COM_AUTH = 0,
    COM_UI = 1,
    COM_SOUND = 2,
    COM_ALARM = 3,
    COM_DEVICE = 4,
    COM_TEST = 242,
    COM_EXP = 248,
    COM_ENUM_SIZE = 7,
}eComCategory_t, *peComCategory_t;

typedef enum {
    DEVICE_BOOT = 0,
    DEVICE_GETSTATE = 1,
    DEVICE_SETNAME = 2,
    DEVICE_COMPASS_CALIB = 3,
    DEVICE_SHUTDOWN = 4,
    DEVICE_GETSERIAL = 5,
    DEVICE_SETDATE = 6,
    DEVICE_STM_DFU_CRC = 7,
    DEVICE_STM_DFU_DATA = 8,
    DEVICE_STM_DFU_REBOOT = 9,
    DEVICE_STM_INSTALL_GOLDEN = 10,
    DEVICE_SLEEP = 11,
    DEVICE_ENUM_SIZE = 12,
} eComDevice_t, *peComDevice_t;

typedef enum {
    SOUND_PLAY = 0,
    SOUND_STOP = 1,
    SOUND_TAP_EN = 2,
    SOUND_ENUM_SIZE = 3,
} eComSound_t, *peComSound_t;

typedef enum {
    UI_LOGO = 0,
    UI_NAV = 1,
    UI_NAV_REROUTE = 2,
    UI_NAV_OFF = 3,
    UI_FRONTLIGHT = 4,
    UI_PROGRESS = 5,
    UI_PROGRESS_OFF = 6,
    UI_NOTIF = 7,
    UI_NOTIF_OFF = 8,
    UI_HB = 9,
    UI_HB_OFF = 10,
    UI_COMPASS = 11,
    UI_COMPASS_OFF = 12,
    UI_DISCONNECT = 13,
    UI_ANIM_OFF = 14,
    UI_SETBRIGHTNESS = 15,
    UI_FRONTLIGHT_SETTINGS = 16,
    UI_NAV_ANGLE = 17,
    UI_SPEEDOMETER = 18,
    UI_SPEEDOMETER_OFF = 19,
    UI_ROUNDABOUT = 20,
    UI_LOWBAT = 21,
    UI_POINTER = 22,
    UI_POINTER_TURNOFF = 23,
    UI_POINTER_STANDBY = 24,
    UI_DEMO = 25,
    UI_SPEEDOMETER_INTRO = 26,
    UI_FITNESS_INTRO = 27,
    UI_CLOCK = 28,
    UI_CLOCK_OFF = 29,
    UI_FRONT_LIGHT_TOGGLE_MODE = 30,
    UI_SHOW_STATE_OF_CHARGE = 31,
    UI_TURN_BY_TURN_INTRO = 32,
    UI_POINTER_WITH_INTRO = 33,
    UI_ROUNDABOUT_OLED = 34,
    UI_CAROUSEL = 35,
    UI_CAROUSEL_POSITION = 36,
    UI_ONBOARDING = 37,
    UI_ENUM_SIZE,
} eComUI_t, *peComUI_t;

typedef enum {
    AUTH_GETVERSIONS = 0,
    AUTH_GETPERIPHPUBKEY = 1,
    AUTH_SETCENTRALPUBKEY = 2,
    AUTH_AUTHENTICATE = 3,
    AUTH_SETPASSWORD = 4,
    AUTH_GETSEED = 6,
    AUTH_RESETPASSWORD = 7,
    AUTH_ENUM_SIZE = 8,
} eComAuth_t, *peComAuth_t;

typedef enum {
    ALARM_REPORT = 0,
    ALARM_GETSEED = 1,
    ALARM_ARM = 2,
    ALARM_SETCONFIG = 3,
    ALARM_ENUM_SIZE = 4,
} eComAlarm_t, *peComAlarm_t;

typedef enum {
    TEST_TOUCH = 0,
    TEST_HALO = 1,
    TEST_FRONT = 2,
    TEST_HARDWARE = 3,
    TEST_OLED = 4,
    TEST_PHOTO = 5,
    TEST_POWER = 6,
    TEST_ENUM_SIZE     // Should always be last
} eComTest_t, *peComTest_t;

// there are a lot of missing enums here, but they aren't really documented so the size value is just the maximum for now.
typedef enum {
    EXP_OLED_SHOW = 21,
    EXP_OLED_OFF = 22,
    EXP_OLED_CONTRAST = 23,
    EXP_OLED_BRIGHTNESS = 24,
    EXP_TOUCH_CAL = 25,
    EXP_SWIPE_CAL = 26,
    EXP_SWIPE_AREA_CAL = 27,
    EXP_OLED_SHOW_IMG =     0x1d, //29
    EXP_FS_PUT =            0x1e, //30
    EXP_FS_PUT_DATA =       0x1f, //31
    EXP_FS_DEL =            0x20, //32
    EXP_ENUM_SIZE           // should always be last
} eComEXP_t, *peComEXP_t;

typedef enum {
    StandbyOff = 0,
    StandbySleep = 254,
    StandbyArmed = 255,
} eStandbyReason_t;


// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
void init_CommunicationTask();
eStandbyReason_t getStandbyReason_CommunicationTask();
void toggleStandby_CommunicationTask(eStandbyReason_t reason);
bool interpret_CommunicationTask(oBLERXMessage_t *msg);
bool sendData_CommunicationTask(uint8_t u8Type, uint8_t u8Command, uint16_t u16Length, void * pu8Data);
void responseWithPayload_CommunicationTask(uint16_t length, void * data);
void deniedResponse_CommunicationTask(eDenyReason_t reason);
void genericResponse_CommunicationTask(eCOM_RETURN_STATUS_t status);
void responseFromResult_CommunicationTask(bool bResult);
void assignFunction_CommunicationTask(eComCategory_t category, uint8_t type, void * function);
void testOledDisplay_CommunicationTask();
void subscribeToConnectionState_CommunicationTask(void (*function)(bool isConnected));
void unsubscribeToConnectionState_CommunicationTask(void (*function)(bool isConnected));
void subscribeToPairedState_CommunicationTask(void (*function)(bool isPaired));
void unsubscribeToPairedState_CommunicationTask(void (*function)(bool isPaired));
uint8_t * getVersion_CommunicationTask();

#endif  /* COMMUNICATION_TASK_H_ */

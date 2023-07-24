// ------------------------------------------------------------------------------------------------
/*!@file    SystemUtilitiesTask.h

 */
// ------------------------------------------------------------------------------------------------


#ifndef __SYSTEM_UTILITIES_TASK_H__
#define __SYSTEM_UTILITIES_TASK_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <Shell.h>
#include <time.h>
#include <spiffs.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "semphr.h"
#include "stm32_defines.h"

#include "cmsis_os.h"

#include "main.h"
#include "reboot.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define NUMBER_OF_ADC_CHANNEL 16
#define EXT_SPIFF_START_OFFSET     0x00080000

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

typedef enum {
	TEST_BIT_ECOMPASS = 0,
	TEST_BIT_PHOTO = 1,
	TEST_BIT_OLED = 2,
	TEST_BIT_HALO = 3,
	TEST_BIT_FLASH = 4,
} TEST_BITS_E;

typedef enum{
    english,
    french,
    german,
    spanish,
} Localization_E;

typedef enum {
    eResetType_INVALID = 0,
    eResetType_Normal,
    eResetType_Crash,
} EResetType_t;

// ================================================================================================
// ================================================================================================
//            EXTERNAL VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================

extern const char pairedFile[];

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
void init_SystemUtilities();
void setLightSetting_SystemUtilities(bool isReady);
void setLightState_SystemUtilities(bool isShowing);
void displayStateOfCharge_SystemUtilities(bool showIdle);
bool readFile_SystemUtilities(const char *fileName, void *buf, size_t length);
bool writeFile_SystemUtilities(const char *fileName, void *buf, size_t length);
uint8_t hardwareTests_SystemUtilities();
uint8_t getStateOfCharge_SystemUtilities(void);
bool testFlash_SystemUtilities();
bool getIsChargingAnimationAllowed_SystemUtilities();
bool getIsPlugged_SystemUtilities();
bool getIsPaired_SystemUtilities();
void fsLock_SystemUtilities(spiffs *fs);
void fsUnlock_SystemUtilities(spiffs *fs);
bool setConductivityTest_SystemUtilities(char port, uint8_t pin, uint16_t period);
void fsFormat_SystemUtilities();
bool isTestMode_SystemUtilities();
bool waitForFS_SystemUtilities(const char *filename);
bool isFSMounted_SystemUtilities();
float getRawBatteryVoltage_SystemUtilities();
void enableEarlyHalt_SystemUtilities(void);
Localization_E getLocale_SystemUtilities();
void setLocale_SystemUtilities(Localization_E newLocale);
void adcRetrigger_SystemUtilities(void);
void saveResetType_SystemUtilities(void);
EResetType_t getResetType_SystemUtilities(void);
eRebootReason_t getRebootReason_SystemUtilities(void);
void saveRebootReason_SystemUtilities(eRebootReason_t reason);

#endif  /* __SYSTEM_UTILITIES_TASK_H__ */

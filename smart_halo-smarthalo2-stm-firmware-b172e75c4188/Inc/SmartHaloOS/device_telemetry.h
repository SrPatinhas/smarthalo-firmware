/**
 * @defgroup   DEVICE_TELEMETRY device telemetry
 *
 * @brief      This file implements device telemetry.
 *
 * @author     Georgn
 * @date       2020
 */

#ifndef __DEVICE_TELEMETRY__
#define __DEVICE_TELEMETRY__

#include <stdbool.h>

typedef enum {
    eNOP,
    eSLEEP,
    eWAKEUP,
    eCRASH,
    eREBOOT,
    eBOOTREASON,
    eHALT,
    ePAIR,
    eUNPAIR,
    eCONNECT,
    eDISCONNECT,
    eSOFTWD,
    eSTACKOVERFLOW,
    eSTACKHW_0,          // aligns with SHTaskUtils.h list of names
    eSTACKHW_1,
    eSTACKHW_2,
    eSTACKHW_3,
    eSTACKHW_4,
    eSTACKHW_5,
    eSTACKHW_6,
    eSTACKHW_7,
    eSTACKHW_8,
    eSTACKHW_9,
    eSTACKHW_A,
    eSTACKHW_B,
    eSTACKHW_C,
    eSTACKHW_D,
    eSTACKHW_E,
    eWWDG,
    eCRASHADDR,
    eCRASHD0,           // these eCRASHDn items need to be
    eCRASHD1,           // consecutive
    eCRASHD2,
    eCRASHD3,
    eIWDG,
    eSOFTCRASH,
    eLASTTASK,
    eSKIPWWDG,
    eI2CERROR,
} telemetry_event_type_t;

#define LASTTASK_STRING_LEN 36  // should be at least 16 * 2 + comma + NULL

typedef struct {
    telemetry_event_type_t  type;
    struct {
        uint8_t uploaded:1;
        uint8_t soc:7;
    };
    uint32_t                timestamp;
    uint32_t                arg;
} telemetry_log_t;

void init_deviceTelemetry(void);
void log_deviceTelemetry(telemetry_event_type_t event_type, uint32_t arg);
void dump_deviceTelemetry(void);
void store_deviceTelemetry(void);
void upload_deviceTelemetry(void);

#endif
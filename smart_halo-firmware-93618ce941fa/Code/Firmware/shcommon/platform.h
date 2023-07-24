#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "nordic_common.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_rng.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "app_twi.h"
#include "app_uart.h"
#include "nrf_drv_common.h"
#include "nrf_drv_pwm.h"
#include "nrf_drv_ppi.h"
#include "mem_manager.h"
#include "ble.h"
#include "ecc.h"
#include "fds.h"

#include "pinmap.h"

//Key definitions
#define KEY_FACTORY 	1
#define KEY_PASSWORD 	2
#define KEY_ALARMSETTINGS 	3
#define KEY_METSETTINGS	4
#define KEY_DEVNAME 5
#define KEY_LEDSETTINGS 6

#define KEY_SERIAL_PRODUCT 7
#define KEY_SERIAL_PCBA 8
#define KEY_SERIAL_LCK 9

//Notify definition
#define NOTIFY_TOUCH 0
#define NOTIFY_ALARM 1
#define NOTIFY_MOVEMENT 2
#define NOTIFY_FRONTLIGHT 3
#define NOTIFY_DEVICE 4
#define NOTIFY_MAG 5

#define NOTIFY_TEST 0xf0

//
#define SERIALMAXLEN 16

typedef enum {
	TOUCH_SHORT=0,
	TOUCH_LONG,
} touch_type_t;

#define ERR_CHECK(CTX, ERR_CODE)                      \
    do                                                      \
    {                                                       \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);         \
        if (LOCAL_ERR_CODE != NRF_SUCCESS)                  \
        {                                                   \
            printf("%s %d\r\n",CTX,ERR_CODE);               \
            nrf_delay_ms(100);                              \
            APP_ERROR_HANDLER(LOCAL_ERR_CODE);              \
        }                                                   \
    } while (0)

#endif

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

/*
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#pragma message STR(RETARGET_ENABLED)
*/

//#define ALLFEATURE (!defined(PLATFORM_pca10040) && !defined(PLATFORM_shmp))
//#define BAREMINIMUM 1

typedef enum {
    HW_V11=0,
    HW_V12,
} platform_hw_t;

platform_hw_t platform_getHW(void);

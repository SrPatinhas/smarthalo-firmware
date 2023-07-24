/*
This software is subject to the license described in the license.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * @defgroup ant_bpwr_display_main ANT Bicycle Power display example
 * @{
 * @ingroup nrf_ant_bicycle_power
 *
 * @brief Example of ANT Bicycle Power profile display.
 *
 * Before compiling this example for NRF52, complete the following steps:
 * - Download the S212 SoftDevice from <a href="https://www.thisisant.com/developer/components/nrf52832" target="_blank">thisisant.com</a>.
 * - Extract the downloaded zip file and copy the S212 SoftDevice headers to <tt>\<InstallFolder\>/components/softdevice/s212/headers</tt>.
 * If you are using Keil packs, copy the files into a @c headers folder in your example folder.
 * - Make sure that @ref ANT_LICENSE_KEY in @c nrf_sdm.h is uncommented.
 */

#include <stdio.h>
#include "nrf.h"
#include "app_uart.h"
#include "nrf_soc.h"
#include "bsp.h"
#include "app_error.h"
#include "nordic_common.h"
#include "ant_stack_config.h"
#include "softdevice_handler.h"
#include "ant_bpwr.h"
#include "app_trace.h"
#include "ant_state_indicator.h"
#include "ant_key_manager.h"
#include "app_timer.h"
#include "bsp_btn_ant.h"

#define APP_TIMER_PRESCALER         0x00 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE     0x04 /**< Size of timer operation queues. */

#define BPWR_CHANNEL_NUMBER         0x00 /**< Channel number assigned to Bicycle Power profile. */

#define WILDCARD_TRANSMISSION_TYPE  0x00 /**< Wildcard transmission type. */
#define WILDCARD_DEVICE_NUMBER      0x00 /**< Wildcard device number. */

#define ANTPLUS_NETWORK_NUMBER      0 /**< Network number. */

/** @snippet [ANT BPWR RX Instance] */
void ant_bpwr_evt_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_evt_t event);

BPWR_DISP_CHANNEL_CONFIG_DEF(m_ant_bpwr,
                             BPWR_CHANNEL_NUMBER,
                             WILDCARD_TRANSMISSION_TYPE,
                             WILDCARD_DEVICE_NUMBER,
                             ANTPLUS_NETWORK_NUMBER);
BPWR_DISP_PROFILE_CONFIG_DEF(m_ant_bpwr,
                             ant_bpwr_evt_handler);

ant_bpwr_profile_t m_ant_bpwr;
/** @snippet [ANT BPWR RX Instance] */


/**@brief Function for dispatching an ANT stack event to all modules with an ANT stack event handler.
 *
 * @details This function is called from the ANT Stack event interrupt handler after an ANT stack
 *          event has been received.
 *
 * @param[in] p_ant_evt  ANT stack event.
 *
 * @snippet [ANT BPWR RX Profile handling] */
void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
    ant_bpwr_disp_evt_handler(&m_ant_bpwr, p_ant_evt);
    ant_state_indicator_evt_handler(p_ant_evt);
    bsp_btn_ant_on_ant_evt(p_ant_evt);
}
/** @snippet [ANT BPWR RX Profile handling] */

/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    uint32_t              err_code;
    ant_bpwr_page1_data_t page1;

    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            // request to calibrating the sensor
            page1    = ANT_BPWR_GENERAL_CALIB_REQUEST();
            err_code = ant_bpwr_calib_request(&m_ant_bpwr, &page1);
            APP_ERROR_CHECK(err_code);
            break;

        case BSP_EVENT_SLEEP:
            ant_state_indicator_sleep_mode_enter();
            break;
        
        default:
            break;
    }
}


/**@brief Function for handling Bicycle Power profile's events
 *
 */
void ant_bpwr_evt_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_evt_t event)
{
    switch (event)
    {
        case ANT_BPWR_PAGE_1_UPDATED:
            // calibration data received from sensor
            app_trace_log("Received calibration data\n\r\n\r");
            break;

        case ANT_BPWR_PAGE_16_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_17_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_18_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_80_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_81_UPDATED:
            // data actualization
            app_trace_log("Page was updated\n\r\n\r");
            break;

        case ANT_BPWR_CALIB_TIMEOUT:
            // calibration request time-out
            app_trace_log("ANT_BPWR_CALIB_TIMEOUT\n\r\n\r");
            break;

        case ANT_BPWR_CALIB_REQUEST_TX_FAILED:
            // Please consider retrying the request.
            app_trace_log("ANT_BPWR_CALIB_REQUEST_TX_FAILED\n\r\n\r");
            break;

        default:
            // never occurred
            break;
    }
}


/**
 * @brief Function for setup all thinks not directly associated with ANT stack/protocol.
 * @desc Initialization of: @n
 *         - app_tarce for debug.
 *         - app_timer, pre-setup for bsp.
 *         - bsp for signaling LEDs and user buttons.
 */
static void utils_setup(void)
{
    uint32_t err_code;

    app_trace_init();
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                        APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                        bsp_evt_handler);
    APP_ERROR_CHECK(err_code);
    
    err_code = bsp_btn_ant_init();
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for ANT stack initialization.
 *
 * @details Initializes the SoftDevice and the ANT event interrupt.
 */
static void softdevice_setup(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    err_code = softdevice_ant_evt_handler_set(ant_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_handler_init(&clock_lf_cfg, NULL, 0, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config(); // set ant resource
    APP_ERROR_CHECK(err_code);

    err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUMBER);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for Bicycle Power profile initialization.
 *
 * @details Initializes the Bicycle Power profile and open ANT channel.
 */
static void profile_setup(void)
{
/** @snippet [ANT BPWR RX Profile Setup] */
    uint32_t err_code;

    err_code = ant_bpwr_disp_init(&m_ant_bpwr,
                                  BPWR_DISP_CHANNEL_CONFIG(m_ant_bpwr),
                                  BPWR_DISP_PROFILE_CONFIG(m_ant_bpwr));
    APP_ERROR_CHECK(err_code);

    err_code = ant_bpwr_disp_open(&m_ant_bpwr);
    APP_ERROR_CHECK(err_code);

    err_code = ant_state_indicator_channel_opened();
    APP_ERROR_CHECK(err_code);
/** @snippet [ANT BPWR RX Profile Setup] */
}


/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    utils_setup();
    softdevice_setup();
    ant_state_indicator_init(m_ant_bpwr.channel_number, BPWR_DISP_CHANNEL_TYPE);
    profile_setup();

    for (;; )
    {
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
}


/**
 *@}
 **/

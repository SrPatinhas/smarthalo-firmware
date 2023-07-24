/*
This software is subject to the license described in the license.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * @defgroup ant_bpwr_sensor_main ANT Bicycle Power sensor example
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
#include <string.h>
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
#include "ant_bpwr_simulator.h"

#ifndef MODIFICATION_TYPE // can be provided as preprocesor global symbol
/**
 * @brief Depending of this define value Heart Rate value will be: @n
 *          - periodicaly rise and fall, use value  MODIFICATION_TYPE_AUTO
 *          - changing by button, use value         MODIFICATION_TYPE_BUTTON
 */
    #define MODIFICATION_TYPE (MODIFICATION_TYPE_AUTO)
#endif

#define MODIFICATION_TYPE_BUTTON 0 /* predefined value, MUST REMAIN UNCHANGED */
#define MODIFICATION_TYPE_AUTO   1 /* predefined value, MUST REMAIN UNCHANGED */

#if (MODIFICATION_TYPE != MODIFICATION_TYPE_BUTTON) \
    && (MODIFICATION_TYPE != MODIFICATION_TYPE_AUTO)

    #error Unsupported value of MODIFICATION_TYPE.
#endif

#ifndef SENSOR_TYPE // can be provided as preprocesor global symbol
    #define SENSOR_TYPE (TORQUE_NONE)
#endif

#define APP_TIMER_PRESCALER         0x00 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE     0x04 /**< Size of timer operation queues. */

#define BPWR_CHANNEL_NUMBER         0x00 /**< Channel number assigned to Bicycle Power profile. */

#define BPWR_DEVICE_NUMBER          1u /**< Denotes the used ANT device number. */
#define BPWR_TRANSMISSION_TYPE      5u /**< Denotes the used ANT transmission type. */

#define HW_REVISION                 0x7Fu   /**< Hardware revision for manufacturer's identification common page. */
#define MANUFACTURER_ID             0xAAAAu /**< Manufacturer ID for manufacturer's identification common page. */
#define MODEL_NUMBER                0x5555u /**< Model number for manufacturer's identification common page. */

#define SW_REVISION_MAJOR           0xAAu       /**< Software revision major number for product information common page. */
#define SW_REVISION_MINOR           0xFFu       /**< Software revision minor number for product information common page, unused value. */
#define SERIAL_NUMBER               0xAA55AA55u /**< Serial number for product information common page. */

#define ANTPLUS_NETWORK_NUMBER      0       /**< Network number. */
#define CALIBRATION_DATA            0x55AAu /**< General calibration data value. */

/** @snippet [ANT BPWR TX Instance] */
void ant_bpwr_evt_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_evt_t event);
void ant_bpwr_calib_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_page1_data_t * p_page1);

BPWR_SENS_CHANNEL_CONFIG_DEF(m_ant_bpwr,
                             BPWR_CHANNEL_NUMBER,
                             BPWR_TRANSMISSION_TYPE,
                             BPWR_DEVICE_NUMBER,
                             ANTPLUS_NETWORK_NUMBER);
BPWR_SENS_PROFILE_CONFIG_DEF(m_ant_bpwr,
                            (ant_bpwr_torque_t)(SENSOR_TYPE),
                            ant_bpwr_calib_handler,
                            ant_bpwr_evt_handler);

ant_bpwr_profile_t m_ant_bpwr;
/** @snippet [ANT BPWR TX Instance] */

ant_bpwr_simulator_t m_ant_bpwr_simulator;    /**< Simulator used to simulate profile data. */

/**@brief Function for dispatching an ANT stack event to all modules with an ANT stack event handler.
 *
 * @details This function is called from the ANT Stack event interrupt handler after an ANT stack
 *          event has been received.
 *
 * @param[in] p_ant_evt  ANT stack event.
 *
 * @snippet [ANT BPWR TX Profile handling] */
void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
    ant_bpwr_sens_evt_handler(&m_ant_bpwr, p_ant_evt);
    ant_state_indicator_evt_handler(p_ant_evt);
}
/** @snippet [ANT BPWR TX Profile handling] */


/**@brief Function for handling bsp events.
 */
/** @snippet [ANT BPWR simulator button] */
void bsp_evt_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            ant_bpwr_simulator_increment(&m_ant_bpwr_simulator);
            break;

        case BSP_EVENT_KEY_1:
            ant_bpwr_simulator_decrement(&m_ant_bpwr_simulator);
            break;

        case BSP_EVENT_KEY_2:
            ant_bpwr_calib_response(&m_ant_bpwr);
            break;

        default:
            break;
    }
}
/** @snippet [ANT BPWR simulator button] */


/**@brief Function for handling ANT BPWR events.
 */
/** @snippet [ANT BPWR simulator call] */
void ant_bpwr_evt_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_evt_t event)
{
    switch (event)
    {
        case ANT_BPWR_PAGE_1_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_16_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_17_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_18_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_80_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_81_UPDATED:
            ant_bpwr_simulator_one_iteration(&m_ant_bpwr_simulator, event);
            break;

        default:
            break;
    }
}
/** @snippet [ANT BPWR simulator call] */


/**@brief Function for handling ANT BPWR events.
 */
/** @snippet [ANT BPWR calibration] */
void ant_bpwr_calib_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_page1_data_t * p_page1)
{
    switch (p_page1->calibration_id)
    {
        case ANT_BPWR_CALIB_ID_MANUAL:
            m_ant_bpwr.BPWR_PROFILE_calibration_id     = ANT_BPWR_CALIB_ID_MANUAL_SUCCESS;
            m_ant_bpwr.BPWR_PROFILE_general_calib_data = CALIBRATION_DATA;
            break;

        case ANT_BPWR_CALIB_ID_AUTO:
            m_ant_bpwr.BPWR_PROFILE_calibration_id     = ANT_BPWR_CALIB_ID_MANUAL_SUCCESS;
            m_ant_bpwr.BPWR_PROFILE_auto_zero_status   = p_page1->auto_zero_status;
            m_ant_bpwr.BPWR_PROFILE_general_calib_data = CALIBRATION_DATA;
            break;

        case ANT_BPWR_CALIB_ID_CUSTOM_REQ:
            m_ant_bpwr.BPWR_PROFILE_calibration_id = ANT_BPWR_CALIB_ID_CUSTOM_REQ_SUCCESS;
            memcpy(m_ant_bpwr.BPWR_PROFILE_custom_calib_data, p_page1->data.custom_calib,
                   sizeof (m_ant_bpwr.BPWR_PROFILE_custom_calib_data));
            break;

        case ANT_BPWR_CALIB_ID_CUSTOM_UPDATE:
            m_ant_bpwr.BPWR_PROFILE_calibration_id = ANT_BPWR_CALIB_ID_CUSTOM_UPDATE_SUCCESS;
            memcpy(m_ant_bpwr.BPWR_PROFILE_custom_calib_data, p_page1->data.custom_calib,
                   sizeof (m_ant_bpwr.BPWR_PROFILE_custom_calib_data));
            break;

        default:
            break;
    }
}
/** @snippet [ANT BPWR calibration] */


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

    // Initialize and start a single continuous mode timer, which is used to update the event time
    // on the main data page.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);

    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                        APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                        bsp_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the BPWR simulator initialization.
 */
void simulator_setup(void)
{
    /** @snippet [ANT BPWR simulator init] */
    const ant_bpwr_simulator_cfg_t simulator_cfg =
    {
        .p_profile   = &m_ant_bpwr,
        .sensor_type = (ant_bpwr_torque_t)(SENSOR_TYPE),
    };

    /** @snippet [ANT BPWR simulator init] */

#if MODIFICATION_TYPE == MODIFICATION_TYPE_AUTO
    /** @snippet [ANT BPWR simulator auto init] */
    ant_bpwr_simulator_init(&m_ant_bpwr_simulator, &simulator_cfg, true);
    /** @snippet [ANT BPWR simulator auto init] */
#else
    /** @snippet [ANT BPWR simulator button init] */
    ant_bpwr_simulator_init(&m_ant_bpwr_simulator, &simulator_cfg, false);
    /** @snippet [ANT BPWR simulator button init] */
#endif
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
/** @snippet [ANT BPWR TX Profile Setup] */
    uint32_t err_code;

    err_code = ant_bpwr_sens_init(&m_ant_bpwr,
                                  BPWR_SENS_CHANNEL_CONFIG(m_ant_bpwr),
                                  BPWR_SENS_PROFILE_CONFIG(m_ant_bpwr));
    APP_ERROR_CHECK(err_code);

    // fill manufacturer's common data page.
    m_ant_bpwr.page_80 = ANT_COMMON_page80(HW_REVISION,
                                           MANUFACTURER_ID,
                                           MODEL_NUMBER);
    // fill product's common data page.
    m_ant_bpwr.page_81 = ANT_COMMON_page81(SW_REVISION_MAJOR,
                                           SW_REVISION_MINOR,
                                           SERIAL_NUMBER);

    m_ant_bpwr.BPWR_PROFILE_auto_zero_status = ANT_BPWR_AUTO_ZERO_OFF;

    err_code = ant_bpwr_sens_open(&m_ant_bpwr);
    APP_ERROR_CHECK(err_code);

    err_code = ant_state_indicator_channel_opened();
    APP_ERROR_CHECK(err_code);
/** @snippet [ANT BPWR TX Profile Setup] */
}


/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    utils_setup();
    softdevice_setup();
    ant_state_indicator_init(m_ant_bpwr.channel_number, BPWR_SENS_CHANNEL_TYPE);
    simulator_setup();
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

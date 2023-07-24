/*
This software is subject to the license described in the license.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * @defgroup ant_bsc_rx_example ANT BSC RX example
 * @{
 * @ingroup nrf_ant_bsc
 *
 * @brief Example of ANT BSC RX profile.
 *
 * Before compiling this example for NRF52, complete the following steps:
 * - Download the S212 SoftDevice from <a href="https://www.thisisant.com/developer/components/nrf52832" target="_blank">thisisant.com</a>.
 * - Extract the downloaded zip file and copy the S212 SoftDevice headers to <tt>\<InstallFolder\>/components/softdevice/s212/headers</tt>.
 * If you are using Keil packs, copy the files into a @c headers folder in your example folder.
 * - Make sure that @ref ANT_LICENSE_KEY in @c nrf_sdm.h is uncommented.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_error.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "ant_stack_config.h"
#include "softdevice_handler.h"
#include "ant_bsc.h"
#include "app_trace.h"
#include "ant_key_manager.h"
#include "ant_state_indicator.h"

#define APP_TIMER_PRESCALER         0x00                                                            /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE     0x04                                                            /**< Size of timer operation queues. */

#define BSC_CHANNEL_NUMBER          0x00                                                            /**< Channel number assigned to BSC profile. */

#define WILDCARD_TRANSMISSION_TYPE  0x00                                                            /**< Wildcard transmission type. */
#define WILDCARD_DEVICE_NUMBER      0x00                                                            /**< Wildcard device number. */

#define ANTPLUS_NETWORK_NUMBER      0x00                                                            /**< Network number. */

#define WHEEL_CIRCUMFERENCE         2070                                                            /**< Bike wheel circumference [mm] */
#define BSC_EVT_TIME_FACTOR         1024                                                            /**< Time unit factor for BSC events */
#define BSC_RPM_TIME_FACTOR         60                                                              /**< Time unit factor for RPM unit */
#define BSC_MS_TO_KPH_NUM           36                                                              /**< Numerator of [m/s] to [kph] ratio */
#define BSC_MS_TO_KPH_DEN           10                                                              /**< Denominator of [m/s] to [kph] ratio */
#define BSC_MM_TO_M_FACTOR          1000                                                            /**< Unit factor [m/s] to [mm/s] */
#define BSC_SPEED_UNIT_FACTOR       (BSC_MS_TO_KPH_DEN * BSC_MM_TO_M_FACTOR)                        /**< Speed unit factor */
#define SPEED_COEFFICIENT           (WHEEL_CIRCUMFERENCE * BSC_EVT_TIME_FACTOR * BSC_MS_TO_KPH_NUM) /**< Coefficient for speed value calculation */
#define CADENCE_COEFFICIENT         (BSC_EVT_TIME_FACTOR * BSC_RPM_TIME_FACTOR)                     /**< Coefficient for cadence value calculation */

#ifdef ENABLE_DEBUG_LOG_SUPPORT
static int32_t accumulated_s_rev_cnt, previous_s_evt_cnt, prev_s_accumulated_rev_cnt,
               accumulated_s_evt_time, previous_s_evt_time, prev_s_accumulated_evt_time = 0;

static int32_t accumulated_c_rev_cnt, previous_c_evt_cnt, prev_c_accumulated_rev_cnt,
               accumulated_c_evt_time, previous_c_evt_time, prev_c_accumulated_evt_time = 0;
#endif // ENABLE_DEBUG_LOG_SUPPORT

/** @snippet [ANT BSC RX Instance] */
void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event);

BSC_DISP_CHANNEL_CONFIG_DEF(m_ant_bsc,
                            BSC_CHANNEL_NUMBER,
                            WILDCARD_TRANSMISSION_TYPE,
                            DISPLAY_TYPE,
                            WILDCARD_DEVICE_NUMBER,
                            ANTPLUS_NETWORK_NUMBER,
                            BSC_MSG_PERIOD_4Hz);
BSC_DISP_PROFILE_CONFIG_DEF(m_ant_bsc,
                            ant_bsc_evt_handler);
ant_bsc_profile_t m_ant_bsc;
/** @snippet [ANT BSC RX Instance] */

/**@brief Function for dispatching a ANT stack event to all modules with a ANT stack event handler.
 *
 * @details This function is called from the ANT Stack event interrupt handler after a ANT stack
 *          event has been received.
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
    ant_bsc_disp_evt_handler(&m_ant_bsc, p_ant_evt);
    ant_state_indicator_evt_handler(p_ant_evt);
}

#ifdef ENABLE_DEBUG_LOG_SUPPORT
__STATIC_INLINE uint32_t calculate_speed(int32_t rev_cnt, int32_t evt_time)
{
    static uint32_t computed_speed   = 0;

    if (rev_cnt != previous_s_evt_cnt)
    {
        accumulated_s_rev_cnt  += rev_cnt - previous_s_evt_cnt;
        accumulated_s_evt_time += evt_time - previous_s_evt_time;

        /* Process rollover */
        if (previous_s_evt_cnt > rev_cnt)
        {
            accumulated_s_rev_cnt += UINT16_MAX + 1;
        }
        if (previous_s_evt_time > evt_time)
        {
            accumulated_s_evt_time += UINT16_MAX + 1;
        }

        previous_s_evt_cnt  = rev_cnt;
        previous_s_evt_time = evt_time;

        computed_speed   = SPEED_COEFFICIENT *
                           (accumulated_s_rev_cnt  - prev_s_accumulated_rev_cnt) /
                           (accumulated_s_evt_time - prev_s_accumulated_evt_time)/
                           BSC_SPEED_UNIT_FACTOR;

        prev_s_accumulated_rev_cnt  = accumulated_s_rev_cnt;
        prev_s_accumulated_evt_time = accumulated_s_evt_time;
    }

    return (uint32_t) computed_speed;
}

static uint32_t calculate_cadence(int32_t rev_cnt, int32_t evt_time)
{
    static uint32_t computed_cadence = 0;

    if (rev_cnt != previous_c_evt_cnt)
    {
        accumulated_c_rev_cnt  += rev_cnt - previous_c_evt_cnt;
        accumulated_c_evt_time += evt_time - previous_c_evt_time;

        /* Process rollover */
        if (previous_c_evt_cnt > rev_cnt)
        {
            accumulated_c_rev_cnt += UINT16_MAX + 1;
        }
        if (previous_c_evt_time > evt_time)
        {
            accumulated_c_evt_time += UINT16_MAX + 1;
        }

        previous_c_evt_cnt  = rev_cnt;
        previous_c_evt_time = evt_time;

        computed_cadence = CADENCE_COEFFICIENT *
                           (accumulated_c_rev_cnt  - prev_c_accumulated_rev_cnt) /
                           (accumulated_c_evt_time - prev_c_accumulated_evt_time);

        prev_c_accumulated_rev_cnt  = accumulated_c_rev_cnt;
        prev_c_accumulated_evt_time = accumulated_c_evt_time;
    }

    return (uint32_t) computed_cadence;
}
#endif // ENABLE_DEBUG_LOG_SUPPORT

void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event)
{

    switch (event)
    {
        case ANT_BSC_PAGE_0_UPDATED:
            /* fall through */
        case ANT_BSC_PAGE_1_UPDATED:
            /* fall through */
        case ANT_BSC_PAGE_2_UPDATED:
            /* fall through */
        case ANT_BSC_PAGE_3_UPDATED:
            /* fall through */
        case ANT_BSC_PAGE_4_UPDATED:
            /* fall through */
        case ANT_BSC_PAGE_5_UPDATED:
            /* Log computed value */
            app_trace_log("Page was updated\n\r");

            if (DISPLAY_TYPE == BSC_SPEED_DEVICE_TYPE)
            {
                app_trace_log("%-30s %u kph\n\r",
                              "Computed speed value:",
                              (unsigned int) calculate_speed(p_profile->BSC_PROFILE_rev_count,
                                                             p_profile->BSC_PROFILE_event_time));
            }
            else if (DISPLAY_TYPE == BSC_CADENCE_DEVICE_TYPE)
            {
                app_trace_log("%-30s %u rpm\n\r",
                              "Computed cadence value:",
                              (unsigned int) calculate_cadence(p_profile->BSC_PROFILE_rev_count,
                                                               p_profile->BSC_PROFILE_event_time));
            }

            app_trace_log("\r\n\r\n");
            break;

        case ANT_BSC_COMB_PAGE_0_UPDATED:

            app_trace_log("%-30s %u kph\n\r",
                          "Computed speed value:",
                          (unsigned int) calculate_speed(p_profile->BSC_PROFILE_speed_rev_count,
                                                         p_profile->BSC_PROFILE_speed_event_time));
            app_trace_log("%-30s %u rpm\n\r",
                          "Computed cadence value:",
                          (unsigned int) calculate_cadence(p_profile->BSC_PROFILE_cadence_rev_count,
                                                           p_profile->BSC_PROFILE_cadence_event_time));
            app_trace_log("\r\n\r\n");
            break;

        default:
            break;
    }
}


/**@brief Function for the Timer, Tracer and BSP initialization.
 */
static void utils_setup(void)
{
    uint32_t err_code;

    app_trace_init();
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for ANT stack initialization.
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

    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);

    err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUMBER);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for BSC profile initialization.
 *
 * @details Initializes the BSC profile and open ANT channel.
 */
static void profile_setup(void)
{
/** @snippet [ANT BSC RX Profile Setup] */
    uint32_t err_code;

    err_code = ant_bsc_disp_init(&m_ant_bsc,
                                 BSC_DISP_CHANNEL_CONFIG(m_ant_bsc),
                                 BSC_DISP_PROFILE_CONFIG(m_ant_bsc));
    APP_ERROR_CHECK(err_code);

    err_code = ant_bsc_disp_open(&m_ant_bsc);
    APP_ERROR_CHECK(err_code);

    err_code = ant_state_indicator_channel_opened();
    APP_ERROR_CHECK(err_code);
/** @snippet [ANT BSC RX Profile Setup] */
}


/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    utils_setup();
    softdevice_setup();
    ant_state_indicator_init(m_ant_bsc.channel_number, BSC_DISP_CHANNEL_TYPE);
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

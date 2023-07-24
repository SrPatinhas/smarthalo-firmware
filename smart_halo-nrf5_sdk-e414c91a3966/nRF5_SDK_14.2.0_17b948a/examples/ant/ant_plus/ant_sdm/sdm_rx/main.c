/**
 * This software is subject to the ANT+ Shared Source License
 * www.thisisant.com/swlicenses
 * Copyright (c) Dynastream Innovations, Inc. 2012
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 * 1) Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 
 * 2) Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 * 
 * 3) Neither the name of Dynastream nor the names of its
 *    contributors may be used to endorse or promote products
 *    derived from this software without specific prior
 *    written permission.
 * 
 * The following actions are prohibited:
 * 1) Redistribution of source code containing the ANT+ Network
 *    Key. The ANT+ Network Key is available to ANT+ Adopters.
 *    Please refer to http://thisisant.com to become an ANT+
 *    Adopter and access the key.
 * 
 * 2) Reverse engineering, decompilation, and/or disassembly of
 *    software provided in binary form under this license.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE HEREBY
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; DAMAGE TO ANY DEVICE, LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE. SOME STATES DO NOT ALLOW
 * THE EXCLUSION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES, SO THE
 * ABOVE LIMITATIONS MAY NOT APPLY TO YOU.
 * 
 */
/**@file
 * @defgroup ant_sdm_rx_example ANT SDM RX example
 * @{
 * @ingroup nrf_ant_sdm
 *
 * @brief Example of ANT SDM RX Profile.
 *
 * Before compiling this example for NRF52, complete the following steps:
 * - Download the S212 SoftDevice from <a href="https://www.thisisant.com/developer/components/nrf52832" target="_blank">thisisant.com</a>.
 * - Extract the downloaded zip file and copy the S212 SoftDevice headers to <tt>\<InstallFolder\>/components/softdevice/s212/headers</tt>.
 * If you are using Keil packs, copy the files into a @c headers folder in your example folder.
 * - Make sure that @ref ANT_LICENSE_KEY in @c nrf_sdm.h is uncommented.
 */

#include <stdio.h>
#include "nrf.h"
#include "bsp.h"
#include "hardfault.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ant.h"
#include "ant_key_manager.h"
#include "ant_sdm.h"
#include "bsp_btn_ant.h"
#include "ant_state_indicator.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/** @snippet [ANT SDM RX Instance] */
void ant_sdm_evt_handler(ant_sdm_profile_t * p_profile, ant_sdm_evt_t event);

SDM_DISP_CHANNEL_CONFIG_DEF(m_ant_sdm,
                            SDM_CHANNEL_NUM,
                            CHAN_ID_TRANS_TYPE,
                            CHAN_ID_DEV_NUM,
                            ANTPLUS_NETWORK_NUM,
                            SDM_MSG_PERIOD_4Hz);
SDM_DISP_PROFILE_CONFIG_DEF(m_ant_sdm,
                            ant_sdm_evt_handler);

static ant_sdm_profile_t m_ant_sdm;
/** @snippet [ANT SDM RX Instance] */


NRF_SDH_ANT_OBSERVER(m_ant_observer, ANT_SDM_ANT_OBSERVER_PRIO, ant_sdm_disp_evt_handler, &m_ant_sdm);


/**@brief Function for handling ANT SDM events.
 */
void ant_sdm_evt_handler(ant_sdm_profile_t * p_profile, ant_sdm_evt_t event)
{
    nrf_pwr_mgmt_feed();

    switch (event)
    {
        case ANT_SDM_PAGE_1_UPDATED:
            /* fall through */
        case ANT_SDM_PAGE_2_UPDATED:
            /* fall through */
        case ANT_SDM_PAGE_3_UPDATED:
            /* fall through */
        case ANT_SDM_PAGE_16_UPDATED:
            /* fall through */
        case ANT_SDM_PAGE_22_UPDATED:
            /* fall through */
        case ANT_SDM_PAGE_80_UPDATED:
            /* fall through */
        case ANT_SDM_PAGE_81_UPDATED:
            NRF_LOG_INFO("Page was updated");
            break;

        case ANT_SDM_PAGE_REQUEST_SUCCESS:
            NRF_LOG_INFO("ANT_SDM_PAGE_REQUEST_SUCCESS");
            break;

        case ANT_SDM_PAGE_REQUEST_FAILED:
            NRF_LOG_INFO("ANT_SDM_PAGE_REQUEST_FAILED");
            break;

        default:
            break;
    }
}

/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    ret_code_t               err_code;
    ant_common_page70_data_t page70;

    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            page70   = ANT_COMMON_PAGE_DATA_REQUEST(ANT_SDM_PAGE_16);
            err_code = ant_sdm_page_request(&m_ant_sdm, &page70);
            APP_ERROR_CHECK(err_code);
            break;

        case BSP_EVENT_KEY_1:
            page70   = ANT_COMMON_PAGE_DATA_REQUEST(ANT_SDM_PAGE_22);
            err_code = ant_sdm_page_request(&m_ant_sdm, &page70);
            APP_ERROR_CHECK(err_code);
            break;

        case BSP_EVENT_SLEEP:
            nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
            break;

        default:
            return; // no implementation needed
    }
}

/**
 * @brief Function for shutdown events.
 *
 * @param[in]   event       Shutdown type.
 */
static bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            err_code = bsp_btn_ant_sleep_mode_prepare();
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }

    return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler,  APP_SHUTDOWN_HANDLER_PRIORITY);

/**@brief Function for the timer, tracer, and BSP initialization.
 */
static void utils_setup(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                        bsp_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ant_init(m_ant_sdm.channel_number, SDM_DISP_CHANNEL_TYPE);
    APP_ERROR_CHECK(err_code);

    err_code = ant_state_indicator_init(m_ant_sdm.channel_number, SDM_DISP_CHANNEL_TYPE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for ANT stack initialization.
 *
 * @details Initializes the SoftDevice and the ANT event interrupt.
 */
static void softdevice_setup(void)
{
    ret_code_t err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    ASSERT(nrf_sdh_is_enabled());

    err_code = nrf_sdh_ant_enable();
    APP_ERROR_CHECK(err_code);

    err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUM);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for SDM Profile initialization.
 *
 * @details Initializes the SDM Profile and opens the ANT channel.
 */
static void profile_setup(void)
{
/** @snippet [ANT SDM RX Profile Setup] */
    uint32_t err_code;

    err_code = ant_sdm_disp_init(&m_ant_sdm,
                                 SDM_DISP_CHANNEL_CONFIG(m_ant_sdm),
                                 SDM_DISP_PROFILE_CONFIG(m_ant_sdm));
    APP_ERROR_CHECK(err_code);

    err_code = ant_sdm_disp_open(&m_ant_sdm);
    APP_ERROR_CHECK(err_code);

    err_code = ant_state_indicator_channel_opened();
    APP_ERROR_CHECK(err_code);
/** @snippet [ANT SDM RX Profile Setup] */
}

/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    utils_setup();
    softdevice_setup();
    profile_setup();

    for (;;)
    {
        NRF_LOG_FLUSH();
        nrf_pwr_mgmt_run();
    }
}


/**
 *@}
 **/

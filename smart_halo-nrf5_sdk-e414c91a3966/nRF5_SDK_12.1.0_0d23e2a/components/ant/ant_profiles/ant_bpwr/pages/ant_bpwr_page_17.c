/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "sdk_config.h"
#if ANT_BPWR_ENABLED

#include "ant_bpwr_page_17.h"
#include "app_util.h"

#define NRF_LOG_MODULE_NAME "ANT_BPWR_PAGE_17"
#if ANT_BPWR_PAGE_17_LOG_ENABLED
#define NRF_LOG_LEVEL       ANT_BPWR_PAGE_17_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  ANT_BPWR_PAGE_17_INFO_COLOR
#else // ANT_BPWR_PAGE_17_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // ANT_BPWR_PAGE_17_LOG_ENABLED
#include "nrf_log.h"

static void page17_data_log(ant_bpwr_page17_data_t const * p_page_data)
{
    NRF_LOG_INFO("Wheel:\r\n");
    ant_bpwr_page_torque_log((ant_bpwr_page_torque_data_t *) p_page_data);
}


void ant_bpwr_page_17_encode(uint8_t                      * p_page_buffer,
                             ant_bpwr_page17_data_t const * p_page_data)
{
    ant_bpwr_page_torque_encode(p_page_buffer, (ant_bpwr_page_torque_data_t *)p_page_data);
    page17_data_log(p_page_data);
}


void ant_bpwr_page_17_decode(uint8_t const          * p_page_buffer,
                             ant_bpwr_page17_data_t * p_page_data)
{
    ant_bpwr_page_torque_decode(p_page_buffer, (ant_bpwr_page_torque_data_t *) p_page_data);
    page17_data_log(p_page_data);
}

#endif // ANT_BPWR_ENABLED

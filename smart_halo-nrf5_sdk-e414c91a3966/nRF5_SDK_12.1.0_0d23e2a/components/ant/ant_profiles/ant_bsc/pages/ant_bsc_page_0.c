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
#if ANT_BSC_ENABLED

#include "ant_bsc_page_0.h"
#include "ant_bsc_utils.h"

#define NRF_LOG_MODULE_NAME "ANT_BCS_PAGE_0"
#if ANT_BSC_PAGE_0_LOG_ENABLED
#define NRF_LOG_LEVEL       ANT_BSC_PAGE_0_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  ANT_BSC_PAGE_0_INFO_COLOR
#else // ANT_BSC_PAGE_0_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // ANT_BSC_PAGE_0_LOG_ENABLED
#include "nrf_log.h"

/**@brief BSC page 0 data layout structure. */
typedef struct
{
    uint8_t reserved[3];
    uint8_t bsc_evt_time_LSB;
    uint8_t bsc_evt_time_MSB;
    uint8_t bsc_rev_count_LSB;
    uint8_t bsc_rev_count_MSB;
}ant_bsc_page0_data_layout_t;

/**@brief Function for printing speed or cadence page0 data. */
static void page0_data_log(ant_bsc_page0_data_t const * p_page_data)
{
    NRF_LOG_INFO("Revolution count:          %u\r\n", (unsigned int)p_page_data->rev_count);

    NRF_LOG_INFO("BSC event time:            %u.%03us\r\n",
              (unsigned int)ANT_BSC_EVENT_TIME_SEC(p_page_data->event_time),
              (unsigned int)ANT_BSC_EVENT_TIME_MSEC(p_page_data->event_time));

//    NRF_LOG_INFO("%03us\r\n", (unsigned int)ANT_BSC_EVENT_TIME_MSEC(p_page_data->event_time));
}

void ant_bsc_page_0_encode(uint8_t * p_page_buffer, ant_bsc_page0_data_t const * p_page_data)
{
    ant_bsc_page0_data_layout_t * p_outcoming_data = (ant_bsc_page0_data_layout_t *) p_page_buffer;
    uint16_t                      event_time       = p_page_data->event_time;
    uint16_t                      rev_count        = p_page_data->rev_count;

    p_outcoming_data->reserved[0]       = UINT8_MAX;
    p_outcoming_data->reserved[1]       = UINT8_MAX;
    p_outcoming_data->reserved[2]       = UINT8_MAX;
    p_outcoming_data->bsc_evt_time_LSB  = (uint8_t)(event_time & UINT8_MAX);
    p_outcoming_data->bsc_evt_time_MSB  = (uint8_t)((event_time >> 8) & UINT8_MAX);
    p_outcoming_data->bsc_rev_count_LSB = (uint8_t)(rev_count & UINT8_MAX);
    p_outcoming_data->bsc_rev_count_MSB = (uint8_t)((rev_count >> 8) & UINT8_MAX);

    page0_data_log(p_page_data);
}

void ant_bsc_page_0_decode(uint8_t const * p_page_buffer, ant_bsc_page0_data_t * p_page_data)
{
    ant_bsc_page0_data_layout_t const * p_incoming_data = (ant_bsc_page0_data_layout_t *)p_page_buffer;

    uint16_t event_time = (uint16_t)((p_incoming_data->bsc_evt_time_MSB << 8)
                          + p_incoming_data->bsc_evt_time_LSB);

    uint16_t revolution_count = (uint16_t) ((p_incoming_data->bsc_rev_count_MSB << 8)
                                + p_incoming_data->bsc_rev_count_LSB);

    p_page_data->event_time = event_time;
    p_page_data->rev_count  = revolution_count;

    page0_data_log(p_page_data);
}

#endif // ANT_BSC_ENABLED

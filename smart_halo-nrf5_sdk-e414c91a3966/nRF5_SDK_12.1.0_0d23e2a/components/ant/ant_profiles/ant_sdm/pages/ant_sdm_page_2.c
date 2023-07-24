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
#if ANT_SDM_ENABLED

#include "ant_sdm_page_2.h"
#include "ant_sdm_utils.h"

#define NRF_LOG_MODULE_NAME "ANT_SDM_PAGE_2"
#if ANT_SDM_PAGE_2_LOG_ENABLED
#define NRF_LOG_LEVEL       ANT_SDM_PAGE_2_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  ANT_SDM_PAGE_2_INFO_COLOR
#else // ANT_SDM_PAGE_2_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // ANT_SDM_PAGE_2_LOG_ENABLED
#include "nrf_log.h"


/**@brief SDM page 2 data layout structure. */
typedef struct
{
    uint8_t reserved0[2];
    uint8_t cadence_integer;
    uint8_t reserved1           : 4;
    uint8_t cadence_fractional  : 4;
    uint8_t reserved2;
    uint8_t reserved3;
    uint8_t status;
}ant_sdm_page2_data_layout_t;

/**@brief Function for page 2 data.
 *
 * @param[in]  p_common_data      Pointer to the page 2 data.
 */
static void page_2_data_log(ant_sdm_page2_data_t const * p_page_data)
{
    static const char * p_location[4] = {"Laces", "Midsole", "Other", "Ankle"};
    static const char * p_battery[4]  = {"New", "Good", "OK", "Low"};
    static const char * p_health[4]   = {"OK", "Error", "Warning", ""};
    static const char * p_state[4]    = {"Inactive", "Active", "", ""};

    uint16_t cadence = ANT_SDM_CADENCE_RESCALE(p_page_data->cadence);

    NRF_LOG_INFO("Status:\r\n");
    NRF_LOG_INFO("state:                                %s\r\n",
                (uint32_t)p_state[p_page_data->status.items.state]);
    NRF_LOG_INFO("health:                               %s\r\n",
                (uint32_t)p_health[p_page_data->status.items.health]);
    NRF_LOG_INFO("battery:                              %s\r\n",
                (uint32_t)p_battery[p_page_data->status.items.battery]);
    NRF_LOG_INFO("location:                             %s\r\n",
                (uint32_t)p_location[p_page_data->status.items.location]);
    NRF_LOG_INFO("Cadence                               %u.%01u strides/min\r\n",
                cadence / ANT_SDM_CADENCE_DISP_PRECISION,
                cadence % ANT_SDM_CADENCE_DISP_PRECISION);
}


void ant_sdm_page_2_encode(uint8_t                    * p_page_buffer,
                           ant_sdm_page2_data_t const * p_page_data)
{
    ant_sdm_page2_data_layout_t * p_outcoming_data = (ant_sdm_page2_data_layout_t *)p_page_buffer;
    uint8_t                       status           = p_page_data->status.byte;
    uint16_t                      cadence          = p_page_data->cadence;

    p_outcoming_data->reserved0[0]       = UINT8_MAX;
    p_outcoming_data->reserved0[1]       = UINT8_MAX;
    p_outcoming_data->cadence_integer    = cadence / ANT_SDM_CADENCE_UNIT_REVERSAL;
    p_outcoming_data->cadence_fractional = cadence % ANT_SDM_CADENCE_UNIT_REVERSAL;
    p_outcoming_data->reserved3          = UINT8_MAX;
    p_outcoming_data->status             = status;
    page_2_data_log(p_page_data);
}


void ant_sdm_page_2_decode(uint8_t const        * p_page_buffer,
                           ant_sdm_page2_data_t * p_page_data)
{
    ant_sdm_page2_data_layout_t const * p_incoming_data =
        (ant_sdm_page2_data_layout_t *)p_page_buffer;

    uint8_t  status  = p_incoming_data->status;
    uint16_t cadence = p_incoming_data->cadence_integer * ANT_SDM_CADENCE_UNIT_REVERSAL
                       + p_incoming_data->cadence_fractional;

    p_page_data->cadence     = cadence;
    p_page_data->status.byte = status;

    page_2_data_log(p_page_data);
}

#endif // ANT_SDM_ENABLED

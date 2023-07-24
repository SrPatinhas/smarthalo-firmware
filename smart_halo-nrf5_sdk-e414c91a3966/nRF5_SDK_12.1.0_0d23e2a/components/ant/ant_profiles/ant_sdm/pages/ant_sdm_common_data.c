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

#include "ant_sdm_common_data.h"
#include "ant_sdm_utils.h"
#include "app_util.h"
#include "nordic_common.h"

#define NRF_LOG_MODULE_NAME "ANT_SDM"
#if ANT_SDM_LOG_ENABLED
#define NRF_LOG_LEVEL       ANT_SDM_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  ANT_SDM_INFO_COLOR
#else // ANT_SDM_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // ANT_SDM_LOG_ENABLED
#include "nrf_log.h"

/**@brief SDM common page data layout structure. */
typedef struct
{
    uint8_t reserved0[3];
    uint8_t speed_integer   :4;
    uint8_t reserved1       :4;
    uint8_t speed_fractional;
    uint8_t reserved2[2];
}ant_sdm_speed_data_layout_t;

/**@brief Function for tracing common data.
 *
 * @param[in]  p_common_data      Pointer to the common data.
 */
static void speed_data_log(ant_sdm_common_data_t const * p_common_data)
{
    uint32_t speed = ANT_SDM_SPEED_RESCALE(p_common_data->speed);
    UNUSED_VARIABLE(speed);

    NRF_LOG_INFO("Speed                                        %u.%02u m/s\r\n\n",
                 (unsigned int)(speed / ANT_SDM_SPEED_DISP_PRECISION),
                 (unsigned int)(speed % ANT_SDM_SPEED_DISP_PRECISION));
}

void ant_sdm_speed_encode(uint8_t                     * p_page_buffer,
                          ant_sdm_common_data_t const * p_common_data)
{
    ant_sdm_speed_data_layout_t * p_outcoming_data  = (ant_sdm_speed_data_layout_t *)p_page_buffer;
    uint16_t speed                                  = p_common_data->speed;

    p_outcoming_data->speed_integer                 = speed / ANT_SDM_SPEED_UNIT_REVERSAL;
    p_outcoming_data->speed_fractional              = speed % ANT_SDM_SPEED_UNIT_REVERSAL;

    speed_data_log(p_common_data);
}

void ant_sdm_speed_decode(uint8_t const         * p_page_buffer,
                          ant_sdm_common_data_t * p_common_data)
{
    ant_sdm_speed_data_layout_t const * p_incoming_data = (ant_sdm_speed_data_layout_t *)p_page_buffer;
    uint16_t                            speed           = (p_incoming_data->speed_integer
                                                        * ANT_SDM_SPEED_UNIT_REVERSAL)
                                                        + p_incoming_data->speed_fractional;

    p_common_data->speed  = speed;

    speed_data_log(p_common_data);
}

#endif // ANT_SDM_ENABLED

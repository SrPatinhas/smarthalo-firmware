/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gattc_app.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gattc_characteristics_discover_req_enc(
    uint16_t                               conn_handle,
    ble_gattc_handle_range_t const * const p_handle_range,
    uint8_t * const                        p_buf,
    uint32_t *                             p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 1 + 2, *p_buf_len);

    p_buf[index++] = SD_BLE_GATTC_CHARACTERISTICS_DISCOVER;
    index         += uint16_encode(conn_handle, &p_buf[index]);

    SER_ASSERT_LENGTH_LEQ(index + 1, *p_buf_len);
    p_buf[index++] = (p_handle_range != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_handle_range != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 2 + 2, *p_buf_len);

        index += uint16_encode(p_handle_range->start_handle, &p_buf[index]);
        index += uint16_encode(p_handle_range->end_handle, &p_buf[index]);
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gattc_characteristics_discover_rsp_dec(uint8_t const * const p_buf,
                                                    uint32_t              packet_len,
                                                    uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GATTC_CHARACTERISTICS_DISCOVER,
                               p_result_code);
}

/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gap_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "nordic_common.h"
#include "app_util.h"


uint32_t ble_gap_ppcp_set_req_dec(uint8_t const * const           p_buf,
                                  uint32_t                        packet_len,
                                  ble_gap_conn_params_t * * const pp_conn_params)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_conn_params);

    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 1, packet_len);
    SER_ASSERT(p_buf[index] == SD_BLE_GAP_PPCP_SET, NRF_ERROR_INVALID_PARAM);

    index++;

    switch (p_buf[index++])
    {
        case SER_FIELD_NOT_PRESENT:
            *pp_conn_params = NULL;
            break;

        case SER_FIELD_PRESENT:
            SER_ASSERT_LENGTH_LEQ(index + 8, packet_len);
            SER_ASSERT_NOT_NULL(*pp_conn_params);
            uint16_dec(p_buf, packet_len, &index, &(*pp_conn_params)->min_conn_interval);
            uint16_dec(p_buf, packet_len, &index, &(*pp_conn_params)->max_conn_interval);
            uint16_dec(p_buf, packet_len, &index, &(*pp_conn_params)->slave_latency);
            uint16_dec(p_buf, packet_len, &index, &(*pp_conn_params)->conn_sup_timeout);
            break;

        default:
            return NRF_ERROR_INVALID_DATA;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}


uint32_t ble_gap_ppcp_set_rsp_enc(uint32_t         return_code,
                                  uint8_t * const  p_buf,
                                  uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_PPCP_SET, return_code, p_buf, p_buf_len);
}

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
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"


uint32_t ble_gap_lesc_oob_data_set_req_dec(uint8_t const * const          p_buf,
                                          uint32_t                       packet_len,
                                          uint16_t *                     p_conn_handle,
                                          ble_gap_lesc_oob_data_t * *       pp_oobd_own,
                                          ble_gap_lesc_oob_data_t * *       pp_oobd_peer)
{
    uint32_t index    = SER_CMD_HEADER_SIZE;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_oobd_own);
    SER_ASSERT_NOT_NULL(pp_oobd_peer);

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void **)pp_oobd_own, ble_gap_lesc_oob_data_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void **)pp_oobd_peer, ble_gap_lesc_oob_data_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}

uint32_t ble_gap_lesc_oob_data_set_rsp_enc(uint32_t                     return_code,
                                          uint8_t * const              p_buf,
                                          uint32_t * const             p_buf_len)
{
    uint32_t err_code = NRF_SUCCESS;

    err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_LESC_OOB_DATA_SET,
                                               return_code,
                                               p_buf,
                                               p_buf_len);

    return err_code;
}

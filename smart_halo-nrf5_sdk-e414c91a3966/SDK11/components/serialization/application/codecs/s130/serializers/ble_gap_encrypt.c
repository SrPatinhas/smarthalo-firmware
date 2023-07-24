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

#include "ble_gap_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap.h"
#include "app_util.h"


uint32_t ble_gap_encrypt_req_enc( uint16_t             	            conn_handle,
								                  ble_gap_master_id_t const * const p_master_id,
								                  ble_gap_enc_info_t const  * const p_enc_info,
								                  uint8_t                   * const p_buf,
								                  uint32_t                  * const p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 1 + 2 + 1, *p_buf_len);

    p_buf[index++] = SD_BLE_GAP_ENCRYPT;
    index         += uint16_encode(conn_handle, &p_buf[index]);

    SER_ASSERT_LENGTH_LEQ(index + 1, *p_buf_len);
    p_buf[index++] = (p_master_id != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_master_id != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + BLE_GAP_SEC_RAND_LEN + 2, *p_buf_len);
        index += uint16_encode(p_master_id->ediv, &p_buf[index]);
        memcpy(&p_buf[index], p_master_id->rand, BLE_GAP_SEC_RAND_LEN);
        index += BLE_GAP_SEC_RAND_LEN;
    }

    p_buf[index++] = (p_enc_info != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_enc_info != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + BLE_GAP_SEC_KEY_LEN + 1, *p_buf_len);
        memcpy(&p_buf[index], p_enc_info->ltk, BLE_GAP_SEC_KEY_LEN);
        index += BLE_GAP_SEC_KEY_LEN;
        p_buf[index++] = (p_enc_info->auth | (p_enc_info->ltk_len << 1));
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gap_encrypt_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint32_t      * const p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_ENCRYPT, p_result_code);
}

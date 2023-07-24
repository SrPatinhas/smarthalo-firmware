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

#include "ble_gatts_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "app_util.h"
#include "cond_field_serialization.h"

uint32_t ble_gatts_attr_get_req_dec(uint8_t   const * const p_buf,
                                    uint32_t                packet_len,
                                    uint16_t              * p_handle,
                                    ble_uuid_t          * * pp_uuid,
                                    ble_gatts_attr_md_t * * pp_md)
{
    uint32_t index = 0;
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  opcode;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_uuid);
    SER_ASSERT_NOT_NULL(pp_md);
    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 2 + 1, packet_len);

    err_code = uint8_t_dec(p_buf, packet_len, &index, &opcode);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT(opcode == SD_BLE_GATTS_ATTR_GET, NRF_ERROR_INVALID_PARAM);

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_uuid, ble_uuid_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_md, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_gatts_attr_get_rsp_enc(uint32_t               return_code,
                                    uint8_t * const        p_buf,
                                    uint32_t * const       p_buf_len,
                                    ble_gatts_attr_md_t  * p_md)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t buflen   = *p_buf_len;
    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTS_ATTR_GET, return_code,
                                                        p_buf, p_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }
    uint32_t index = *p_buf_len;
    err_code = cond_field_enc(p_md, p_buf, buflen, &index, ble_gatts_attr_md_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}

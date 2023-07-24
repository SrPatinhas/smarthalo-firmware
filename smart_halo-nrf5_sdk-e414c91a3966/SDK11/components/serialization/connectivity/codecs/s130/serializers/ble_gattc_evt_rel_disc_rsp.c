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

#include "ble_gattc_evt_conn.h"
#include "ble_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "app_util.h"


uint32_t ble_gattc_evt_rel_disc_rsp_enc(ble_evt_t const * const p_event,
                                        uint32_t                event_len,
                                        uint8_t * const         p_buf,
                                        uint32_t * const        p_buf_len)
{
    uint32_t index      = 0;
    uint32_t error_code = NRF_SUCCESS;
    uint16_t evt_header = BLE_GATTC_EVT_REL_DISC_RSP;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;

    error_code = uint16_t_enc(&evt_header, p_buf, total_len, &index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = uint16_t_enc(&p_event->evt.gattc_evt.conn_handle, p_buf, total_len, &index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = uint16_t_enc(&p_event->evt.gattc_evt.gatt_status, p_buf, total_len, &index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = uint16_t_enc(&p_event->evt.gattc_evt.error_handle, p_buf, total_len, &index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = ble_gattc_evt_rel_disc_rsp_t_enc(&p_event->evt.gattc_evt.params.rel_disc_rsp,
                                                  p_buf,
                                                  total_len,
                                                  &index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    *p_buf_len = index;

    return error_code;
}

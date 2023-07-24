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

#include "ble_gap_evt_app.h"
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "app_util.h"


uint32_t ble_gap_evt_sec_info_request_dec(uint8_t const * const p_buf,
                                          uint32_t              packet_len,
                                          ble_evt_t * const     p_event,
                                          uint32_t * const      p_event_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(SER_EVT_CONN_HANDLE_SIZE + 2, packet_len);

    uint32_t event_len = (uint16_t) (offsetof(ble_evt_t, evt.gap_evt.params.sec_info_request)) +
                         sizeof (ble_gap_evt_sec_info_request_t) -
                         sizeof (ble_evt_hdr_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id  = BLE_GAP_EVT_SEC_INFO_REQUEST;
    p_event->header.evt_len = event_len;
    uint16_dec(p_buf, packet_len, &index, &p_event->evt.gap_evt.conn_handle);

    err_code = ble_gap_evt_sec_info_request_t_dec(p_buf, packet_len, &index, &p_event->evt.gap_evt.params.sec_info_request);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   SER_ASSERT_LENGTH_EQ(index, packet_len);
   *p_event_len = event_len;

   return NRF_SUCCESS;
}

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

#include "ble_gatts_evt_app.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gatts_evt_hvc_dec(uint8_t const * const p_buf,
                               uint32_t              packet_len,
                               ble_evt_t * const     p_event,
                               uint32_t * const      p_event_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(SER_EVT_CONN_HANDLE_SIZE + 1, packet_len);

    uint32_t event_len = SER_EVT_CONN_HANDLE_SIZE + sizeof (ble_gatts_evt_hvc_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id  = BLE_GATTS_EVT_HVC;
    p_event->header.evt_len = event_len;

    uint16_dec(p_buf, packet_len, &index, &p_event->evt.gatts_evt.conn_handle);
    uint16_dec(p_buf, packet_len, &index, &p_event->evt.gatts_evt.params.hvc.handle);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    *p_event_len = event_len;

    return NRF_SUCCESS;
}

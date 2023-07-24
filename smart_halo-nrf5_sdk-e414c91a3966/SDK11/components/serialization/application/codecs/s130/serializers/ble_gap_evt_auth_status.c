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
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "app_util.h"
#include "app_ble_gap_sec_keys.h"

extern ser_ble_gap_app_keyset_t m_app_keys_table[];

uint32_t ble_gap_evt_auth_status_dec(uint8_t const * const p_buf,
                                     uint32_t              packet_len,
                                     ble_evt_t * const     p_event,
                                     uint32_t * const      p_event_len)
{
    uint32_t index = 0;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t conn_index;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(SER_EVT_CONN_HANDLE_SIZE + 6, packet_len);

    uint32_t event_len = (uint16_t) (offsetof(ble_evt_t, evt.gap_evt.params.auth_status)) +
                         sizeof (ble_gap_evt_adv_report_t) -
                         sizeof (ble_evt_hdr_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    p_event->header.evt_id  = BLE_GAP_EVT_AUTH_STATUS;
    p_event->header.evt_len = event_len;

    err_code = uint16_t_dec(p_buf, packet_len, &index, &(p_event->evt.gap_evt.conn_handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_evt_auth_status_t_dec(p_buf, packet_len, &index, &(p_event->evt.gap_evt.params.auth_status));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_event->evt.gap_evt.params.auth_status.bonded)
    {
		// keyset is an extension of standard event data - used to synchronize keys at application
		err_code = app_ble_gap_sec_context_find(p_event->evt.gap_evt.conn_handle, &conn_index);
		SER_ASSERT(err_code == NRF_SUCCESS, err_code);

		err_code = ble_gap_sec_keyset_t_dec(p_buf, packet_len, &index, &(m_app_keys_table[conn_index].keyset));
		SER_ASSERT(err_code == NRF_SUCCESS, err_code);

		err_code = app_ble_gap_sec_context_destroy(p_event->evt.gap_evt.conn_handle);
		SER_ASSERT(err_code == NRF_SUCCESS, err_code);

		SER_ASSERT_LENGTH_EQ(index, packet_len);
		*p_event_len = event_len;
    }

    return err_code;
}

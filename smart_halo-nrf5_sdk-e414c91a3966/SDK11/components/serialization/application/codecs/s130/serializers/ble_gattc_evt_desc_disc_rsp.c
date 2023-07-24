/* Copyright (c) Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of other
 * contributors to this software may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * 4. This software must only be used in a processor manufactured by Nordic
 * Semiconductor ASA, or in a processor manufactured by a third party that
 * is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ble_gattc_evt_app.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gattc_evt_desc_disc_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         ble_evt_t * const     p_event,
                                         uint32_t * const      p_event_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(SER_EVT_CONN_HANDLE_SIZE + 6, packet_len);

    uint16_t tmp_conn_handle;
    uint16_t tmp_gatt_status;
    uint16_t tmp_error_handle;
    uint16_t tmp_service_count;
    uint16_dec(p_buf, packet_len, &index, &tmp_conn_handle);
    uint16_dec(p_buf, packet_len, &index, &tmp_gatt_status);
    uint16_dec(p_buf, packet_len, &index, &tmp_error_handle);
    uint16_dec(p_buf, packet_len, &index, &tmp_service_count);

    uint32_t event_len = offsetof(ble_evt_t, evt.gattc_evt.params.desc_disc_rsp) +
                         sizeof (uint16_t) + tmp_service_count * sizeof (ble_gattc_desc_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id                            = BLE_GATTC_EVT_DESC_DISC_RSP;
    p_event->header.evt_len                           = event_len;
    p_event->evt.gattc_evt.conn_handle                = tmp_conn_handle;
    p_event->evt.gattc_evt.gatt_status                = tmp_gatt_status;
    p_event->evt.gattc_evt.error_handle               = tmp_error_handle;
    p_event->evt.gattc_evt.params.desc_disc_rsp.count = tmp_service_count;

    SER_ASSERT_LENGTH_LEQ(index + (tmp_service_count * 5), packet_len);

    for (uint16_t i = 0; i < tmp_service_count; i++)
    {
        uint16_dec(p_buf, packet_len, &index,
                   &p_event->evt.gattc_evt.params.desc_disc_rsp.descs[i].handle);
        uint16_dec(p_buf, packet_len, &index,
                   &p_event->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.uuid);
        uint8_dec(p_buf, packet_len, &index,
                  &p_event->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.type);
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return NRF_SUCCESS;
}

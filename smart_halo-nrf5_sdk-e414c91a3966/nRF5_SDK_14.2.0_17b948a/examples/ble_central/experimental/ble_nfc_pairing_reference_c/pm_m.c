/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "pm_m.h"
#include "sdk_errors.h"
#include "peer_manager.h"
#include "ble_conn_state.h"
#include "ble_m.h"
#include "nfc_pair_m.h"

#define NRF_LOG_MODULE_NAME PM_M
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Verify bonding and keys distribution settings.
#if ((BLE_NFC_SEC_PARAM_BOND)           &&  \
    !(BLE_NFC_SEC_PARAM_KDIST_OWN_ENC)  &&  \
    !(BLE_NFC_SEC_PARAM_KDIST_OWN_ID)   &&  \
    !(BLE_NFC_SEC_PARAM_KDIST_PEER_ENC) &&  \
    !(BLE_NFC_SEC_PARAM_KDIST_PEER_ID))
    #error "At least one of the BLE_NFC_SEC_PARAM_KDIST flags must be set to 1 when bonding is enabled."
#endif

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
            NRF_LOG_DEBUG("Connected to previously bonded device");
            err_code = pm_peer_rank_highest(p_evt->peer_id);
            if (err_code != NRF_ERROR_BUSY)
            {
                    APP_ERROR_CHECK(err_code);
            }
            break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
            NRF_LOG_DEBUG("Link secured. Role: %d. conn_handle: %d, Procedure: %d",
                           ble_conn_state_role(p_evt->conn_handle),
                           p_evt->conn_handle,
                           p_evt->params.conn_sec_succeeded.procedure);

            if (p_evt->params.conn_sec_succeeded.procedure == PM_LINK_SECURED_PROCEDURE_BONDING)
            {
                err_code = pm_peer_rank_highest(p_evt->peer_id);
                if (err_code != NRF_ERROR_BUSY)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        case PM_EVT_CONN_SEC_FAILED:
            /** In some cases, when securing fails, it can be restarted directly. Sometimes it can
             *  be restarted, but only after changing some Security Parameters. Sometimes, it cannot
             *  be restarted until the link is disconnected and reconnected. Sometimes it is
             *  impossible, to secure the link, or the peer device does not support it. How to
             *  handle this error is highly application dependent. */
            NRF_LOG_DEBUG("PM_EVT_CONN_SEC_FAILED: %u", p_evt->params.conn_sec_failed.error);

            if (p_evt->params.conn_sec_failed.error == PM_CONN_SEC_ERROR_PIN_OR_KEY_MISSING)
            {
                // Rebond if one party has lost its keys.
                err_code = pm_conn_secure(p_evt->conn_handle, true);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            scan_start();
            break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
            break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        case PM_EVT_PEER_DELETE_FAILED:
        case PM_EVT_PEERS_DELETE_FAILED:
        case PM_EVT_ERROR_UNEXPECTED:
            // Assert.
            APP_ERROR_CHECK_BOOL(false);
            break;

        case PM_EVT_CONN_SEC_START:
            NRF_LOG_DEBUG("PM_EVT_CONN_SEC_START");
            break;

        // Events that do not need to be implemented.
        case PM_EVT_STORAGE_FULL:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            // No implementation needed.
            break;
    }
}

void peer_manager_init(bool erase_bonds)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    if (erase_bonds)
    {
        err_code = pm_peers_delete();
        APP_ERROR_CHECK(err_code);
    }

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.mitm              = BLE_NFC_SEC_PARAM_MITM;
    sec_param.lesc              = BLE_NFC_SEC_PARAM_LESC;
    sec_param.keypress          = BLE_NFC_SEC_PARAM_KEYPRESS;
    sec_param.io_caps           = BLE_NFC_SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob               = BLE_NFC_SEC_PARAM_OOB;
    sec_param.min_key_size      = BLE_NFC_SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size      = BLE_NFC_SEC_PARAM_MAX_KEY_SIZE;
    sec_param.bond              = BLE_NFC_SEC_PARAM_BOND;

#if (BLE_NFC_SEC_PARAM_BOND)
    // If bonding is enabled, set key distribution flags.
    sec_param.kdist_own.enc     = BLE_NFC_SEC_PARAM_KDIST_OWN_ENC;
    sec_param.kdist_own.id      = BLE_NFC_SEC_PARAM_KDIST_OWN_ID;
    sec_param.kdist_peer.enc    = BLE_NFC_SEC_PARAM_KDIST_PEER_ENC;
    sec_param.kdist_peer.id     = BLE_NFC_SEC_PARAM_KDIST_PEER_ID;
#else
    // If bonding is not enabled, no keys can be distributed.
    sec_param.kdist_own.enc     = 0;
    sec_param.kdist_own.id      = 0;
    sec_param.kdist_peer.enc    = 0;
    sec_param.kdist_peer.id     = 0;
#endif

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

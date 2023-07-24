/**
 * @file       shimano.c
 *
 * @brief      This file provides access to the bluetooth Shimano Information Service (shiis)
 *
 * @author     Sean Beitz
 * @copyright  Copyright (c) 2021 SmartHalo Inc
 */

#include "shimano.h"
#include "sdk_common.h"
#include "ble_types.h"
#include "ble_db_discovery.h"
#include "ble_gattc.h"
#define NRF_LOG_MODULE_NAME ble_shiis_c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

/**@brief Function for intercepting errors of GATTC and BLE GATT Queue.
 *
 * @param[in] nrf_error   Error code.
 * @param[in] p_ctx       Parameter from the event handler.
 * @param[in] conn_handle Connection handle.
 */
static void gatt_error_handler(uint32_t   nrf_error,
                               void     * p_ctx,
                               uint16_t   conn_handle)
{
    ble_shiis_c_t * p_shiis_c = (ble_shiis_c_t *)p_ctx;

    NRF_LOG_DEBUG("A GATT Client error has occurred on conn_handle: 0X%X", conn_handle);

    if (p_shiis_c->error_handler != NULL)
    {
        p_shiis_c->error_handler(nrf_error);
    }
}


/**@brief     Function for handling read response events.
 *
 * @details   This function validates the read response and raises the appropriate
 *            event to the application.
 *
 * @param[in] p_shiis_c   Pointer to the Shimano Information Service Client Structure.
 * @param[in] p_ble_evt Pointer to the SoftDevice event.
 */
static void on_read_rsp(ble_shiis_c_t * p_shiis_c, ble_evt_t const * p_ble_evt)
{
    const ble_gattc_evt_read_rsp_t * p_response;

    // Check if the event is on the link for this instance.
    if (p_shiis_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }

    p_response = &p_ble_evt->evt.gattc_evt.params.read_rsp;

    if (p_response->handle == p_shiis_c->peer_shiis_db.bl_handle)
    {
        ble_shiis_c_evt_t evt;

        evt.conn_handle = p_ble_evt->evt.gattc_evt.conn_handle;
        evt.evt_type = BLE_SHIIS_C_EVT_READ_RESP;

        evt.params.shiis_data.assist_mode   = p_ble_evt->evt.gattc_evt.params.hvx.data[1];
        evt.params.shiis_data.current_speed[0] = p_ble_evt->evt.gattc_evt.params.hvx.data[2];
        evt.params.shiis_data.current_speed[1] = p_ble_evt->evt.gattc_evt.params.hvx.data[3];
        evt.params.shiis_data.assist_level  = p_ble_evt->evt.gattc_evt.params.hvx.data[4];
        evt.params.shiis_data.cadence       = p_ble_evt->evt.gattc_evt.params.hvx.data[5];
        evt.params.shiis_data.travel_time[0]   = p_ble_evt->evt.gattc_evt.params.hvx.data[6];
        evt.params.shiis_data.travel_time[1]   = p_ble_evt->evt.gattc_evt.params.hvx.data[7];
        evt.params.shiis_data.travel_time[2]   = p_ble_evt->evt.gattc_evt.params.hvx.data[8];
        evt.params.shiis_data.travel_time[3]   = p_ble_evt->evt.gattc_evt.params.hvx.data[9];

        p_shiis_c->evt_handler(p_shiis_c, &evt);
    }
}


/**@brief     Function for handling Handle Value Notification received from the SoftDevice.
 *
 * @details   This function handles the Handle Value Notification received from the SoftDevice
 *            and checks whether it is a notification of the Periodic Information from the peer. If
 *            it is, this function decodes the Periodic Information and sends it to the
 *            application.
 *
 * @param[in] p_ble_shiis_c Pointer to the Shimano Information Service Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_hvx(ble_shiis_c_t * p_ble_shiis_c, ble_evt_t const * p_ble_evt)
{
    // Check if the event is on the link for this instance.
    if (p_ble_shiis_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }
    // Check if this notification is a Periodic Information notification.
    if (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_shiis_c->peer_shiis_db.bl_handle)
    {
        if (p_ble_evt->evt.gattc_evt.params.hvx.len == 10)
        {
            ble_shiis_c_evt_t ble_shiis_c_evt;
            ble_shiis_c_evt.conn_handle = p_ble_evt->evt.gattc_evt.conn_handle;
            ble_shiis_c_evt.evt_type    = BLE_SHIIS_C_EVT_NOTIFICATION;
 
            ble_shiis_c_evt.params.shiis_data.assist_mode   = p_ble_evt->evt.gattc_evt.params.hvx.data[1];
            ble_shiis_c_evt.params.shiis_data.current_speed[0] = p_ble_evt->evt.gattc_evt.params.hvx.data[2];
            ble_shiis_c_evt.params.shiis_data.current_speed[1] = p_ble_evt->evt.gattc_evt.params.hvx.data[3];
            ble_shiis_c_evt.params.shiis_data.assist_level  = p_ble_evt->evt.gattc_evt.params.hvx.data[4];
            ble_shiis_c_evt.params.shiis_data.cadence       = p_ble_evt->evt.gattc_evt.params.hvx.data[5];
            ble_shiis_c_evt.params.shiis_data.travel_time[0]   = p_ble_evt->evt.gattc_evt.params.hvx.data[6];
            ble_shiis_c_evt.params.shiis_data.travel_time[1]   = p_ble_evt->evt.gattc_evt.params.hvx.data[7];
            ble_shiis_c_evt.params.shiis_data.travel_time[2]   = p_ble_evt->evt.gattc_evt.params.hvx.data[8];
            ble_shiis_c_evt.params.shiis_data.travel_time[3]   = p_ble_evt->evt.gattc_evt.params.hvx.data[9];

            p_ble_shiis_c->evt_handler(p_ble_shiis_c, &ble_shiis_c_evt);
        }
    }
}


void ble_shiis_on_db_disc_evt(ble_shiis_c_t * p_ble_shiis_c, const ble_db_discovery_evt_t * p_evt)
{
    // Check if the Shimano Information Service was discovered.
    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE
        &&
        p_evt->params.discovered_db.srv_uuid.uuid == SHIMANO_IS_UUID
       // &&
        //p_evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_VENDOR_BEGIN
        )//SBEITZ this needs to be fixed ^
    {
        // Find the CCCD Handle of the Periodic Information characteristic.
        uint8_t i;

        ble_shiis_c_evt_t evt;
        evt.evt_type    = BLE_SHIIS_C_EVT_DISCOVERY_COMPLETE;
        evt.conn_handle = p_evt->conn_handle;
        for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid ==
                SHIMANO_PI_CHAR_UUID)
            {
                // Found Periodic Information characteristic. Store CCCD handle and break.
                evt.params.shiis_db.bl_cccd_handle =
                    p_evt->params.discovered_db.charateristics[i].cccd_handle;
                    printf("cccd handle: %d",evt.params.shiis_db.bl_cccd_handle);
                evt.params.shiis_db.bl_handle =
                    p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
                break;
            }
        }

        printf("Shimano Information Service discovered at peer.");

        //If the instance has been assigned prior to db_discovery, assign the db_handles.
        if (p_ble_shiis_c->conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            if ((p_ble_shiis_c->peer_shiis_db.bl_cccd_handle == BLE_GATT_HANDLE_INVALID)&&
                (p_ble_shiis_c->peer_shiis_db.bl_handle      == BLE_GATT_HANDLE_INVALID))
            {
                p_ble_shiis_c->peer_shiis_db = evt.params.shiis_db;
            }
        }
        p_ble_shiis_c->evt_handler(p_ble_shiis_c, &evt);
    }
    else if ((p_evt->evt_type == BLE_DB_DISCOVERY_SRV_NOT_FOUND) ||
             (p_evt->evt_type == BLE_DB_DISCOVERY_ERROR))
    {
        printf("Shimano Information Service discovery failure at peer. ");
    }
    else
    {
        // Do nothing.
    }
}


/**@brief Function for creating a message for writing to the CCCD.
 */
static uint32_t cccd_configure(ble_shiis_c_t * p_ble_shiis_c, bool notification_enable)
{
    NRF_LOG_INFO("Configuring CCCD. CCCD Handle = %d, Connection Handle = %d",
                  p_ble_shiis_c->peer_shiis_db.bl_cccd_handle,
                  p_ble_shiis_c->conn_handle);

    nrf_ble_gq_req_t shiis_c_req;
    uint8_t          cccd[BLE_CCCD_VALUE_LEN];
    uint16_t         cccd_val = notification_enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    cccd[0] = LSB_16(cccd_val);
    cccd[1] = MSB_16(cccd_val);

    memset(&shiis_c_req, 0, sizeof(shiis_c_req));
 
    shiis_c_req.type                        = NRF_BLE_GQ_REQ_GATTC_WRITE;
    shiis_c_req.error_handler.cb            = gatt_error_handler;
    shiis_c_req.error_handler.p_ctx         = p_ble_shiis_c;
    shiis_c_req.params.gattc_write.handle   = p_ble_shiis_c->peer_shiis_db.bl_cccd_handle;
    shiis_c_req.params.gattc_write.len      = BLE_CCCD_VALUE_LEN;
    shiis_c_req.params.gattc_write.p_value  = cccd;
    shiis_c_req.params.gattc_write.offset   = 0;
    shiis_c_req.params.gattc_write.write_op = BLE_GATT_OP_WRITE_REQ;

    return nrf_ble_gq_item_add(p_ble_shiis_c->p_gatt_queue, &shiis_c_req, p_ble_shiis_c->conn_handle);
}


uint32_t ble_shiis_c_init(ble_shiis_c_t * p_ble_shiis_c, ble_shiis_c_init_t * p_ble_shiis_c_init)
{
    VERIFY_PARAM_NOT_NULL(p_ble_shiis_c);
    VERIFY_PARAM_NOT_NULL(p_ble_shiis_c_init);
   
    ret_code_t err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t shiis_uuid = SHIMANO_BASE_UUID;

    // Add vendor specific base UUID
    err_code = sd_ble_uuid_vs_add(&shiis_uuid, &p_ble_shiis_c->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type                = p_ble_shiis_c->uuid_type;
    ble_uuid.uuid                = SHIMANO_IS_UUID;

    p_ble_shiis_c->conn_handle                  = BLE_CONN_HANDLE_INVALID;
    p_ble_shiis_c->peer_shiis_db.bl_cccd_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_shiis_c->peer_shiis_db.bl_handle      = BLE_GATT_HANDLE_INVALID;
    p_ble_shiis_c->evt_handler                  = p_ble_shiis_c_init->evt_handler;
    p_ble_shiis_c->error_handler                = p_ble_shiis_c_init->error_handler;
    p_ble_shiis_c->p_gatt_queue                 = p_ble_shiis_c_init->p_gatt_queue;

    return ble_db_discovery_evt_register(&ble_uuid);
}


/**@brief     Function for handling the Disconnected event received from the SoftDevice.
 *
 * @details   This function checks whether the disconnect event is happening on the link
 *            associated with the current instance of the module. If the event is happening,
 *            the function sets the instance's conn_handle to invalid.
 *
 * @param[in] p_ble_shiis_c Pointer to the Shimano Information Service Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_disconnected(ble_shiis_c_t * p_ble_shiis_c, const ble_evt_t * p_ble_evt)
{
    if (p_ble_shiis_c->conn_handle == p_ble_evt->evt.gap_evt.conn_handle)
    {
        p_ble_shiis_c->conn_handle                = BLE_CONN_HANDLE_INVALID;
        p_ble_shiis_c->peer_shiis_db.bl_cccd_handle = BLE_GATT_HANDLE_INVALID;
        p_ble_shiis_c->peer_shiis_db.bl_handle      = BLE_GATT_HANDLE_INVALID;
    }
}


void ble_shiis_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_ble_evt == NULL) || (p_context == NULL))
    {
        return;
    }

    ble_shiis_c_t * p_ble_shiis_c = (ble_shiis_c_t *)p_context;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX:
            on_hvx(p_ble_shiis_c, p_ble_evt);
            break;

        case BLE_GATTC_EVT_READ_RSP:
            on_read_rsp(p_ble_shiis_c, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnected(p_ble_shiis_c, p_ble_evt);
            break;

        default:
            break;
    }
}


uint32_t ble_shiis_c_bl_notif_enable(ble_shiis_c_t * p_ble_shiis_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_shiis_c);

    if (p_ble_shiis_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    return cccd_configure(p_ble_shiis_c, true);
}


uint32_t ble_shiis_c_bl_read(ble_shiis_c_t * p_ble_shiis_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_shiis_c);
    if (p_ble_shiis_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    nrf_ble_gq_req_t shiis_c_req;

    memset(&shiis_c_req, 0, sizeof(shiis_c_req));
    shiis_c_req.type                     = NRF_BLE_GQ_REQ_GATTC_READ;
    shiis_c_req.error_handler.cb         = gatt_error_handler;
    shiis_c_req.error_handler.p_ctx      = p_ble_shiis_c;
    shiis_c_req.params.gattc_read.handle = p_ble_shiis_c->peer_shiis_db.bl_handle;

    return nrf_ble_gq_item_add(p_ble_shiis_c->p_gatt_queue, &shiis_c_req, p_ble_shiis_c->conn_handle);
}


uint32_t ble_shiis_c_handles_assign(ble_shiis_c_t    * p_ble_shiis_c,
                                  uint16_t         conn_handle,
                                  ble_shiis_c_db_t * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ble_shiis_c);

    p_ble_shiis_c->conn_handle = conn_handle;
    if (p_peer_handles != NULL)
    {
        p_ble_shiis_c->peer_shiis_db = *p_peer_handles;
    }

    return nrf_ble_gq_conn_handle_register(p_ble_shiis_c->p_gatt_queue, conn_handle);
}




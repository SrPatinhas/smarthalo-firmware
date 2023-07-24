/*
 * Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

/**
 * @brief BLE Running Speed and Cadence Collector application main file.
 *
 * This file contains the source code for a sample Running Speed and Cadence collector application.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_db_discovery.h"
#include "softdevice_handler.h"
#include "app_util.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "app_trace.h"
#include "ble_rscs_c.h"
#include "app_util.h"
#include "app_timer.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "peer_manager.h"
#include "fds.h"
#include "fstorage.h"
#include "ble_conn_state.h"

#define CENTRAL_LINK_COUNT        1                                  /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT     0                                  /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define UART_TX_BUF_SIZE          256                                /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE          1                                  /**< UART RX buffer size. */

#define STRING_BUFFER_LEN         50
#define BOND_DELETE_ALL_BUTTON_ID 0                                  /**< Button used for deleting all bonded centrals during startup. */

#define APP_TIMER_PRESCALER       0                                  /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE   2                                  /**< Size of timer operation queues. */

#define APPL_LOG                  NRF_LOG_PRINTF_DEBUG               /**< Debug logger macro that will be used in this file to do logging of debug information over UART. */

#define SEC_PARAM_BOND            1                                  /**< Perform bonding. */
#define SEC_PARAM_MITM            0                                  /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC            0                                  /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS        0                                  /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE               /**< No I/O capabilities. */
#define SEC_PARAM_OOB             0                                  /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE    7                                  /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE    16                                 /**< Maximum encryption key size. */

#define SCAN_INTERVAL             0x00A0                             /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW               0x0050                             /**< Determines scan window in units of 0.625 millisecond. */

#define MIN_CONNECTION_INTERVAL   MSEC_TO_UNITS(7.5, UNIT_1_25_MS)   /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL   MSEC_TO_UNITS(30, UNIT_1_25_MS)    /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY             0                                  /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT       MSEC_TO_UNITS(4000, UNIT_10_MS)    /**< Determines supervision time-out in units of 10 millisecond. */

#define TARGET_UUID               BLE_UUID_RUNNING_SPEED_AND_CADENCE /**< Target device name that application is looking for. */
#define UUID16_SIZE               2                                  /**< Size of 16 bit UUID */

/**@breif Macro to unpack 16bit unsigned UUID from octet stream. */
#define UUID16_EXTRACT(DST, SRC) \
    do                           \
    {                            \
        (*(DST))   = (SRC)[1];   \
        (*(DST)) <<= 8;          \
        (*(DST))  |= (SRC)[0];   \
    } while (0)

/**@brief Variable length data encapsulation in terms of length and pointer to data */
typedef struct
{
    uint8_t * p_data;   /**< Pointer to data. */
    uint16_t  data_len; /**< Length of data. */
}data_t;

typedef enum
{
    BLE_NO_SCAN,        /**< No advertising running. */
    BLE_WHITELIST_SCAN, /**< Advertising with whitelist. */
    BLE_FAST_SCAN,      /**< Fast advertising running. */
} ble_scan_mode_t;

static ble_db_discovery_t    m_ble_db_discovery;                       /**< Structure used to identify the DB Discovery module. */
static ble_rscs_c_t          m_ble_rsc_c;                              /**< Structure used to identify the Running Speed and Cadence client module. */
static ble_gap_scan_params_t m_scan_param;                             /**< Scan parameters requested for scanning and connection. */
static ble_scan_mode_t       m_scan_mode = BLE_FAST_SCAN;              /**< Scan mode used by application. */
static uint16_t              m_conn_handle;                            /**< Current connection handle. */
static volatile bool         m_whitelist_temporarily_disabled = false; /**< True if whitelist has been temporarily disabled. */
static bool                  m_memory_access_in_progress      = false; /**< Flag to keep track of ongoing operations on persistent memory. */

/**
 * @brief Connection parameters requested for connection.
 */
static const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL, // Minimum connection.
    (uint16_t)MAX_CONNECTION_INTERVAL, // Maximum connection.
    (uint16_t)SLAVE_LATENCY,           // Slave latency.
    (uint16_t)SUPERVISION_TIMEOUT      // Supervision time-out.
};

static void scan_start(void);


/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 *
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_rscs_on_db_disc_evt(&m_ble_rsc_c, p_evt);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch(p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            APPL_LOG("[APPL]: Bonded device connected!.\r\n");
            err_code = pm_peer_rank_highest(p_evt->peer_id);
            if(err_code != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(err_code);
            }
        }break;//PM_EVT_BONDED_PEER_CONNECTED

        case PM_EVT_CONN_SEC_START:
            break;//PM_EVT_CONN_SEC_START

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            APPL_LOG("[APPL]: Link secured. Role: %d. conn_handle: %d, Procedure: %d\r\n",
                     ble_conn_state_role(p_evt->conn_handle),
                     p_evt->conn_handle,
                     p_evt->params.conn_sec_succeeded.procedure);
            err_code = pm_peer_rank_highest(p_evt->peer_id);
            if(err_code != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(err_code);
            }
        }break;//PM_EVT_CONN_SEC_SUCCEEDED

        case PM_EVT_CONN_SEC_FAILED:
        {
            /** In some cases, when securing fails, it can be restarted directly. Sometimes it can
             *  be restarted, but only after changing some Security Parameters. Sometimes, it cannot
             *  be restarted until the link is disconnected and reconnected. Sometimes it is
             *  impossible, to secure the link, or the peer device does not support it. How to
             *  handle this error is highly application dependent. */
            switch (p_evt->params.conn_sec_failed.error)
            {
                case PM_CONN_SEC_ERROR_PIN_OR_KEY_MISSING:
                    // Rebond if one party has lost its keys.
                    err_code = pm_conn_secure(p_evt->conn_handle, true);
                    if (err_code != NRF_ERROR_INVALID_STATE)
                    {
                        APP_ERROR_CHECK(err_code);
                    }
                    break;//PM_CONN_SEC_ERROR_PIN_OR_KEY_MISSING

                default:
                    break;
            }
        }break;//PM_EVT_CONN_SEC_FAILED

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        }break;//PM_EVT_CONN_SEC_CONFIG_REQ

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        }break;//PM_EVT_STORAGE_FULL

        case PM_EVT_ERROR_UNEXPECTED:
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
            break;//PM_EVT_ERROR_UNEXPECTED

        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
            break;//PM_EVT_PEER_DATA_UPDATE_SUCCEEDED

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
            // Assert.
            APP_ERROR_CHECK_BOOL(false);
            break;//PM_EVT_PEER_DATA_UPDATE_FAILED

        case PM_EVT_PEER_DELETE_SUCCEEDED:
            break;//PM_EVT_PEER_DELETE_SUCCEEDED

        case PM_EVT_PEER_DELETE_FAILED:
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
            break;//PM_EVT_PEER_DELETE_FAILED

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            scan_start();
            break;//PM_EVT_PEERS_DELETE_SUCCEEDED

        case PM_EVT_PEERS_DELETE_FAILED:
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
            break;//PM_EVT_PEERS_DELETE_FAILED

        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
            break;//PM_EVT_LOCAL_DB_CACHE_APPLIED

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
            break;//PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED

        case PM_EVT_SERVICE_CHANGED_IND_SENT:
            break;//PM_EVT_SERVICE_CHANGED_IND_SENT

        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
            break;//PM_EVT_SERVICE_CHANGED_IND_CONFIRMED

        default:
            // No implementation needed.
            break;
    }
}


/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Type of data to be looked for in advertisement data.
 * @param[in]  Advertisement report length and pointer to report.
 * @param[out] If data type requested is found in the data report, type data length and
 *             pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t              err_code;
    const ble_gap_evt_t * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            // Discover peer's services.
            err_code = ble_db_discovery_start(&m_ble_db_discovery,
                                              p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);
            
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
        
            break;

        case BLE_GAP_EVT_ADV_REPORT:
        {
            data_t adv_data;
            data_t type_data;

            // Initialize advertisement report for parsing.
            adv_data.p_data   = (uint8_t *)p_gap_evt->params.adv_report.data;
            adv_data.data_len = p_gap_evt->params.adv_report.dlen;

            err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE,
                                        &adv_data,
                                        &type_data);

            if (err_code != NRF_SUCCESS)
            {
                // Compare short local name in case complete name does not match.
                err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
                                            &adv_data,
                                            &type_data);
            }

            // Verify if short or complete name matches target.
            if (err_code == NRF_SUCCESS)
            {
                uint16_t extracted_uuid;

                // UUIDs found, look for matching UUID
                for (uint32_t u_index = 0; u_index < (type_data.data_len / UUID16_SIZE); u_index++)
                {
                    UUID16_EXTRACT(&extracted_uuid, &type_data.p_data[u_index * UUID16_SIZE]);

                    APPL_LOG("\t[APPL]: UUID %x\r\n", extracted_uuid);

                    if (extracted_uuid == TARGET_UUID)
                    {
                        // Stop scanning.
                        err_code = sd_ble_gap_scan_stop();

                        if (err_code != NRF_SUCCESS)
                        {
                            APPL_LOG("[APPL]: Scan stop failed, reason %d\r\n", err_code);
                        }
                        err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                        APP_ERROR_CHECK(err_code);

                        m_scan_param.selective = 0;

                        // Initiate connection.
                        err_code = sd_ble_gap_connect(&p_gap_evt->params.adv_report.peer_addr,
                                                      &m_scan_param,
                                                      &m_connection_param);

                        m_whitelist_temporarily_disabled = false;

                        if (err_code != NRF_SUCCESS)
                        {
                            APPL_LOG("[APPL]: Connection Request Failed, reason %d\r\n", err_code);
                        }
                        break;
                    }
                }
            }
            break;
        }

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
            {
                APPL_LOG("[APPL]: Scan timed out.\r\n");
                scan_start();
            }
            else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                APPL_LOG("[APPL]: Connection Request timed out.\r\n");
            }
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            if(ble_conn_state_n_centrals() == 0)
            {
                scan_start();
            }
            break;

        default:
            break;
    }
}


/**@brief Function for handling the Application's system events.
 *
 * @param[in]   sys_evt   system event.
 */
static void on_sys_evt(uint32_t sys_evt)
{
    switch (sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
            /* fall through */
        case NRF_EVT_FLASH_OPERATION_ERROR:

            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                scan_start();
            }
            break;//NRF_EVT_FLASH_OPERATION_SUCCESS and NRF_EVT_FLASH_OPERATION_ERROR

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *  been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_rscs_c_on_ble_evt(&m_ble_rsc_c, p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    fs_sys_event_handler(sys_evt);
    on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for System events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for disabling the use of whitelist for scanning.
 */
static void whitelist_disable(void)
{
    uint32_t err_code;

    if ((m_scan_mode == BLE_WHITELIST_SCAN) && !m_whitelist_temporarily_disabled)
    {
        m_whitelist_temporarily_disabled = true;

        err_code = sd_ble_gap_scan_stop();
        if (err_code == NRF_SUCCESS)
        {
            scan_start();
        }
        else if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
    }
    m_whitelist_temporarily_disabled = true;
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            whitelist_disable();
            break;

        default:
            break;
    }
}


/**@brief Running Speed and Cadence Collector Handler.
 */
static void rscs_c_evt_handler(ble_rscs_c_t * p_rsc_c, ble_rscs_c_evt_t * p_rsc_c_evt)
{
    uint32_t err_code;

    switch (p_rsc_c_evt->evt_type)
    {
        case BLE_RSCS_C_EVT_DISCOVERY_COMPLETE:
            // Initiate bonding.
            err_code = ble_rscs_c_handles_assign(&m_ble_rsc_c ,
                                                 p_rsc_c_evt->conn_handle,
                                                 &p_rsc_c_evt->params.rscs_db);
            APP_ERROR_CHECK(err_code);

            err_code = pm_conn_secure(p_rsc_c_evt->conn_handle, false);
            APP_ERROR_CHECK(err_code);

            // Running Speed and Cadence service discovered. Enable Running Speed and Cadence notifications.
            err_code = ble_rscs_c_rsc_notif_enable(p_rsc_c);
            APP_ERROR_CHECK(err_code);

            printf("Running Speed and Cadence service discovered \r\n");
            break;

        case BLE_RSCS_C_EVT_RSC_NOTIFICATION:
        {
            printf("\r\n");
            APPL_LOG("[APPL]: RSC Measurement received %d \r\n",
                     p_rsc_c_evt->params.rsc.inst_speed);

            printf("Instantanious Speed   = %d\r\n", p_rsc_c_evt->params.rsc.inst_speed);
            if (p_rsc_c_evt->params.rsc.is_inst_stride_len_present)
            {
                printf("Stride Length         = %d\r\n",
                       p_rsc_c_evt->params.rsc.inst_stride_length);
            }
            if (p_rsc_c_evt->params.rsc.is_total_distance_present)
            {
                printf("Total Distance = %u\r\n", (unsigned int)p_rsc_c_evt->params.rsc.total_distance);
            }
            printf("Instantanious Cadence = %d\r\n", p_rsc_c_evt->params.rsc.inst_cadence);
            printf("Flags\r\n");
            printf("  Stride Length Present = %d\r\n",
                   p_rsc_c_evt->params.rsc.is_inst_stride_len_present);
            printf("  Total Distance Present= %d\r\n",
                   p_rsc_c_evt->params.rsc.is_total_distance_present);
            printf("  Is Running            = %d\r\n", p_rsc_c_evt->params.rsc.is_running);
            break;
        }

        default:
            break;
    }
}


/**
 * @brief HRunning Speed and Cadence collector initialization.
 */
static void rscs_c_init(void)
{
    ble_rscs_c_init_t rscs_c_init_obj;

    rscs_c_init_obj.evt_handler = rscs_c_evt_handler;

    uint32_t err_code = ble_rscs_c_init(&m_ble_rsc_c, &rscs_c_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Peer Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Peer Manager.
 */
static void peer_manager_init(bool erase_bonds)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    if (erase_bonds)
    {
        err_code = pm_peers_delete();
        APP_ERROR_CHECK(err_code);
    }

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Database discovery collector initialization.
 */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function to start scanning.
 */
static void scan_start(void)
{
    ble_gap_whitelist_t whitelist;
    ble_gap_addr_t    * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t     * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
    uint32_t            err_code;
    uint32_t            count;

    // Verify if there is any flash access pending, if yes delay starting scanning until
    // it's complete.
    err_code = fs_queued_op_count_get(&count);
    APP_ERROR_CHECK(err_code);

    if (count != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }

    // Initialize whitelist parameters.
    whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    whitelist.irk_count  = 0;
    whitelist.pp_addrs   = p_whitelist_addr;
    whitelist.pp_irks    = p_whitelist_irk;

    // Request creating of whitelist.
    err_code = pm_whitelist_create(NULL, 0, &whitelist);
    APP_ERROR_CHECK(err_code);

    if (((whitelist.addr_count == 0) && (whitelist.irk_count == 0)) ||
        (m_scan_mode != BLE_WHITELIST_SCAN) ||
        (m_whitelist_temporarily_disabled))
    {
        // No devices in whitelist, hence non selective performed.
        m_scan_param.active      = 0;             // Active scanning set.
        m_scan_param.selective   = 0;             // Selective scanning not set.
        m_scan_param.interval    = SCAN_INTERVAL; // Scan interval.
        m_scan_param.window      = SCAN_WINDOW;   // Scan window.
        m_scan_param.p_whitelist = NULL;          // No whitelist provided.
        m_scan_param.timeout     = 0x0000;        // No timeout.
    }
    else
    {
        // Selective scanning based on whitelist first.
        m_scan_param.active      = 0;             // Active scanning set.
        m_scan_param.selective   = 1;             // Selective scanning not set.
        m_scan_param.interval    = SCAN_INTERVAL; // Scan interval.
        m_scan_param.window      = SCAN_WINDOW;   // Scan window.
        m_scan_param.p_whitelist = &whitelist;    // Provide whitelist.
        m_scan_param.timeout     = 0x001E;        // 30 seconds timeout.
    }

    err_code = sd_ble_gap_scan_start(&m_scan_param);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the UART.
 */
static void uart_init(void)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
       {
           RX_PIN_NUMBER,
           TX_PIN_NUMBER,
           RTS_PIN_NUMBER,
           CTS_PIN_NUMBER,
           APP_UART_FLOW_CONTROL_ENABLED,
           false,
           UART_BAUDRATE_BAUDRATE_Baud115200
       };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_error_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/** @brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();

    APP_ERROR_CHECK(err_code);
}


int main(void)
{
    bool erase_bonds;

    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
    buttons_leds_init(&erase_bonds);
    uart_init();
    app_trace_init();
    printf("Running Speed collector example\r\n");
    ble_stack_init();
    ble_conn_state_init();
    peer_manager_init(erase_bonds);
    db_discovery_init();
    rscs_c_init();

    // Start scanning for peripherals and initiate connection
    // with devices that advertise Running Speed and Cadence UUID.
    scan_start();

    for (;;)
    {
        power_manage();
    }
}



#include <stdint.h>
#include <string.h>

//#include "platform.h"

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
//#include "boards.h"
//#include "bsp.h"
#include "softdevice_handler.h"
#include "softdevice_handler_appsh.h"
#include "app_timer.h"
//#include "bsp_btn_ble.h"
#include "fstorage.h"
#include "peer_manager.h"
#include "fds.h"
#include "ble_conn_state.h"

#include "platform.h"

#include "scheduler.h"
#include "keys.h"
#include "leds.h"
#include "power.h"
#include "meters.h"
#include "twi.h"

#include "bleapp.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"


//service id
#define BLE_UUID_SH1_UUID          0x0000

#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE            GATT_MTU_SIZE_DEFAULT                        /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#endif

#define CENTRAL_LINK_COUNT               0                                          /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                          /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define DEVICE_NAME                      "smarthalo"                          /**< Name of device. Will be included in the advertising data. */
//#define MANUFACTURER_NAME                "NordicSemiconductor"                      /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                 800 //300                                        /**< The advertising interval (in units of 0.625 ms. This value (?) corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       0 //180                                        /**< The advertising timeout in units of seconds. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   0                                           /**< not Perform bonding. */
#define SEC_PARAM_MITM                   0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                          /**< Maximum encryption key size. */



#define RR_INTERVAL_INTERVAL             APP_TIMER_TICKS(300, APP_TIMER_PRESCALER)   /**< RR interval interval (ticks). */
#define MIN_RR_INTERVAL                  100                                         /**< Minimum RR interval as returned by the simulated measurement function. */
#define MAX_RR_INTERVAL                  500                                         /**< Maximum RR interval as returned by the simulated measurement function. */
#define RR_INTERVAL_INCREMENT            1                                           /**< Value by which the RR interval is incremented/decremented for each call to the simulated measurement function. */

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(400, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(650, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                    0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */

#define DEAD_BEEF                        0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_FEATURE_NOT_SUPPORTED        BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */

static ble_uuid_t m_adv_uuids_test[] = {{0x5348, BLE_UUID_TYPE_BLE}};

bool bleapp_connected = false;

#define BLEAPP_EVT_CNT 6

bleapp_evt_cb_t bleapp_evt[BLEAPP_EVT_CNT];

uint16_t    bleapp_service_handle; // Handle of Our Service (as provided by the BLE stack).

bool bleapp_state_jumptoBootloader = false;

void bleapp_jumptoBootloader_reset(void *ctx) {
    NVIC_SystemReset();
}

void bleapp_jumptoBootloader() {
    bleapp_state_jumptoBootloader = true;
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    } else {
        leds_shutdown();
        sch_unique_oneshot(bleapp_jumptoBootloader_reset, 100);
    }
}

bool bleapp_state_shutdown = false;

void bleapp_shutdown_do(void *ctx) {
    //uint32_t err_code;
    //pwr_shutdown();
    ////sd_softdevice_disable();
    ////__disable_irq();
    //err_code = sd_power_system_off();
    //APP_ERROR_CHECK(err_code);
    twi_shutdown();
    pwr_shutdown();
    sd_power_system_off();
    while(1) {
        __WFE();
    }
}

void bleapp_shutdown() {
    bleapp_state_shutdown = true;
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    } else {
        leds_shutdown();
        met_shutdown();
        sch_unique_oneshot(bleapp_shutdown_do, 100);
    }
}

void bleapp_kickout() {
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }
}

bool main_context ( void )
{
    static const uint8_t ISR_NUMBER_THREAD_MODE = 0;
    uint8_t isr_number =__get_IPSR();
    if ((isr_number ) == ISR_NUMBER_THREAD_MODE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

bool bleapp_advertising_refresh = false;

void bleapp_advertising_init(void);

void bleapp_advertising_start()
{
    ret_code_t err_code;

    bleapp_advertising_init();

    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

void bleapp_setName(const uint8_t *name) {
    uint32_t                err_code;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&sec_mode);
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                         name,
                                          strlen((char *)name));
    APP_ERROR_CHECK(err_code);

    bleapp_advertising_refresh = true;

}

static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    /*
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);
    */
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


void bleapp_gap_params_update(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = ble_conn_params_change_conn_params(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/*
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}
*/

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    //uint32_t err_code;
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            //APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            //sleep_mode_enter();
            break;
        default:
            break;
    }
}

bool bleapp_isConnected() {
    return bleapp_connected;
}


#define LPF_ALPHA_RSSI 0.1
double bleapp_lpf_rssi;
int bleapp_filtered_rssi;
void bleapp_onRSSI(double rssi) {
    bleapp_lpf_rssi = bleapp_lpf_rssi + LPF_ALPHA_RSSI * (rssi - bleapp_lpf_rssi);
    /*if(bleapp_filtered_rssi != (int)bleapp_lpf_rssi) {
        bleapp_filtered_rssi = (int)bleapp_lpf_rssi;
        printf("RSSI:%d\r\n",bleapp_filtered_rssi);
    }*/
}

double bleapp_getRSSI() {
    return bleapp_lpf_rssi;
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            printf("Connected!\r\n");
            //err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            //APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            bleapp_connected = true;
            pwr_update();
            
            //err_code = sd_ble_gap_tx_power_set(4);
            //APP_ERROR_CHECK(err_code);

            //gap_params_update();

            //sch_unique_oneshot(leds_inner_test, 1000);

            bleapp_lpf_rssi = -100;
            sd_ble_gap_rssi_start(m_conn_handle, 1, 0);

            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DISCONNECTED:
            printf("Disconnected!\r\n");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            bleapp_connected = false;
            pwr_update();
            if(bleapp_state_jumptoBootloader) {
                bleapp_jumptoBootloader();
            } else if(bleapp_state_shutdown) {
                bleapp_shutdown();
            }
            if(bleapp_advertising_refresh) {
                bleapp_advertising_refresh = false;
                bleapp_advertising_start();
            }

            //err_code = sd_ble_gap_tx_power_set(0);
            //APP_ERROR_CHECK(err_code);

            break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_RSSI_CHANGED:
            bleapp_onRSSI(p_ble_evt->evt.gap_evt.params.rssi_changed.rssi);
            break; // BLE_GAP_EVT_RSSI_CHANGED

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            printf("GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            printf("GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

#if (NRF_SD_BLE_API_VERSION == 3)
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                       NRF_BLE_MAX_MTU_SIZE);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif

        default:
            // No implementation needed.
            break;
    }
}

static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    //bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    for(int i = 0; i < BLEAPP_EVT_CNT; i++) {
        if(bleapp_evt[i] != NULL) {
            bleapp_evt[i](p_ble_evt);
        }
    }    
}

static void sys_evt_dispatch(uint32_t sys_evt)
{
    // Dispatch the system event to the fstorage module, where it will be
    // dispatched to the Flash Data Storage (FDS) module.
    fs_sys_event_handler(sys_evt);

    // Dispatch to the Advertising module last, since it will check if there are any
    // pending flash operations in fstorage. Let fstorage process system events first,
    // so that it can report correctly to the Advertising module.
    ble_advertising_on_sys_evt(sys_evt);
}

void ble_stack_init(void)
{
    //printf("ble_stack_init\r\n");
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    //SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    SOFTDEVICE_HANDLER_APPSH_INIT(&clock_lf_cfg, true);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
#if (NRF_SD_BLE_API_VERSION == 3)
    ble_enable_params.gatt_enable_params.att_mtu = NRF_BLE_MAX_MTU_SIZE;
#endif
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}
/*
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            //sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist();
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}
*/
/*
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
            NRF_LOG_DEBUG("Connected to previously bonded device\r\n");
            err_code = pm_peer_rank_highest(p_evt->peer_id);
            if (err_code != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // PM_EVT_BONDED_PEER_CONNECTED

        case PM_EVT_CONN_SEC_START:
            break; // PM_EVT_CONN_SEC_START

        case PM_EVT_CONN_SEC_SUCCEEDED:
            NRF_LOG_DEBUG("Link secured. Role: %d. conn_handle: %d, Procedure: %d\r\n",
                                 ble_conn_state_role(p_evt->conn_handle),
                                 p_evt->conn_handle,
                                 p_evt->params.conn_sec_succeeded.procedure);
            err_code = pm_peer_rank_highest(p_evt->peer_id);
            if (err_code != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;  // PM_EVT_CONN_SEC_SUCCEEDED

        case PM_EVT_CONN_SEC_FAILED:

            // In some cases, when securing fails, it can be restarted directly. Sometimes it can
            //  be restarted, but only after changing some Security Parameters. Sometimes, it cannot
            //  be restarted until the link is disconnected and reconnected. Sometimes it is
            //  impossible, to secure the link, or the peer device does not support it. How to
            //  handle this error is highly application dependent. 
            switch (p_evt->params.conn_sec_failed.error)
            {
                case PM_CONN_SEC_ERROR_PIN_OR_KEY_MISSING:
                    // Rebond if one party has lost its keys.
                    err_code = pm_conn_secure(p_evt->conn_handle, true);
                    if (err_code != NRF_ERROR_INVALID_STATE)
                    {
                        APP_ERROR_CHECK(err_code);
                    }
                    break;

                default:
                    break;
            }
            break; // PM_EVT_CONN_SEC_FAILED

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break; // PM_EVT_CONN_SEC_CONFIG_REQ

        case PM_EVT_STORAGE_FULL:
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // PM_EVT_STORAGE_FULL

        case PM_EVT_ERROR_UNEXPECTED:
            // A likely fatal error occurred. Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
            break;

        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
            break; // PM_EVT_PEER_DATA_UPDATE_SUCCEEDED

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
            // Assert.
            APP_ERROR_CHECK_BOOL(false);
            break; // PM_EVT_ERROR_UNEXPECTED

        case PM_EVT_PEER_DELETE_SUCCEEDED:
            break; // PM_EVT_PEER_DELETE_SUCCEEDED

        case PM_EVT_PEER_DELETE_FAILED:
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
            break; // PM_EVT_PEER_DELETE_FAILED

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            bleapp_advertising_start();
            break; // PM_EVT_PEERS_DELETE_SUCCEEDED

        case PM_EVT_PEERS_DELETE_FAILED:
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
            break; // PM_EVT_PEERS_DELETE_FAILED

        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
            break; // PM_EVT_LOCAL_DB_CACHE_APPLIED

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
            break; // PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED

        case PM_EVT_SERVICE_CHANGED_IND_SENT:
            break; // PM_EVT_SERVICE_CHANGED_IND_SENT

        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
            break; // PM_EVT_SERVICE_CHANGED_IND_SENT

        default:
            // No implementation needed.
            break;
    }
}
*/
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    //ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_CONN_SEC_START:
            sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            break; // PM_EVT_CONN_SEC_START

        default:
            // No implementation needed.
            break;
    }
}

static void peer_manager_init(bool erase_bonds)
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
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 0;//1;
    sec_param.kdist_own.id   = 0;//1;
    sec_param.kdist_peer.enc = 0;//1;
    sec_param.kdist_peer.id  = 0;//1;

    err_code = pm_sec_params_set(&sec_param);
    //printf("%d",err_code);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);

    //err_code = fds_register(fds_evt_handler);
    //APP_ERROR_CHECK(err_code);
}

void bleapp_advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    //ble_advdata_t scanrsp;

    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids_test) / sizeof(m_adv_uuids_test[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids_test;

    //memset(&scanrsp, 0, sizeof(scanrsp));

    uint8_t manufData[12];
    //manufData[0] = 'S'; // SH (Smarthalo)
    //manufData[1] = 'H'; //
    manufData[0] = 1; //payload version
    manufData[1] = 0; //bit 0: 1=bootloader, 0=app
    memcpy(manufData+2, keys_getPubKey(), 8);

    ble_advdata_manuf_data_t   manuf_data;
    manuf_data.company_identifier = 0x0466; //our company_identifier
    manuf_data.data.p_data = (unsigned char*)manufData;
    manuf_data.data.size = 10;
#if 1
    advdata.p_manuf_specific_data = &manuf_data;
#else
    scanrsp.p_manuf_specific_data = &manuf_data;
#endif

    ble_adv_modes_config_t options;
    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

#if 1
    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL); //&scanrsp
#else
    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
#endif
    APP_ERROR_CHECK(err_code);
}

/*
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
*/

void bleapp_evt_register(bleapp_evt_cb_t cb) {
    for(int i = 0; i < BLEAPP_EVT_CNT; i++) {
        if(bleapp_evt[i] == NULL) {
            bleapp_evt[i] = cb;
            return;
        }
    }

    printf("bleapp_evt_register: overflow\r\n");
    APP_ERROR_CHECK(-1);

}

uint32_t bleapp_char_add(uint16_t                        uuid,
                         uint8_t                       * data,
                         uint32_t                        data_len,
//                         bool perm_read,
//                         bool perm_write,
//                         bool feature_notify,
                         uint32_t flags,
                         ble_gatts_char_handles_t      * p_handles)
{
    ble_uuid128_t     base_uuid = BLE_UUID_SH1_BASE_UUID;
    static ble_uuid_t          char_uuid;
    char_uuid.uuid      = uuid;

    sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);

    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;

    APP_ERROR_CHECK_BOOL(data != NULL);
    APP_ERROR_CHECK_BOOL(data_len > 0);

    // The ble_gatts_char_md_t structure uses bit fields. So we reset the memory to zero.
    memset(&char_md, 0, sizeof(char_md));

    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));

//    if(feature_notify) {
    if(flags & CHAR_FEAT_NOTIFY || flags & CHAR_FEAT_INDICATE) {
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
        cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
        char_md.p_cccd_md           = &cccd_md;
        char_md.char_props.notify   = (flags & CHAR_FEAT_NOTIFY) ? 1 : 0;
        char_md.char_props.indicate  = (flags & CHAR_FEAT_INDICATE) ? 1 : 0;
    }

    memset(&attr_md, 0, sizeof(attr_md));

//    if(perm_read) {
    if(flags & CHAR_PERM_READ) {
        char_md.char_props.read = 1;
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    } else {
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    }

//    if(perm_write) {
    if(flags & CHAR_PERM_WRITE) {
        char_md.char_props.write = 1;
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    } else {
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    }

    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &char_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = data_len;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = data_len;
    attr_char_value.p_value   = data;

    return sd_ble_gatts_characteristic_add(bleapp_service_handle, &char_md, &attr_char_value, p_handles);
}

void bleapp_service_init() {
    uint32_t   err_code = 0; // Variable to hold return codes from library and softdevice functions
    ble_uuid128_t     base_uuid = BLE_UUID_SH1_BASE_UUID;
    ble_uuid_t        service_uuid;
    service_uuid.uuid = BLE_UUID_SH1_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);    
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &bleapp_service_handle);
    APP_ERROR_CHECK(err_code);    
}

void bleapp_init(void)
{   
    uint32_t err_code;
    //bool erase_bonds;

    // Initialize.
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    //buttons_leds_init(&erase_bonds);
    peer_manager_init(false);
    gap_params_init();
    
    //bleapp_advertising_init();
    //conn_params_init();

    bleapp_service_init();

    //err_code = sd_ble_gap_tx_power_set(0);
    //APP_ERROR_CHECK(err_code);

    //bleapp_advertising_start();

    for(int i = 0; i < BLEAPP_EVT_CNT; i++) {
        bleapp_evt[i] = NULL;
    }

}



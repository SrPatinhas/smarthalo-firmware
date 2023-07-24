#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "nrf_sdh_ble.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "app_timer.h"
#include "nrf_fstorage.h"
#include "peer_manager.h"
#include "fds.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "ble_gap.h"
#include "platform.h"
#include "nrf_ble_qwr.h"
#include "scheduler.h"
#include "keys.h"
#include "ble_db_discovery.h"
#include "peer_manager_handler.h"
#include "ble_bas_c.h"
#include "shimano.h"
#include "nrf_ble_scan.h"
#include "uart.h"

#include "bleapp.h"

#define NRF_LOG_MODULE_NAME "APP"
//#include "nrf_log.h"
//#include "nrf_log_ctrl.h"
//#include "nrf_log_default_backends.h"

//service id
#define BLE_UUID_SH1_UUID                0x0000

#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define NRF_BLE_MAX_MTU_SIZE             BLE_GATT_ATT_MTU_DEFAULT                    /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */

#define CENTRAL_LINK_COUNT               0                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define APP_ADV_INTERVAL                 800 //300                                   /**< The advertising interval (in units of 0.625 ms. This value (?) corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       0 //180                                     /**< The advertising timeout in units of seconds. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   0                                           /**< not Perform bonding. */
#define SEC_PARAM_MITM                   0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                          /**< Maximum encryption key size. */

#define CONN_INTERVAL_DEFAULT           (uint16_t)(MSEC_TO_UNITS(7.5, UNIT_1_25_MS))    /**< Default connection interval used at connection establishment by central side. */

#define RR_INTERVAL_INTERVAL             APP_TIMER_TICKS(300)   /**< RR interval interval (ticks). */
#define MIN_RR_INTERVAL                  100                                         /**< Minimum RR interval as returned by the simulated measurement function. */
#define MAX_RR_INTERVAL                  500                                         /**< Maximum RR interval as returned by the simulated measurement function. */
#define RR_INTERVAL_INCREMENT            1                                           /**< Value by which the RR interval is incremented/decremented for each call to the simulated measurement function. */

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(400, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(650, UNIT_1_25_MS)            /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                    0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */

#define DEAD_BEEF                        0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_FEATURE_NOT_SUPPORTED        BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define BLEAPP_EVT_CNT 6

#define APP_BLE_CONN_CFG_TAG             1                                           /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO            3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_SOC_OBSERVER_PRIO            1                                           /**< Applications' SoC observer priority. You shouldn't need to modify this value. */

#define APP_ADV_FAST_INTERVAL            0x0028                                      /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL            0x0C80                                      /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */

#define APP_ADV_FAST_DURATION            3000                                        /**< The advertising duration of fast advertising in units of 10 milliseconds. */
#define APP_ADV_SLOW_DURATION            18000                                       /**< The advertising duration of slow advertising in units of 10 milliseconds. */

#define SCAN_DURATION_WITELIST           3000                                        /**< Duration of the scanning in units of 10 milliseconds. */

#define DB_DISCOVERY_INSTANCE_CNT        2                                           /**< Number of DB Discovery instances. */

/**@brief Macro to unpack 16bit unsigned UUID from octet stream. */
#define UUID16_EXTRACT(DST, SRC) \
    do                           \
    {                            \
        (*(DST))   = (SRC)[1];   \
        (*(DST)) <<= 8;          \
        (*(DST))  |= (SRC)[0];   \
    } while (0)

#define LPF_ALPHA_RSSI 0.1f
float bleapp_lpf_rssi;
int bleapp_filtered_rssi;

static pm_peer_id_t m_peer_id;                           /**< Device reference handle to the current bonded central. */
BLE_ADVERTISING_DEF(m_advertising);                      /**< Advertising module instance. */
NRF_SDH_BLE_OBSERVER(m_adv_ble_obs, BLE_ADV_BLE_OBSERVER_PRIO, ble_advertising_on_ble_evt, &m_advertising);
NRF_BLE_GATT_DEF(m_gatt); 
BLE_DB_DISCOVERY_DEF(m_ble_db_discovery);                /**< Database discovery module instance. */
NRF_BLE_QWRS_DEF(m_qwr, NRF_SDH_BLE_TOTAL_LINK_COUNT);   /**< Context for the Queued Write module.*/
NRF_BLE_GQ_DEF(m_ble_gatt_queue,                         /**< BLE GATT Queue instance. */
               NRF_SDH_BLE_CENTRAL_LINK_COUNT,
               NRF_BLE_GQ_QUEUE_SIZE);
BLE_SHIIS_C_DEF(m_shiis_c);                              /**< Structure used to identify the Shimano Information client module. */
BLE_BAS_C_DEF(m_bas_c);                                  /**< Structure used to identify the Battery Service client module. */
NRF_BLE_SCAN_DEF(m_scan);                                           /**< Scanning module instance. */

ble_gap_phys_t const phys =
{
    .tx_phys = BLE_GAP_PHY_AUTO,
    .rx_phys = BLE_GAP_PHY_AUTO,
};

typedef struct
{
    uint16_t        att_mtu;                    /**< GATT ATT MTU, in bytes. */
    uint16_t        conn_interval;              /**< Connection interval expressed in units of 1.25 ms. */
    ble_gap_phys_t  phys;                       /**< Preferred PHYs. */
    uint8_t         data_len;                   /**< Data length. */
    bool            conn_evt_len_ext_enabled;   /**< Connection event length extension status. */
} test_params_t;

static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static ble_uuid_t m_adv_uuids[] = {{0x5348, BLE_UUID_TYPE_BLE}};
bool bleapp_connected = false;
bleapp_evt_cb_t bleapp_evt[BLEAPP_EVT_CNT];
uint16_t    bleapp_service_handle; // Handle of Our Service (as provided by the BLE stack).
bool bleapp_state_jumptoBootloader = false;
bool bleapp_advertising_refresh = false;
static uint16_t m_conn_handle;                                      /**< Current connection handle. */
static bool     m_whitelist_disabled;                               /**< True if whitelist has been temporarily disabled. */
static bool     m_memory_access_in_progress;                        /**< Flag to keep track of ongoing operations on persistent memory. */

/**@brief Names which the central applications will scan for, and which will be advertised by the peripherals.
 *  if these are set to empty strings, the UUIDs defined below will be used
 */
static char const m_target_periph_name[] = "EWEN100";      /**< If you want to connect to a peripheral using a given advertising name, type its name here. */
static bool is_connect_per_addr = false;            /**< If you want to connect to a peripheral with a given address, set this to true and put the correct address in the variable below. */

/**< Scan parameters requested for scanning and connection. */
static ble_gap_scan_params_t const m_scan_param =
{
    .active        = 0x01,
    .interval      = NRF_BLE_SCAN_SCAN_INTERVAL,
    .window        = NRF_BLE_SCAN_SCAN_WINDOW,
    .filter_policy = BLE_GAP_SCAN_FP_WHITELIST,
    .timeout       = SCAN_DURATION_WITELIST,
    .scan_phys     = BLE_GAP_PHY_1MBPS,
};

static test_params_t m_mtu_params =
{
    .att_mtu                  = NRF_SDH_BLE_GATT_MAX_MTU_SIZE,
    .data_len                 = NRF_SDH_BLE_GAP_DATA_LENGTH,
    .conn_interval            = CONN_INTERVAL_DEFAULT,
    .conn_evt_len_ext_enabled = true,
};

static ble_gap_addr_t const m_target_periph_addr =
{
    /* Possible values for addr_type:
       BLE_GAP_ADDR_TYPE_PUBLIC,
       BLE_GAP_ADDR_TYPE_RANDOM_STATIC,
       BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE,
       BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE. */
    .addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC,
    .addr      = {0x8D, 0xFE, 0x23, 0x86, 0x77, 0xD9}
};

static void scan_start(void);
void bleapp_advertising_init(void);
static void on_whitelist_req(void);
static void delete_bonds(void);

/**@brief Function to reset device into bootloader
 */
void bleapp_jumptoBootloader_reset(void *ctx) {
    NVIC_SystemReset();
}

/**@brief Function to enter bootloader gracefully
 */
void bleapp_jumptoBootloader() {
    bleapp_state_jumptoBootloader = true;
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    } else {
        sch_unique_oneshot(bleapp_jumptoBootloader_reset, 100);
    }
}

/**@brief Function to disconnect from peer device
 */
void bleapp_kickout() {
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }
}

/**@brief Function for scheduler main context
 */
bool main_context(void)
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

/**@brief Function for handling asserts in the SoftDevice.
 *
 * @details This function is called in case of an assert in the SoftDevice.
 *
 * @param[in] line_num     Line number of the failing assert call.
 * @param[in] p_file_name  File name of the failing assert call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling scan events
 */
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    ret_code_t err_code;
    switch(p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_WHITELIST_REQUEST:
        {
            on_whitelist_req();
            m_whitelist_disabled = false;
        } break;

        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
        {
            err_code = p_scan_evt->params.connecting_err.err_code;
            APP_ERROR_CHECK(err_code);
        } break;

        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
        {
            //NRF_LOG_INFO("Scan timed out.");
            scan_start();
        } break;

        case NRF_BLE_SCAN_EVT_FILTER_MATCH:
        {
             //NRF_LOG_INFO("FILTER EVENT");
        } break;
        case NRF_BLE_SCAN_EVT_WHITELIST_ADV_REPORT:
            break;

        default:
          break;
    }
}

/**@brief Function for initialization scanning and setting filters.
 */
static void scan_init(void)
{
    ret_code_t          err_code;
    nrf_ble_scan_init_t init_scan;

    memset(&init_scan, 0, sizeof(init_scan));

    init_scan.p_scan_param     = &m_scan_param;
    init_scan.connect_if_match = true;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);

    //SBEITZ following commented section will be used to scan by uuid when we are ready to do so.
    //SBEITZ we may also wish to scan by device address depending on the device pairing scheme. see below for address filtering code

    // ble_uuid_t uuid =
    // {
    //     .uuid = TARGET_UUID,
    //     .type = BLE_UUID_TYPE_VENDOR_BEGIN,  //SBEITZ replace by BLE_UUID_TYPE_VENDOR_BEGIN for shimano uuid filtering
    // };

    //ble_uuid128_t uuid = SHIMANO_IS_UUID;

    // err_code = nrf_ble_scan_filter_set(&m_scan,
    //                                    SCAN_UUID_FILTER,
    //                                    &uuid);
    // APP_ERROR_CHECK(err_code);

    if (strlen(m_target_periph_name) != 0)
    {
        err_code = nrf_ble_scan_filter_set(&m_scan,
                                           SCAN_NAME_FILTER,
                                           m_target_periph_name);
        APP_ERROR_CHECK(err_code);
    }

    if (is_connect_per_addr)
    {
       err_code = nrf_ble_scan_filter_set(&m_scan,
                                          SCAN_ADDR_FILTER,
                                          m_target_periph_addr.addr);
       APP_ERROR_CHECK(err_code);
    }

    err_code = nrf_ble_scan_filters_enable(&m_scan,
                                           NRF_BLE_SCAN_ALL_FILTER,
                                           false);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for starting a scan, or instead trigger it from peer manager (after
 *        deleting bonds).
 *
 * @param[in] p_erase_bonds Pointer to a bool to determine if bonds will be deleted before scanning.
 */
void scanning_start(bool * p_erase_bonds)
{
    // Start scanning for peripherals and initiate connection
    // with devices that advertise GATT Service UUID.
    if (*p_erase_bonds == true)
    {
        // Scan is started by the PM_EVT_PEERS_DELETE_SUCCEEDED event.
        delete_bonds();
    }
    else
    {
        scan_start();
    }
}

/**@brief Function for initializing the scanning.
 */
static void scan_start(void)
{
    ret_code_t err_code;

    if (nrf_fstorage_is_busy(NULL))
    {
        m_memory_access_in_progress = true;
        return;
    }

   // NRF_LOG_INFO("Starting scan.");

    err_code = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the advertising and the scanning.
 */
void bleapp_advertising_scan_start()
{
    ret_code_t err_code;
     //check if there are no flash operations in progress
    if (!nrf_fstorage_is_busy(NULL))
    {
        // Start scanning for peripherals and initiate connection to devices which advertise for shiis
        scan_start();

        bleapp_advertising_init();

        err_code = ble_advertising_start(&m_advertising,BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for setting size of gatt MTU in bytes
 *
 * @param[in]   value      MTU size, typical is 23 for ble 4.2, can go higher for ble 5.0+
 */
void gatt_mtu_set(uint16_t att_mtu)
{
    ret_code_t err_code;

    m_mtu_params.att_mtu = att_mtu;

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, att_mtu);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for setting length of gatt data
 *
 * @param[in]   value      data length
 */
void data_len_set(uint8_t value)
{
    ret_code_t err_code;
    err_code = nrf_ble_gatt_data_length_set(&m_gatt, BLE_CONN_HANDLE_INVALID, value);
    APP_ERROR_CHECK(err_code);

    m_mtu_params.data_len = value;
}

/**@brief Function for setting device advertised name
 *
 * @param[in]   *name      pointer to advertised name of device
 */
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

/**@brief Function for handling Queued Write module errors.
 *
 * @details A pointer to this function is passed to each service that may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code that contains information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the Shimano Information Service Client and Battery Service Client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief shimano information service Collector Handler.
 */
static void shiis_c_evt_handler(ble_shiis_c_t * p_shiis_c, ble_shiis_c_evt_t * p_shiis_c_evt)
{
    ret_code_t err_code;

    switch (p_shiis_c_evt->evt_type)
    {
        case BLE_SHIIS_C_EVT_DISCOVERY_COMPLETE:
            err_code = ble_shiis_c_handles_assign(p_shiis_c,
                                                p_shiis_c_evt->conn_handle,
                                                &p_shiis_c_evt->params.shiis_db);
            APP_ERROR_CHECK(err_code);

            printf("SHIMANO INFORMATION SERVICE discovered. Reading travel information now.");

            err_code = ble_shiis_c_bl_read(p_shiis_c);
            APP_ERROR_CHECK(err_code);

            printf("Enabling SHIIS Notifications.");
            err_code = ble_shiis_c_bl_notif_enable(p_shiis_c);
            APP_ERROR_CHECK(err_code);

            break;

        case BLE_SHIIS_C_EVT_NOTIFICATION:
        {
            printf("assist mode is %d .\r\n", p_shiis_c_evt->params.shiis_data.assist_mode);
            printf("evt : current speed 0 is %d .\r\n", p_shiis_c_evt->params.shiis_data.current_speed[0]);
            printf("evt : current speed 1 is %d .\r\n", p_shiis_c_evt->params.shiis_data.current_speed[1]);
            printf("evt : assist level is %d .\r\n", p_shiis_c_evt->params.shiis_data.assist_level);
            printf("evt : cadence is %d .\r\n", p_shiis_c_evt->params.shiis_data.cadence);
            printf("evt : travel time 0 is %d .\r\n", p_shiis_c_evt->params.shiis_data.travel_time[0]);
            printf("evt : travel time 1 is %d .\r\n", p_shiis_c_evt->params.shiis_data.travel_time[1]);
            printf("evt : travel time 2 is %d .\r\n", p_shiis_c_evt->params.shiis_data.travel_time[2]);
            printf("evt : travel time 3 is %d .\r\n", p_shiis_c_evt->params.shiis_data.travel_time[3]);

             uint8_t shiis_speed[3]= {UI_SHIMANO_SPEEDO,p_shiis_c_evt->params.shiis_data.current_speed[0], p_shiis_c_evt->params.shiis_data.current_speed[1]}; 
             uint8_t shiis_assist_mode[2]= {UI_SHIMANO_ASSIST_MODE,2};
             uint8_t shiis_assist_level[2]= {UI_SHIMANO_ASSIST_LEVEL,2};
           
            //SBEITZ may want to place this in uart file, will see
             uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,shiis_speed,3); 
             uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,shiis_assist_mode,2); 
             uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,shiis_assist_level,2); 
        }   break;

        case BLE_SHIIS_C_EVT_READ_RESP:

            break;

        default:
            break;
    }
}

/**@brief Battery level Collector Handler.
 */
static void bas_c_evt_handler(ble_bas_c_t * p_bas_c, ble_bas_c_evt_t * p_bas_c_evt)
{
    ret_code_t err_code;

    switch (p_bas_c_evt->evt_type)
    {
        case BLE_BAS_C_EVT_DISCOVERY_COMPLETE:
        {
            err_code = ble_bas_c_handles_assign(p_bas_c,
                                                p_bas_c_evt->conn_handle,
                                                &p_bas_c_evt->params.bas_db);
            APP_ERROR_CHECK(err_code);

            // Battery service discovered. Enable notification of Battery Level.
            printf("Battery Service discovered. Reading battery level.");

            err_code = ble_bas_c_bl_read(p_bas_c);
            APP_ERROR_CHECK(err_code);

            printf("Enabling Battery Level Notification.");
            err_code = ble_bas_c_bl_notif_enable(p_bas_c);
            APP_ERROR_CHECK(err_code);

        } break;

        case BLE_BAS_C_EVT_BATT_NOTIFICATION:
            printf("Battery Level received %d %%.", p_bas_c_evt->params.battery_level);
            uint8_t shiis_bat_level[2] = {UI_SHIMANO_BATTERY_LEVEL,p_bas_c_evt->params.battery_level};
            uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,shiis_bat_level,2);
            break;

        case BLE_BAS_C_EVT_BATT_READ_RESP:
            printf("Battery Level Read as %d %%.", p_bas_c_evt->params.battery_level);
            //uint8_t shiis_bat_level[2] = {UI_SHIMANO_BATTERY_LEVEL,p_bas_c_evt->params.battery_level};
            //uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,shiis_bat_level,2);
            break;

        default:
            break;
    }
}

/**
 * @brief shimano information service collector initialization.
 */
void shiis_c_init(void)
{
    ble_shiis_c_init_t shiis_c_init_obj;

    shiis_c_init_obj.evt_handler   = shiis_c_evt_handler;
    shiis_c_init_obj.error_handler = service_error_handler;
    shiis_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;

    ret_code_t err_code = ble_shiis_c_init(&m_shiis_c, &shiis_c_init_obj);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Battery level collector initialization.
 */
static void bas_c_init(void)
{
    ble_bas_c_init_t bas_c_init_obj;

    bas_c_init_obj.evt_handler   = bas_c_evt_handler;
    bas_c_init_obj.error_handler = service_error_handler;
    bas_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;

    ret_code_t err_code = ble_bas_c_init(&m_bas_c, &bas_c_init_obj);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for assigning new connection handle to available instance of QWR module.
 *
 * @param[in] conn_handle New connection handle.
 */
static void multi_qwr_conn_handle_assign(uint16_t conn_handle)
{
    for (uint32_t i = 0; i < NRF_SDH_BLE_TOTAL_LINK_COUNT; i++)
    {
        if (m_qwr[i].conn_handle == BLE_CONN_HANDLE_INVALID)
        {
            ret_code_t err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr[i], conn_handle);
            APP_ERROR_CHECK(err_code);
            break;
        }
    }
}

/**@brief Function for initializing central services that are be used by the application and the queued write module.
 *
 * @details Initialize the Shimano information Profile service and Battery level collector.
 */
static void central_services_init(void)
{
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write module instances.
    qwr_init.error_handler = nrf_qwr_error_handler;

    shiis_c_init();
    bas_c_init();

    for (uint32_t i = 0; i < NRF_SDH_BLE_TOTAL_LINK_COUNT; i++) 
    {
        err_code = nrf_ble_qwr_init(&m_qwr[i], &qwr_init);
        APP_ERROR_CHECK(err_code);
    }

    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the GATT library. */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    switch (p_evt->evt_id)
    {
        case NRF_BLE_GATT_EVT_ATT_MTU_UPDATED:
        {
          //  m_mtu_exchanged = true;
          //  NRF_LOG_INFO("ATT MTU exchange completed. MTU set to %u bytes.",
          //  p_evt->params.att_mtu_effective);
        } break;

        case NRF_BLE_GATT_EVT_DATA_LENGTH_UPDATED:
        {
         //   m_data_length_updated = true;
         //   NRF_LOG_INFO("Data length updated to %u bytes.", p_evt->params.data_length);
        } break;
    }
}

/**@brief Function for initializing the GAP.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device, including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = m_scan.conn_params.min_conn_interval;
    gap_conn_params.max_conn_interval = m_scan.conn_params.max_conn_interval;
    gap_conn_params.slave_latency     = m_scan.conn_params.slave_latency;
    gap_conn_params.conn_sup_timeout  = m_scan.conn_params.conn_sup_timeout;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

#ifdef OVERRIDE_GAP_PARAM
void bleapp_gap_params_update(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
    //uint16_t   conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    err_code = ble_conn_params_change_conn_params(conn_handle, &gap_conn_params);
    APP_ERROR_CHECK(err_code);
}
#endif

/**@brief Function for handling advertising events.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            break;
        case BLE_ADV_EVT_IDLE: 
        {
            ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
        }   break;
        default:
            break;
    }
}

/**@brief Function to query peripheral connection status
 */
bool bleapp_isConnected() {
    return bleapp_connected;
}

/**@brief Function to handle rssi changes
 */
void bleapp_onRSSI(float rssi) {
    bleapp_lpf_rssi = bleapp_lpf_rssi + LPF_ALPHA_RSSI * (rssi - bleapp_lpf_rssi);
}

/**@brief Function to query peripheral device RSSI level 
 * 
 * @details gives rssi to smartphone
 */
float bleapp_getRSSI() {
    return bleapp_lpf_rssi;
}

/**@brief Function for handling database discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function forwards the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_evt_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_shiis_on_db_disc_evt(&m_shiis_c, p_evt);
    ble_bas_on_db_disc_evt(&m_bas_c, p_evt);
}

/**
 * @brief CLient/Database discovery initialization.
 * 
 * @details also known as db_discovery_init() in the sdk code
 */
static void client_init(void)
{
    ble_db_discovery_init_t db_init;

    memset(&db_init, 0, sizeof(db_init));

    db_init.evt_handler  = db_disc_evt_handler;
    db_init.p_gatt_queue = &m_ble_gatt_queue;

    ret_code_t err_code = ble_db_discovery_init(&db_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void on_ble_central_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t            err_code;
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            printf("SHIMANO connected. \r\n");
            // Discover peer's services.
            err_code = ble_db_discovery_start(&m_ble_db_discovery, p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);

            // Assign connection handle to the QWR module.
            multi_qwr_conn_handle_assign(p_gap_evt->conn_handle);

            if (ble_conn_state_central_conn_count() < NRF_SDH_BLE_CENTRAL_LINK_COUNT) 
            {
                scan_start();
            }
        } break;

        case BLE_GAP_EVT_DISCONNECTED:
        {
           // NRF_LOG_INFO("Disconnected, reason 0x%x.",
           //              p_gap_evt->params.disconnected.reason);
           printf("SHIMANO DISCONNECTED. \r\n");

            if (ble_conn_state_central_conn_count() < NRF_SDH_BLE_CENTRAL_LINK_COUNT)
            {
                scan_start();
            }
        } break;

        case BLE_GAP_EVT_TIMEOUT:
        {
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
             //   NRF_LOG_INFO("Connection Request timed out.");
            }
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
           // NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
           // NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
           // NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
    
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
           // NRF_LOG_DEBUG("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
            break;

        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
           // NRF_LOG_INFO("BLE_GAP_EVT_AUTH_KEY_REQUEST");
            break;

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            //NRF_LOG_INFO("BLE_GAP_EVT_LESC_DHKEY_REQUEST");
            break;

         case BLE_GAP_EVT_AUTH_STATUS:
            //  NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x kdist_peer:0x%x",
            //               p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
            //               p_ble_evt->evt.gap_evt.params.auth_status.bonded,
            //               p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
            //               *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
            //               *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
            break;

        default:
            break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void on_ble_peripheral_evt(ble_evt_t * p_ble_evt, void * p_context)
{
    uint32_t err_code;
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

    // if(p_ble_evt->header.evt_id != BLE_GAP_EVT_RSSI_CHANGED)
    // {
    //     printf("+++EVENT ID: %d\r\n",p_ble_evt->header.evt_id);
    // }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            printf("Connected! (%u)\r\n", BLE_GAP_EVT_CONNECTED);
            m_conn_handle = p_gap_evt->conn_handle;
            bleapp_connected = true;
            
            bleapp_lpf_rssi = -100;
            sd_ble_gap_rssi_start(m_conn_handle, 1, 0);

            sd_ble_gap_adv_stop(m_advertising.adv_handle);
            
            // Assign connection handle to the QWR module.
            multi_qwr_conn_handle_assign(p_ble_evt->evt.gap_evt.conn_handle);
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DISCONNECTED:
            printf("Disconnected, reason %d.\r\n",
                          p_gap_evt->params.disconnected.reason);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            bleapp_connected = false;
            if(bleapp_state_jumptoBootloader) {
                bleapp_jumptoBootloader();
            }
            if(bleapp_advertising_refresh) {
                bleapp_advertising_refresh = false;
                sd_ble_gap_adv_stop(m_advertising.adv_handle);
                bleapp_advertising_scan_start();
            }
 
            break; // BLE_GAP_EVT_DISCONNECTED
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
                printf("BLE_GAP_EVT_PHY_UPDATE_REQUEST\r\n");
                err_code = sd_ble_gap_phy_update(p_gap_evt->conn_handle, &phys);
                printf("sd_ble_gap_phy_update returns %u\r\n", err_code);
                APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_RSSI_CHANGED:
            bleapp_onRSSI(p_gap_evt->params.rssi_changed.rssi);
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
            printf("BLE_EVT_USER_MEM_REQUEST\r\n");
            err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
            printf("sd_ble_user_mem_reply returns %u\r\n", err_code);
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
                    printf("sd_ble_gatts_rw_authorize_reply returns %u\r\n", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
             printf("Exchange MTU Request\n");
          //   err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
          //                                               NRF_BLE_MAX_MTU_SIZE);
           //  APP_ERROR_CHECK(err_code);

           //SBEITZ Invetigate this ^
            break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST

        case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
            printf("BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST\r\n");
            err_code = sd_ble_gap_data_length_update(p_gap_evt->conn_handle, NULL, NULL);
            printf("sd_ble_gap_data_length_update returns %u\r\n", err_code);
            break;

        case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
            printf("data length update ++++++\r\n");
            break;
            
        default:
            // This routine seems like it's called a lot for everything, including data packets.
            // The below printf is only interesting in discovering which ones _needed_ to be
            // handled here.
            // printf("%s: default case, evt_id: %u\r\n", __func__, p_ble_evt->header.evt_id);
            // No implementation needed.
            break;
    }
}

/**@brief Function for checking whether a bluetooth stack event is an advertising timeout.
 *
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static bool ble_evt_is_advertising_timeout(ble_evt_t const * p_ble_evt)
{
    return (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_SET_TERMINATED);
}

/**@brief Function for dispatching Central or Peripheral role BLE events to handlers.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt, void * p_context)
{
    uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    uint16_t role        = ble_conn_state_role(conn_handle);

    // Based on the role this device plays in the connection, dispatch to the right handler.
    if (role == BLE_GAP_ROLE_PERIPH || ble_evt_is_advertising_timeout(p_ble_evt))
    {
        on_ble_peripheral_evt(p_ble_evt, NULL);

        ble_advertising_on_ble_evt(p_ble_evt, &m_advertising);
        for(int i = 0; i < BLEAPP_EVT_CNT; i++) {
            if(bleapp_evt[i] != NULL) {
                bleapp_evt[i](p_ble_evt);
            }
        }    
    }
    else if ((role == BLE_GAP_ROLE_CENTRAL) || (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT))
    {
        ble_shiis_c_on_ble_evt(p_ble_evt, NULL);
        on_ble_central_evt(p_ble_evt, NULL);
    }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupts.
 */
void ble_stack_init(void)
{
    uint32_t err_code;
    ble_cfg_t     ble_cfg;
  
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code); 
    memset(&ble_cfg, 0, sizeof(ble_cfg));

    // Configure the number of custom UUIDS.
    ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 4;
    err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register handlers for BLE and SoC events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, (void*)ble_evt_dispatch, NULL);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    printf("Peer Manager Event Handler\n");

    pm_handler_on_pm_evt(p_evt);
    //pm_handler_disconnect_on_sec_failure(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_CONN_SEC_SUCCEEDED:
            m_peer_id = p_evt->peer_id;

            // Discover peer's services.
            err_code  = ble_db_discovery_start(&m_ble_db_discovery, p_evt->conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case PM_EVT_CONN_SEC_START:
            sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            break; // PM_EVT_CONN_SEC_START

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            bleapp_advertising_scan_start();
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_CONN_HANDLE_INVALID; // Start upon connection.
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;  // Ignore events.
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/** @brief Clear bonding information from persistent storage
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    //NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

// /**@brief Function for disabling the use of whitelist for scanning.
//  */
// static void whitelist_disable(void)
// {
//     if (!m_whitelist_disabled)
//     {
//        // NRF_LOG_INFO("Whitelist temporarily disabled.");
//         m_whitelist_disabled = true;
//         nrf_ble_scan_stop();
//         scan_start();
//     }
// }

/**@brief Retrieve a list of peer manager peer IDs.
 *
 * @param[inout] p_peers   The buffer where to store the list of peer IDs.
 * @param[inout] p_size    In: The size of the @p p_peers buffer.
 *                         Out: The number of peers copied in the buffer.
 */
static void peer_list_get(pm_peer_id_t * p_peers, uint32_t * p_size)
{
    pm_peer_id_t peer_id;
    uint32_t     peers_to_copy;

    peers_to_copy = (*p_size < BLE_GAP_WHITELIST_ADDR_MAX_COUNT) ?
                     *p_size : BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

    peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
    *p_size = 0;

    while ((peer_id != PM_PEER_ID_INVALID) && (peers_to_copy--))
    {
        p_peers[(*p_size)++] = peer_id;
        peer_id = pm_next_peer_id_get(peer_id);
    }
}

/**@brief load peers into whitelist
 */
static void whitelist_load()
{
    ret_code_t   ret;
    pm_peer_id_t peers[8];
    uint32_t     peer_cnt;

    memset(peers, PM_PEER_ID_INVALID, sizeof(peers));
    peer_cnt = (sizeof(peers) / sizeof(pm_peer_id_t));

    // Load all peers from flash and whitelist them.
    peer_list_get(peers, &peer_cnt);

    ret = pm_whitelist_set(peers, peer_cnt);
    APP_ERROR_CHECK(ret);

    // Setup the device identies list.
    // Some SoftDevices do not support this feature.
    ret = pm_device_identities_list_set(peers, peer_cnt);
    if (ret != NRF_ERROR_NOT_SUPPORTED)
    {
        APP_ERROR_CHECK(ret);
    }
}

/**@brief whitelist request handler
 */
static void on_whitelist_req(void)
{
    ret_code_t err_code;

    // Whitelist buffers.
    ble_gap_addr_t whitelist_addrs[8];
    ble_gap_irk_t  whitelist_irks[8];

    memset(whitelist_addrs, 0x00, sizeof(whitelist_addrs));
    memset(whitelist_irks,  0x00, sizeof(whitelist_irks));

    uint32_t addr_cnt = (sizeof(whitelist_addrs) / sizeof(ble_gap_addr_t));
    uint32_t irk_cnt  = (sizeof(whitelist_irks)  / sizeof(ble_gap_irk_t));

    // Reload the whitelist and whitelist all peers.
    whitelist_load();

    // Get the whitelist previously set using pm_whitelist_set().
    err_code = pm_whitelist_get(whitelist_addrs, &addr_cnt,
                                whitelist_irks,  &irk_cnt);

    if (((addr_cnt == 0) && (irk_cnt == 0)) ||
        (m_whitelist_disabled))
    {
        // Don't use whitelist.
        err_code = nrf_ble_scan_params_set(&m_scan, NULL);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for the Peer Manager initialization.
 * 
 * @param[in] erase_bonds  erases peer manager bonds if true.
 */
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
}

/**@brief Function for initializing the advertising functionality.
 */
void bleapp_advertising_init(void)
{
    uint32_t      err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    //SBEITZ maybe : init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.evt_handler   = on_adv_evt;
    init.error_handler = NULL;
    
    uint8_t manufData[12];
    manufData[0] = 1; //payload version
    manufData[1] = 2; //bit 0: 1=bootloader, 0=app, 2=smarthalo 2
    memcpy(manufData+2, keys_getPubKey(), 8);

    ble_advdata_manuf_data_t   manuf_data;
    manuf_data.company_identifier = 0x0466; //our company_identifier
    manuf_data.data.p_data = (unsigned char*)manufData;
    manuf_data.data.size = 10;

    init.advdata.p_manuf_specific_data = &manuf_data;

    ble_adv_modes_config_t options;
    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    init.config = options;

    err_code = ble_advertising_init(&m_advertising, &init); //&scanrsp
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for registering peripheral events outside of this file
 * 
 * @param[in] cb  callback to be registered
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

/**@brief Function for adding characteristics
 *  
 * @param[in] uuid       characteristic uuid
 * @param[in] data       charateristic attributes value
 * @param[in] data_len   length of data
 * @param[in] flags      read-write-notify permissions 
 * @param[in] p_handles  gatt characteristic handle
 * 
 */
uint32_t bleapp_char_add(uint16_t                        uuid,
                         uint8_t                       * data,
                         uint32_t                        data_len,
                         uint32_t                        flags,
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

    if(flags & CHAR_FEAT_NOTIFY || flags & CHAR_FEAT_INDICATE) {
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
        cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
        char_md.p_cccd_md           = &cccd_md;
        char_md.char_props.notify   = (flags & CHAR_FEAT_NOTIFY) ? 1 : 0;
        char_md.char_props.indicate  = (flags & CHAR_FEAT_INDICATE) ? 1 : 0;
    }

    memset(&attr_md, 0, sizeof(attr_md));

    if(flags & CHAR_PERM_READ) {
        char_md.char_props.read = 1;
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    } else {
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    }

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

/**@brief Function for initialising smarthalo service
 */
void bleapp_service_init() {
    uint32_t   err_code = 0; // Variable to hold return codes from library and softdevice functions
    ble_uuid128_t     base_uuid = BLE_UUID_SH1_BASE_UUID;
    ble_uuid_t        service_uuid;
    service_uuid.uuid = BLE_UUID_SH1_UUID;
    BLE_UUID_BLE_ASSIGN(service_uuid, BLE_UUID_SH1_UUID);
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);    
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &bleapp_service_handle);
    APP_ERROR_CHECK(err_code);    
}

/**@brief Function for initialising smarthalo bluetooth application
 */
void bleapp_init(void)
{   
    // Initialize.
    scan_init();
    conn_params_init();
    peer_manager_init(false);
    gap_params_init();
    nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    client_init();
    bleapp_service_init();
    central_services_init();

    gatt_mtu_set(m_mtu_params.att_mtu);
    for(int i = 0; i < BLEAPP_EVT_CNT; i++) {
        bleapp_evt[i] = NULL;
    }

}
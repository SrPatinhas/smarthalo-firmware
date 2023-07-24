/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 *
 * @defgroup ble_sdk_app_hids_keyboard_main main.c
 * @{
 * @ingroup ble_sdk_app_hids_keyboard
 * @brief HID Keyboard Sample Application main file.
 *
 * This file contains is the source code for a sample application using the HID, Battery and Device
 * Information Services for implementing a simple keyboard functionality.
 * Pressing Button 0 will send text 'hello' to the connected peer. On receiving output report,
 * it toggles the state of LED 2 on the mother board based on whether or not Caps Lock is on.
 * This application uses the @ref app_scheduler.
 *
 * Also it would accept pairing requests from any peer device.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"
#include "ble_advdata.h"
#include "ble_hids.h"
#include "ble_bas.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "bsp.h"
#include "sensorsim.h"
#include "bsp_btn_ble.h"
#include "app_scheduler.h"
#include "softdevice_handler_appsh.h"
#include "app_timer_appsh.h"
#include "fstorage.h"
#include "fds.h"
#include "peer_manager.h"
#include "app_button.h"
#include "ble_conn_state.h"
#include "hal_nfc_t2t.h"

#if BUTTONS_NUMBER <2
#error "Not enough resources on board"
#endif

#define IS_SRVC_CHANGED_CHARACT_PRESENT  0                                              /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define CENTRAL_LINK_COUNT               0                                              /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                              /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define UART_TX_BUF_SIZE                 256                                            /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                 1                                              /**< UART RX buffer size. */

#define KEY_PRESS_BUTTON_ID              0                                              /**< Button used as Keyboard key press. */
#define SHIFT_BUTTON_ID                  1                                              /**< Button used as 'SHIFT' Key. */

#define DEVICE_NAME                      "Nordic_Keyboard"                              /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "NordicSemiconductor"                          /**< Manufacturer. Will be passed to Device Information Service. */

#define APP_TIMER_PRESCALER              0                                              /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                              /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL      APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)     /**< Battery level measurement interval (ticks). */
#define MIN_BATTERY_LEVEL                81                                             /**< Minimum simulated battery level. */
#define MAX_BATTERY_LEVEL                100                                            /**< Maximum simulated battery level. */
#define BATTERY_LEVEL_INCREMENT          1                                              /**< Increment between each simulated battery level measurement. */

#define PNP_ID_VENDOR_ID_SOURCE          0x02                                           /**< Vendor ID Source. */
#define PNP_ID_VENDOR_ID                 0x1915                                         /**< Vendor ID. */
#define PNP_ID_PRODUCT_ID                0xEEEE                                         /**< Product ID. */
#define PNP_ID_PRODUCT_VERSION           0x0001                                         /**< Product Version. */

#define APP_ADV_FAST_INTERVAL            0x0028                                         /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL            0x0C80                                         /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */
#define APP_ADV_FAST_TIMEOUT             30                                             /**< The duration of the fast advertising period (in seconds). */
#define APP_ADV_SLOW_TIMEOUT             180                                            /**< The duration of the slow advertising period (in seconds). */

/*lint -emacro(524, MIN_CONN_INTERVAL) // Loss of precision */
#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(7.5, UNIT_1_25_MS)               /**< Minimum connection interval (7.5 ms) */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(30, UNIT_1_25_MS)                /**< Maximum connection interval (30 ms). */
#define SLAVE_LATENCY                    6                                              /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(430, UNIT_10_MS)                 /**< Connection supervisory timeout (430 ms). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)     /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)    /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                              /**< Number of attempts before giving up the connection parameter negotiation. */


#define OUTPUT_REPORT_INDEX              0                                              /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN            1                                              /**< Maximum length of Output Report. */
#define INPUT_REPORT_KEYS_INDEX          0                                              /**< Index of Input Report. */
#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK 0x02                                           /**< CAPS LOCK bit in Output Report (based on 'LED Page (0x08)' of the Universal Serial Bus HID Usage Tables). */
#define INPUT_REP_REF_ID                 0                                              /**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_REF_ID                0                                              /**< Id of reference to Keyboard Output Report. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2            /**< Reply when unsupported features are requested. */

#define MAX_BUFFER_ENTRIES               5                                              /**< Number of elements that can be enqueued */

#define BASE_USB_HID_SPEC_VERSION        0x0101                                         /**< Version number of base USB HID Specification implemented by this application. */

#define INPUT_REPORT_KEYS_MAX_LEN        8                                              /**< Maximum length of the Input Report characteristic. */

#define DEAD_BEEF                        0xDEADBEEF                                     /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SCHED_MAX_EVENT_DATA_SIZE        MAX(APP_TIMER_SCHED_EVT_SIZE,\
                                             BLE_STACK_HANDLER_SCHED_EVT_SIZE)          /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                 10                                             /**< Maximum number of events in the scheduler queue. */

#define MODIFIER_KEY_POS                 0                                              /**< Position of the modifier byte in the Input Report. */
#define SCAN_CODE_POS                    2                                              /**< This macro indicates the start position of the key scan code in a HID Report. As per the document titled 'Device Class Definition for Human Interface Devices (HID) V1.11, each report shall have one modifier byte followed by a reserved constant byte and then the key scan code. */
#define SHIFT_KEY_CODE                   0x02                                           /**< Key code indicating the press of the Shift Key. */

#define MAX_KEYS_IN_ONE_REPORT           (INPUT_REPORT_KEYS_MAX_LEN - SCAN_CODE_POS)    /**< Maximum number of key presses that can be sent in one Input Report. */


/**Buffer queue access macros
 *
 * @{ */
/** Initialization of buffer list */
#define BUFFER_LIST_INIT()                                                                        \
        do                                                                                        \
        {                                                                                         \
            buffer_list.rp = 0;                                                                   \
            buffer_list.wp = 0;                                                                   \
            buffer_list.count = 0;                                                                \
        } while (0)

/** Provide status of data list is full or not */
#define BUFFER_LIST_FULL()\
        ((MAX_BUFFER_ENTRIES == buffer_list.count - 1) ? true : false)

/** Provides status of buffer list is empty or not */
#define BUFFER_LIST_EMPTY()\
        ((0 == buffer_list.count) ? true : false)

#define BUFFER_ELEMENT_INIT(i)\
        do                                                                                        \
        {                                                                                         \
            buffer_list.buffer[(i)].p_data = NULL;                                                \
        } while (0)

/** @} */

typedef enum
{
    BLE_NO_ADV,               /**< No advertising running. */
    BLE_DIRECTED_ADV,         /**< Direct advertising to the latest central. */
    BLE_FAST_ADV_WHITELIST,   /**< Advertising with whitelist. */
    BLE_FAST_ADV,             /**< Fast advertising running. */
    BLE_SLOW_ADV,             /**< Slow advertising running. */
    BLE_SLEEP,                /**< Go to system-off. */
} ble_advertising_mode_t;

/** Abstracts buffer element */
typedef struct hid_key_buffer
{
    uint8_t    data_offset;   /**< Max Data that can be buffered for all entries */
    uint8_t    data_len;      /**< Total length of data */
    uint8_t    * p_data;      /**< Scanned key pattern */
    ble_hids_t * p_instance;  /**< Identifies peer and service instance */
}buffer_entry_t;

STATIC_ASSERT(sizeof(buffer_entry_t) % 4 == 0);

/** Circular buffer list */
typedef struct
{
    buffer_entry_t buffer[MAX_BUFFER_ENTRIES]; /**< Maximum number of entries that can enqueued in the list */
    uint8_t        rp;                         /**< Index to the read location */
    uint8_t        wp;                         /**< Index to write location */
    uint8_t        count;                      /**< Number of elements in the list */
}buffer_list_t;

STATIC_ASSERT(sizeof(buffer_list_t) % 4 == 0);

static ble_hids_t                        m_hids;                                        /**< Structure used to identify the HID service. */
static ble_bas_t                         m_bas;                                         /**< Structure used to identify the battery service. */
static bool                              m_in_boot_mode = false;                        /**< Current protocol mode. */
static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;       /**< Handle of the current connection. */

static sensorsim_cfg_t                   m_battery_sim_cfg;                             /**< Battery Level sensor simulator configuration. */
static sensorsim_state_t                 m_battery_sim_state;                           /**< Battery Level sensor simulator state. */

APP_TIMER_DEF(m_battery_timer_id);                                                      /**< Battery timer. */

static pm_peer_id_t                      m_peer_id;                                     /**< Device reference handle to the current bonded central. */
static bool                              m_caps_on = false;                             /**< Variable to indicate if Caps Lock is turned on. */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

static uint8_t m_sample_key_press_scan_str[] =                                          /**< Key pattern to be sent when the key press button has been pushed. */
{
    0x0b, /* Key h */
    0x08, /* Key e */
    0x0f, /* Key l */
    0x0f, /* Key l */
    0x12, /* Key o */
    0x28  /* Key Return */
};

static uint8_t m_caps_on_key_scan_str[] =                                                /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit set. */
{
    0x06, /* Key C */
    0x04, /* Key a */
    0x13, /* Key p */
    0x16, /* Key s */
    0x12, /* Key o */
    0x11, /* Key n */
};

static uint8_t m_caps_off_key_scan_str[] =                                               /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit cleared. */
{
    0x06, /* Key C */
    0x04, /* Key a */
    0x13, /* Key p */
    0x16, /* Key s */
    0x12, /* Key o */
    0x09, /* Key f */
};


/** List to enqueue not just data to be sent, but also related information like the handle, connection handle etc */
static buffer_list_t buffer_list;

static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt);


/***************************************************************************************************
 * Start of modifications needed for BLE pairing over NFC
 **************************************************************************************************/
#include "nfc_t2t_lib.h"
#include "nfc_ble_pair_msg.h"
#include "nfc_ndef_msg.h"
#include "nfc_text_rec.h"
#include "nfc_fixes.h"

#define SEC_PARAM_OOB                    1                                              /**< Out Of Band data available. */
#define SEC_PARAM_MITM                   1                                              /**< Man In The Middle protection supported (via OOB). */
#define SEC_PARAM_BOND                   1                                              /**< Perform bonding. */
#define SEC_PARAM_LESC                   0                                              /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                              /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                           /**< No I/O capabilities. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                              /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                             /**< Maximum encryption key size. */

// Hardcoded authentication OOB key
// In this example, this value is constant. However, to be compatible with
// the BLE Core Specification for OOB pairing, generate a random value for the Temporary Key.
#define OOB_AUTH_KEY                 {                            \
                                        {                         \
                                          0xAA, 0xBB, 0xCC, 0xDD, \
                                          0xEE, 0xFF, 0x99, 0x88, \
                                          0x77, 0x66, 0x55, 0x44, \
                                          0x33, 0x22, 0x11, 0x00  \
                                        }                         \
                                     }

static ble_advdata_tk_value_t        m_oob_auth_key = OOB_AUTH_KEY;

/* Data for displaying NFC message to user when device is paired */
static const uint8_t m_nfc_text[] = {'A', 'l', 'r', 'e', 'a', 'd', 'y', ' ', 'p', 'a', 'i', 'r', 'e', 'd', '!'};
static const uint8_t m_language[] = {'e', 'n'};

static ble_advdata_t                     m_advdata;
static volatile uint8_t                  m_advertising_flag = 0;
uint8_t                                  ndef_msg_buf[256];

static void advertising_disable(void);
static void advertising_enable(void);
static void on_ble_evt(ble_evt_t * p_ble_evt);

 /**@brief Function for setting the BLE pairing data for the NFC tag.
 *
 * @details This function does not stop and start the NFC Tag data emulation.
 *
 */
static void nfc_pairing_data_set(void);

/**@brief Function for setting the text message for the NFC tag.
 *
 * @details This function does not stop and start the NFC Tag data emulation.
 *
 */
static void nfc_text_data_set(void);

/***************************************************************************************************
 * End of modifications needed for BLE pairing over NFC
 **************************************************************************************************/

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling advertising errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void ble_advertising_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for performing a battery measurement, and update the Battery Level characteristic in the Battery Service.
 */
static void battery_level_update(void)
{
    uint32_t err_code;
    uint8_t  battery_level;

    battery_level = (uint8_t)sensorsim_measure(&m_battery_sim_state, &m_battery_sim_cfg);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != BLE_ERROR_NO_TX_PACKETS) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_level_update();
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    // Create battery timer.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HID_KEYBOARD);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing Device Information Service.
 */
static void dis_init(void)
{
    uint32_t         err_code;
    ble_dis_init_t   dis_init_obj;
    ble_dis_pnp_id_t pnp_id;

    pnp_id.vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
    pnp_id.vendor_id        = PNP_ID_VENDOR_ID;
    pnp_id.product_id       = PNP_ID_PRODUCT_ID;
    pnp_id.product_version  = PNP_ID_PRODUCT_VERSION;

    memset(&dis_init_obj, 0, sizeof(dis_init_obj));

    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    dis_init_obj.p_pnp_id = &pnp_id;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing Battery Service.
 */
static void bas_init(void)
{
    uint32_t       err_code;
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler          = NULL;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref         = NULL;
    bas_init_obj.initial_batt_level   = 100;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_report_read_perm);

    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing HID Service.
 */
static void hids_init(void)
{
    uint32_t                   err_code;
    ble_hids_init_t            hids_init_obj;
    ble_hids_inp_rep_init_t    input_report_array[1];
    ble_hids_inp_rep_init_t  * p_input_report;
    ble_hids_outp_rep_init_t   output_report_array[1];
    ble_hids_outp_rep_init_t * p_output_report;
    uint8_t                    hid_info_flags;

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));
    
    static uint8_t report_map_data[] =
    {
        0x05, 0x01,                 // Usage Page (Generic Desktop)
        0x09, 0x06,                 // Usage (Keyboard)
        0xA1, 0x01,                 // Collection (Application)
        0x05, 0x07,                 //     Usage Page (Key Codes)
        0x19, 0xe0,                 //     Usage Minimum (224)
        0x29, 0xe7,                 //     Usage Maximum (231)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x25, 0x01,                 //     Logical Maximum (1)
        0x75, 0x01,                 //     Report Size (1)
        0x95, 0x08,                 //     Report Count (8)
        0x81, 0x02,                 //     Input (Data, Variable, Absolute)

        0x95, 0x01,                 //     Report Count (1)
        0x75, 0x08,                 //     Report Size (8)
        0x81, 0x01,                 //     Input (Constant) reserved byte(1)

        0x95, 0x05,                 //     Report Count (5)
        0x75, 0x01,                 //     Report Size (1)
        0x05, 0x08,                 //     Usage Page (Page# for LEDs)
        0x19, 0x01,                 //     Usage Minimum (1)
        0x29, 0x05,                 //     Usage Maximum (5)
        0x91, 0x02,                 //     Output (Data, Variable, Absolute), Led report
        0x95, 0x01,                 //     Report Count (1)
        0x75, 0x03,                 //     Report Size (3)
        0x91, 0x01,                 //     Output (Data, Variable, Absolute), Led report padding

        0x95, 0x06,                 //     Report Count (6)
        0x75, 0x08,                 //     Report Size (8)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x25, 0x65,                 //     Logical Maximum (101)
        0x05, 0x07,                 //     Usage Page (Key codes)
        0x19, 0x00,                 //     Usage Minimum (0)
        0x29, 0x65,                 //     Usage Maximum (101)
        0x81, 0x00,                 //     Input (Data, Array) Key array(6 bytes)

        0x09, 0x05,                 //     Usage (Vendor Defined)
        0x15, 0x00,                 //     Logical Minimum (0)
        0x26, 0xFF, 0x00,           //     Logical Maximum (255)
        0x75, 0x08,                 //     Report Count (2)
        0x95, 0x02,                 //     Report Size (8 bit)
        0xB1, 0x02,                 //     Feature (Data, Variable, Absolute)

        0xC0                        // End Collection (Application)
    };

    // Initialize HID Service
    p_input_report                      = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    p_input_report->max_len             = INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = INPUT_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

    p_output_report                      = &output_report_array[OUTPUT_REPORT_INDEX];
    p_output_report->max_len             = OUTPUT_REPORT_MAX_LEN;
    p_output_report->rep_ref.report_id   = OUTPUT_REP_REF_ID;
    p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.write_perm);

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler                    = on_hids_evt;
    hids_init_obj.error_handler                  = service_error_handler;
    hids_init_obj.is_kb                          = true;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = 1;
    hids_init_obj.p_inp_rep_array                = input_report_array;
    hids_init_obj.outp_rep_count                 = 1;
    hids_init_obj.p_outp_rep_array               = output_report_array;
    hids_init_obj.feature_rep_count              = 0;
    hids_init_obj.p_feature_rep_array            = NULL;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags          = hid_info_flags;
    hids_init_obj.included_services_count        = 0;
    hids_init_obj.p_included_services_array      = NULL;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.rep_map.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.rep_map.security_mode.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.hid_information.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.hid_information.security_mode.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(
        &hids_init_obj.security_mode_boot_kb_inp_rep.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_inp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_boot_kb_inp_rep.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_ctrl_point.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_ctrl_point.write_perm);

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    dis_init();
    bas_init();
    hids_init();
}


/**@brief Function for initializing the battery sensor simulator.
 */
static void sensor_simulator_init(void)
{
    m_battery_sim_cfg.min          = MIN_BATTERY_LEVEL;
    m_battery_sim_cfg.max          = MAX_BATTERY_LEVEL;
    m_battery_sim_cfg.incr         = BATTERY_LEVEL_INCREMENT;
    m_battery_sim_cfg.start_at_max = true;

    sensorsim_init(&m_battery_sim_state, &m_battery_sim_cfg);
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
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
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void timers_start(void)
{
    uint32_t err_code;

    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}



/** @brief   Function for checking if the Shift key is pressed.
 *
 *  @returns true if the SHIFT_BUTTON is pressed. false otherwise.
 */
static bool is_shift_key_pressed(void)
{
    bool result;
    uint32_t err_code = bsp_button_is_pressed(SHIFT_BUTTON_ID,&result);
    APP_ERROR_CHECK(err_code);
    return result;
}


/**@brief   Function for transmitting a key scan Press & Release Notification.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  p_instance     Identifies the service for which Key Notifications are requested.
 * @param[in]  p_key_pattern  Pointer to key pattern.
 * @param[in]  pattern_len    Length of key pattern. 0 < pattern_len < 7.
 * @param[in]  pattern_offset Offset applied to Key Pattern for transmission.
 * @param[out] actual_len     Provides actual length of Key Pattern transmitted, making buffering of
 *                            rest possible if needed.
 * @return     NRF_SUCCESS on success, BLE_ERROR_NO_TX_PACKETS in case transmission could not be
 *             completed due to lack of transmission buffer or other error codes indicating reason
 *             for failure.
 *
 * @note       In case of BLE_ERROR_NO_TX_PACKETS, remaining pattern that could not be transmitted
 *             can be enqueued \ref buffer_enqueue function.
 *             In case a pattern of 'cofFEe' is the p_key_pattern, with pattern_len as 6 and
 *             pattern_offset as 0, the notifications as observed on the peer side would be
 *             1>    'c', 'o', 'f', 'F', 'E', 'e'
 *             2>    -  , 'o', 'f', 'F', 'E', 'e'
 *             3>    -  ,   -, 'f', 'F', 'E', 'e'
 *             4>    -  ,   -,   -, 'F', 'E', 'e'
 *             5>    -  ,   -,   -,   -, 'E', 'e'
 *             6>    -  ,   -,   -,   -,   -, 'e'
 *             7>    -  ,   -,   -,   -,   -,  -
 *             Here, '-' refers to release, 'c' refers to the key character being transmitted.
 *             Therefore 7 notifications will be sent.
 *             In case an offset of 4 was provided, the pattern notifications sent will be from 5-7
 *             will be transmitted.
 */
static uint32_t send_key_scan_press_release(ble_hids_t *   p_hids,
                                            uint8_t *      p_key_pattern,
                                            uint16_t       pattern_len,
                                            uint16_t       pattern_offset,
                                            uint16_t *     p_actual_len)
{
    uint32_t err_code;
    uint16_t offset;
    uint16_t data_len;
    uint8_t  data[INPUT_REPORT_KEYS_MAX_LEN];
    
    // HID Report Descriptor enumerates an array of size 6, the pattern hence shall not be any
    // longer than this.
    STATIC_ASSERT((INPUT_REPORT_KEYS_MAX_LEN - 2) == 6);

    ASSERT(pattern_len <= (INPUT_REPORT_KEYS_MAX_LEN - 2));

    offset   = pattern_offset;
    data_len = pattern_len;

    do
    {
        // Reset the data buffer. 
        memset(data, 0, sizeof(data));
        
        // Copy the scan code.
        memcpy(data + SCAN_CODE_POS + offset, p_key_pattern + offset, data_len - offset);
        
        if (is_shift_key_pressed())
        {
            data[MODIFIER_KEY_POS] |= SHIFT_KEY_CODE;
        }

        if (!m_in_boot_mode)
        {
            err_code = ble_hids_inp_rep_send(p_hids, 
                                             INPUT_REPORT_KEYS_INDEX,
                                             INPUT_REPORT_KEYS_MAX_LEN,
                                             data);
        }
        else
        {
            err_code = ble_hids_boot_kb_inp_rep_send(p_hids,
                                                     INPUT_REPORT_KEYS_MAX_LEN,
                                                     data);
        }
        
        if (err_code != NRF_SUCCESS)
        {
            break;
        }
        
        offset++;
    } while (offset <= data_len);

    *p_actual_len = offset;

    return err_code;
}


/**@brief   Function for initializing the buffer queue used to key events that could not be
 *          transmitted
 *
 * @warning This handler is an example only. You need to analyze how you wish to buffer or buffer at
 *          all.
 *
 * @note    In case of HID keyboard, a temporary buffering could be employed to handle scenarios
 *          where encryption is not yet enabled or there was a momentary link loss or there were no
 *          Transmit buffers.
 */
static void buffer_init(void)
{
    uint32_t buffer_count;

    BUFFER_LIST_INIT();

    for (buffer_count = 0; buffer_count < MAX_BUFFER_ENTRIES; buffer_count++)
    {
        BUFFER_ELEMENT_INIT(buffer_count);
    }
}


/**@brief Function for enqueuing key scan patterns that could not be transmitted either completely
 *        or partially.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  p_hids         Identifies the service for which Key Notifications are buffered.
 * @param[in]  p_key_pattern  Pointer to key pattern.
 * @param[in]  pattern_len    Length of key pattern.
 * @param[in]  offset         Offset applied to Key Pattern when requesting a transmission on
 *                            dequeue, @ref buffer_dequeue.
 * @return     NRF_SUCCESS on success, else an error code indicating reason for failure.
 */
static uint32_t buffer_enqueue(ble_hids_t *            p_hids,
                               uint8_t *               p_key_pattern,
                               uint16_t                pattern_len,
                               uint16_t                offset)
{
    buffer_entry_t * element;
    uint32_t         err_code = NRF_SUCCESS;

    if (BUFFER_LIST_FULL())
    {
        // Element cannot be buffered.
        err_code = NRF_ERROR_NO_MEM;
    }
    else
    {
        // Make entry of buffer element and copy data.
        element                 = &buffer_list.buffer[(buffer_list.wp)];
        element->p_instance     = p_hids;
        element->p_data         = p_key_pattern;
        element->data_offset    = offset;
        element->data_len       = pattern_len;

        buffer_list.count++;
        buffer_list.wp++;

        if (buffer_list.wp == MAX_BUFFER_ENTRIES)
        {
            buffer_list.wp = 0;
        }
    }

    return err_code;
}


/**@brief   Function to dequeue key scan patterns that could not be transmitted either completely of
 *          partially.
 *
 * @warning This handler is an example only. You need to analyze how you wish to send the key
 *          release.
 *
 * @param[in]  tx_flag   Indicative of whether the dequeue should result in transmission or not.
 * @note       A typical example when all keys are dequeued with transmission is when link is
 *             disconnected.
 *
 * @return     NRF_SUCCESS on success, else an error code indicating reason for failure.
 */
static uint32_t buffer_dequeue(bool tx_flag)
{
    buffer_entry_t * p_element;
    uint32_t         err_code = NRF_SUCCESS;
    uint16_t         actual_len;

    if (BUFFER_LIST_EMPTY()) 
    {
        err_code = NRF_ERROR_NOT_FOUND;
    }
    else
    {
        bool remove_element = true;

        p_element = &buffer_list.buffer[(buffer_list.rp)];

        if (tx_flag)
        {
            err_code = send_key_scan_press_release(p_element->p_instance,
                                                   p_element->p_data,
                                                   p_element->data_len,
                                                   p_element->data_offset,
                                                   &actual_len);
            // An additional notification is needed for release of all keys, therefore check
            // is for actual_len <= element->data_len and not actual_len < element->data_len
            if ((err_code == BLE_ERROR_NO_TX_PACKETS) && (actual_len <= p_element->data_len))
            {
                // Transmission could not be completed, do not remove the entry, adjust next data to
                // be transmitted
                p_element->data_offset = actual_len;
                remove_element         = false;
            }
        }

        if (remove_element)
        {
            BUFFER_ELEMENT_INIT(buffer_list.rp);

            buffer_list.rp++;
            buffer_list.count--;

            if (buffer_list.rp == MAX_BUFFER_ENTRIES)
            {
                buffer_list.rp = 0;
            }
        }
    }

    return err_code;
}


/**@brief Function for sending sample key presses to the peer.
 *
 * @param[in]   key_pattern_len   Pattern length.
 * @param[in]   p_key_pattern     Pattern to be sent.
 */
static void keys_send(uint8_t key_pattern_len, uint8_t * p_key_pattern)
{
    uint32_t err_code;
    uint16_t actual_len;

    err_code = send_key_scan_press_release(&m_hids,
                                           p_key_pattern,
                                           key_pattern_len,
                                           0,
                                           &actual_len);
    // An additional notification is needed for release of all keys, therefore check
    // is for actual_len <= key_pattern_len and not actual_len < key_pattern_len.
    if ((err_code == BLE_ERROR_NO_TX_PACKETS) && (actual_len <= key_pattern_len))
    {
        // Buffer enqueue routine return value is not intentionally checked.
        // Rationale: Its better to have a a few keys missing than have a system
        // reset. Recommendation is to work out most optimal value for
        // MAX_BUFFER_ENTRIES to minimize chances of buffer queue full condition
        UNUSED_VARIABLE(buffer_enqueue(&m_hids, p_key_pattern, key_pattern_len, actual_len));
    }


    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != BLE_ERROR_NO_TX_PACKETS) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
    )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for handling the HID Report Characteristic Write event.
 *
 * @param[in]   p_evt   HID service event.
 */
static void on_hid_rep_char_write(ble_hids_evt_t *p_evt)
{
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT)
    {
        uint32_t err_code;
        uint8_t  report_val;
        uint8_t  report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX)
        {
            // This code assumes that the outptu report is one byte long. Hence the following
            // static assert is made.
            STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

            err_code = ble_hids_outp_rep_get(&m_hids,
                                             report_index,
                                             OUTPUT_REPORT_MAX_LEN,
                                             0,
                                             &report_val);
            APP_ERROR_CHECK(err_code);

            if (!m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) != 0))
            {
                // Caps Lock is turned On.
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_3);
                APP_ERROR_CHECK(err_code);

                keys_send(sizeof(m_caps_on_key_scan_str), m_caps_on_key_scan_str);
                m_caps_on = true;
            }
            else if (m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) == 0))
            {
                // Caps Lock is turned Off .
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
                APP_ERROR_CHECK(err_code);

                keys_send(sizeof(m_caps_off_key_scan_str), m_caps_off_key_scan_str);
                m_caps_on = false;
            }
            else
            {
                // The report received is not supported by this application. Do nothing.
            }
        }
    }
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


/**@brief Function for handling HID events.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service structure.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t *p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            m_in_boot_mode = true;
            break;//BLE_HIDS_EVT_BOOT_MODE_ENTERED

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            m_in_boot_mode = false;
            break;//BLE_HIDS_EVT_REPORT_MODE_ENTERED

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            on_hid_rep_char_write(p_evt);
            break;//BLE_HIDS_EVT_REP_CHAR_WRITE

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_DIRECTED:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
            APP_ERROR_CHECK(err_code);
            break;//BLE_ADV_EVT_DIRECTED

        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;//BLE_ADV_EVT_FAST

        case BLE_ADV_EVT_SLOW:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
            APP_ERROR_CHECK(err_code);
            break;//BLE_ADV_EVT_SLOW

        case BLE_ADV_EVT_FAST_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;//BLE_ADV_EVT_FAST_WHITELIST

        case BLE_ADV_EVT_SLOW_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;//BLE_ADV_EVT_SLOW_WHITELIST

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;//BLE_ADV_EVT_IDLE

        case BLE_ADV_EVT_WHITELIST_REQUEST:
        {
            // Storage for the whitelist.
            ble_gap_irk_t  * irks[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_addr_t * addrs[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
            ble_gap_whitelist_t whitelist = {.pp_irks = irks, .pp_addrs = addrs};

            // Construct a list of peer IDs
            pm_peer_id_t peer_ids[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            pm_peer_id_t peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
            uint32_t n_peer_ids = 0;

            while((peer_id != PM_PEER_ID_INVALID) && (n_peer_ids < BLE_GAP_WHITELIST_ADDR_MAX_COUNT))
            {
                peer_ids[n_peer_ids++] = peer_id;
                peer_id = pm_next_peer_id_get(peer_id);
            }

            // Create the whitelist.
            err_code = pm_whitelist_create(peer_ids, n_peer_ids, &whitelist);
            APP_ERROR_CHECK(err_code);

            // Apply the whitelist.
            err_code = ble_advertising_whitelist_reply(&whitelist);
            APP_ERROR_CHECK(err_code);

        }break;//BLE_ADV_EVT_WHITELIST_REQUEST

        case BLE_ADV_EVT_PEER_ADDR_REQUEST:
        {
            pm_peer_data_bonding_t peer_bonding_data;

            // Only Give peer address if we have a handle to the bonded peer.
            if(m_peer_id != PM_PEER_ID_INVALID)
            {
                err_code = pm_peer_data_bonding_load(m_peer_id, &peer_bonding_data);
                if (err_code != NRF_ERROR_NOT_FOUND)
                {
                    APP_ERROR_CHECK(err_code);

                    ble_gap_addr_t * p_peer_addr = &(peer_bonding_data.peer_id.id_addr_info);
                    err_code = ble_advertising_peer_addr_reply(p_peer_addr);
                    APP_ERROR_CHECK(err_code);
                }
            }
        }break;//BLE_ADV_EVT_PEER_ADDR_REQUEST

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                              err_code;
    NfcRetval                             ret_val;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            m_conn_handle      = p_ble_evt->evt.gap_evt.conn_handle;

            /* Disable advertising, so when disconnected it does not restart. */
            advertising_disable();
            break;//BLE_GAP_EVT_CONNECTED

        case BLE_EVT_TX_COMPLETE:
            // Send next key event
            (void) buffer_dequeue(true);
            break;//BLE_EVT_TX_COMPLETE

        case BLE_GAP_EVT_DISCONNECTED:
        {
            // Dequeue all keys without transmission.
            (void) buffer_dequeue(false);

            m_conn_handle = BLE_CONN_HANDLE_INVALID;

            // Reset m_caps_on variable. Upon reconnect, the HID host will re-send the Output 
            // report containing the Caps lock state.
            m_caps_on = false;
            // disabling alert 3. signal - used for capslock ON
            err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
            APP_ERROR_CHECK(err_code);
            
            m_advertising_flag = 0;
        }break;//BLE_GAP_EVT_DISCONNECTED

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;//BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            if(p_ble_evt->evt.gatts_evt.params.authorize_request.type
               != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_PREP_WRITE_REQ)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (p_ble_evt->evt.gatts_evt.params.authorize_request.type
                        == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(m_conn_handle,&auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        }break;//BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
        {
            err_code = sd_ble_gap_auth_key_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_AUTH_KEY_TYPE_OOB, m_oob_auth_key.tk);
            APP_ERROR_CHECK(err_code);
        }break;//BLE_GAP_EVT_AUTH_KEY_REQUEST

        case BLE_GAP_EVT_AUTH_STATUS:
            if (p_ble_evt->evt.gap_evt.params.auth_status.auth_status == BLE_GAP_SEC_STATUS_SUCCESS)
            {
                /* Configure NFC launchapp data (requires stopping NFC Tag emulation) */
                ret_val = nfcStopEmulation();
                if (ret_val != NFC_RETVAL_OK)
                {
                    APP_ERROR_CHECK((uint32_t) ret_val);
                }

                nfc_text_data_set();

                ret_val = nfcStartEmulation();
                if (ret_val != NFC_RETVAL_OK)
                {
                    APP_ERROR_CHECK((uint32_t) ret_val);
                }
            }
            break; //BLE_GAP_EVT_AUTH_STATUS

        case BLE_GATTC_EVT_TIMEOUT:
        /* fall through */
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;//BLE_GATTC_EVT_TIMEOUT and BLE_GATTS_EVT_TIMEOUT

        default:
            // No implementation needed.
            break;
    }
}


/**@brief   Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_hids_on_ble_evt(&m_hids, p_ble_evt);
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
}


/**@brief   Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    fs_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
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
    SOFTDEVICE_HANDLER_APPSH_INIT(&clock_lf_cfg, true);

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

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    static uint8_t * p_key = m_sample_key_press_scan_str;
    static uint8_t size = 0;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;//BSP_EVENT_SLEEP

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;//BSP_EVENT_DISCONNECT

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;//BSP_EVENT_WHITELIST_OFF

        case BSP_EVENT_KEY_0:
            if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                keys_send(1, p_key);
                p_key++;
                size++;
                if (size == MAX_KEYS_IN_ONE_REPORT)
                {
                    p_key = m_sample_key_press_scan_str;
                    size = 0;
                }
            }
            break;//BSP_EVENT_KEY_0

        default:
            // No implementation needed.
            break;
    }
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
            NRF_LOG_PRINTF_DEBUG("Connected to previously bonded device\r\n");
            m_peer_id = p_evt->peer_id;
            err_code = pm_peer_rank_highest(p_evt->peer_id);
            if (err_code != NRF_ERROR_BUSY)
            {
                    APP_ERROR_CHECK(err_code);
            }
        }break;//PM_EVT_BONDED_PEER_CONNECTED

        case PM_EVT_CONN_SEC_START:
            break;//PM_EVT_CONN_SEC_START

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_PRINTF_DEBUG("Link secured. Role: %d. conn_handle: %d, Procedure: %d\r\n",
                           ble_conn_state_role(p_evt->conn_handle),
                           p_evt->conn_handle,
                           p_evt->params.conn_sec_succeeded.procedure);
            err_code = pm_peer_rank_highest(p_evt->peer_id);
            if (err_code != NRF_ERROR_BUSY)
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
                    break;

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
            break;//PM_EVT_STORAGE_FULL

        case PM_EVT_ERROR_UNEXPECTED:
            // A likely fatal error occurred. Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
            break;//PM_EVT_ERROR_UNEXPECTED

        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
            break;//PM_EVT_PEER_DATA_UPDATE_SUCCEEDED

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
            break;//PM_EVT_PEER_DATA_UPDATE_FAILED

        case PM_EVT_PEER_DELETE_SUCCEEDED:
            break;//PM_EVT_PEER_DELETE_SUCCEEDED

        case PM_EVT_PEER_DELETE_FAILED:
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
            break;//PM_EVT_PEER_DELETE_FAILED

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            break;//PM_EVT_PEERS_DELETE_SUCCEEDED

        case PM_EVT_PEERS_DELETE_FAILED:
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
    sec_param.bond              = SEC_PARAM_BOND;
    sec_param.mitm              = SEC_PARAM_MITM;
    sec_param.lesc              = SEC_PARAM_LESC;
    sec_param.keypress          = SEC_PARAM_KEYPRESS;
    sec_param.io_caps           = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob               = SEC_PARAM_OOB;
    sec_param.min_key_size      = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size      = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc     = 1;
    sec_param.kdist_own.id      = 1;
    sec_param.kdist_peer.enc    = 1;
    sec_param.kdist_peer.id     = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and LEDs.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds, bool * p_wake_on_button)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds    = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
    *p_wake_on_button = (startup_event == BSP_EVENT_WAKEUP);
}

/***************************************************************************************************
 * Start of modifications needed for BLE pairing over NFC
 **************************************************************************************************/
 
/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&m_advdata, 0, sizeof(m_advdata));
    //Only set up adv_data. Options will be set depending on if advertising will be enabled or not.
    m_advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    m_advdata.include_appearance      = true;
    m_advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    m_advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    m_advdata.uuids_complete.p_uuids  = m_adv_uuids;
}


/**@brief Function for re-initializing the Advertising module with fast advertising mode enabled.
 */
static void advertising_enable(void)
{
    uint32_t      err_code;

    ble_adv_modes_config_t options =
    {
        BLE_ADV_WHITELIST_ENABLED,
        BLE_ADV_DIRECTED_ENABLED,
        BLE_ADV_DIRECTED_SLOW_DISABLED, 0,0,
        BLE_ADV_FAST_ENABLED, APP_ADV_FAST_INTERVAL, APP_ADV_FAST_TIMEOUT,
        BLE_ADV_SLOW_ENABLED, APP_ADV_SLOW_INTERVAL, APP_ADV_SLOW_TIMEOUT
    };

    err_code = ble_advertising_init(&m_advdata, NULL, &options, on_adv_evt, ble_advertising_error_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for re-initializing the Advertising module with all advertising modes disabled.
 */
static void advertising_disable(void)
{
    uint32_t      err_code;

    ble_adv_modes_config_t options    = {0};
    options.ble_adv_directed_enabled  = BLE_ADV_DIRECTED_DISABLED;
    options.ble_adv_fast_enabled      = BLE_ADV_FAST_DISABLED;
    options.ble_adv_slow_enabled      = BLE_ADV_SLOW_DISABLED;
    options.ble_adv_whitelist_enabled = BLE_ADV_WHITELIST_DISABLED;

    err_code = ble_advertising_init(&m_advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}

static void nfc_pairing_data_set(void)
{
    NfcRetval ret_val;
    uint32_t  err_code;
   
    /** @snippet [NFC BLE pair usage_1] */
    /* Provide information about available buffer size to encoding function. */
    uint32_t ndef_msg_len = sizeof(ndef_msg_buf);

    /* Encode BLE pairing message into the buffer. */
    err_code = nfc_ble_pair_default_msg_encode(NFC_BLE_PAIR_MSG_FULL,
                                               &m_oob_auth_key,
                                               ndef_msg_buf,
                                               &ndef_msg_len);
    APP_ERROR_CHECK(err_code);
    /** @snippet [NFC BLE pair usage_1] */

    /* Configure the NFC Tag data */
    ret_val = nfcSetPayload((char *)ndef_msg_buf, ndef_msg_len);
    if (ret_val != NFC_RETVAL_OK)
    {
        APP_ERROR_CHECK((uint32_t) ret_val);
    }
}

static void nfc_text_data_set(void)
{
    nfc_ndef_msg_desc_t *    p_nfc_text_message;
    nfc_ndef_record_desc_t * p_text_rec;
    NfcRetval                ret_val;
    uint32_t                 err_code = NRF_SUCCESS;

    /* Declare a text message to inform user that device is paired */
    NFC_NDEF_MSG_DEF(text_msg, 1);
    NFC_NDEF_TEXT_RECORD_DESC_DEF(text_rec,
                                  UTF_8,
                                  m_language,
                                  sizeof(m_language),
                                  m_nfc_text,
                                  sizeof(m_nfc_text));

    p_nfc_text_message = &NFC_NDEF_MSG(text_msg);
    p_text_rec         = &NFC_NDEF_TEXT_RECORD_DESC(text_rec);
    err_code           = nfc_ndef_msg_record_add(p_nfc_text_message, p_text_rec);
    APP_ERROR_CHECK(err_code);

    uint32_t ndef_msg_len = sizeof(ndef_msg_buf);
    err_code              = nfc_ndef_msg_encode(p_nfc_text_message, ndef_msg_buf, &ndef_msg_len);
    APP_ERROR_CHECK(err_code);

    ret_val = nfcSetPayload((char *)ndef_msg_buf, ndef_msg_len);
    if (ret_val != NFC_RETVAL_OK)
    {
        APP_ERROR_CHECK((uint32_t) ret_val);
    }
}

static void nfc_callback(void * context, NfcEvent event, const char *data, size_t dataLength)
{
    (void) context;
    uint32_t err_code;

    switch (event)
    {
        case NFC_EVENT_FIELD_ON:
        {
            NRF_LOG_PRINTF_DEBUG("NFC_EVENT_FIELD_ON");
            if (m_caps_on)
            {
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_2);
            }
            else
            {
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_3);
            }
            APP_ERROR_CHECK(err_code);

            /* Start advertising to become connectable. */
            if (!m_advertising_flag)
            {
                m_advertising_flag  = 1;
                advertising_enable();
                err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
                APP_ERROR_CHECK(err_code);
            }
        }break;//NFC_EVENT_FIELD_ON

        case NFC_EVENT_FIELD_OFF:
        {
            if (m_caps_on)
            {
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_3);
            }
            else
            {
                err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
            }

            APP_ERROR_CHECK(err_code);
        }break;//NFC_EVENT_FIELD_OFF

        default:
            break;
    }
    return;
}


static void nfc_init(bool erase_bonds, bool wake_on_button)
{
    NfcRetval     ret_val;
    uint32_t      err_code;

    /* Start NFC */
    ret_val = nfcSetup(nfc_callback, NULL);
    if (ret_val != NFC_RETVAL_OK)
    {
        APP_ERROR_CHECK((uint32_t) ret_val);
    }

    /* Get the bonded peer number from peer_manager */
    if (!pm_peer_count() || erase_bonds)
    {
        nfc_pairing_data_set();
    }
    else
    {
        nfc_text_data_set();

        /* Bond with peer exists, so advertising should be started when wake up from button */
        if (wake_on_button)
        {
            m_advertising_flag  = 1;
            advertising_enable();
            err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
        }
    }

    ret_val = nfcStartEmulation();
    if (ret_val != NFC_RETVAL_OK)
    {
        APP_ERROR_CHECK((uint32_t) ret_val);
    }

    return;
}


/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    /* Begin: Bugfix for FTPAN-45 */
    if (!NFC_NEED_MCU_RUN_STATE())
    {
    /* End:   Bugfix for FTPAN-45 */
        uint32_t err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    /* Begin: Bugfix for FTPAN-45 */
    }
    /* End:   Bugfix for FTPAN-45 */
}


/***************************************************************************************************
 * End of modifications needed for BLE pairing over NFC
 **************************************************************************************************/

/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    bool     erase_bonds, wake_on_button;

    // Initialize.
    err_code = NRF_LOG_INIT();
    APP_ERROR_CHECK(err_code);
    timers_init();
    buttons_leds_init(&erase_bonds, &wake_on_button);
    ble_stack_init();
    scheduler_init();
    ble_conn_state_init();
    peer_manager_init(erase_bonds);
    gap_params_init();
    advertising_init();
    services_init();
    sensor_simulator_init();
    conn_params_init();
    buffer_init();
    nfc_init(erase_bonds, wake_on_button);
    

    // Start execution.
    timers_start();

    // Enter main loop.
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}

/**
 * @}
 */

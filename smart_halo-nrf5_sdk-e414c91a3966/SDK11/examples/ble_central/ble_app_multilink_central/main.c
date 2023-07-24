/*
 * Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

/**
 * @brief BLE LED Button Service central and client application main file.
 *
 * This example can be a central for up to 8 peripherals.
 * The peripheral is called ble_app_blinky and can be found in the ble_peripheral
 * folder.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_trace.h"
#include "boards.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "ble.h"
#include "app_uart.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "ble_lbs_c.h"
#include "ble_conn_state.h"
#include "nrf_log.h"

#define CENTRAL_LINK_COUNT        8                                          /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT     0                                          /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#define TOTAL_LINK_COUNT          CENTRAL_LINK_COUNT + PERIPHERAL_LINK_COUNT /**< Total number of links used by the application. */
#define APPL_LOG                  app_trace_log                              /**< Macro used to log debug information over UART. */

#define CENTRAL_SCANNING_LED      BSP_LED_0_MASK
#define CENTRAL_CONNECTED_LED     BSP_LED_1_MASK

#define APP_TIMER_PRESCALER       0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS      (2+BSP_APP_TIMERS_NUMBER)                  /**< Maximum number of timers used by the application. */
#define APP_TIMER_OP_QUEUE_SIZE   2                                          /**< Size of timer operation queues. */

#define SCAN_INTERVAL             0x00A0                                     /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW               0x0050                                     /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_TIMEOUT              0x0000                                     /**< Timout when scanning. 0x0000 disables timeout. */
#define SCAN_REQUEST              0                                          /**< Active scannin is not set. */
#define SCAN_WHITELIST_ONLY       0                                          /**< We will not ignore unknown devices. */
                                                                             
#define MIN_CONNECTION_INTERVAL   MSEC_TO_UNITS(7.5, UNIT_1_25_MS)           /**< Determines minimum connection interval in milliseconds. */
#define MAX_CONNECTION_INTERVAL   MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Determines maximum connection interval in milliseconds. */
#define SLAVE_LATENCY             0                                          /**< Determines slave latency in terms of connection events. */
#define SUPERVISION_TIMEOUT       MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Determines supervision time-out in units of 10 milliseconds. */

#define UUID16_SIZE               2                                          /**< Size of a UUID, in bytes. */

#define LEDBUTTON_LED             BSP_LED_2_MASK                             /**< LED to indicate a change of state of the the Button characteristic on the peer. */

#define LEDBUTTON_BUTTON_PIN      BSP_BUTTON_0                               /**< Button that will write to the LED characteristic of the peer */
#define BUTTON_DETECTION_DELAY    APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

static const char m_target_periph_name[] = "Nordic_Blinky";                  /**< Name of the device we try to connect to. This name is searched for in the scan report data*/


/** @brief Scan parameters requested for scanning and connection. */
static const ble_gap_scan_params_t m_scan_param =
{
    SCAN_REQUEST,
    SCAN_WHITELIST_ONLY,
    NULL,
    (uint16_t)SCAN_INTERVAL,
    (uint16_t)SCAN_WINDOW,
    SCAN_TIMEOUT
};

/**@brief Connection parameters requested for connection. */
static const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,
    (uint16_t)MAX_CONNECTION_INTERVAL,
    (uint16_t)SLAVE_LATENCY,
    (uint16_t)SUPERVISION_TIMEOUT
};

static ble_lbs_c_t        m_ble_lbs_c[TOTAL_LINK_COUNT];           /**< Main structures used by the LED Button client module. */
static uint8_t            m_ble_lbs_c_count;                       /**< Keeps track of how many instances of LED Button client module have been initialized. >*/
static ble_db_discovery_t m_ble_db_discovery[TOTAL_LINK_COUNT];    /**< list of DB structures used by the database discovery module. */

/**@brief Function to handle asserts in the SoftDevice.
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


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    LEDS_CONFIGURE(CENTRAL_SCANNING_LED | CENTRAL_CONNECTED_LED | LEDBUTTON_LED);
    LEDS_OFF(CENTRAL_SCANNING_LED | CENTRAL_CONNECTED_LED | LEDBUTTON_LED);
}


/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  type       Type of data to be looked for in advertisement data.
 * @param[in]  p_advdata  Advertisement report length and pointer to report.
 * @param[out] p_typedata If data type requested is found in the data report, type data length and
 *                        pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, uint8_array_t * p_advdata, uint8_array_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->size)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data = &p_data[index + 2];
            p_typedata->size   = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}


/**@brief Function to start scanning.
 */
static void scan_start(void)
{
    ret_code_t err_code;

    err_code = sd_ble_gap_scan_stop();
    // It is okay to ignore this error since we are stopping the scan anyway.
    if (err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }

    NRF_LOG_PRINTF("[APP]: start scanning for device name %s\r\n", m_target_periph_name);
    err_code = sd_ble_gap_scan_start(&m_scan_param);
    APP_ERROR_CHECK(err_code);

}


/**@brief Handles events coming from the LED Button central module.
 *
 * @param[in] p_lbs_c     The instance of LBS_C that triggered the event.
 * @param[in] p_lbs_c_evt The LBS_C event.
 */
static void lbs_c_evt_handler(ble_lbs_c_t * p_lbs_c, ble_lbs_c_evt_t * p_lbs_c_evt)
{
    const uint16_t conn_handle = p_lbs_c_evt->conn_handle;
    switch (p_lbs_c_evt->evt_type)
    {
        case BLE_LBS_C_EVT_DISCOVERY_COMPLETE:
        {
            ret_code_t err_code;

            NRF_LOG_PRINTF("[APP]: LED Button service discovered on conn_handle 0x%x\r\n", 
                            conn_handle);
            
            err_code = app_button_enable();
            APP_ERROR_CHECK(err_code);

            // LED Button service discovered. Enable notification of Button.
            err_code = ble_lbs_c_button_notif_enable(p_lbs_c);
            APP_ERROR_CHECK(err_code);
        } break; // BLE_LBS_C_EVT_DISCOVERY_COMPLETE

        case BLE_LBS_C_EVT_BUTTON_NOTIFICATION:
        {
            NRF_LOG_PRINTF("[APP]: Link 0x%x, Button state changed on peer to 0x%x\r\n", 
                           conn_handle,
                           p_lbs_c_evt->params.button.button_state);
            if (p_lbs_c_evt->params.button.button_state)
            {
                LEDS_ON(LEDBUTTON_LED);
            }
            else
            {
                LEDS_OFF(LEDBUTTON_LED);
            }
        } break; // BLE_LBS_C_EVT_BUTTON_NOTIFICATION

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the advertising report BLE event.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_adv_report(const ble_evt_t * const p_ble_evt)
{
    uint32_t      err_code;
    uint8_array_t adv_data;
    uint8_array_t dev_name;
    bool          do_connect = false;

    // For readibility.
    const ble_gap_evt_t * const p_gap_evt    = &p_ble_evt->evt.gap_evt;
    const ble_gap_addr_t  * const peer_addr  = &p_gap_evt->params.adv_report.peer_addr;

    // Initialize advertisement report for parsing
    adv_data.p_data = (uint8_t *)p_gap_evt->params.adv_report.data;
    adv_data.size   = p_gap_evt->params.adv_report.dlen;


    //search for advertising names
    bool found_name = false;
    err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                &adv_data,
                                &dev_name);
    if (err_code != NRF_SUCCESS)
    {
        // Look for the short local name if it was not found as complete
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, &adv_data, &dev_name);
        if (err_code != NRF_SUCCESS)
        {
            // If we can't parse the data, then exit
            return;
        }
        else
        {
            found_name = true;
        }
    }
    else
    {
        found_name = true;
    }
    if (found_name)
    {
        if (strlen(m_target_periph_name) != 0)
        {
            if(memcmp(m_target_periph_name, dev_name.p_data, dev_name.size) == 0)
            {
                do_connect = true;
            }
        }
    }

    if (do_connect)
    {
        // Initiate connection.
        err_code = sd_ble_gap_connect(peer_addr, &m_scan_param, &m_connection_param);
        if (err_code != NRF_SUCCESS)
        {
            APPL_LOG("[APPL]: Connection Request Failed, reason %d\r\n", err_code);
        }
    }
}

/**@brief Function for handling BLE Stack events concerning central applications.
 *
 * @details This function keeps the connection handles of central applications up-to-date. It
 *          parses scanning reports, initiating a connection attempt to peripherals when a
 *          target UUID is found, and manages connection parameter update requests. Additionally,
 *          it updates the status of LEDs used to report central applications activity.
 *
 * @note Since this function updates connection handles, @ref BLE_GAP_EVT_DISCONNECTED events
 *       should be dispatched to the target application before invoking this function.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(const ble_evt_t * const p_ble_evt)
{
    // For readability.
    const ble_gap_evt_t * const p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        // Upon connection, check which peripheral has connected, initiate DB
        // discovery, update LEDs status and resume scanning if necessary.
        case BLE_GAP_EVT_CONNECTED:
        {
            uint32_t err_code;

            NRF_LOG_PRINTF("[APP]: link 0x%x established, start discovery on it\r\n", 
                           p_gap_evt->conn_handle);
            APP_ERROR_CHECK_BOOL(p_gap_evt->conn_handle < TOTAL_LINK_COUNT);
           
            err_code = ble_lbs_c_handles_assign(&m_ble_lbs_c[p_gap_evt->conn_handle], 
                                                p_gap_evt->conn_handle, 
                                                NULL);
            APP_ERROR_CHECK(err_code);

            err_code = ble_db_discovery_start(&m_ble_db_discovery[p_gap_evt->conn_handle],
                                              p_gap_evt->conn_handle);
            if (err_code != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(err_code);
            }

            // Update LEDs status, and check if we should be looking for more
            // peripherals to connect to.
            LEDS_ON(CENTRAL_CONNECTED_LED);
            if (ble_conn_state_n_centrals() == CENTRAL_LINK_COUNT)
            {
                LEDS_OFF(CENTRAL_SCANNING_LED);
            }
            else
            {
                // Resume scanning.
                LEDS_ON(CENTRAL_SCANNING_LED);
                scan_start();
            }
        } break; // BLE_GAP_EVT_CONNECTED

        // Upon disconnection, reset the connection handle of the peer which disconnected, update
        // the LEDs status and start scanning again.
        case BLE_GAP_EVT_DISCONNECTED:
        {
            uint32_t central_link_cnt; // Number of central links.

            NRF_LOG_PRINTF("LBS central link 0x%x disconnected (reason: %d)\r\n",
                           p_gap_evt->conn_handle,
                           p_gap_evt->params.disconnected.reason);

            uint32_t err_code = app_button_disable();
            APP_ERROR_CHECK(err_code);

            // Start scanning
            scan_start();

            // Update LEDs status.
            LEDS_ON(CENTRAL_SCANNING_LED);
            central_link_cnt = ble_conn_state_n_centrals();
            if (central_link_cnt == 0)
            {
                LEDS_OFF(CENTRAL_CONNECTED_LED);
            }
        } break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_ADV_REPORT:
            on_adv_report(p_ble_evt);
        break; // BLE_GAP_ADV_REPORT

        case BLE_GAP_EVT_TIMEOUT:
        {
            // We have not specified a timeout for scanning, so only connection attemps can timeout.
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                APPL_LOG("[APPL]: Connection Request timed out.\r\n");
            }
        } break; // BLE_GAP_EVT_TIMEOUT

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        {
            // Accept parameters requested by peer.
            ret_code_t err_code;
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                        &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
        } break; // BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 * been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    uint16_t conn_handle;
    conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_conn_state_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);

    // Make sure taht an invalid connection handle are not passed since
    // our array of modules is bound to TOTAL_LINK_COUNT.
    if (conn_handle < TOTAL_LINK_COUNT)
    {
        ble_db_discovery_on_ble_evt(&m_ble_db_discovery[conn_handle], p_ble_evt);
        ble_lbs_c_on_ble_evt(&m_ble_lbs_c[conn_handle], p_ble_evt);
    }
}


/**@brief LED Button collector initialization.
 */
static void lbs_c_init(void)
{
    uint32_t         err_code;
    ble_lbs_c_init_t lbs_c_init_obj;

    lbs_c_init_obj.evt_handler = lbs_c_evt_handler;

    for(m_ble_lbs_c_count = 0; m_ble_lbs_c_count < TOTAL_LINK_COUNT; m_ble_lbs_c_count++)
    {
        err_code = ble_lbs_c_init(&m_ble_lbs_c[m_ble_lbs_c_count], &lbs_c_init_obj);
        APP_ERROR_CHECK(err_code);
    }
    m_ble_lbs_c_count = 0;
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupts.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;
    
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Stack checks first if there are still entries in the table before checking if a vendor
    // specific UUID is already in the table thus to be able to call sd_ble_uuid_vs_add several
    // times with the same entry, vs_uuid_count has to be 1 bigger than what is actually needed.
    ble_enable_params.common_enable_params.vs_uuid_count = 2;
    
    // Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function to write to the LED characterestic of all connected clients.
 *
 * @details Based on if the button is pressed or released, we write a high or low LED status to
 *          the server.
 *
 * @param[in] button_action The button action (press/release).
 *            Determines if the LEDs of the servers will be ON or OFF.
 *
 * @return NRF_SUCCESS on success, else the error code from ble_lbs_led_status_send.
 */
static uint32_t led_status_send_to_all(uint8_t button_action)
{
    uint32_t err_code;

    for (uint32_t i = 0; i< CENTRAL_LINK_COUNT; i++)
    {
        err_code = ble_lbs_led_status_send(&m_ble_lbs_c[i], button_action);
        if (err_code != NRF_SUCCESS &&
            err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
            err_code != NRF_ERROR_INVALID_STATE)
        {
            return err_code;
        }
    }
        return NRF_SUCCESS;
}


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    uint32_t err_code;

    switch (pin_no)
    {
        case LEDBUTTON_BUTTON_PIN:
            err_code = led_status_send_to_all(button_action);
            if (err_code == NRF_SUCCESS)
            {
                NRF_LOG_PRINTF("LBS write LED state %d\r\n", button_action);
            }
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}


/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    uint32_t err_code;

   //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {LEDBUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler}
    };

    err_code = app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    NRF_LOG_PRINTF("[APP]: call to ble_lbs_on_db_disc_evt for instance %d and link 0x%x!\r\n", 
                    p_evt->conn_handle,
                    p_evt->conn_handle);
    ble_lbs_on_db_disc_evt(&m_ble_lbs_c[p_evt->conn_handle], p_evt);
}


/** @brief Database discovery initialization.
 */
static void db_discovery_init(void)
{
    ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}



/** @brief Function to sleep until a BLE event is received by the application.
 */
static void power_manage(void)
{
    ret_code_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


int main(void)
{
    ret_code_t err_code;

    err_code = NRF_LOG_INIT();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_PRINTF("[APP]: Multilink Example\r\n");
    leds_init();
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);
    buttons_init();
    ble_stack_init();

    db_discovery_init();
    lbs_c_init();

    // Start scanning for peripherals and initiate connection to devices which
    // advertise.
    scan_start();

    // Turn on the LED to signal scanning.
    LEDS_ON(CENTRAL_SCANNING_LED);

    for (;;)
    {
        // Wait for BLE events.
        power_manage();
    }
}

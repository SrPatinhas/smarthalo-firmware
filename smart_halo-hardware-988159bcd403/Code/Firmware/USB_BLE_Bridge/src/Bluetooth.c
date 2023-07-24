/*
 * Bluetooth.c
 *
 *  Created on: 2016-06-27
 *      Author: Seb
 */

#include "Bluetooth.h"
#include "CommandLineInterface.h"
#include "UART.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "app_trace.h"
#include "ble_db_discovery.h"
#include "app_timer.h"
#include "app_trace.h"
#include "app_util.h"
#include "app_error.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "boards.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_hci.h"
#include "softdevice_handler.h"
#include "ble_advdata.h"
#include "ble_nus_c.h"

#define BLE_NEWLINE_SEPARATOR		"\\n"

#define CENTRAL_LINK_COUNT      1                               /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT   0                               /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN      /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_TIMER_PRESCALER     0                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 2                               /**< Size of timer operation queues. */

#define APPL_LOG                app_trace_log                   /**< Debug logger macro that will be used in this file to do logging of debug information over UART. */

#define SCAN_INTERVAL           0x00A0                          /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW             0x0050                          /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_ACTIVE             1                               /**< If 1, performe active scanning (scan requests). */
#define SCAN_SELECTIVE          0                               /**< If 1, ignore unknown devices (non whitelisted). */
#define SCAN_TIMEOUT            0x0000                          /**< Timout when scanning. 0x0000 disables timeout. */

#define MIN_CONNECTION_INTERVAL MSEC_TO_UNITS(20, UNIT_1_25_MS) /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL MSEC_TO_UNITS(75, UNIT_1_25_MS) /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY           0                               /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT     MSEC_TO_UNITS(4000, UNIT_10_MS) /**< Determines supervision time-out in units of 10 millisecond. */

#define UUID16_SIZE             2                               /**< Size of 16 bit UUID */
#define UUID32_SIZE             4                               /**< Size of 32 bit UUID */
#define UUID128_SIZE            16                              /**< Size of 128 bit UUID */
#define BLE_TX_BUF_SIZE			1024
#define BLE_RX_BUF_SIZE			1024
#define BLE_TX_BUF_ALMOST_FULL_COUNT   BLE_NUS_MAX_DATA_LEN

static ble_nus_c_t              m_ble_nus_c;                    /**< Instance of NUS service. Must be passed to all NUS_C API calls. */
static ble_db_discovery_t       m_ble_db_discovery;             /**< Instance of database discovery module. Must be passed to all db_discovert API calls */

// If we have less then 20 characters in BLE TX buffer, we will force a send after this amount of time
#define BLE_TX_CHAR_TIMEOUT_MS	500

static bool						isScanning = false;
static bool						isConnected = false;
static bool						ble_tx_buffer_available = true;
static bool						ble_tx_buffer_full = false;
static bool						ble_tx_buffer_empty = false;
static bool						ble_tx_char_timer_started = false;
static uint32_t					ble_tx_char_timer_cnt;

static uint8_t ble_tx_buffer[BLE_TX_BUF_SIZE];
static int ble_tx_write_index = 0;
static int ble_tx_read_index = 0;

static int ble_wire_tx_cnt = 0;
static int ble_wire_rx_cnt = 0;

//static uint8_t	LastPacketSent[BLE_NUS_MAX_DATA_LEN] = {0,0};


//int new_tx_cnt=0;

static int8_t 							rssidBm = -100;
static TXPOWER 							txPowerdBm = POWER_0dBm;

APP_TIMER_DEF(ble_tx_timeout_id);

int Bluetooth_sendPacket();
int Bluetooth_wireModePacket(uint8_t * p_data, uint16_t length);
static void Bluetooth_buttons_leds_init(void);
static void Bluetooth_db_discovery_init(void);
static void Bluetooth_ble_stack_init(void);
static void Bluetooth_nus_c_init(void);


/*
 * THESE FUNCTIONS SHOULD BE KEPT PRIVATE AND INACESSIBLE FROM OUTSIDE
 * THE ARE ENCLOSED IN CRITICAL SECTION MARKERS TO AVOID BEING
 * INTERRUPTED. UNDEFINED BEHAVIOR MAY HAPPEN IF THIS IS REMOVED
 */
int Bluetooth_fetchBLETXBuffer(uint8_t * p_data, uint16_t *length);
void Bluetooth_confirmReadBLETXBuffer(int updated_read_index);
int Bluetooth_writeBLETXBuffer(uint8_t * p_data, uint16_t length);
int Bluetooth_getBLETXBufferAvailableChars();
bool Bluetooth_isBLETXBufferEmpty();
bool Bluetooth_isBLETXBufferFull();
void Bluetooth_clearBLETXBuffer();

/**
 * @brief Connection parameters requested for connection.
 */
static const ble_gap_conn_params_t m_connection_param =
  {
    (uint16_t)MIN_CONNECTION_INTERVAL,  // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,  // Maximum connection
    (uint16_t)SLAVE_LATENCY,            // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT       // Supervision time-out
  };

/**
 * @brief Parameters used when scanning.
 */
static const ble_gap_scan_params_t m_scan_params =
  {
    .active      = SCAN_ACTIVE,
    .selective   = SCAN_SELECTIVE,
    .p_whitelist = NULL,
    .interval    = SCAN_INTERVAL,
    .window      = SCAN_WINDOW,
    .timeout     = SCAN_TIMEOUT
  };

/**
 * @brief NUS uuid
 */
static const ble_uuid_t m_nus_uuid =
  {
    .uuid = BLE_UUID_NUS_SERVICE,
    .type = NUS_SERVICE_UUID_TYPE
  };

/*
 * No need to buffer anything here as the BLE link is the weak link
 * (not UART) and RX serves 2 purposes : printf information from remote UUT
 * or loopback data. Incoming data is not parsed so it doesn't need an extra
 * buffer layer.
 */
void Bluetooth_onNewData(uint8_t * p_data, uint16_t length)
{
	// BLE to USB wire is ALWAYS active
	int i = 0;
	while(i<length)
		UART_put_character(p_data[i++]);

	ble_wire_rx_cnt += length;
	i = 0;
}

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
void Bluetooth_assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

static void Bluetooth_ble_tx_buffer_char_timeout_handler(void * p_context)
{
	if(Bluetooth_getBLETXBufferAvailableChars())
		Bluetooth_sendPacket();
}

void Bluetooth_createTimer()
{
	ret_code_t err_code;

	err_code = app_timer_create(&ble_tx_timeout_id, APP_TIMER_MODE_REPEATED, Bluetooth_ble_tx_buffer_char_timeout_handler);
	APP_ERROR_CHECK(err_code);
}

void Bluetooth_resetTimerCnt()
{
	ret_code_t err_code;

	if(!ble_tx_char_timer_started)
	{
		err_code = app_timer_start(ble_tx_timeout_id, APP_TIMER_TICKS(BLE_TX_CHAR_TIMEOUT_MS, APP_TIMER_PRESCALER), NULL);
		APP_ERROR_CHECK(err_code);
		ble_tx_char_timer_started = true;
	}

	err_code = app_timer_cnt_get(&ble_tx_char_timer_cnt);
	APP_ERROR_CHECK(err_code);
}

bool Bluetooth_TXcharHasTimedOut()
{
	ret_code_t err_code;
	uint32_t new_cnt, diff_cnt;
	float time_elapsed_ms = 0;

	if(ble_tx_char_timer_started)
	{
		err_code = app_timer_cnt_get(&new_cnt);
		APP_ERROR_CHECK(err_code);

		err_code = app_timer_cnt_diff_compute(new_cnt, ble_tx_char_timer_cnt, &diff_cnt);
		APP_ERROR_CHECK(err_code);

		time_elapsed_ms = ((float)diff_cnt * 1000.0)/(float)APP_TIMER_CLOCK_FREQ;

		if(time_elapsed_ms >= 100)
		{
			Bluetooth_resetTimerCnt();
			return true;
		}
	}

	return false;
}

void Bluetooth_setup()
{
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);

	Bluetooth_buttons_leds_init();
	Bluetooth_db_discovery_init();
	Bluetooth_ble_stack_init();
	Bluetooth_nus_c_init();

	Bluetooth_createTimer();
}

/**@brief Function to start scanning.
 */
void Bluetooth_scan_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_scan_start(&m_scan_params);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
    APP_ERROR_CHECK(err_code);

    isScanning = true;
}

/**@brief Function to stop scanning.
 */
void Bluetooth_scan_stop(void)
{
    uint32_t err_code;

    isScanning = false;

    err_code = sd_ble_gap_scan_stop();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}

bool Bluetooth_isScanning()
{
	return isScanning;
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void Bluetooth_db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}





/**@brief Callback handling NUS Client events.
 *
 * @details This function is called to notify the application of NUS client events.
 *
 * @param[in]   p_ble_nus_c   NUS Client Handle. This identifies the NUS client
 * @param[in]   p_ble_nus_evt Pointer to the NUS Client event.
 */

/**@snippet [Handling events from the ble_nus_c module] */
static void Bluetooth_ble_nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, const ble_nus_c_evt_t * p_ble_nus_evt)
{
    uint32_t err_code;
    switch (p_ble_nus_evt->evt_type)
    {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
            err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_nus_c_rx_notif_enable(p_ble_nus_c);
            APP_ERROR_CHECK(err_code);
            CommandLineInterface_printLine("Starting Wire Mode");
            UART_startStopWireMode(true);

            ble_wire_tx_cnt = 0;
            ble_wire_rx_cnt = 0;

            isScanning = false;

            break;

        case BLE_NUS_C_EVT_NUS_RX_EVT:

            Bluetooth_onNewData(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);

            break;

        case BLE_NUS_C_EVT_DISCONNECTED:

        	// HANDLED IN BLE_GAP_EVT_DISCONNECTED EVENT

            break;
    }
}
/**@snippet [Handling events from the ble_nus_c module] */

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
//static void Bluetooth_sleep_mode_enter(void)
//{
//    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
//    APP_ERROR_CHECK(err_code);
//
//    // Prepare wakeup buttons.
//    err_code = bsp_btn_ble_sleep_mode_prepare();
//    APP_ERROR_CHECK(err_code);
//
//    // Go to system-off mode (this function will not return; wakeup will cause a reset).
//    err_code = sd_power_system_off();
//    APP_ERROR_CHECK(err_code);
//}

/**@brief Reads an advertising report and checks if a uuid is present in the service list.
 *
 * @details The function is able to search for 16-bit, 32-bit and 128-bit service uuids.
 *          To see the format of a advertisement packet, see
 *          https://www.bluetooth.org/Technical/AssignedNumbers/generic_access_profile.htm
 *
 * @param[in]   p_target_uuid The uuid to search fir
 * @param[in]   p_adv_report  Pointer to the advertisement report.
 *
 * @retval      true if the UUID is present in the advertisement report. Otherwise false
 */
static bool Bluetooth_is_uuid_present(const ble_uuid_t *p_target_uuid,
                            const ble_gap_evt_adv_report_t *p_adv_report)
{
    uint32_t err_code;
    uint32_t index = 0;
    uint8_t *p_data = (uint8_t *)p_adv_report->data;
    ble_uuid_t extracted_uuid;

    while (index < p_adv_report->dlen)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index+1];

        if ( (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE)
           || (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE)
           )
        {
            for (uint32_t u_index = 0; u_index < (field_length/UUID16_SIZE); u_index++)
            {
                err_code = sd_ble_uuid_decode(  UUID16_SIZE,
                                                &p_data[u_index * UUID16_SIZE + index + 2],
                                                &extracted_uuid);
                if (err_code == NRF_SUCCESS)
                {
                    if ((extracted_uuid.uuid == p_target_uuid->uuid)
                        && (extracted_uuid.type == p_target_uuid->type))
                    {
                        return true;
                    }
                }
            }
        }

        else if ( (field_type == BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_MORE_AVAILABLE)
                || (field_type == BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_COMPLETE)
                )
        {
            for (uint32_t u_index = 0; u_index < (field_length/UUID32_SIZE); u_index++)
            {
                err_code = sd_ble_uuid_decode(UUID16_SIZE,
                &p_data[u_index * UUID32_SIZE + index + 2],
                &extracted_uuid);
                if (err_code == NRF_SUCCESS)
                {
                    if ((extracted_uuid.uuid == p_target_uuid->uuid)
                        && (extracted_uuid.type == p_target_uuid->type))
                    {
                        return true;
                    }
                }
            }
        }

        else if ( (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE)
                || (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE)
                )
        {
            err_code = sd_ble_uuid_decode(UUID128_SIZE,
                                          &p_data[index + 2],
                                          &extracted_uuid);
            if (err_code == NRF_SUCCESS)
            {
                if ((extracted_uuid.uuid == p_target_uuid->uuid)
                    && (extracted_uuid.type == p_target_uuid->type))
                {
                    return true;
                }
            }
        }
        index += field_length + 1;
    }
    return false;
}

bool Bluetooth_isConnected()
{
	return isConnected;
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void Bluetooth_on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t              err_code;
    const ble_gap_evt_t * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
        {
            const ble_gap_evt_adv_report_t * p_adv_report = &p_gap_evt->params.adv_report;

            if (Bluetooth_is_uuid_present(&m_nus_uuid, p_adv_report))
            {

                err_code = sd_ble_gap_connect(&p_adv_report->peer_addr,
                                              &m_scan_params,
                                              &m_connection_param);

                if (err_code == NRF_SUCCESS)
                {
                    // scan is automatically stopped by the connect
                    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                    APP_ERROR_CHECK(err_code);
                    CommandLineInterface_printf("Connecting to target %02x%02x%02x%02x%02x%02x\r\n",
                             p_adv_report->peer_addr.addr[0],
                             p_adv_report->peer_addr.addr[1],
                             p_adv_report->peer_addr.addr[2],
                             p_adv_report->peer_addr.addr[3],
                             p_adv_report->peer_addr.addr[4],
                             p_adv_report->peer_addr.addr[5]
                             );
                }
            }
            break;
        }

        case BLE_GAP_EVT_CONNECTED:
        	CommandLineInterface_printLine("Bluetooth device connected");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            // start discovery of services. The NUS Client waits for a discovery result
            err_code = ble_db_discovery_start(&m_ble_db_discovery, p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);

            err_code = sd_ble_gap_rssi_start(p_ble_evt->evt.gap_evt.conn_handle,5,0); // Generate event if rssi changes by 5 dBm. Do not skip any sample
			APP_ERROR_CHECK(err_code);

            isConnected = true;
            break;

        case BLE_GAP_EVT_DISCONNECTED:

        	// On disconnect all TX buffered data is dropped
        	isConnected = false;
        	Bluetooth_clearBLETXBuffer();

			ble_tx_buffer_available = true;

			if(!UART_startStopWireMode(false))
			{
				CommandLineInterface_printLine("Bluetooth device disconnected (GAP)");
				CommandLineInterface_printLine("Stopped Wire Mode (GAP)");
			}
			//printf("\r\n true send count: %d \r\n", new_tx_cnt);

			break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
            {
            	CommandLineInterface_printLine("[APPL]: Scan timed out.");
                Bluetooth_scan_start();
            }
            else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
            	CommandLineInterface_printLine("[APPL]: Connection Request timed out.");
            }

            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_RSSI_CHANGED:
			rssidBm = p_ble_evt->evt.gap_evt.params.rssi_changed.rssi;
			break;

        case BLE_EVT_TX_COMPLETE: ;

        	static uint8_t flow_control_counter = 0;

        	// A transfer has completed which means a BLE TX buffer is available
        	ble_tx_buffer_available = true;

			if(Bluetooth_getBLETXBufferAvailableChars() > 0)
				Bluetooth_sendPacket();

			// Re-enable flow control if activated
			if(UART_isUARTFlowStopped() )
			{
				flow_control_counter++;
				if(flow_control_counter > 4)//let some packets through before reenabling flow control
				{
					flow_control_counter = 0;
					UART_reenableFlowAndProcessData();
				}
			}

			break;

        default:
            break;
    }
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *          been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void Bluetooth_ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
	Bluetooth_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_nus_c_on_ble_evt(&m_ble_nus_c,p_ble_evt);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void Bluetooth_ble_stack_init(void)
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
    err_code = softdevice_ble_evt_handler_set(Bluetooth_ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the NUS Client.
 */
static void Bluetooth_nus_c_init(void)
{
    uint32_t         err_code;
    ble_nus_c_init_t nus_c_init_t;

    nus_c_init_t.evt_handler = Bluetooth_ble_nus_c_evt_handler;

    err_code = ble_nus_c_init(&m_ble_nus_c, &nus_c_init_t);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in] event  Event generated by button press.
 */
void Bluetooth_bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
        	//Bluetooth_sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_ble_nus_c.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}

/**@brief Function for initializing buttons and leds.
 */
static void Bluetooth_buttons_leds_init(void)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
								 Bluetooth_bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);
}


/** @brief Function for initializing the Database Discovery Module.
 */
static void Bluetooth_db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init(Bluetooth_db_disc_handler);
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for the Power manager.
 */
void Bluetooth_power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();

    APP_ERROR_CHECK(err_code);
}

void Bluetooth_disconnect()
{
	ret_code_t err_code;

	err_code = sd_ble_gap_disconnect(m_ble_nus_c.conn_handle,
									  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
	if (err_code != NRF_ERROR_INVALID_STATE)
	{
		APP_ERROR_CHECK(err_code);
	}
}

int Bluetooth_wireModePacket(uint8_t * p_data, uint16_t length)
{
	if(Bluetooth_isBLETXBufferFull())
	{
		printf("BLUETOOTH TX BUFFER OVERFLOW");
		return 0;
	}

	ble_wire_tx_cnt += Bluetooth_writeBLETXBuffer(p_data, length);

	if(ble_tx_buffer_available)
	{
		return Bluetooth_sendPacket();
	}

	return 0;
}


int Bluetooth_sendPacket()
{
	uint32_t       err_code;
	uint8_t	tempBuff[BLE_NUS_MAX_DATA_LEN];
	memset(tempBuff,0,BLE_NUS_MAX_DATA_LEN);
	int updated_tx_read_index;

	uint16_t lengthSent = Bluetooth_getBLETXBufferAvailableChars();

	// Wait for a full packet to send (or timeout)
	if( (lengthSent >= BLE_NUS_MAX_DATA_LEN) || Bluetooth_TXcharHasTimedOut() )
	{
		lengthSent = MIN(lengthSent, BLE_NUS_MAX_DATA_LEN);

		// We need to fetch data and check if TX buffers are available
		updated_tx_read_index = Bluetooth_fetchBLETXBuffer(tempBuff, &lengthSent);



		err_code = ble_nus_c_string_send(&m_ble_nus_c, tempBuff, lengthSent);


		if(err_code == BLE_ERROR_NO_TX_PACKETS)
		{
			ble_tx_buffer_available = false;
			return 0;
		}
		else if (err_code != NRF_ERROR_INVALID_STATE)
		{
			APP_ERROR_CHECK(err_code);
		}

#ifdef DEBUG
		printf("R%d->%dW%d ",ble_tx_read_index,updated_tx_read_index,ble_tx_write_index);
#endif

		// Successful transfer: mark read bytes as read
		Bluetooth_confirmReadBLETXBuffer(updated_tx_read_index);

#ifdef DEBUG
		printf("Sent %d:",lengthSent);

		for(int i=0;i<lengthSent;i++)
		{
			printf("%02x",tempBuff[i]);
		}
		printf("\r\n");
#endif

		return lengthSent;
	}



	return 0;
}

int8_t Bluetooth_getRSSIdBm()
{
	if(isConnected)
		return rssidBm;
	else
	{
		CommandLineInterface_printLine("No Bluetooth device connected");
		return -100;
	}
}

TXPOWER Bluetooth_getTXPowerdBm()
{
	return txPowerdBm;
}

bool Bluetooth_isValidPower(int8_t txPowerdBm)
{
	return ((txPowerdBm == POWER_M40dBm) ||
			(txPowerdBm == POWER_M30dBm) ||
			(txPowerdBm == POWER_M20dBm) ||
			(txPowerdBm == POWER_M16dBm) ||
			(txPowerdBm == POWER_M12dBm) ||
			(txPowerdBm == POWER_M8dBm) ||
			(txPowerdBm == POWER_M4dBm) ||
			(txPowerdBm == POWER_0dBm) ||
			(txPowerdBm == POWER_4dBm));
}

void Bluetooth_setTXPowerdBm(TXPOWER powerdBm)
{
	ret_code_t err_code;

	if(powerdBm != txPowerdBm)
	{
		txPowerdBm = powerdBm;
		err_code = sd_ble_gap_tx_power_set(txPowerdBm);
		APP_ERROR_CHECK(err_code);
	}
}

int Bluetooth_getWireModeTXCnt()
{
	return ble_wire_tx_cnt;
}

int Bluetooth_getWireModeRXCnt()
{
	return ble_wire_rx_cnt;
}

/*
 * CRITICAL CODE SECTION FOR ALL READ/WRITE ACCESS TO CIRCULAR BUFFER.
 * MUST NOT BE INTERRUPTIBLE
 */

bool Bluetooth_isBLETXBufferFull()
{
	CRITICAL_REGION_ENTER();

	// Only way it is full in this case is if write = BLE_TX_BUF_SIZE-1 and read = 0
	if (ble_tx_write_index > ble_tx_read_index)
		ble_tx_buffer_full = ((ble_tx_write_index == (BLE_TX_BUF_SIZE-1)) && (ble_tx_read_index == 0));
	else
		ble_tx_buffer_full = (ble_tx_read_index == (ble_tx_write_index + 1));

	CRITICAL_REGION_EXIT();

	return ble_tx_buffer_full;
}

bool Bluetooth_isBLETXBufferAlmostFull()
{
	int spaceLeft = Bluetooth_getBLETXBufferSpaceLeft();

	if(spaceLeft < BLE_TX_BUF_ALMOST_FULL_COUNT)
		return true;
	else
		return false;
}

bool Bluetooth_isBLETXBufferEmpty()
{
	CRITICAL_REGION_ENTER();

	ble_tx_buffer_empty = (ble_tx_read_index == ble_tx_write_index);

	CRITICAL_REGION_EXIT();

	return ble_tx_buffer_empty;
}

int Bluetooth_getBLETXBufferAvailableChars()
{
	int availableChars;

	CRITICAL_REGION_ENTER();

	if (ble_tx_write_index >= ble_tx_read_index)
	{
		availableChars = ble_tx_write_index - ble_tx_read_index;
	}
	else
	{
		availableChars = BLE_TX_BUF_SIZE - ble_tx_read_index + ble_tx_write_index;
	}

	CRITICAL_REGION_EXIT();

	return availableChars;
}

// There is always a slot that cannot be used to respect write = read - 1 ==> Buffer full condition
int Bluetooth_getBLETXBufferSpaceLeft()
{
	return BLE_TX_BUF_SIZE - Bluetooth_getBLETXBufferAvailableChars()-1;
}

/*
 * The circular buffer implementation will manage the buffer overflows the following way:
 * All further write after a buffer full event will be discarded. All exceeding data will
 * be discarded to preserve buffer integrity
 */
int Bluetooth_writeBLETXBuffer(uint8_t * p_data, uint16_t length)
{
	uint16_t writeLength = length;
	int new_write_index;

	if(!Bluetooth_isBLETXBufferFull())
	{
		Bluetooth_resetTimerCnt();

		CRITICAL_REGION_ENTER();

		// Calculate new write index
		if(ble_tx_write_index + length >= BLE_TX_BUF_SIZE)
		{
			new_write_index = length - (BLE_TX_BUF_SIZE - ble_tx_write_index);
		}
		else
		{
			new_write_index = ble_tx_write_index + length;
		}

		// If buffer overflows: drop exceeding packets and print ERROR
		if( ((new_write_index < ble_tx_write_index) && (new_write_index >= ble_tx_read_index)) ||
			((new_write_index > ble_tx_write_index) && (new_write_index >= ble_tx_read_index) && (ble_tx_write_index < ble_tx_read_index)))
		{
			printf("CRITICAL: BLE TX OVERFLOW!");

			writeLength = length - (new_write_index - ble_tx_read_index);

			// Update new write index
			if(ble_tx_write_index + writeLength >= BLE_TX_BUF_SIZE)
			{
				new_write_index = writeLength - (BLE_TX_BUF_SIZE - ble_tx_write_index);
			}
			else
			{
				new_write_index = ble_tx_write_index + writeLength;
			}
		}

		// Write upper portion of data
		uint16_t currLength = 0;

		currLength = MIN(writeLength, (BLE_TX_BUF_SIZE-ble_tx_write_index));

		memcpy(ble_tx_buffer+ble_tx_write_index, p_data, currLength*sizeof(uint8_t));

		// If a rollover happen, write second portion of data
		if((writeLength - currLength) > 0)
		{
			memcpy(ble_tx_buffer, p_data+currLength, (writeLength - currLength)*sizeof(uint8_t));

#ifdef DEBUG
			printf("pdata(W%dCL%d):",writeLength,currLength);

			for(int i=0;i<writeLength;i++)
			{
				printf("%02x",p_data[i]);
			}
#endif
		}

		// Update write index
		ble_tx_write_index = new_write_index;

		CRITICAL_REGION_EXIT();

		return writeLength;
	}
	else
	{
		return 0;
	}


}

/*
 * This function will fill the supplied buffer pointer with all available characters to read
 * IMPORTANT: It is the responsibility of the caller to check available characters and
 * allocate sufficient memory for this purpose.
 */
int Bluetooth_fetchBLETXBuffer(uint8_t * p_data, uint16_t *length)
{
	ASSERT(p_data != NULL);

	uint16_t readLength;
	int new_read_index;

	if(!Bluetooth_isBLETXBufferEmpty())
	{
		CRITICAL_REGION_ENTER();

		// If caller wants to read less characters then available
		// or if caller wants to read more then available
		readLength = MIN(Bluetooth_getBLETXBufferAvailableChars(),*length);

		// Calculate new read index
		if(ble_tx_read_index + readLength >= BLE_TX_BUF_SIZE)
		{
			new_read_index = readLength - (BLE_TX_BUF_SIZE - ble_tx_read_index);
		}
		else
		{
			new_read_index = ble_tx_read_index + readLength;
		}

		// Make sure we are not trying to read more characters then what exists
		// THIS SHOULD NEVER HAPPEN UNLESS CRITICAL CODE WAS INTERRUPTED SOMEHOW
		if( ((new_read_index > ble_tx_write_index) && (new_read_index < ble_tx_read_index)) ||
			((new_read_index > ble_tx_write_index) && (ble_tx_write_index > ble_tx_read_index))	)
		{
			printf("CRITICAL: FETCHING MORE THEN AVI");

			readLength = readLength - (new_read_index - ble_tx_write_index);

			// Update new read index
			if(ble_tx_read_index + readLength >= BLE_TX_BUF_SIZE)
			{
				new_read_index = readLength - (BLE_TX_BUF_SIZE - ble_tx_read_index);
			}
			else
			{
				new_read_index = ble_tx_read_index + readLength;
			}
		}

		// Read upper portion of data
		uint16_t currLength = 0;

		currLength = MIN(readLength, (BLE_TX_BUF_SIZE-ble_tx_read_index));

		memcpy(p_data, ble_tx_buffer+ble_tx_read_index, currLength*sizeof(uint8_t));

		// If a rollover happen, read second portion of data
		if((readLength - currLength) > 0)
		{
			memcpy(p_data+currLength, ble_tx_buffer, (readLength - currLength)*sizeof(uint8_t));

#ifdef DEBUG
			printf("pdata(R%dCL%d):",readLength,currLength);

			for(int i=0;i<readLength;i++)
			{
				printf("%02x",p_data[i]);
			}
#endif
		}

		CRITICAL_REGION_EXIT();

		*length = readLength;

		return new_read_index;
	}
	else
	{
		*length = 0;

		return ble_tx_read_index;
	}
}

void Bluetooth_confirmReadBLETXBuffer(int updated_read_index)
{
	CRITICAL_REGION_ENTER();
	ble_tx_read_index = updated_read_index;
	CRITICAL_REGION_EXIT();
}

void Bluetooth_clearBLETXBuffer()
{
	CRITICAL_REGION_ENTER();

	// Reset pointers
	ble_tx_read_index = 0;
	ble_tx_write_index = 0;

	// Update flags
	Bluetooth_isBLETXBufferEmpty();
	Bluetooth_isBLETXBufferFull();

	CRITICAL_REGION_EXIT();
}

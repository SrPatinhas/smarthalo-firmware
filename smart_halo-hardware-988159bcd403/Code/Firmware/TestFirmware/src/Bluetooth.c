/*
 * Bluetooth.c
 *
 *  Created on: Jun 15, 2016
 *      Author: sgelinas
 */

#ifndef SRC_BLUETOOTH_C_
#define SRC_BLUETOOTH_C_

#include "SmartHalo.h"
#include "CommandLineInterface.h"
#include "UART.h"

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_gap.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "nrf_delay.h"
#include "Bluetooth.h"
#define BLE_NEWLINE_SEPARATOR		"\\n"
#define BLE_COMMAND_SEPARATOR		"\\r\\n"

#define TF_IS_SRVC_CHANGED_CHARACT_PRESENT 1                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define CENTRAL_LINK_COUNT              0                                           /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                           /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define TF_DEVICE_NAME                     BUILDTARGET "_UART"                         /**< Name of device. Will be included in the advertising data. */
#define TF_MANUFACTURER_NAME               "CycleLabs Solutions Inc."                  /**< Manufacturer. Will be passed to Device Information Service. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define TF_APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define TF_APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

#define TF_MIN_CONN_INTERVAL            MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define TF_MAX_CONN_INTERVAL            MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define BLE_TX_BUF_SIZE					1024
#define BLE_RX_BUF_SIZE					1024
#define BLE_TX_BUF_ALMOST_FULL_COUNT	BLE_NUS_MAX_DATA_LEN

static ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */

static ble_uuid_t                       m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};  /**< Universally unique service identifier. */

// If we have less then 20 characters in BLE TX buffer, we will force a send after this amount of time
#define BLE_TX_CHAR_TIMEOUT_MS	500

static int8_t 							rssidBm = -100;
static TXPOWER 							txPowerdBm = POWER_0dBm;
static bool								loopbackTest = false;
static bool								isConnected = false;
static bool								ble_tx_buffer_available = true;
static bool								disablePrintf = false;
static bool								ble_tx_buffer_full = false;
static bool								ble_tx_buffer_empty = false;
static bool								ble_rx_buffer_full = false;
static bool								ble_tx_char_timer_started = false;
static uint32_t							ble_tx_char_timer_cnt;

static int cnt = 0;
static uint8_t ble_rx_buffer[BLE_RX_BUF_SIZE];
static uint8_t ble_tx_buffer[BLE_TX_BUF_SIZE];
static int ble_tx_write_index = 0;
static int ble_tx_read_index = 0;

static int ble_loopback_tx_cnt = 0;
static int ble_loopback_rx_cnt = 0;

APP_TIMER_DEF(ble_tx_timeout_id);

int Bluetooth_sendPacket();
int Bluetooth_loopbackPacket(uint8_t * p_data, uint16_t length);

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

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void Bluetooth_assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void Bluetooth_gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) TF_DEVICE_NAME,
                                          strlen(TF_DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = TF_MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = TF_MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/*
 * All Bluetooth received data is buffered for 1 sole reason: to be able
 * to parse and interpret incoming data. Buffer overflow should never
 * happen on this end since BLE connection is the weak link (not UART).
 * Provision for this are still made to be safe.
 */
void Bluetooth_onNewData(uint8_t * p_data, uint16_t length)
{
	uint8_t c;
	uint16_t writelength = length;
	static uint8_t	LastPacketSent[BLE_NUS_MAX_DATA_LEN] = {0,0};

	if(cnt + length > BLE_RX_BUF_SIZE)
	{
		ble_rx_buffer_full = true;

		writelength = BLE_RX_BUF_SIZE - cnt;

		printf("BLE RX Buffer overflow");
	}
	
	// if the same data is received twice, do nothing
	if((memcmp(LastPacketSent, p_data, length) == 0)&&(length==BLE_NUS_MAX_DATA_LEN))
	{
		//memcpy(LastPacketSent, p_data, (length)*sizeof(uint8_t));
		return;
	}
	else
	{
		memcpy(LastPacketSent, p_data, (length)*sizeof(uint8_t));
	}
	
	
	for( int i=0; i<writelength; i++)
	{
		c = p_data[i];
		ble_rx_buffer[cnt++] = c;
	}

	if ((strncmp(((char *)ble_rx_buffer+(cnt-2)), BLE_NEWLINE_SEPARATOR, 2)==0) || ble_rx_buffer_full || Bluetooth_isLoopbackTestActive())
	{
		// Terminate with null character
		//ble_rx_buffer[cnt] = (uint8_t)'\0';

		if(Bluetooth_isLoopbackTestActive())
		{
			ble_loopback_rx_cnt += length;

			// Queue packet to be sent
			Bluetooth_loopbackPacket(ble_rx_buffer, cnt);
		}
		else
		{
			// Convert \\r\\n into \r\n and remove extra characters
			if(strncmp(((char *)(ble_rx_buffer+(cnt-4))), BLE_COMMAND_SEPARATOR, 4)==0)
			{
				ble_rx_buffer[cnt-4] = (uint8_t)'\r';
				ble_rx_buffer[cnt-3] = (uint8_t)'\n';
				ble_rx_buffer[cnt-2] = (uint8_t)'\0';
				ble_rx_buffer[cnt-1] = (uint8_t)'\0';
				cnt -= 2;
			}

			// Case-insensitive: convert to upper case
			for(int i=0; i<cnt; i++) ble_rx_buffer[i] = (uint8_t)toupper((int)ble_rx_buffer[i]);

			CommandLineInterface_parseAndExecuteCommand(INTERFACE_BLE, (char *)ble_rx_buffer, cnt);
		}

		// Command parsed (even if invalid): must reset count
		cnt = 0;
		ble_rx_buffer_full = false;
	}
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void Bluetooth_nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
#ifdef DEBUG
	// Show that we received something
	CommandLineInterface_printf("Received %d characters via Bluetooth\r\n", length);
	for(int i=0; i<length; i++)
		CommandLineInterface_printf("%c",p_data[i]);
	CommandLineInterface_printf("\r\n");
#endif

	if(!ble_rx_buffer_full)
		Bluetooth_onNewData(p_data, length);
}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void Bluetooth_services_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = Bluetooth_nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void Bluetooth_on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void Bluetooth_conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void Bluetooth_conn_params_init(void)
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
    cp_init.evt_handler                    = Bluetooth_on_conn_params_evt;
    cp_init.error_handler                  = Bluetooth_conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


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


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void Bluetooth_on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
        	//Bluetooth_sleep_mode_enter();
        	// We don't want to reset!
            break;
        default:
            break;
    }
}

bool Bluetooth_isConnected()
{
	return isConnected;
}


/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
static void Bluetooth_on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:

        	//CommandLineInterface_printLine("Bluetooth device connected");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

			err_code = sd_ble_gap_rssi_start(m_conn_handle,5,0); // Generate event if rssi changes by 5 dBm. Do not skip any sample
			APP_ERROR_CHECK(err_code);

			isConnected = true;

            break;

        case BLE_GAP_EVT_DISCONNECTED:

        	// On disconnect all TX buffered data is dropped
        	Bluetooth_clearBLETXBuffer();

        	// And all RX buffered data as well
        	ble_rx_buffer_full = false;
        	cnt = 0;

        	// If in loopback mode on ble, stop it
        	if(Bluetooth_isLoopbackTestActive())
        		Bluetooth_startStopLoopbackTest(false);

        	// Update flag
        	isConnected = false;

        	//CommandLineInterface_printLine("Bluetooth device disconnected");

            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;

            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_RSSI_CHANGED:
        	rssidBm = p_ble_evt->evt.gap_evt.params.rssi_changed.rssi;
			break;

        case BLE_EVT_TX_COMPLETE:

        	// A transfer has completed which means a BLE TX buffer is available
        	ble_tx_buffer_available = true;

        	if(Bluetooth_getBLETXBufferAvailableChars() > 0)
        		Bluetooth_sendPacket();

    		break;
		
		case BLE_GAP_EVT_TIMEOUT:
			err_code = ble_advertising_start(BLE_ADV_MODE_FAST);

            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a SoftDevice event to all modules with a SoftDevice
 *        event handler.
 *
 * @details This function is called from the SoftDevice event interrupt handler after a
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  SoftDevice event.
 */
static void Bluetooth_ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
    Bluetooth_on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);

}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void Bluetooth_ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize SoftDevice.
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

    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(Bluetooth_ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void Bluetooth_bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
        	//Bluetooth_sleep_mode_enter();
        	// Not sure what this is but we don't want a reset!
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}

/**@brief Function for initializing the Advertising functionality.
 */
static void Bluetooth_advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = TF_APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = TF_APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, Bluetooth_on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
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

/**@brief Application main function.
 */
void Bluetooth_setup()
{
    uint32_t err_code;

    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    Bluetooth_ble_stack_init();
    Bluetooth_gap_params_init();
    Bluetooth_services_init();
    Bluetooth_advertising_init();
    Bluetooth_conn_params_init();
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    Bluetooth_createTimer();
}

void Bluetooth_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         RADIO");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("getrssidbm:\t\t\tReturns the RSSI (dBm)");
	CommandLineInterface_printLine("gettxpowerdbm:\t\t\tReturns the transmitter output power (dBm)");
	CommandLineInterface_printLine("settxpowerdbm <value>:\t\tSets the transmitter output power (dBm)");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("startloopbacktest <interface>:\tEnables loopback on interface (BLE or UART)");
	CommandLineInterface_printLine("stoploopbacktest <interface>:\tDisables loopback on interface (BLE or UART)");
	CommandLineInterface_printLine("\\estop:\t\t\tDisables loopback on this interface");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("gettxcnt:\t\t\tReturns the number of characters sent in loopback mode");
    CommandLineInterface_printLine("getrxcnt:\t\t\tReturns the number of characters received in loopback mode");
    nrf_delay_ms(50);
}

bool Bluetooth_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	int value = 0;
	bool parsed = true;

	if(strncmp(RxBuff,"RESET",5)==0)
	{
		CommandLineInterface_printLine("Performing a soft reset...");
		UART_flush();
		nrf_delay_ms(200);
		NVIC_SystemReset();
	}
#ifdef SMARTHALO_EE
	else if(strncmp(RxBuff,"POWERRESET",10)==0)
	{
		CommandLineInterface_printLine("Performing a hard reset...");
		UART_flush();
		nrf_delay_ms(200);
		nrf_gpio_pin_write(MCU_POWER_CYCLE, 1);
	}
#endif
	else if(strncmp(RxBuff, "GETRSSIDBM", 10)==0)
	{
		value = Bluetooth_getRSSIdBm();
		CommandLineInterface_printf("RSSI = %d dBm\r\n", value);
	}
	else if(strncmp(RxBuff, "GETTXPOWERDBM", 13)==0)
	{
		value = Bluetooth_getTXPowerdBm();
		CommandLineInterface_printf("TX Power = %d dBm\r\n", value);
	}
	else if(sscanf(RxBuff, "SETTXPOWERDBM %d\r\n", &value)==1)
	{
		if(!Bluetooth_isValidPower(value))
			CommandLineInterface_printf("Invalid TX power: %d dBm. Valid values are -40, -30, -20, -16, -12, -8, -4, 0, 4 dBm\r\n", value);
		else
		{
			Bluetooth_setTXPowerdBm(value);
			CommandLineInterface_printf("TX Power = %d dBm\r\n", Bluetooth_getTXPowerdBm());
		}
	}
	else if(strncmp(RxBuff, "STARTLOOPBACKTEST BLE", 21)==0)
	{
		if(Bluetooth_isLoopbackTestActive())
			CommandLineInterface_printLine("Loopback already active on BLE interface");
		else
		{
			CommandLineInterface_printLine("Started loopback test on BLE interface");
			Bluetooth_startStopLoopbackTest(true);
		}
	}
	else if(strncmp(RxBuff, "STOPLOOPBACKTEST BLE", 20)==0)
	{
		if(!Bluetooth_isLoopbackTestActive())
			CommandLineInterface_printLine("Loopback is not active on BLE interface");
		else
		{
			if(!Bluetooth_startStopLoopbackTest(false))
				CommandLineInterface_printLine("Stopped loopback test on BLE interface");
		}
	}
	else if(strncmp(RxBuff, "GETTXCNT", 8)==0)
	{
		value = Bluetooth_getLoopbackModeTXCnt();
		CommandLineInterface_printf("TX: %d chars sent\r\n", value);
	}
		else if(strncmp(RxBuff, "GETRXCNT", 8)==0)
	{
		value = Bluetooth_getLoopbackModeRXCnt();
		CommandLineInterface_printf("RX: %d chars received\r\n", value);
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

/**
 * @}
 */

int8_t Bluetooth_getRSSIdBm()
{
	if(isConnected)
		return rssidBm;
	else
	{
		//CommandLineInterface_printLine("No Bluetooth device connected");
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

bool Bluetooth_startStopLoopbackTest(bool start)
{
	if(isConnected)
	{
		if(start != loopbackTest)
		{
			loopbackTest = start;
		}

		if(start)
		{
			ble_loopback_tx_cnt = 0;
			ble_loopback_rx_cnt = 0;
		}

		disablePrintf = start;

		return start;
	}
	else
	{
		//CommandLineInterface_printLine("No Bluetooth device connected");
		return false;
	}
}

bool Bluetooth_isLoopbackTestActive()
{
	return loopbackTest;
}

int Bluetooth_loopbackPacket(uint8_t * p_data, uint16_t length)
{
	if(Bluetooth_isBLETXBufferFull())
	{
		printf("BLUETOOTH TX BUFFER OVERFLOW");
		return 0;
	}

	ble_loopback_tx_cnt += Bluetooth_writeBLETXBuffer(p_data, length);

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
	int updated_tx_read_index;

	uint16_t lengthSent = Bluetooth_getBLETXBufferAvailableChars();

	// Wait for a full packet to send (or timeout)
	if( (lengthSent >= BLE_NUS_MAX_DATA_LEN) || Bluetooth_TXcharHasTimedOut() )
	{
		lengthSent = MIN(lengthSent, BLE_NUS_MAX_DATA_LEN);

		// We need to fetch data and check if TX buffers are available
		updated_tx_read_index = Bluetooth_fetchBLETXBuffer(tempBuff, &lengthSent);

		err_code = ble_nus_string_send(&m_nus, tempBuff, lengthSent);
		if(err_code == BLE_ERROR_NO_TX_PACKETS)
		{
			ble_tx_buffer_available = false;
			return 0;
		}
		else if (err_code != NRF_ERROR_INVALID_STATE)
		{
			APP_ERROR_CHECK(err_code);
		}

		// Successful transfer: mark read bytes as read
		Bluetooth_confirmReadBLETXBuffer(updated_tx_read_index);

		return lengthSent;
	}

	return 0;
}

int Bluetooth_printf(const char * fmt, va_list args)
{
	int length = 0;
	uint8_t tempBuff[BLE_TX_BUF_SIZE];

	if(isConnected && !disablePrintf)
	{
		length = vsprintf((char *)(tempBuff), fmt, args);

		if(Bluetooth_isBLETXBufferFull())
		{
			printf("BLUETOOTH TX BUFFER OVERFLOW");
			return 0;
		}

		Bluetooth_writeBLETXBuffer(tempBuff, length);

		if(ble_tx_buffer_available)
		{
			return Bluetooth_sendPacket();
		}
	}

	return 0;
}

int Bluetooth_getLoopbackModeTXCnt()
{
	return ble_loopback_tx_cnt;
}

int Bluetooth_getLoopbackModeRXCnt()
{
	return ble_loopback_rx_cnt;
}

/*
 * CRITICAL CODE SECTION FOR ALL READ/WRITE ACCESS TO CIRCULAR BUFFER.
 * MUST NOT BE INTERRUPTIBLE
 */

bool Bluetooth_isBLETXBufferFull()
{
	CRITICAL_REGION_ENTER();

	// Only way it is full in this case is if write = BLE_TX_BUF_SIZE and read = 0
	if (ble_tx_write_index > ble_tx_read_index)
		ble_tx_buffer_full = ((ble_tx_write_index == (BLE_TX_BUF_SIZE-1)) && (ble_tx_read_index == 0));
	else
		ble_tx_buffer_full = (ble_tx_read_index == (ble_tx_write_index + 1));

	CRITICAL_REGION_EXIT();

	return ble_tx_buffer_full;
}

bool Bluetooth_isBLETXBufferAlmostFull()
{
	int charsInBuffer = Bluetooth_getBLETXBufferAvailableChars();

	int spaceLeft = BLE_TX_BUF_SIZE - charsInBuffer;

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

/*
 * The circular buffer implementation will manage the buffer overflows the following way:
 * All further write after a buffe full event will be discarded. All exceeding data will
 * be discarded to preserve buffer integrity
 */
int Bluetooth_writeBLETXBuffer(uint8_t * p_data, uint16_t length)
{
	uint16_t writeLength = length;
	int new_write_index;

	if(!Bluetooth_isBLETXBufferFull())
	{
		// New characters arrived: reset timeout timer
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
 * IMPORTANT: YOU MUST CALL Bluetooth_confirmReadBLETXBuffer() with the returned value
 * to mark returned bytes as read
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


#endif

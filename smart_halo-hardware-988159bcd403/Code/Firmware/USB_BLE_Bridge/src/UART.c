/*
 * UART.c
 *
 *  Created on: 2016-06-27
 *      Author: Seb
 */

#include "UART.h"
#include "SmartHalo.h"
#include "CommandLineInterface.h"
#include "Bluetooth.h"

#include "nrf_error.h"

#include <string.h>
#include <ctype.h>

#define UART_TX_BUF_SIZE        1024                             /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        1024                             /**< UART RX buffer size. */

#define BLE_NEWLINE_SEPARATOR		"\\n"
#define UART_ESCAPE_CHAR			"\e"
#define UART_STOPWIRE_CMD			"\eSTOP\r\n"
#define UART_GETRSSI_CMD			"\egetrssidbm\r\n"

static bool wireMode = false;
static bool disablePrintf = false;

static int cnt = 0;
static char RxBuff[UART_RX_BUF_SIZE];
static bool uart_flow_stopped = false;
static bool last_uart_flow_stopped = false;

static void UART_event_handle(app_uart_evt_t * p_event);


/**@brief Function for initializing the UART.
 */
void UART_setup(void)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
      {
        .rx_pin_no    = UART_RXD_PIN,
        .tx_pin_no    = UART_TXD_PIN,
        .rts_pin_no   = UART_RTS_PIN,
        .cts_pin_no   = UART_CTS_PIN,
        .flow_control = APP_UART_FLOW_CONTROL_ENABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        UART_event_handle,
                        APP_IRQ_PRIORITY_LOW,
                        err_code);

    APP_ERROR_CHECK(err_code);
}

void UART_onNewData()
{
	uint8_t c;

	if(!UART_isUARTFlowStopped())
	{
		// If we re-enabled uart flow
		if(last_uart_flow_stopped)
		{
			last_uart_flow_stopped = false;
			//printf("UART_onNewData: new data after re-enabling UART flow\r\n");
		}

		// If buffer close to being full, must not read incoming characters (will trigger flow control)
		if(UART_isWireModeActive() && Bluetooth_isBLETXBufferAlmostFull())
		{
			//printf("Disabling UART flow\r\n");

			uart_flow_stopped = true;
			last_uart_flow_stopped = false;

			return;
		}

		// Never read more chars then BLE can handle
		while( (cnt < sizeof(RxBuff)-1) && (cnt < Bluetooth_getBLETXBufferSpaceLeft()) && (UART_get_character(&c) == NRF_SUCCESS) )
		{
			RxBuff[cnt++] = (char)c;
		}

		if ((c == NEWLINE_SEPARATOR) || (cnt == sizeof(RxBuff)-1) || UART_isWireModeActive())
		{
			// Terminate with null character
			//RxBuff[cnt] = '\0';

			if((cnt==7) && strncmp(RxBuff, UART_STOPWIRE_CMD,7)==0)
			{
				va_list empty_va_list;

				if(!UART_isWireModeActive())
					UART_printf("WireMode is not active\r\n", empty_va_list);
				else
				{
					UART_startStopWireMode(false);
					UART_printf("Stopped Wire Mode \r\n", empty_va_list);
				}
			}
			else if((cnt==13) && strncmp(RxBuff, UART_GETRSSI_CMD,13)==0)
			{
				int value = Bluetooth_getRSSIdBm();
				printf("RSSI = %d dBm\r\n", value);
			}
			else if(((strncmp(RxBuff, UART_ESCAPE_CHAR,1)!=0) || (c == NEWLINE_SEPARATOR) || (cnt >=13)) && UART_isWireModeActive())
			{
				// Send through BLE
				Bluetooth_wireModePacket((uint8_t *)RxBuff, cnt);
			}
			else if(!UART_isWireModeActive())
			{
				// Case-insensitive: convert to upper case
				for(int i=0; i<cnt; i++) RxBuff[i] = (char)toupper((int)RxBuff[i]);

				CommandLineInterface_parseAndExecuteCommand(RxBuff, cnt);
			}
			else
			{
				return;
			}

			// Command parsed (even if invalid): must reset count
			cnt = 0;
		}
	}
}

// This function should be called to re-enable UART flow and process queued data (in Wire Mode only)
void UART_reenableFlowAndProcessData()
{
	uint8_t c;

	if(UART_isUARTFlowStopped())// && !Bluetooth_isBLETXBufferAlmostFull())
	{
		// Get RX chars, but make sure not to get more then available space in RxBuff and BLE TX Buffer
		while((cnt < sizeof(RxBuff)-1) && (cnt < Bluetooth_getBLETXBufferSpaceLeft()) && (UART_get_character(&c) == NRF_SUCCESS))
		{
			RxBuff[cnt++] = (char)c;
		}

		//printf("%d chars processed upon re-enabling UART flow (%d space BLE)\r\n",cnt,Bluetooth_getBLETXBufferSpaceLeft());

		if((cnt==7) && strncmp(RxBuff, UART_STOPWIRE_CMD,7)==0)
		{
			va_list empty_va_list;

			if(!UART_isWireModeActive())
				UART_printf("WireMode is not active\r\n", empty_va_list);
			else
			{
				UART_startStopWireMode(false);
				UART_printf("Stopped Wire Mode \r\n", empty_va_list);
			}
		}
		else if((cnt==13) && strncmp(RxBuff, UART_GETRSSI_CMD,13)==0)
		{
			int value = Bluetooth_getRSSIdBm();
			printf("RSSI = %d dBm\r\n", value);
		}
		else if(((strncmp(RxBuff,UART_ESCAPE_CHAR,1)!=0) || (c == NEWLINE_SEPARATOR) || (cnt >=13)) && UART_isWireModeActive())
		{
			// Send through BLE
			Bluetooth_wireModePacket((uint8_t *)RxBuff, cnt);
		}
		else
		{
			return;
		}

		// Command parsed (even if invalid): must reset count
		cnt = 0;

		//printf("%d chars left after re-enable\r\n",(int)(app_uart_get_nb_rx_fifo_chars()));

		// Only re-enable if RX buffer is clear. Otherwise, we need to process the leftover characters on the
		// next BLE TX COMPLETE event (no RX event will be generated).
		if(app_uart_get_nb_rx_fifo_chars() == 0)
		{
			// Make sure to reset the flag for new UART data to be read and processed
			uart_flow_stopped = false;
			last_uart_flow_stopped = true;
		}
	}
}

static void UART_event_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
    	printf("UART COMMUNICATION ERROR\r\n");
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
    	// FIFO is full. Should not be a problem since we have flow control ON
        if(p_event->data.error_code == NRF_ERROR_NO_MEM)
        {
        	printf("UART FIFO is Full\r\n");
        }
        // Other unmanaged error code
        else
        {
        	printf("UART FIFO ERROR\r\n");
        	APP_ERROR_HANDLER(p_event->data.error_code);
        }
    }
    else if (p_event->evt_type == APP_UART_DATA_READY)
    {
    	UART_onNewData();
    }
    else if(p_event->evt_type == APP_UART_TX_EMPTY)
    {
    	return;
    }
    else if(p_event->evt_type == APP_UART_DATA)
	{
    	printf("UART DATA EVENT: Should not happen\r\n");
	}
    else
    {
    	printf("Unmanaged UART event %d\r\n",p_event->evt_type);
    }

}

bool UART_isUARTFlowStopped()
{
	return uart_flow_stopped;
}

uint32_t UART_get_character(uint8_t * c)
{
	return app_uart_get(c);
}

uint32_t UART_put_character(uint8_t c)
{
	return app_uart_put(c);
}

uint32_t UART_flush()
{
	return app_uart_flush();
}

bool UART_startStopWireMode(bool start)
{
	if(start != wireMode)
	{
		wireMode = start;
		disablePrintf = start;
	}

	// If disabling Wire Mode, we must disconnect as well
	if(Bluetooth_isConnected() && !start)
	{
		Bluetooth_disconnect();
	}

	return wireMode;
}

bool UART_isWireModeActive()
{
	return wireMode;
}

int UART_printf(const char * fmt, va_list args)
{
	if(!disablePrintf)
	{
		int length = vprintf(fmt, args);

		return length;
	}

	return 0;
}

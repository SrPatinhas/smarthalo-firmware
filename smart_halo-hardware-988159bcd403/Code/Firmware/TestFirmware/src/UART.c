/*
 * UART.c
 *
 *  Created on: Jun 15, 2016
 *      Author: sgelinas
 */

#include "UART.h"

#include "app_uart.h"
#include "app_error.h"

#include "SmartHalo.h"
#include "CommandLineInterface.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define UART_TX_BUF_SIZE 1024                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1024						 /**< UART RX buffer size. */

#define UART_ESCAPE_CHAR			"\e"
#define UART_STOPWIRE_CMD			"\eSTOP\r\n"

static bool loopbackTest = false;
static bool disablePrintf = false;

static int cnt = 0;
static char RxBuff[COMMAND_MAX_LENGTH];

static uint32_t UART_init();
void UART_error_handle(app_uart_evt_t * p_event);

void UART_setup()
{
	UART_init();
}

void UART_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         UART");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("startloopbacktest <interface>\tEnables loopback on interface (BLE or UART)");
	CommandLineInterface_printLine("stoploopbacktest <interface>\tDisables loopback on interface (BLE or UART)");
	CommandLineInterface_printLine("\\estop:\t\t\tDisables loopback on this interface");
}

bool UART_parseAndExecuteCommand(char * RxBuff, int cnt) 
{
	bool parsed = true;

	if(strncmp(RxBuff, "STARTLOOPBACKTEST UART", 22)==0)
	{
		if(UART_isLoopbackTestActive())
			CommandLineInterface_printLine("Loopback already active on UART interface");
		else
		{
			CommandLineInterface_printLine("Started loopback test on UART interface");
			UART_startStopLoopbackTest(true);
		}
	}
	else if(strncmp(RxBuff, "STOPLOOPBACKTEST UART", 21)==0)
	{
		if(!UART_isLoopbackTestActive())
			CommandLineInterface_printLine("Loopback is not active on UART interface");
		else
		{
			if(!UART_startStopLoopbackTest(false))
				CommandLineInterface_printLine("Stopped loopback test on UART interface");
		}
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

static uint32_t UART_init()
{
	uint32_t err_code;
	const app_uart_comm_params_t comm_params =
	{
		  UART_RXD_PIN,
		  UART_TXD_PIN,
		  UART_RTS_PIN,
		  UART_CTS_PIN,
		  APP_UART_FLOW_CONTROL_ENABLED,
		  false,
		  UART_BAUDRATE_BAUDRATE_Baud115200
	};

	APP_UART_FIFO_INIT(	&comm_params,
						 UART_RX_BUF_SIZE,
						 UART_TX_BUF_SIZE,
						 UART_error_handle,
						 APP_IRQ_PRIORITY_LOW,
						 err_code);

	APP_ERROR_CHECK(err_code);
	return err_code;
}

void UART_onNewData()
{
	uint8_t c;

	while( UART_get_character(&c) == NRF_SUCCESS )
	{
		RxBuff[cnt++] = (char)c;
	}

	if ((c == NEWLINE_SEPARATOR) || (cnt == sizeof(RxBuff)-1) || UART_isLoopbackTestActive())
	{
		// Terminate with null character
		RxBuff[cnt] = '\0';

		if((cnt==7) && strncmp(RxBuff,UART_STOPWIRE_CMD,7)==0)
		{
			va_list empty_va_list;

			if(!UART_isLoopbackTestActive())
			  UART_printf("Loopback is not active on UART interface\r\n", empty_va_list);
			else
			{
			  UART_printf("Stopped loopback test on UART interface\r\n", empty_va_list);
			  UART_startStopLoopbackTest(false);
			}
		}
		else if(((strncmp(RxBuff,UART_ESCAPE_CHAR,1)!=0) || (c == '\n') || (cnt >=7)) && UART_isLoopbackTestActive())
		{
			int i = 0;
			while(i<cnt)
				UART_put_character(RxBuff[i++]);
		}
		else if(!UART_isLoopbackTestActive())
		{
			// Case-insensitive: convert to upper case
			for(int i=0; i<cnt; i++) RxBuff[i] = (char)toupper((int)RxBuff[i]);

			CommandLineInterface_parseAndExecuteCommand(INTERFACE_UART, RxBuff, cnt);
		}
		else
		{
			return;
		}

		// Command parsed (even if invalid): must reset count
		cnt = 0;
	}
}

void UART_error_handle(app_uart_evt_t * p_event)
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

bool UART_startStopLoopbackTest(bool start)
{
	if(start != loopbackTest)
	{
		loopbackTest = start;
		disablePrintf = start;
	}

	return loopbackTest;
}

bool UART_isLoopbackTestActive()
{
	return loopbackTest;
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

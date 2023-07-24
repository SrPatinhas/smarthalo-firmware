/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Command Line Interface to parse input strings and call appropriate internal commands
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#include "CommandLineInterface.h"
#include "USB_BLE_Bridge.h"

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

// NORDIC SDK

#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"

// SMARTHALO
#include "SmartHalo.h"
#include "UART.h"
#include "Bluetooth.h"


void CommandLineInterface_printHeader();
int CommandLineInterface_printSeparator();
bool CommandLineInterface_parseAndExecuteGeneralCommand(char * RxBuff, int cnt);
void CommandLineInterface_printVersion();


void CommandLineInterface_setup()
{
	UART_setup();
	Bluetooth_setup();

	CommandLineInterface_printHeader();
	CommandLineInterface_printLine("Type (H)elp for a list of commands");
}

void CommandLineInterface_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	// Parse string
	if(CommandLineInterface_parseAndExecuteGeneralCommand(RxBuff, cnt)){}
	else
	{
	  CommandLineInterface_printLine("Invalid command. Type (H) for usage");
	}
}

void CommandLineInterface_printGeneralHelp()
{
  CommandLineInterface_printHeader();
  CommandLineInterface_printLine("reset:\t\t\tPerforms a soft reset");
  CommandLineInterface_printLine("version:\t\tReturns the current version");
  CommandLineInterface_printLine("scanstart:\t\tStarts scanning for BLE NUS devices");
  CommandLineInterface_printLine("scanstop:\t\tStops scanning for BLE NUS devices");
  CommandLineInterface_printLine("\egetrssidbm:\t\tReturns the RSSI (dBm)");
  CommandLineInterface_printLine("gettxpowerdbm:\t\tReturns the transmitter output power (dBm)");
  CommandLineInterface_printLine("settxpowerdbm <value>:\tSets the transmitter output power (dBm)");
  CommandLineInterface_printLine("gettxcnt:\t\tReturns the number of characters sent in wire mode");
  CommandLineInterface_printLine("getrxcnt:\t\tReturns the number of characters received in wire mode");
}

bool CommandLineInterface_parseAndExecuteGeneralCommand(char * RxBuff, int cnt)
{
	bool parsed = true;
	int value;

	if((cnt==3 && (RxBuff[0]=='H')) || (cnt==6 && (strncmp(RxBuff,"HELP",4)==0)) )
	{
		CommandLineInterface_printGeneralHelp();
	}
	else if(strncmp(RxBuff,"RESET",5)==0)
	{
		CommandLineInterface_printLine("Performing a soft reset...");
		UART_flush();
		nrf_delay_ms(200);
		NVIC_SystemReset();
	}
	else if((strncmp(RxBuff,"VERSION",7)==0))
	{
		CommandLineInterface_printVersion();
	}
	else if((strncmp(RxBuff,"SCANSTART",9)==0))
	{
		if(Bluetooth_isConnected())
			CommandLineInterface_printLine("Device already connected. Use \eSTOP\r\n to disconnect");
		else if(Bluetooth_isScanning())
		{
			CommandLineInterface_printLine("Device already scanning. Use scanstop to stop");
		}
		else
		{
			CommandLineInterface_printLine("Started scanning for BLE NUS Devices...");
			Bluetooth_scan_start();
		}
	}
	else if((strncmp(RxBuff,"SCANSTOP",8)==0))
	{
		if(Bluetooth_isConnected())
			CommandLineInterface_printLine("Device already connected. Use \eSTOP\r\n to disconnect");
		else if(!Bluetooth_isScanning())
		{
			CommandLineInterface_printLine("Device is not scanning. Use scanstart to start scanning");
		}
		else
		{
			CommandLineInterface_printLine("Stopped scanning for BLE NUS Devices...");
			Bluetooth_scan_stop();
		}
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
	else if(strncmp(RxBuff, "GETTXCNT", 8)==0)
	{
		value = Bluetooth_getWireModeTXCnt();
		CommandLineInterface_printf("TX: %d chars sent\r\n", value);
	}
	else if(strncmp(RxBuff, "GETRXCNT", 8)==0)
	{
		value = Bluetooth_getWireModeRXCnt();
		CommandLineInterface_printf("RX: %d chars received\r\n", value);
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

int CommandLineInterface_printLine(const char * line )
{
	return CommandLineInterface_printf("%s\r\n", line);
}
int CommandLineInterface_printSeparator()
{
	return CommandLineInterface_printLine("--------------------------------------------------------");
}

void CommandLineInterface_printHeader()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printf("  USB-BLE Bridge, v%s.%s%s, %s, %s\r\n", MAJORVERSION, MINORVERSION, BUILDTAG, __DATE__, __TIME__);
	CommandLineInterface_printf("  %s\r\n", BUILDTARGET);
	CommandLineInterface_printSeparator();
	nrf_delay_ms(50);
}

void CommandLineInterface_printVersion()
{
	CommandLineInterface_printf("  USB-BLE Bridge, v%s.%s%s, %s, %s\r\n", MAJORVERSION, MINORVERSION, BUILDTAG, __DATE__, __TIME__);
}

int CommandLineInterface_printf(const char * fmt, ... )
{
	int length;
	va_list args;
	va_start(args, fmt);

	length = UART_printf(fmt, args);

	va_end(args);

	return length;
}

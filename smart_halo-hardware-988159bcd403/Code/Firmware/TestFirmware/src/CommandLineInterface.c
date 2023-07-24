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
#include "TestFirmware.h"

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
#include "I2C.h"
#include "Bluetooth.h"
#include "LEDTest.h"
#include "SoundTest.h"
#include "TouchTest.h"
#include "Batmon.h"
#include "AcceleroTest.h"
#include "MagnetoTest.h"
#include "PowerSupply.h"

void CommandLineInterface_printHeader();
int CommandLineInterface_printSeparator();
bool CommandLineInterface_parseAndExecuteGeneralCommand(char * RxBuff, int cnt);
bool CommandLineInterface_checkAvailableModule(TESTMODULES module);
char * CommandLineInterface_getModuleStr(TESTMODULES module);
void CommandLineInterface_printModuleHelp(TESTMODULES module);
void CommandLineInterface_printVersion();

static CLI_Interface activeInterface = INTERFACE_UART;

void CommandLineInterface_setup()
{
	if(CommandLineInterface_checkAvailableModule(POWERSUPPLYMODULE))
		PowerSupply_setup();

	if(CommandLineInterface_checkAvailableModule(RADIOMODULE))
		Bluetooth_setup();

	if(CommandLineInterface_checkAvailableModule(UARTMODULE))
		UART_setup();

	if(CommandLineInterface_checkAvailableModule(I2CMODULE))
		I2C_setup(I2C_SCL_PIN, I2C_SDA_PIN);

	if(CommandLineInterface_checkAvailableModule(LEDMODULE))
		LEDTest_setup(I2C_SDB_PIN, FRONTLED_PIN, CENTRAL_RED_PIN, CENTRAL_GREEN_PIN, CENTRAL_BLUE_PIN);

#ifdef SOUNDTEST
	if(CommandLineInterface_checkAvailableModule(SOUNDMODULE))
		SoundTest_setup(PIEZO_DRIVE_PIN,PIEZO_VOLUME_PIN);
#endif

#ifdef TOUCHTEST
	if(CommandLineInterface_checkAvailableModule(TOUCHMODULE))
		TouchTest_setup(TOUCH_OUT_PIN, TOUCH_MODE_PIN);
#endif

#ifdef BATTEST
	if(CommandLineInterface_checkAvailableModule(BATMODULE))
		Batmon_setup(BATMON_ALARM_PIN, USB_CHARGING_N_PIN, USB_POWERGOOD_N_PIN);
#endif

#ifdef ACCTEST
	if(CommandLineInterface_checkAvailableModule(ACCELEROMODULE))
		AcceleroTest_setup();
#endif

#ifdef MAGTEST
	if(CommandLineInterface_checkAvailableModule(MAGNETOMODULE))
		MagnetoTest_setup();
#endif

	CommandLineInterface_printHeader();
	CommandLineInterface_printLine("Type (H)elp for a list of commands");
}

bool CommandLineInterface_checkAvailableModule(TESTMODULES module)
{
	switch(module)
		{
			case RADIOMODULE:
				#ifdef RADIOTEST
					return true;
				#else
					return false;
				#endif
				break;
			case I2CMODULE:
				#if defined(LEDTEST) || defined(BATTEST) || defined(ACCTEST) || defined(MAGTEST)
					return true;
				#else
					return false;
				#endif
				break;
			case UARTMODULE: // Always active
				return true;
				break;
			case LEDMODULE:
				#ifdef LEDTEST
					return true;
				#else
					return false;
				#endif
				break;
			case TOUCHMODULE:
				#ifdef TOUCHTEST
					return true;
				#else
					return false;
				#endif
				break;
			case SOUNDMODULE:
				#ifdef SOUNDTEST
					return true;
				#else
					return false;
				#endif
				break;
			case BATMODULE:
				#ifdef BATTEST
					return true;
				#else
					return false;
				#endif
				break;
			case ACCELEROMODULE:
				#ifdef ACCTEST
					return true;
				#else
					return false;
				#endif
				break;
			case MAGNETOMODULE:
				#ifdef MAGTEST
					return true;
				#else
					return false;
				#endif
				break;
			case POWERSUPPLYMODULE:
				return true;
				break;
			case INVALIDMODULE:
			default:
				return false;
				break;
		};
}

void CommandLineInterface_ListModules()
{
	int count = 0;

	CommandLineInterface_printLine("Available modules:");

	for(TESTMODULES module=RADIOMODULE; module<=POWERSUPPLYMODULE; module++)
	{
		if(strncmp(CommandLineInterface_getModuleStr(module),"",1)!=0)
		{
			CommandLineInterface_printLine(CommandLineInterface_getModuleStr(module));
			count++;
		}
	}

	if( count == 0 )
	{
		CommandLineInterface_printLine("No module enabled");
	}
}

char * CommandLineInterface_getModuleStr(TESTMODULES module)
{
	if(!CommandLineInterface_checkAvailableModule(module))
	{
		return "";
	}
	else
	{
		switch(module)
		{
			case RADIOMODULE:
				return "RADIO";
				break;
			case I2CMODULE:
				return "I2C";
				break;
			case UARTMODULE: // Always active
				return "UART";
				break;
			case LEDMODULE:
				return "LEDS";
				break;
			case TOUCHMODULE:
				return "TOUCH";
				break;
			case SOUNDMODULE:
				return "SOUND";
				break;
			case BATMODULE:
				return "BATTERY";
				break;
			case ACCELEROMODULE:
				return "ACCELERO";
				break;
			case MAGNETOMODULE:
				return "MAGNETO";
				break;
			case POWERSUPPLYMODULE:
				return "POWERSUPPLY";
				break;
			case INVALIDMODULE:
			default:
				return "";
				break;
		};
	}
}

TESTMODULES CommandLineInterface_getModuleFromStr(char * moduleStr, int length)
{
	if(strncmp(CommandLineInterface_getModuleStr(RADIOMODULE),moduleStr,length) == 0)
	{
		return RADIOMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(I2CMODULE),moduleStr,length) == 0)
	{
		return I2CMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(UARTMODULE),moduleStr,length) == 0)
	{
		return UARTMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(LEDMODULE),moduleStr,length) == 0)
	{
		return LEDMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(TOUCHMODULE),moduleStr,length) == 0)
	{
		return TOUCHMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(SOUNDMODULE),moduleStr,length) == 0)
	{
		return SOUNDMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(BATMODULE),moduleStr,length) == 0)
	{
		return BATMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(ACCELEROMODULE),moduleStr,length) == 0)
	{
		return ACCELEROMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(MAGNETOMODULE),moduleStr,length) == 0)
	{
		return MAGNETOMODULE;
	}
	else if(strncmp(CommandLineInterface_getModuleStr(POWERSUPPLYMODULE),moduleStr,length) == 0)
	{
		return POWERSUPPLYMODULE;
	}
	else
		return INVALIDMODULE;
}

void CommandLineInterface_parseAndExecuteCommand(CLI_Interface interface, char * RxBuff, int cnt)
{
	activeInterface = interface;

	// Parse string
	if(CommandLineInterface_parseAndExecuteGeneralCommand(RxBuff, cnt)){}
	else if(Bluetooth_parseAndExecuteCommand(RxBuff, cnt)){}
	else if(UART_parseAndExecuteCommand(RxBuff, cnt)){}
#ifdef LEDTEST
	else if(LEDTest_parseAndExecuteCommand(RxBuff, cnt)){}
#endif
#ifdef SOUNDTEST
	else if(SoundTest_parseAndExecuteCommand(RxBuff, cnt)){}
#endif
#ifdef TOUCHTEST
	else if(TouchTest_parseAndExecuteCommand(RxBuff, cnt)){}
#endif
#ifdef BATTEST
	else if(Batmon_parseAndExecuteCommand(RxBuff, cnt)){}
#endif
#if defined(LEDTEST) || defined(BATTEST) || defined(ACCTEST) || defined(MAGTEST)
	else if(I2C_parseAndExecuteCommand(RxBuff, cnt)){}
#endif
#ifdef ACCTEST
	else if(AcceleroTest_parseAndExecuteCommand(RxBuff, cnt)){}
#endif
#ifdef MAGTEST
	else if(MagnetoTest_parseAndExecuteCommand(RxBuff, cnt)){}
#endif
	else if(PowerSupply_parseAndExecuteCommand(RxBuff, cnt)){}
	else
	{
	  CommandLineInterface_printLine("Invalid command. Type (H) for usage");
	}
}

void CommandLineInterface_printGeneralHelp()
{
  CommandLineInterface_printHeader();
  CommandLineInterface_printLine("help <module>:\t\tReturns a list of commands for <module>");
  CommandLineInterface_printLine("modules:\t\tLists the available modules");
  CommandLineInterface_printLine("reset:\t\t\tPerform a soft reset");
  CommandLineInterface_printLine("powerreset:\t\tPerform a hard reset (if available)");
  CommandLineInterface_printLine("version:\t\tReturns the current version");
}

bool CommandLineInterface_parseAndExecuteGeneralCommand(char * RxBuff, int cnt)
{
	char RxBuff2[COMMAND_MAX_LENGTH];
	int length;
	bool parsed = true;

	if((cnt==3 && (RxBuff[0]=='H')) || (cnt==6 && (strncmp(RxBuff,"HELP",4)==0)) )
	{
		CommandLineInterface_printGeneralHelp();
	}
	else if(strncmp(RxBuff,"MODULES",7) == 0 )
	{
		CommandLineInterface_ListModules();
	}
	else if((sscanf(RxBuff,"HELP %s%n",RxBuff2,&length)==1))
	{
		TESTMODULES module = CommandLineInterface_getModuleFromStr(RxBuff2, length-5);

		CommandLineInterface_printModuleHelp(module);
	}
	else if((strncmp(RxBuff,"VERSION",7)==0))
	{
		CommandLineInterface_printVersion();
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void CommandLineInterface_printModuleHelp(TESTMODULES module)
{
	switch(module)
	{
		case RADIOMODULE:
			Bluetooth_printHelp();
			break;
		case I2CMODULE:
			I2C_printHelp();
			break;
		case UARTMODULE:
			UART_printHelp();
			break;
		case LEDMODULE:
			LEDTest_printHelp();
			break;
		case TOUCHMODULE:
			TouchTest_printHelp();
			break;
		case SOUNDMODULE:
			SoundTest_printHelp();
			break;
		case BATMODULE:
			Batmon_printHelp();
			break;
		case ACCELEROMODULE:
			AcceleroTest_printHelp();
			break;
		case MAGNETOMODULE:
			MagnetoTest_printHelp();
			break;
		case POWERSUPPLYMODULE:
			PowerSupply_printHelp();
			break;
		case INVALIDMODULE:
		default:
			CommandLineInterface_printLine("Invalid module. Type modules for a list of available modules");
			break;
	};
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
	CommandLineInterface_printf("  SmartHalo test program, v%s.%s%s, %s, %s\r\n", MAJORVERSION, MINORVERSION, BUILDTAG, __DATE__, __TIME__);
	CommandLineInterface_printf("  %s\r\n", BUILDTARGET);
	CommandLineInterface_printSeparator();
	nrf_delay_ms(50);
}

void CommandLineInterface_printVersion()
{
	CommandLineInterface_printf("  SmartHalo test program, v%s.%s%s, %s, %s\r\n", MAJORVERSION, MINORVERSION, BUILDTAG, __DATE__, __TIME__);
}

int CommandLineInterface_printf(const char * fmt, ... )
{
	int length;
	va_list args;
	va_start(args, fmt);

	// If we loose connection at some point, make sure we have a valid logging interface
	if(!Bluetooth_isConnected())
		activeInterface = INTERFACE_UART;

	if(activeInterface == INTERFACE_UART)
	{
		length = UART_printf(fmt, args);
	}
	else // INTERFACE_BLE
	{
		length = Bluetooth_printf(fmt, args);
	}

	va_end(args);

	return length;
}

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

#ifndef COMMANDLINEINTERFACE_H_
#define COMMANDLINEINTERFACE_H_

typedef enum
{
	INVALIDMODULE = -1,
	RADIOMODULE,
	I2CMODULE,
	UARTMODULE,
	LEDMODULE,
	TOUCHMODULE,
	SOUNDMODULE,
	BATMODULE,
	ACCELEROMODULE,
	MAGNETOMODULE,
	POWERSUPPLYMODULE
} TESTMODULES;

typedef enum
{
	INTERFACE_UART,
	INTERFACE_BLE
} CLI_Interface;

#define NEWLINE_SEPARATOR		'\n'
#define COMMAND_SEPARATOR		"\r\n"

#define COMMAND_MAX_LENGTH		64

void CommandLineInterface_setup();

void CommandLineInterface_parseAndExecuteCommand(CLI_Interface interface, char * RxBuff, int cnt);

void CommandLineInterface_printHelp();

int CommandLineInterface_printLine(const char * line );

int CommandLineInterface_printSeparator();

int CommandLineInterface_printf(const char * fmt, ...);

//CLI_Mode CommandLineInterface_getCLIMode();

//void CommandLineInterface_getCLIMode(CLI_Mode mode);

#endif /* COMMANDLINEINTERFACE_H_ */

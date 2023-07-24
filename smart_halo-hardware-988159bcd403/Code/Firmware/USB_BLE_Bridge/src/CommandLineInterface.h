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

#define NEWLINE_SEPARATOR		'\n'
#define COMMAND_SEPARATOR		"\r\n"

#define COMMAND_MAX_LENGTH		64

void CommandLineInterface_setup();

void CommandLineInterface_parseAndExecuteCommand(char * RxBuff, int cnt);

void CommandLineInterface_printHelp();

int CommandLineInterface_printLine(const char * line );

int CommandLineInterface_printSeparator();

int CommandLineInterface_printf(const char * fmt, ...);

#endif /* COMMANDLINEINTERFACE_H_ */

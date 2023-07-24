///
/// \file 		Shell.h
/// \brief 		[header file]
///				
/// \author 	NOVO
///
#ifndef __SHELL__H
#define __SHELL__H


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <SystemUtilitiesTask.h>
#include "DebugConsole.h"
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define SHELL_VERSION_MAJOR 1
#define SHELL_VERSION_MINOR 0

#define SHELL_SEND_NEW_LINE	1

#define MAX_ARGS	15
#define MAX_LINE	80
#define MAX_FN		40

#define SHELL_RX_BUFFER_SIZE	50	///< RX buffer size, in bytes.

#define CONSOLE_NEW_MESSAGE		(1)
#define CONSOLE_SUCCESS				(0)
#define CONSOLE_BAD_PARAMETER (-1)
#define CONSOLE_BAD_MESSAGE		(-2)
#define CONSOLE_NO_MESSAGE		(-3)
#define CONSOLE_TX_BUSY				(-4)
#define CONSOLE_RX_BUSY				(-5)
#define CONSOLE_INVALIDE_BYTE (-6)

/* Message size and definition */
/*
#define CONSOLE_MAX_ARG 				20
#define CONSOLE_CMD_PREFIX_SIZE 4
#define CONSOLE_CMD_END_CHAR		(uint8_t)'\n'
 */
////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

bool init_Shell (void);
bool task_Shell (void);
int log_Shell(const char* format,...) __attribute__ ((format (printf, 1, 2)));
void xxd_Shell(void *buf, uint32_t len);
#endif

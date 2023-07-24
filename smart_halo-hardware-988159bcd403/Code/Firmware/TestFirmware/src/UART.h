/*
 * UART.h
 *
 *  Created on: Jun 15, 2016
 *      Author: sgelinas
 */

#ifndef SRC_UART_H_
#define SRC_UART_H_

#include "nrf.h"
#include "bsp.h"

void UART_setup();

void UART_printHelp();

bool UART_parseAndExecuteCommand(char * RxBuff, int cnt);

uint32_t UART_get_character(uint8_t * c);

uint32_t UART_put_character(uint8_t c);

uint32_t UART_flush();

bool UART_startStopLoopbackTest(bool start);

bool UART_isLoopbackTestActive();

int UART_printf(const char * fmt, va_list args);

#endif /* SRC_UART_H_ */

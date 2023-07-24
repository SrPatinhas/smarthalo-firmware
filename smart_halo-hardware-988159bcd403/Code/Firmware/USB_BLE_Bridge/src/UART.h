/*
 * UART.h
 *
 *  Created on: 2016-06-27
 *      Author: Seb
 */

#ifndef SRC_UART_H_
#define SRC_UART_H_

#include "nrf.h"
#include "bsp.h"

void UART_setup(void);

uint32_t UART_put_character(uint8_t c);

uint32_t UART_get_character(uint8_t * c);

uint32_t UART_flush();

bool UART_isUARTFlowStopped();

void UART_reenableFlowAndProcessData();

bool UART_startStopWireMode(bool start);

int UART_printf(const char * fmt, va_list args);

bool UART_isWireModeActive();

int UART_getRXWireCounter();

int UART_getTXWireCounter();

int UART_incrementRXWireCounter(int increment);

void UART_resetWireCounters();

#endif /* SRC_UART_H_ */

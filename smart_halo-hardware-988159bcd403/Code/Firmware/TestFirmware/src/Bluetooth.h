/*
 * Bluetooth.h
 *
 *  Created on: Jun 15, 2016
 *      Author: sgelinas
 */

#ifndef SRC_BLUETOOTH_H_
#define SRC_BLUETOOTH_H_

#include <stdbool.h>
#include "nrf.h"
#include "bsp.h"

typedef enum
{
	POWER_M40dBm = -40,
	POWER_M30dBm = -30,
	POWER_M20dBm = -20,
	POWER_M16dBm = -16,
	POWER_M12dBm = -12,
	POWER_M8dBm = -8,
	POWER_M4dBm = -4,
	POWER_0dBm = 0,
	POWER_4dBm = 4,
} TXPOWER;

void Bluetooth_setup();

bool Bluetooth_isConnected();

void Bluetooth_printHelp();

bool Bluetooth_parseAndExecuteCommand(char * RxBuff, int cnt);

bool Bluetooth_isBLETXBufferAlmostFull();

int8_t Bluetooth_getRSSIdBm();

TXPOWER Bluetooth_getTXPowerdBm();

bool Bluetooth_isValidPower(int8_t txPowerdBm);

void Bluetooth_setTXPowerdBm(TXPOWER powerdBm);

bool Bluetooth_startStopLoopbackTest(bool start);

bool Bluetooth_isLoopbackTestActive();

int Bluetooth_printf(const char * fmt, va_list args);

int Bluetooth_getLoopbackModeTXCnt();

int Bluetooth_getLoopbackModeRXCnt();

#endif /* SRC_BLUETOOTH_H_ */

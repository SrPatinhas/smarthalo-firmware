/*
 * Bluetooth.h
 *
 *  Created on: 2016-06-27
 *      Author: Seb
 */

#ifndef SRC_BLUETOOTH_H_
#define SRC_BLUETOOTH_H_

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

void Bluetooth_scan_start(void);

void Bluetooth_scan_stop(void);

int Bluetooth_getBLETXBufferAvailableChars();

int Bluetooth_getBLETXBufferSpaceLeft();

int Bluetooth_wireModePacket(uint8_t * p_data, uint16_t length);

void Bluetooth_power_manage(void);

bool Bluetooth_isBLETXBufferAlmostFull();

int8_t Bluetooth_getRSSIdBm();

TXPOWER Bluetooth_getTXPowerdBm();

bool Bluetooth_isValidPower(int8_t txPowerdBm);

void Bluetooth_setTXPowerdBm(TXPOWER powerdBm);

bool Bluetooth_isConnected();

void Bluetooth_disconnect();

bool Bluetooth_isScanning();

bool Bluetooth_isBLETxBufferFull();

int Bluetooth_getWireModeTXCnt();

int Bluetooth_getWireModeRXCnt();


#endif /* SRC_BLUETOOTH_H_ */

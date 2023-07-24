/*
 * BLEDriver.h
 *
 *  Created on: Jul 4, 2019
 *      Author: Nzo
 */

#ifndef PERIPHERALS_BLEDRIVER_H_
#define PERIPHERALS_BLEDRIVER_H_

#include "main.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define BLE_MAX_DATA_SIZE 256
#define BLE_NBR_DATA_BUFFER 4

#define BLE_TX_MAX_DATA_SIZE 80
#define BLE_TX_NBR_DATA_BUFFER 4
// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================

typedef struct __attribute__((__packed__)){
	uint8_t msg_type;
	uint8_t msg_cmd;
	uint8_t seqID;
	uint16_t size;
	uint8_t data[BLE_MAX_DATA_SIZE];
} oBLERXMessage_t, *poBLERXMessage_t;

typedef struct {
	uint8_t msg_type;
	uint8_t msg_cmd;
	uint8_t seqID;
	uint16_t size;
	uint8_t data[BLE_TX_MAX_DATA_SIZE];
} oBLETXMessage_t, *poBLETXMessage_t;

// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
bool init_BLEDriver(void);
bool toggleStandby_BLEDriver(uint8_t reason);
bool powerOff_BLEDriver(void);
void checkUART_BLEDriver();
bool tx_BLEDriver(poBLETXMessage_t pmsg);
void restartDMA_BLEDriver(void);
void printBLEStatus_BLEDriver(void);

#endif /* PERIPHERALS_BLEDRIVER_H_ */

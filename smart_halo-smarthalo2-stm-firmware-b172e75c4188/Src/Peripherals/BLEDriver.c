/*
 * BLEDriver.c
 *
 *  Created on: Jul 4, 2019
 *      Author: Matt
 */

#include <inttypes.h>
#include <CommunicationTask.h>
#include <SoundTask.h>
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <SystemUtilitiesTask.h>
#include "BLEDriver.h"
#include "usart.h"
#include "HaloLedsDriver.h"

#include "GraphicsTask.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define STACK_SIZE 				(configMINIMAL_STACK_SIZE + 256)
#define TASK_PRIORITY 			PRIORITY_NORMAL
#define QUEUE_TX_LENGTH 		4
#define ACK_TIMEOUT 			750
#define TX_RETRY_COUNT 			3

#define _ENABLE_BLE() HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, GPIO_PIN_SET)
#define _DISABLE_BLE() HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, GPIO_PIN_RESET)
#define _DISABLE_UART_TX() HAL_GPIO_WritePin(BLE_TX_GPIO_Port, BLE_TX_Pin, GPIO_PIN_RESET)
#define _DISABLE_UART_RX() HAL_GPIO_WritePin(BLE_RX_GPIO_Port, BLE_RX_Pin, GPIO_PIN_RESET)

// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE ENUM DEFINITION
// ================================================================================================
// ================================================================================================
typedef enum {
    BLERXMsg_STATE_S = 0,
    BLERXMsg_STATE_T,
    BLERXMsg_STATE_R,
    BLERXMsg_STATE_0,
    BLERXMsg_STATE_TYPE,
    BLERXMsg_STATE_CMD,
    BLERXMsg_STATE_SEQ,
    BLERXMsg_STATE_LSB,
    BLERXMsg_STATE_MSB,
    BLERXMsg_STATE_DATA,
    BLERXMsg_STATE_MAX
} BLERXMsg_STATE_E;


// ================================================================================================
// ================================================================================================
//            PRIVATE STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================



// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================

/* Queue with jobs ------------ */

typedef struct
{
    uint32_t 			actualPosition;
    uint32_t 			previousPosition;
    uint32_t 			wmCounter;

    StaticQueue_t 		xStaticQueueTX;
    uint8_t 			ucQueueTX[QUEUE_TX_LENGTH * sizeof(oBLETXMessage_t)];
    QueueHandle_t 		xQueue_tx;

    uint32_t            msgTotalRetryCounter;       ///< Debug purpose
    uint32_t            msgSeqIDNotMatchCounter;    ///< Debug purpose
    uint32_t            msgDropCounter;             ///< Debug purpose
    uint32_t            msgSendCounter;             ///< Debug purpose
    uint32_t            ackReceivedCounter;         ///< Debug purpose
    uint32_t            ackTimeout;                 ///< Timeout management for ACKs
    uint32_t            rxTimeout;                  ///< Timeout management for incomplete RX messages
    uint32_t            rxDrop;                     ///< Number of packets duplicate packets dropped
    UBaseType_t         uxHighWaterMark;
    struct {
        uint32_t 		ackIsWaiting:1;
    };
    uint16_t 			sizeToReceive;				///< Number of expected byte for the payload.
    uint16_t 			rxDataCounter;				///< Index in the RX buffer.
    uint8_t 			retryCounter;					///< Number of retry.
    uint8_t 			rxBuffer[BLE_NBR_DATA_BUFFER * BLE_MAX_DATA_SIZE];
    uint8_t 			txBuffer[BLE_TX_MAX_DATA_SIZE];

    BLERXMsg_STATE_E 	BLERXStateMachine;				///< State machine to manage the received bytes.
    oBLETXMessage_t 	BLEPreviousMsgToSend;			///< Structure of the sent message for a retry purpose.
    oBLETXMessage_t 	BLEMsgToSend;					///< Structure to send a complete message (via queue).
    oBLERXMessage_t 	BLERXMsg;						///< Structure to receive a complete message
    UART_HandleTypeDef	UART_Handle;					///< UART hanle to manage more than one uart for the same protocol. Further improvement.
}oBLEDriver_t, *poBLEDriver_t;

static bool isStandby = false;

static oBLEDriver_t oBLEDriver;
// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
static void processRXBytes( uint8_t * buffer, uint32_t length);
static bool processReceivedPacket(void);
static bool processQueueTX(void);
static bool sendMessage(uint8_t type, uint8_t cmd,uint8_t seq,  uint16_t size, uint8_t * data);
static bool verifyRXTimeout(void);
static bool verifyACK(void);

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

void restartDMA_BLEDriver(void)
{
    bool bResult = false;
    oBLEDriver.actualPosition = 0;
    oBLEDriver.previousPosition = 0;
    memset(oBLEDriver.rxBuffer, 0, sizeof(oBLEDriver.rxBuffer));
    HAL_UART_Read_DMA(&BLE_UART,oBLEDriver.rxBuffer, BLE_NBR_DATA_BUFFER * BLE_MAX_DATA_SIZE, &bResult);
}

bool powerOff_BLEDriver(void)
{
    HAL_UART_MspDeInit(&BLE_UART);
    _DISABLE_UART_TX();
    _DISABLE_UART_RX();
    _DISABLE_BLE();

    return true;
}

/**
 * @brief		bleInit()
 * @details		Init function for the BLEDriver
 * @public
 */
bool init_BLEDriver(void)
{
    memset(&oBLEDriver, 0, sizeof(oBLEDriver_t));
    oBLEDriver.UART_Handle = BLE_UART;

    if (oBLEDriver.xQueue_tx == NULL) {
        oBLEDriver.xQueue_tx = xQueueCreateStatic(QUEUE_TX_LENGTH, sizeof(oBLETXMessage_t) , oBLEDriver.ucQueueTX, &oBLEDriver.xStaticQueueTX);
        configASSERT(oBLEDriver.xQueue_tx);
    }

    oBLEDriver.actualPosition = 0;
    oBLEDriver.previousPosition = 0;
    oBLEDriver.wmCounter = 0;

    powerOff_BLEDriver();
    vTaskDelay(1000);
    _ENABLE_BLE();
    HAL_UART_MspInit(&BLE_UART);

    restartDMA_BLEDriver();

    return true;
}

/**
 * @brief       toggle BLE state to standby mode
 * @details     While in standby the BLE chip is disabled meaning no connection can happen to disable
 *              for any reason. If standby is off it will operate normally.
 * @return      Returns whether or not it is now is standby
 */
bool toggleStandby_BLEDriver(uint8_t reason){
    if(isStandby){
        xQueueReset(oBLEDriver.xQueue_tx);
        _ENABLE_BLE();
        HAL_UART_MspInit(&BLE_UART);
        restartDMA_BLEDriver();
    }else{
        powerOff_BLEDriver();
        oBLERXMessage_t disconnectMsg ={
                .msg_cmd = BLE_COMMAND_SYNC,
                .data = {BLE_CONNECTION_STATE, 0, reason},
                .size = 3,
        };
        interpret_CommunicationTask(&disconnectMsg);
    }
    isStandby = !isStandby;

    return isStandby;
}

/**
 * @brief Service the BLE UART
 * @details This is called from the communication task to process both incoming
 *          and outgoing data on the BLE UART.
 */
void checkUART_BLEDriver()
{
    if(isStandby)
        return;

    HAL_UART_Get_RX_Count_DMA(&BLE_UART, &oBLEDriver.actualPosition);

    /* Get current position */
    oBLEDriver.actualPosition = sizeof(oBLEDriver.rxBuffer) - oBLEDriver.actualPosition;

    /* we have data */
    if (oBLEDriver.actualPosition != oBLEDriver.previousPosition) {
        /* We are linear position, simple buffer managment */
        if (oBLEDriver.actualPosition > oBLEDriver.previousPosition) {
            processRXBytes(&oBLEDriver.rxBuffer[oBLEDriver.previousPosition],
                           oBLEDriver.actualPosition - oBLEDriver.previousPosition);
        } else {
            /* DMA buffer wrapped, get the first bit to the end of the buffer */
            processRXBytes(&oBLEDriver.rxBuffer[oBLEDriver.previousPosition],
                           sizeof(oBLEDriver.rxBuffer) - oBLEDriver.previousPosition);
            /* get the rest, if any */
            oBLEDriver.previousPosition = 0;
            if (oBLEDriver.actualPosition > 0) {
                processRXBytes(&oBLEDriver.rxBuffer[0], oBLEDriver.actualPosition);
            }
        }
        oBLEDriver.previousPosition = oBLEDriver.actualPosition;
    }

    verifyRXTimeout();
    processQueueTX();

    if (oBLEDriver.wmCounter++ >= 1000) {
        oBLEDriver.wmCounter       = 0;
        oBLEDriver.uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // if (oBLEDriver.uxHighWaterMark < 64)
    }
}

/************************************************************************************************
 * @brief		BLEDriverVerifyACK()
 * @details		Verify if the received packet is
 * @public
 * @param[in]	pmsg: Handle on the message to send.
 * @return      bool: true if success, false otherwise.
 */
bool tx_BLEDriver(poBLETXMessage_t pmsg){
    if ((oBLEDriver.xQueue_tx == NULL) || (pmsg == NULL)) return false;
    xQueueSend( oBLEDriver.xQueue_tx, ( void* ) pmsg,( TickType_t ) 0 );
    return true;
}
// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================
/************************************************************************************************
 * @brief		BLEProcessRXBytes()
 * @details		Verify if an incomplete message is received.
 * @private
 * @return      bool: true if success, false otherwise.
 */
static bool verifyRXTimeout(void) {
    // RX is not idle.
    if (oBLEDriver.BLERXStateMachine != BLERXMsg_STATE_S){
        if ((HAL_GetTick() - oBLEDriver.rxTimeout) >= ACK_TIMEOUT){
            // Drop the message
            oBLEDriver.msgDropCounter++;
            oBLEDriver.BLERXStateMachine = BLERXMsg_STATE_S;
            oBLEDriver.rxDataCounter = 0;
        }
    }
    return true;
}

/**
 * @brief		processRXBytes()
 * @details		Process the received bytes.
 * @private
 * @param[in]	buffer  pointer to the rx buffer
 * @param[in]	length  number of bytes to process
 * @return      true on success, false otherwise
 */
static void processRXBytes(uint8_t *buffer, uint32_t length)
{
    uint8_t *b;

    if (buffer == NULL) return;

    for (b = buffer; length--; b++) {
        oBLEDriver.rxTimeout = HAL_GetTick();
        switch (oBLEDriver.BLERXStateMachine) {
        case BLERXMsg_STATE_S:
            oBLEDriver.rxDataCounter = 0;
            if (*b == 'S') oBLEDriver.BLERXStateMachine++;
            break;
        case BLERXMsg_STATE_T:
            if (*b == 'T')
                oBLEDriver.BLERXStateMachine++;
            else
                oBLEDriver.BLERXStateMachine = BLERXMsg_STATE_S;
            break;
        case BLERXMsg_STATE_R:
            if (*b == 'R')
                oBLEDriver.BLERXStateMachine++;
            else
                oBLEDriver.BLERXStateMachine = BLERXMsg_STATE_S;
            break;
        case BLERXMsg_STATE_0:
            if (*b == 0)
                oBLEDriver.BLERXStateMachine++;
            else
                oBLEDriver.BLERXStateMachine = BLERXMsg_STATE_S;
            break;
        case BLERXMsg_STATE_TYPE:
            oBLEDriver.BLERXMsg.msg_type = *b;
            oBLEDriver.BLERXStateMachine++;
            break;
        case BLERXMsg_STATE_CMD:
            oBLEDriver.BLERXMsg.msg_cmd = *b;
            oBLEDriver.BLERXStateMachine++;
            break;
        case BLERXMsg_STATE_SEQ:
            oBLEDriver.BLERXMsg.seqID = *b;
            oBLEDriver.BLERXStateMachine++;
            break;
        case BLERXMsg_STATE_LSB:
            oBLEDriver.BLERXMsg.size = *b;
            oBLEDriver.BLERXStateMachine++;
            break;
        case BLERXMsg_STATE_MSB:
            oBLEDriver.BLERXMsg.size |= ((uint16_t)*b << 8);
            if (oBLEDriver.BLERXMsg.size > BLE_MAX_DATA_SIZE) {
                // Error
                log_Shell("BLE Error, message too long at %d",
                          oBLEDriver.BLERXMsg.size);
                // Reset state machine and keep on consuming bytes
                // TODO maybe send back error to BLE
                oBLEDriver.BLERXStateMachine = BLERXMsg_STATE_S;
            } else {
                oBLEDriver.BLERXStateMachine++;
                oBLEDriver.sizeToReceive =
                    oBLEDriver.BLERXMsg.size + 1;  // +1 for the checksum
            }
            break;
        case BLERXMsg_STATE_DATA:
            oBLEDriver.BLERXMsg.data[oBLEDriver.rxDataCounter] = *b;
            oBLEDriver.rxDataCounter++;
            if (oBLEDriver.rxDataCounter >= oBLEDriver.sizeToReceive) {
                if (processReceivedPacket() == false) return;
                oBLEDriver.BLERXStateMachine = BLERXMsg_STATE_S;
                oBLEDriver.rxDataCounter     = 0;
                memset(&oBLEDriver.BLERXMsg, 0, sizeof(oBLETXMessage_t));
            }
            break;
        default:
            break;
        }
    }
}

/**
 * @brief       Validate the received CS.
 * @param[in]	u8CSProcessed calculated checksum
 * @param[in]	u8CSToValidate expected checksum
 * @param[out]	pbResp pointer to the result of the comparison
 * @return      true if success, false otherwise.
 */
static bool BLEValidateProcessCS(uint8_t u8CSProcessed, uint8_t u8CSToValidate, bool * pbResp){
    if (pbResp == NULL) return false;
    *pbResp = false;
    if (u8CSProcessed == u8CSToValidate) *pbResp = true;
    return true;
}

/**
 * @brief		Calculate the (XOR) checksum
 * @param[in]	pMsg pointer to the message
 * @param[in]	u8Length number of bytes in the message
 * @param[out]	pu8CS pointer into which the checksum is returned
 * @return      true on success, false if either of the pointers are null
 */
static bool BLEProcessCS(void *pMsg, uint8_t u8Length, uint8_t *pu8CS)
{
    uint8_t *p       = pMsg;
    uint8_t  u8CS    = 0;
    uint8_t  u8Index = 0;

    if ((p == NULL) || (pu8CS == NULL)) return false;

    while (u8Index < u8Length) {
        u8CS ^= ~(p[u8Index]);
        u8Index++;
    }
    *pu8CS = u8CS;
    return true;
}

/**
 * @brief      Convert UART_COMMAND_D to a string (for debugging/logging)
 * @param[in]  cmd   the BLE command from the msg
 * @return     Pointer to string representation of cmd
 */
static const char *UART_CMD2STR(UART_COMMAND_E cmd)
{
    // this array must match enum in CommunicationTask.h
    static const char *uartCmdStrings[] = {
        "BLE_RX_COMMAND_BLE",
        "BLE_TX_COMMAND_BLE_NOTIFY",
        "BLE_TX_COMMAND_BLE_RESPONSE",
        "BLE_COMMAND_SYNC",
    };
    if (cmd < 0 || cmd >= BLE_CMD_MAX) return "unknown cmd";
    return uartCmdStrings[cmd];
}

static bool acknowledgementCheck(){
    if (oBLEDriver.BLERXMsg.msg_type == BLE_TYPE_MSG) {
            if (sendMessage(BLE_TYPE_ACK, oBLEDriver.BLERXMsg.msg_cmd,
                    oBLEDriver.BLERXMsg.seqID, 0,
                    oBLEDriver.BLERXMsg.data) == false)
                return false;
    }
    return true;
}

/**
 * @brief       Process the received packet.
 * @details     After processReceivedBytes() puts together a packet, validate
 *              checksum and send to communication task for synchronous handling.
 *              Maybe send a response back to the Nordic.
 * @return      bool: true if success, false otherwise.
 */
static bool processReceivedPacket(void)
{
    static uint8_t lastSeq = 255;
    bool    bResponse = false;
    uint8_t u8CS      = 0;

    // If the received packet is an ack, verify the ack.
    if (oBLEDriver.BLERXMsg.msg_type == BLE_TYPE_ACK) {
        if (verifyACK() == false) return false;
        return true;
    } else if (oBLEDriver.BLERXMsg.msg_type == BLE_TYPE_ERR) {
        // If an error is received, do nothing.
        return true;
    }
    // Calculate the checksum
    if (BLEProcessCS(&oBLEDriver.BLERXMsg, oBLEDriver.BLERXMsg.size + 5, &u8CS) == false) {
        return false;
    }
    // Validate
    if (BLEValidateProcessCS(u8CS,
                             oBLEDriver.BLERXMsg.data[oBLEDriver.BLERXMsg.size],
                             &bResponse) == false) {
        return false;
    }
    // If the CS is valid.... Otherwise, do nothing.
    if (bResponse) {
        log_Shell("%s: BLE msg: cmd: %s, size: %d, seq: %d", __func__,
                  UART_CMD2STR(oBLEDriver.BLERXMsg.msg_cmd),
                  oBLEDriver.BLERXMsg.size, oBLEDriver.BLERXMsg.seqID);
        if (lastSeq == oBLEDriver.BLERXMsg.seqID) {
            oBLEDriver.rxDrop++;
            log_Shell("%s: dropping duplicate packet", __func__);
            return acknowledgementCheck();
        }
        lastSeq = oBLEDriver.BLERXMsg.seqID;
        // Send it onto the CommunicationTask
        if (!interpret_CommunicationTask(&oBLEDriver.BLERXMsg)) return false;

        // If the received packet was a msg, send an ack.
        return acknowledgementCheck();
    }
    return true;
}

/************************************************************************************************
 * @brief		BLEDriverProcessQueueTX()
 * @details		Process the received packet.
 * @private
 * @return      bool: true if success, false otherwise.
 */
static bool processQueueTX(void) {
    // If a ack is pending
    if( oBLEDriver.ackIsWaiting == true ) {
        if (oBLEDriver.ackTimeout == 0) oBLEDriver.ackTimeout = HAL_GetTick();
        if ((HAL_GetTick() - oBLEDriver.ackTimeout) >= ACK_TIMEOUT){
            if (oBLEDriver.retryCounter++ < TX_RETRY_COUNT)
            {
                oBLEDriver.ackTimeout = HAL_GetTick();
                oBLEDriver.msgTotalRetryCounter++;
                if (sendMessage(oBLEDriver.BLEPreviousMsgToSend.msg_type,
                        oBLEDriver.BLEPreviousMsgToSend.msg_cmd,
                        oBLEDriver.BLEPreviousMsgToSend.seqID,
                        oBLEDriver.BLEPreviousMsgToSend.size,
                        oBLEDriver.BLEPreviousMsgToSend.data) == false) return false;
            }
            else
            {
                oBLEDriver.retryCounter = 0;
                oBLEDriver.ackIsWaiting = false;
            }
        }
        return true;
    }

    if( oBLEDriver.xQueue_tx == NULL ) return false;

    if( xQueueReceive( oBLEDriver.xQueue_tx, &( oBLEDriver.BLEMsgToSend ), ( TickType_t ) 10 ) )
    {

        oBLEDriver.msgSendCounter++;
        oBLEDriver.BLEMsgToSend.seqID = oBLEDriver.BLEPreviousMsgToSend.seqID +1;
        if (sendMessage(oBLEDriver.BLEMsgToSend.msg_type,
                oBLEDriver.BLEMsgToSend.msg_cmd,
                oBLEDriver.BLEMsgToSend.seqID,
                oBLEDriver.BLEMsgToSend.size,
                oBLEDriver.BLEMsgToSend.data) == false) return false;
        oBLEDriver.BLEPreviousMsgToSend = oBLEDriver.BLEMsgToSend;
        oBLEDriver.ackIsWaiting = true;
        oBLEDriver.ackTimeout = HAL_GetTick();
        oBLEDriver.retryCounter = 0;
    }
    return true;
}
/************************************************************************************************
 * @brief		BLEDriverVerifyACK()
 * @details		Verify if the received packet is
 * @private
 * @return      bool: true if success, false otherwise.
 */
static bool verifyACK(void){
    if ( oBLEDriver.ackIsWaiting == false ) {
        return true;
    }

    if (oBLEDriver.BLERXMsg.msg_cmd != oBLEDriver.BLEPreviousMsgToSend.msg_cmd ||
            oBLEDriver.BLERXMsg.seqID != oBLEDriver.BLEPreviousMsgToSend.seqID) {
        oBLEDriver.msgSeqIDNotMatchCounter++;
        return true;
    }

    oBLEDriver.ackIsWaiting = false;
    oBLEDriver.ackReceivedCounter++;
    return true;
}

/************************************************************************************************
 * @brief		BLEDriverSendMessage()
 * @details		Send a BLE TX message via uart.
 * @private
 * @param[in]	u8Type: Type to put in the header.
 * @param[in]	u8Cmd: Command to put in the header.
 * @param[in]	u8Seq: Sequence ID to put in the header.
 * @param[in]	u16Size: NUmber of byte in the payload
 * @param[in]	pu8Data: handle on the buffer to send.
 * @return      bool: true if success, false otherwise.
 */
static bool sendMessage(uint8_t type, uint8_t cmd,uint8_t seq,  uint16_t size, uint8_t * data){

    uint8_t tx_data_count = 0;
    uint8_t u8CS = 0;
    bool bResult = false;

    if (data == NULL) return false;

    oBLEDriver.txBuffer[tx_data_count++] = 'S';
    oBLEDriver.txBuffer[tx_data_count++] = 'T';
    oBLEDriver.txBuffer[tx_data_count++] = 'R';
    oBLEDriver.txBuffer[tx_data_count++] = '\0';
    oBLEDriver.txBuffer[tx_data_count++] = type;
    oBLEDriver.txBuffer[tx_data_count++] = cmd;
    oBLEDriver.txBuffer[tx_data_count++] = seq;
    oBLEDriver.txBuffer[tx_data_count++] = size&0xff;
    oBLEDriver.txBuffer[tx_data_count++] = size>>8;
    for(int i=0; i<size;i++){
        oBLEDriver.txBuffer[tx_data_count++] = data[i];
    }
    BLEProcessCS(&oBLEDriver.txBuffer[4], size + 5, &u8CS);
    oBLEDriver.txBuffer[tx_data_count++] = u8CS;

    if (HAL_UART_Write_IT(&BLE_UART, oBLEDriver.txBuffer, tx_data_count, &bResult) == false) return false;
    return true;

}

void printBLEStatus_BLEDriver(void)
{
    log_Shell("BLE Status\n");
    log_Shell("msgTotalRetryCounter:    %10" PRIu32, oBLEDriver.msgTotalRetryCounter);
    log_Shell("msgSeqIDNotMatchCounter: %10" PRIu32, oBLEDriver.msgSeqIDNotMatchCounter);
    log_Shell("msgDropCounter:          %10" PRIu32, oBLEDriver.msgDropCounter);
    log_Shell("msgSendCounter:          %10" PRIu32, oBLEDriver.msgSendCounter);
    log_Shell("ackReceivedCounter:      %10" PRIu32, oBLEDriver.ackReceivedCounter);
    log_Shell("rxDrop:                  %10" PRIu32, oBLEDriver.rxDrop);
    log_Shell("uxHighWaterMark:         %10" PRIu32, oBLEDriver.uxHighWaterMark);
}

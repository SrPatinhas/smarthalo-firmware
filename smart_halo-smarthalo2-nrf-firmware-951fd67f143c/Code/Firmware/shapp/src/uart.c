#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "scheduler.h"
#include "app_timer.h"
#include "bslink.h"
#include "device.h"
#include "dispatch.h"
#include "auth.h"
#include "keys.h"

#include "app_uart.h"
#include "nrf_delay.h"
#include "app_fifo.h"
#include "uart.h"
#include "pca10040.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

#define MINIMUM_MILLISECONDS_BETWEEN_UART 5

#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 1024

#define UART_TX_QUEUE_SIZE 20
#define UART_TX_RETRY_TIMER_MS 40
#define UART_TX_RETRY_COUNT 5
#define UART_RX_RETRY_TIMER_MS 100

// ================================================================================================
// ================================================================================================
//            PRIVATE ENUM DEFINITION
// ================================================================================================
// ================================================================================================

/**
 * \enum BLE_RX_State_t
 * \brief State machine to receive a new packet.
 */
typedef enum {
	BLE_RX_STATE_S = 0,
	BLE_RX_STATE_T,
	BLE_RX_STATE_R,
	BLE_RX_STATE_0,
	BLE_RX_STATE_TYPE,
	BLE_RX_STATE_CMD,
    BLE_RX_STATE_SEQ,
	BLE_RX_STATE_LSB,
	BLE_RX_STATE_MSB,
	BLE_RX_STATE_DATA,
	BLE_RX_STATE_MAX
} BLE_RX_State_t;

// ================================================================================================
// ================================================================================================
//            PRIVATE STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================
/**
 * \struct oHeader_t poHeader_t
 * \brief Struct for the header of a packet.
 *
 * oHeader_t Contains header's information.
 * The STR and Z are not process in the CS. The arribute packed if to be sure
 * that all the byte are properly stored to parse the buffer.
 */
typedef struct __attribute__((packed))
{
    struct
    {
        uint8_t u8STR[3];   // Not in CS
        uint8_t u8Z;        // Not in CS
    };
    uint8_t u8Type;
    uint8_t u8CMD;
    uint8_t u8SeqID;
    union
    {
        uint16_t u16Length;
        uint8_t u8Length[2];
    };
}oHeader_t, *poHeader_t;

typedef struct{
    uint8_t length;
    uint8_t buffer[UART_TX_BUF_SIZE];
    uint8_t seqID;
} txBuf_t;

/**
 * \struct oUART_t poUART_t
 * \brief Struct for the UART module
 *
 */
typedef struct
{
    uint8_t u8SeqIDAckToReceive;        ///< Keep the seq ID to receive
    uint8_t u8LastestStatus;            ///< Keep the lastest received status.
    uint8_t u8SequenceID;
    struct
    {
        uint32_t bIsAckPending:1;       ///< flag to indicates that an ack is waiting.
        uint32_t bAckSent:1;            ///< flag to indicates that a ack as been sent.
        uint32_t bUTSendBadCS:1;        ///< reserved for UT.
    };
    BLE_RX_State_t BLE_RX_State;        ///< Contains the state machine for the RX.
    uint8_t u8RXIndex;                  ///< RX index.
    union
    {
        uint8_t u8RXLength[2];          ///< used to keep the expected length for the rx buffer. The com received the LSB first.
        uint16_t u16RXLength;           ///< used to keep the expected length for the rx buffer. to used as a 16bits.
    };
    uint32_t u32UTTXCounter;            ///< reserved for the UT.
    uint32_t u32ACKReceiveCounter;      ///< Contains the number of ack received.
    uint32_t u32ACKSendCounter;         ///< Contains the number of ack sent.
    uint8_t u8RetryCounter;             ///< Retry counter.
    uint32_t u32RetryTime;              ///< Retry Timer.
    uint32_t u32RXTimeout;              ///< Time management for the RX buffer.


    uint8_t queueLocation;
    uint8_t queueCount;
    txBuf_t txBuffers[UART_TX_QUEUE_SIZE];
    uint8_t u8RXBuffer[UART_RX_BUF_SIZE];

}oUART_t, *poUART_t;

// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================
oUART_t oUART;
// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
bool UartProcessCS(uint8_t * pu8DataIn, uint8_t u8Length, uint8_t * pu8CS);
void UartSendPacket(uart_msg_id msg_id, uart_cmd_type cmd ,uint8_t u8Seq,uint8_t *payload, uint32_t length);
bool UartManagePacketPending(void);
void bufferToFiFo();
void uart_handler(app_uart_evt_t * p_event);
void uart_consume(void *ctx);
#if UART_UT
void UartUT(void);
#endif

/**
 * @brief       UartInit()
 * @details     Function to init the UART
 * @public
 * @return      bool: true if success, false otherwise.
 */
bool UartInit(void)
{
    uint8_t u8ErrorCode = 0;
    // Clear all buffer.
    memset(&oUART, 0, sizeof(oUART_t));
    
    // UART config
    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud115200
      };
    
    // Configure the UART
    APP_UART_FIFO_INIT(&comm_params,
                     UART_RX_BUF_SIZE,
                     UART_TX_BUF_SIZE,
                     uart_handler,
                     APP_IRQ_PRIORITY_HIGHEST,
                     u8ErrorCode);

    APP_ERROR_CHECK(u8ErrorCode); 

    app_uart_flush();
   
#if UART_UT
    UartUT();
#endif

    sch_unique_oneshot(uart_consume, 500);

    return true;
}

/**
 * @brief       uart_handler()
 * @details     Not used.
 * @public
 * @param[in]	p_event: 
 * @return      bool: true if success, false otherwise.
 */
void uart_handler(app_uart_evt_t * p_event)
{
    switch (p_event->evt_type)
    {
        /**@snippet [Handling data from UART] */
        case APP_UART_DATA_READY:
            break;
        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            printf("APP_UART_COMMUNICATION_ERROR: error_communication: %x\n", p_event->data.error_communication);
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            printf("APP_UART_FIFO_ERROR\n");
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}

static void clearAndAdvanceAck(bool timeout)
{
    // printf("%s: clearing ack for msg %u, %s\n",
    //        __func__,
    //        oUART.u8SeqIDAckToReceive,
    //        timeout ? "timed out" : "normal");

    oUART.u8RetryCounter = oUART.u32RetryTime = 0;
    oUART.bIsAckPending = false;
    oUART.queueCount--;
    if (oUART.queueCount) {
        bufferToFiFo();
    }
}

/**
 * @brief       UartManagePacketPending()
 * @details     Manage the packet that is waiting for an ACK.
 * @public
 * @return      bool: true if success, false otherwise.
 */
bool UartManagePacketPending(void)
{
    // a ack is pending.
    if (oUART.bIsAckPending)
    {
//#warning todo: adapt with the schedulor.        
        // timeout when a ack is pending.
        if (oUART.u32RetryTime++ >= UART_TX_RETRY_TIMER_MS)
        {
            // max number of retry.
            if (oUART.u8RetryCounter++ < UART_TX_RETRY_COUNT)
            {
                oUART.u32RetryTime = 0;
                // Retry...
                // printf("%s: Resending UART message for %u, no ack received\n", __func__, oUART.u8SeqIDAckToReceive);
                bufferToFiFo();
            }
            else
            {
                // drop the packet
                clearAndAdvanceAck(true);
            }
        }
    }
    else
    {
        oUART.u32RetryTime = 0;
    }
    
    // incomplete packet and timeout.
    if(oUART.BLE_RX_State != BLE_RX_STATE_S)
    {
        if(oUART.u32RXTimeout++ > UART_RX_RETRY_TIMER_MS)
        {
            // printf("%s: reset RX state after timeout %u\n", __func__, UART_RX_RETRY_TIMER_MS);
            oUART.u32RXTimeout = 0;
            oUART.u8RXIndex = 0;
            oUART.BLE_RX_State = BLE_RX_STATE_S;
        }
    }
    return true;
}

/**
 * @brief       UartTask
 * @details     UART Task to manage the UART buffer.
 * @public
 * @return      bool: true if success, false otherwise.
 */
bool UartTask(void)
{
    uint8_t u8Byte = 0;
    bool bSuccess = false;
    bool bProcess = false;
    // Manage the pending ack is required.
    if (UartManagePacketPending() == false) return false;
    // Process all bytes in the buffer.
    // uint32_t chars = app_uart_get_nb_rx_fifo_chars();
    // if (chars)
    //     printf("app_uart_get_nb_rx_fifo_chars returns: %u\n", app_uart_get_nb_rx_fifo_chars());
    while(app_uart_get(&u8Byte) == NRF_SUCCESS)
    {
        bProcess = false;
        // Get a byte in the fifo.
        switch(oUART.BLE_RX_State)
        {
            case BLE_RX_STATE_S:
                {
                    if (u8Byte == 'S')
                    {
                        bSuccess = true; 
                    }
                }break;
            case BLE_RX_STATE_T:
             {
                if (u8Byte == 'T')
                {
                    bSuccess = true; 
                }
            }break;
            case BLE_RX_STATE_R:
            {
                if (u8Byte == 'R')
                {
                    bSuccess = true; 
                }
            }break;
            case BLE_RX_STATE_0:
            {
                if (u8Byte == 0)
                {
                    bSuccess = true; 
                }
            }break;
            case BLE_RX_STATE_TYPE:
            case BLE_RX_STATE_CMD:
            case BLE_RX_STATE_SEQ:
            {
                bSuccess = true; 
            }break;
            case BLE_RX_STATE_LSB:
            {
                oUART.u8RXLength[0] = u8Byte;
                bSuccess = true; 
            }break;
            case BLE_RX_STATE_MSB:
            {
                oUART.u8RXLength[1] = u8Byte;
                oUART.u16RXLength++;// for the CS
                bSuccess = true; 
            }break;
            case BLE_RX_STATE_DATA:
            {
                if (0 == --oUART.u16RXLength)
                {
                    // Ready to process.
                    bProcess = true;
                }
                bSuccess = true; 
            }break;
            case BLE_RX_STATE_MAX:
            break;
        }
        // Valide byte.
        if (bSuccess)
        {                                    
            oUART.u32RXTimeout = 0;
            // Change the SM.
            if (oUART.BLE_RX_State < (BLE_RX_STATE_MAX - 1))
            {
                oUART.BLE_RX_State++;
            }
            // Put the byte in the buffer.
            oUART.u8RXBuffer[oUART.u8RXIndex++] = u8Byte;
            // Ready to process the whole received packet.
            if (bProcess == true)
            {
                // Process RX message.
                // To process the CS, start at the index 4 (remove "STR" and Z)
                // In the length, remove 5 ("STR"' Z and at this point, the CS
                // is included in the length.
                UartProcessCS(&oUART.u8RXBuffer[4], oUART.u8RXIndex - 5, &u8Byte);
                // Validate the processed CS.
                if (oUART.u8RXBuffer[oUART.u8RXIndex - 1] == u8Byte)
                {
                    // Parse the buffer into header handle to process the 
                    // data easily.
                    poHeader_t poHeader = (poHeader_t)oUART.u8RXBuffer;
                    ////////////////////////////////////////////////////////////
                    // If the received message is a ACK, process verify what is 
                    //  expected.
                    if (poHeader->u8Type == (uint8_t)UART_MSG_ID_ACK)
                    {
                        // If a ACK was expected.
                        if (oUART.bIsAckPending)
                        {
                            // Verify if the received sequence ID is the same 
                            // as expected.
                            if (oUART.u8SeqIDAckToReceive == poHeader->u8SeqID)
                            {
                                clearAndAdvanceAck(false);
                                // Debug information.
                                oUART.u32ACKReceiveCounter++;
                            } else {
                                printf("%s: got ack for seq %u, expecting %u\n",
                                       __func__, poHeader->u8SeqID, oUART.u8SeqIDAckToReceive);
                            }
                        }
                    }
                    ////////////////////////////////////////////////////////////
                    // New packet received.
                    else
                    {
                        // Debug information.
                        oUART.bAckSent = true;
                        // Debug information.
                        oUART.u32ACKSendCounter++;
                        // Debug information.
                        oUART.u8LastestStatus = oUART.u8RXBuffer[sizeof(oHeader_t)];
                        // send the ack of the received message.
                        UartSendPacket(UART_MSG_ID_ACK, (uart_cmd_type)poHeader->u8CMD, poHeader->u8SeqID, NULL, 0);
                        ////#warning todo: Send data to mobile
                        if(poHeader->u8CMD == UART_CMD_NOTIFY){
                            // if (oUART.u8RXBuffer[9] == 0xf8)
                            //     printf("Event log: %.*s\n", poHeader->u16Length - 1, &oUART.u8RXBuffer[10]);
                            bslink_up_write(&oUART.u8RXBuffer[9],poHeader->u16Length);
                        }else if(poHeader->u8CMD == UART_CMD_RESPONSE){
                            bslink_write(&oUART.u8RXBuffer[9],poHeader->u16Length);
                        }else if(poHeader->u8CMD == UART_CMD_SYNC){
                            if(oUART.u8RXBuffer[9] == UART_SYNC_BOOTLOADER){
                                if(device_enterBootloader()){
                                    uint8_t reply[1];
                                    reply[0] = UART_SYNC_BOOTLOADER;
                                    uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,reply,1);
                                }
                            }else if(oUART.u8RXBuffer[9] == UART_SYNC_CONNECTION){
                                uint8_t reply[2];
                                reply[0] = UART_SYNC_CONNECTION;
                                reply[1] = auth_isConnected() ? 1 : 0;
                                uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,reply,2);    
                            }else if(oUART.u8RXBuffer[9] == UART_SYNC_NAME){
                                if(device_setName(&oUART.u8RXBuffer[10],poHeader->u16Length-1)){
                                    uint8_t reply[1];
                                    reply[0] = UART_SYNC_NAME;
                                    uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,reply,1);    
                                }
                            }else if(oUART.u8RXBuffer[9] == UART_SYNC_VERSIONS){
                                uint8_t reply[9];
                                uint32_t len = 9;
                                reply[0] = UART_SYNC_VERSIONS;
                                auth_getVersions(reply, len);
                                uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,reply,len);
                            }else if(oUART.u8RXBuffer[9] == UART_SYNC_ID){
                                uint8_t reply[9];
                                uint32_t len = 9;
                                reply[0] = UART_SYNC_ID;
                                memcpy(&reply[1], keys_getPubKey(), 8);
                                uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,reply,len);
                            }
                        }
                    }
                }

                // Clear all informations.
                oUART.u8RXIndex = 0;
                oUART.BLE_RX_State = BLE_RX_STATE_S;

            }
        }
        else
        {
            // Clear all informations.
            oUART.u8RXIndex = 0;
            oUART.BLE_RX_State = BLE_RX_STATE_S;
        }
    }

    return true;
}
/**
 * @brief       UartProcessCS()
 * @details     Function to process the CS.
 * @public
 * @param[in]	pu8DataIn: Hanlde on the buffer to process the CS
 * @param[in]	u8Length: Number of byte to process.
 * @param[out]	pu8CS: Handle on the var to return the CS.
 * @return      bool: true if success, false otherwise.
 */
bool UartProcessCS(uint8_t * pu8DataIn, uint8_t u8Length, uint8_t * pu8CS)
{
    uint8_t u8CS = 0;
    uint8_t u8Index = 0;
    if ((pu8DataIn == NULL) || (pu8CS == NULL)) return false;
    while (u8Index < u8Length)
    {
        u8CS ^= ~(pu8DataIn[u8Index]);
        u8Index++;
    }
    *pu8CS = u8CS;
    return true;
}

/**
 * @brief       put packet into the UART TX buffer.
 * @details     The general case will result in the packet being transmitted
 *              by the UART.
 *              There is, however, a special case where we are sending a
 *              special notification of a BLE connection. It is possible that
 *              the receiver (stm) is asleep, so we send it twice. The first
 *              packet will be lost triggering the stm wake-up.
 * @public
 * @param[in]	msg_id  Message type to put in the packet's header.
 * @param[in]	cmd     Command to put in the packet's header.
 * @param[in]	payload Handle on the buffer to send.
 * @param[in]	length  Number of byte to send in the payload (CS not included)
 */
void uart_put_tx_msg(uart_msg_id msg_id, uart_cmd_type cmd, uint8_t *payload, uint32_t length)
{
    oUART.u8SequenceID++;
    UartSendPacket(msg_id, cmd, oUART.u8SequenceID, payload, length);
}

void bufferToFiFo(){
    uint8_t currentLocation = oUART.queueLocation >= oUART.queueCount 
        ? oUART.queueLocation - oUART.queueCount   
        : UART_TX_QUEUE_SIZE + oUART.queueLocation - oUART.queueCount;
    // Put the buffer in the fifo.
    for(int i=0;i<oUART.txBuffers[currentLocation].length;i++)
    {
        // Send the packet
        app_uart_put(oUART.txBuffers[currentLocation].buffer[i]);
    }
    // keep the seqID to ack.
    oUART.u8SeqIDAckToReceive = oUART.txBuffers[currentLocation].seqID;
    // Indicates that a ack is pending.
    oUART.bIsAckPending = true;

}

/**
 * @brief       UartSendPacket()
 * @details     Function to packet in the TX buffer (FIFO).
 * @public
 * @param[in]	msg_id: Message type to put in the packet's header.
 * @param[in]	cmd: Command to put in the packet's header.
 * @param[in]	u8Seq: Sequence number.
 * @param[in]	payload: Handle on the buffer to send.
 * @param[in]	length: Number of byte to send in the payload (CS not included)
 */
void UartSendPacket(uart_msg_id msg_id, uart_cmd_type cmd, uint8_t u8Seq, uint8_t *payload, uint32_t length)
{
    uint8_t u8BufferLength = sizeof(oHeader_t);
    
    // parse the buffer in header struct
    poHeader_t poHeader = (poHeader_t)oUART.txBuffers[oUART.queueLocation].buffer;        

    // Package the header.
    memcpy(poHeader->u8STR, "STR", 3);
    poHeader->u8Z = 0;
    poHeader->u8Type = msg_id;
    poHeader->u8CMD = cmd;
    poHeader->u8SeqID = u8Seq;
    poHeader->u8Length[0] = (length & 0xFF);
    poHeader->u8Length[1] = ((length >> 8) & 0xFF);
    
    // Put the data in the TX buffer.
    if(length > 0){
        // cpy the data in the tx buffer.
        memcpy(&oUART.txBuffers[oUART.queueLocation].buffer[u8BufferLength], payload, length);
        u8BufferLength += length;
    }  

    oUART.txBuffers[oUART.queueLocation].seqID = u8Seq;

    // Process the CS and package it in the buffer.
    // start at the index of type (remove "STR" and Z).
    // Remove 4 byte to process the CS  (remove "STR" and Z).
    UartProcessCS(&oUART.txBuffers[oUART.queueLocation].buffer[4], (u8BufferLength - 4), &oUART.txBuffers[oUART.queueLocation].buffer[u8BufferLength]);
    // To test....
    if (true == oUART.bUTSendBadCS){
        oUART.bUTSendBadCS = false;
        oUART.txBuffers[u8BufferLength].buffer[oUART.queueLocation]++;
    }
    // Added one byte to send for the CS.
    u8BufferLength += 1;
    oUART.txBuffers[oUART.queueLocation].length = u8BufferLength;
    oUART.queueLocation++;
    if(oUART.queueLocation >= UART_TX_QUEUE_SIZE)
        oUART.queueLocation = 0;

    if (msg_id != UART_MSG_ID_ACK){
        oUART.queueCount++;
        if(oUART.queueCount > UART_TX_QUEUE_SIZE){
            oUART.queueCount = UART_TX_QUEUE_SIZE;
            printf("Dropping oldest item on the queue it's full\n");
        }
        if(!oUART.bIsAckPending){
            bufferToFiFo();
        }
    }else if(msg_id == UART_MSG_ID_ACK){
        //if it is an ack, just send it immediately
        for(int i=0;i<u8BufferLength;i++){
            // Send the packet
            uint8_t location = oUART.queueLocation == 0 ?  UART_TX_QUEUE_SIZE-1 : oUART.queueLocation-1;
            app_uart_put(oUART.txBuffers[location].buffer[i]);
        }
        //Allow ack to be overwritten
        if(oUART.queueLocation)
            oUART.queueLocation--;
        else
            oUART.queueLocation = UART_TX_QUEUE_SIZE-1;
    }
}

void uart_consume(void *ctx) {
    if(!UartTask()){
        printf("Whoops UartTask Failed\n");
    }
    sch_unique_oneshot(uart_consume, 10);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// UT Section
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#if UART_UT

#define UART_UT_NORMAL                  0
#define UART_UT_LOOP_NORMAL             1
#define UART_UT_UNIMPLEMENTED           0
#define UART_UT_LOOP_UNIMPLEMENTED      0
#define UART_UT_SEQ_ID                  0
#define UART_UT_CS                      0

void UartUTDelay(uint32_t u32DelayMS);

#if UART_UT_NORMAL
void UartUTNormalPacket(void);
#endif
#if UART_UT_LOOP_NORMAL
void UartUTLoopNormalPacket(void);
#endif
#if UART_UT_UNIMPLEMENTED
void UartUTUnimplementedPacket(void);
#endif
#if UART_UT_LOOP_UNIMPLEMENTED
void UartUTLoopUnimplementedPacket(void);
#endif
#if UART_UT_SEQ_ID
void UartUTSeqIDError(void);
#endif
#if UART_UT_CS
void UartUTCSError(void);
#endif

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void UartUT(void)
{
#if UART_UT_NORMAL
    UartUTNormalPacket();
#endif
#if UART_UT_LOOP_NORMAL
    UartUTLoopNormalPacket();
#endif
#if UART_UT_UNIMPLEMENTED
    UartUTUnimplementedPacket();
#endif
#if UART_UT_LOOP_UNIMPLEMENTED
    UartUTLoopUnimplementedPacket();
#endif
#if UART_UT_SEQ_ID
    UartUTSeqIDError();
#endif
#if UART_UT_CS
    UartUTCSError();
#endif
    while(1);
}
#if UART_UT_NORMAL
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Test 1 -  Write normal buffer and validate ACK.
////////////////////////////////////////////////////////////////////////////
//  1) Send known command (4,1)
//  2) Validate ACK received.
//  3) Wait the reception of the sucessfull packet.
//  4) Send a ack.
////////////////////////////////////////////////////////////////////////////
void UartUTNormalPacket(void)
{
    uint8_t u8UTBuffer[10];
    
    u8UTBuffer[0] = 4; // This command should be supported for test 1.
    u8UTBuffer[1] = 1; // This command should be supported for test 1.
    oUART.u8LastestStatus = 0xFF;
    uart_put_tx_msg(UART_MSG_ID_MSG, UART_CMD_USER, u8UTBuffer, 2);
    oUART.u32UTTXCounter++;
    if (oUART.bAckSent == true) while(1);
    UartUTDelay(2);
    if (oUART.bIsAckPending == false) while(1);
    if (oUART.u8SeqIDAckToReceive != oUART.u8SequenceID) while(1);
    UartUTDelay(200);// might be shorter

    if (oUART.bIsAckPending == true) while(1);
    if (oUART.bAckSent == false) while(1);
    oUART.bAckSent = false;
    if (oUART.u8LastestStatus != eCOM_RETURN_STATUS_OK) while(1);
}
#endif
#if UART_UT_LOOP_NORMAL
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Test 2 -  Write in loop - Modify the ST
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void UartUTLoopNormalPacket(void)
{
    uint8_t u8UTBuffer[10];

    u8UTBuffer[0] = 4; // This command should be supported for test 1.
    u8UTBuffer[1] = 1; // This command should be supported for test 1.
    oUART.u32UTTXCounter = 0;
    oUART.bIsAckPending = 0;
    while(oUART.u32UTTXCounter < 1000)
    {
        oUART.u32UTTXCounter++;
        while(oUART.bIsAckPending == true)
        {
            UartUTDelay(10);
        }
        while(oUART.bAckSent == false)
        {
            UartUTDelay(10);
        }
        oUART.bAckSent = false;
        uart_put_tx_msg(UART_MSG_ID_MSG, UART_CMD_USER, u8UTBuffer, 2);
    };
}
#endif
#if UART_UT_UNIMPLEMENTED
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Test 3 -  Write unsupported command
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void UartUTUnimplementedPacket(void)
{
    uint8_t u8UTBuffer[10];
    
    UartUTDelay(1000);// might be shorter
    u8UTBuffer[0] = 23; // This command should be unsupported for test 3.
    u8UTBuffer[1] = 99; // This command should be unsupported for test 3.
    uart_put_tx_msg(UART_MSG_ID_MSG, UART_CMD_USER, u8UTBuffer, 2);
    oUART.u32UTTXCounter++;
    //UartUTDelay(2);
    if (oUART.bIsAckPending == false) while(1);
    if (oUART.u8SeqIDAckToReceive != oUART.u8SequenceID) while(1);
    UartUTDelay(200);// might be shorter
    if (oUART.bIsAckPending == true) while(1);
    if (oUART.bAckSent == false) while(1);
    oUART.bAckSent = false;
    if (oUART.u8LastestStatus != eCOM_RETURN_STATUS_UNIMPLEMENTED) while(1);
    
}
#endif
#if UART_UT_LOOP_UNIMPLEMENTED
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Test 4 -  Write in loop - to test unimplemented
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void UartUTLoopUnimplementedPacket(void)
{
    uint8_t u8UTBuffer[10];
    oUART.u32UTTXCounter = 0;
    
    while(oUART.u32UTTXCounter++ < 1000)
    {
        u8UTBuffer[0] = 54; // This command should be unsupported for test 3.
        u8UTBuffer[1] = 66; // This command should be unsupported for test 3.
        uart_put_tx_msg(UART_MSG_ID_MSG, UART_CMD_USER, u8UTBuffer, 2);
        oUART.u32UTTXCounter++;
        while(oUART.bIsAckPending == true)
        {
            UartUTDelay(10);
        }
        while(oUART.bAckSent == false)
        {
            UartUTDelay(10);
        }
        oUART.bAckSent = false;
        if (oUART.u8LastestStatus != eCOM_RETURN_STATUS_UNIMPLEMENTED) while(1);
    };
}
#endif
#if UART_UT_SEQ_ID
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Test 5 - Sequence ID Error
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void UartUTSeqIDError(void)
{
    uint8_t u8UTBuffer[10];
    
    u8UTBuffer[0] = 4; // This command should be supported for test 1.
    u8UTBuffer[1] = 1; // This command should be supported for test 1.
    
    oUART.u8RetryCounter = 0;
    uart_put_tx_msg(UART_MSG_ID_MSG, UART_CMD_USER, u8UTBuffer, 2);
    oUART.u8SeqIDAckToReceive++;
    
    while(oUART.u8RetryCounter == false)
    {
        UartUTDelay(10);
    }
    oUART.u8RetryCounter = 0;
}
#endif
#if UART_UT_CS
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Test 6 - CS error
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void UartUTCSError(void)
{
    uint8_t u8UTBuffer[10];
    u8UTBuffer[0] = 4; // This command should be supported for test 1.
    u8UTBuffer[1] = 1; // This command should be supported for test 1.
    
    oUART.u8RetryCounter = 0;
    oUART.bUTSendBadCS = 1;
    uart_put_tx_msg(UART_MSG_ID_MSG, UART_CMD_USER, u8UTBuffer, 2);
    while(oUART.u8RetryCounter == false)
    {
        UartUTDelay(10);
    }
    UartUTDelay(500);
    // Only one retry is allowed.
    if (oUART.u8RetryCounter != 1) while(1);
    oUART.u8RetryCounter = 0;
}
#endif
////////////////////////////////////////////////////////////////////////////
void UartUTDelay(uint32_t u32DelayMS)
{
    while(1)
    {
        while(u32DelayMS-- > 0)
        {
            nrf_delay_ms(1);
            UartTask();
        }
        return;
    }
}
#endif
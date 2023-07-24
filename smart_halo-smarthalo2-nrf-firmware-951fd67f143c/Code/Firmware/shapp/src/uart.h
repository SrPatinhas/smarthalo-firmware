
#ifndef _UART_H
#define _UART_H

#define UART_UT 0

typedef enum {
    UART_MSG_ID_MSG,
    UART_MSG_ID_ACK,
    UART_MSG_ID_NAK,
} uart_msg_id;

typedef enum {
    UART_CMD_USER,
    UART_CMD_NOTIFY,
    UART_CMD_RESPONSE,
    UART_CMD_SYNC,
} uart_cmd_type;

typedef enum {
    UART_SYNC_BOOTLOADER = 0,
    UART_SYNC_CONNECTION = 1,
    UART_SYNC_NAME = 2,
    UART_SYNC_VERSIONS = 3,
    UART_SYNC_ID = 4,
    UART_SYNC_PAIRED = 5,
} uart_sync_type;

typedef enum {
    BLE_DISCONNECTED,
    BLE_CONNECTED,
} uart_connect_type;

typedef enum {
    BLE_UNPAIRED,
    BLE_PAIRED,
} uart_paired_type;

typedef enum{
	eCOM_RETURN_STATUS_OK = 0,
	eCOM_RETURN_STATUS_FAIL = 1,
	eCOM_RETURN_STATUS_DENIED = 2,
	eCOM_RETURN_STATUS_UNIMPLEMENTED = 3,
}eCOM_RETURN_STATUS_t,*peCOM_RETURN_STATUS_t;

void uart_init();
void uart_put_tx_msg(uart_msg_id msg_id, uart_cmd_type cmd, uint8_t *payload, uint32_t length);
bool UartInit(void);
bool UartTask(void);
#endif
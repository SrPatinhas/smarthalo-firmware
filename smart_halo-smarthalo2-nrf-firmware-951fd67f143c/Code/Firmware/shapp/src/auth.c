
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "nrf_dfu_settings.h"
#include "crc16.h"

#include "scheduler.h"
#include "bleapp.h"
#include "bslink.h"
#include "dispatch.h"
#include "record.h"
#include "device.h"
#include "keys.h"
#include "test.h"
#include "sha256.h"

#include "auth.h"
#include "uart.h"

#define KICKOUT_DELAY 60*1000

#define AUTH_PASSMAXLEN 32
uint8_t auth_password[AUTH_PASSMAXLEN] __attribute__ ((aligned (4)));
uint32_t auth_password_len;

uint8_t auth_central_pk[64];
uint8_t auth_secret[32];

uint8_t auth_seed[4];
bool auth_seed_generate = false;
volatile bool isConnected = false;

bool auth_isConnected(){
    return isConnected;
}

void auth_do_kickout(void *ctx) {
    bleapp_kickout();
}

static void reverse(uint8_t* p_out, uint8_t* p_in, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len ; i++)
    {
        p_out[i] = p_in[len - i - 1];
    }
}

uint8_t auth_fwversion[4] = { FWVERSION };

void auth_printVersion() {
    printf("FW: %d.%d.%d.%d\r\n", auth_fwversion[0],auth_fwversion[1],auth_fwversion[2],auth_fwversion[3]);
    uint32_t blversion = ((nrf_dfu_settings_t*)0x0007F000)->bootloader_version;
    printf("BL: %d.%d.%d.%d\r\n", (blversion >> 24) & 0xff,(blversion >> 16) & 0xff,(blversion >> 8) & 0xff,(blversion) & 0xff);
}

void auth_getVersions(uint8_t *reply, uint32_t len) {
    printf("auth_getVersions\r\n");
    len = 1;

    memcpy(reply+len, auth_fwversion, 4);
    len += 4;

    uint32_t blversion = ((nrf_dfu_settings_t*)0x0007F000)->bootloader_version;
    reply[len++] = (uint8_t)((blversion >> 24) & 0xff);
    reply[len++] = (uint8_t)((blversion >> 16) & 0xff);
    reply[len++] = (uint8_t)((blversion >> 8) & 0xff);
    reply[len++] = (uint8_t)((blversion) & 0xff);
}

void auth_getPeriphPubKey(uint8_t *buf, uint32_t len) {
    printf("auth_getPeriphPubKey\r\n");

    if(auth_seed_generate) {
        auth_seed_generate = false;
        nrf_drv_rng_block_rand(auth_seed, 4);
    }

    printf("auth_getPeriphPubKey, seed %02X%02X%02X%02X\r\n", auth_seed[0],auth_seed[1],auth_seed[2],auth_seed[3]);

    uint8_t reply[1+64+4];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    memcpy(reply+ptr, keys_getPubKey(), 64);
    ptr += 64;
    memcpy(reply+ptr, auth_seed, 4);
    ptr += 4;
    bslink_write(reply, ptr);
}

void auth_setCentralPubKey(uint8_t *buf, uint32_t len) {

    reverse(auth_central_pk, buf, 32);
    reverse(auth_central_pk+32, buf+32, 32);

    uint8_t secret[32];
    keys_shared_secret_compute(auth_central_pk, secret);
    reverse(auth_secret, secret, 32);
    memcpy(auth_secret+16, auth_seed, 4);
    bslink_setSharedSecret(auth_secret);

    printf("Secret: ");
    for(int i = 0; i < 32; i++) {
        printf("%02X ", auth_secret[i]);
    }
    printf("\r\n");


    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

void auth_authenticate(uint8_t *buf, uint32_t len) {
    uint8_t reply[1];
    uint32_t ptr = 0;
    if(len == auth_password_len && (memcmp((const char*)buf, (const char*)auth_password, auth_password_len) == 0)) {
        printf("auth_authenticate ok\r\n");
        sch_unique_cancel(auth_do_kickout);
        disp_setPermissionMask(0x01);
        bslink_onAuth();
        isConnected = true;
        uint8_t connection_msg[2];
        connection_msg[0] = UART_SYNC_CONNECTION;
        connection_msg[1] = BLE_CONNECTED;
        uart_put_tx_msg(UART_MSG_ID_MSG, UART_CMD_SYNC, connection_msg, 2);
        reply[ptr++] = RET_OK;
    } else {
        printf("auth_authenticate failed\r\n");
        reply[ptr++] = RET_FAIL;
    }
    bslink_write(reply, ptr);
}

void auth_setPassword(uint8_t *buf, uint32_t len) {
    uint8_t reply[1];
    uint32_t ptr = 0;

    printf("auth_setPassword\r\n");

    if((len != 0) && (len != AUTH_PASSMAXLEN) && (len != AUTH_PASSMAXLEN+2)) {
        printf("bad len\r\n");
        reply[ptr++] = RET_FAIL;
        bslink_write(reply, ptr);
        return;
    } else {
        
        if((len == AUTH_PASSMAXLEN+2)) {
            int crc = crc16_compute(buf, len, NULL);
            if(crc) {
                printf("bad crc\r\n");
                reply[ptr++] = RET_FAIL;
                bslink_write(reply, ptr);
                return;
            }
            len -= 2;  
        }

        uint8_t permission = disp_getPermissionMask();
        if(permission) {
            auth_password_len = len;
            memset(auth_password, 0, AUTH_PASSMAXLEN);
            if(len) {
                memcpy((char*)auth_password, (const char*)buf, len);
                rec_write(KEY_PASSWORD, auth_password, len, NULL);
                uint8_t paired_msg[2];
                paired_msg[0] = UART_SYNC_PAIRED;
                paired_msg[1] = BLE_PAIRED;
                uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,paired_msg,2);    
            } else {
                rec_delete(KEY_PASSWORD, NULL);
                device_onPasswordClear();
                uint8_t paired_msg[2];
                paired_msg[0] = UART_SYNC_PAIRED;
                paired_msg[1] = BLE_UNPAIRED;
                uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,paired_msg,2);    
            }
            reply[ptr++] = RET_OK;
        } else {
            printf("denied\r\n");
            reply[ptr++] = RET_DENIED;
        }

    }

    bslink_write(reply, ptr);
}

uint8_t auth_seed[4] = {0x11,0x22,0x33,0x44};

uint8_t auth_salt[] = {0x70,0x5f,0x71,0xc5,0x90,0x61,0x9e,0x0c,0xb9,0x9d,0x34,0x70,0x7d,0x23,0x60,0xd8};

void auth_cmd_getSeed(uint8_t *buf, uint32_t len) {
    uint8_t reply[1+4];
    uint32_t ptr = 0;

    printf("auth_cmd_getSeed\r\n");
    nrf_drv_rng_block_rand(auth_seed, 4);

    reply[ptr++] = RET_OK;

    memcpy(reply+ptr, auth_seed, 4);
    ptr += 4;

    bslink_write(reply, ptr);
}


void auth_cmd_resetPassword(uint8_t *buf, uint32_t len) {
    printf("auth_cmd_resetPassword\r\n");

    uint8_t challenge[32];

    sha256_context_t ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, auth_seed, 4);
    sha256_update(&ctx, auth_salt, 16);
    sha256_final(&ctx, challenge, 0);

    uint8_t reply[1];
    uint32_t ptr = 0;

    if(memcmp(challenge, buf, 32) == 0) {
        reply[ptr++] = RET_OK;
        rec_delete(KEY_PASSWORD, NULL);
    } else {
        reply[ptr++] = RET_FAIL;
    }

    bslink_write(reply, ptr);
    uint8_t paired_msg[2];
    paired_msg[0] = UART_SYNC_CONNECTION;
    paired_msg[1] = BLE_UNPAIRED;
    uart_put_tx_msg(UART_MSG_ID_MSG,UART_CMD_SYNC,paired_msg,2);    
}



#if DEBUG_GETPASSWORD
void auth_getPassword(uint8_t *buf, uint32_t len) {
    uint8_t reply[1+AUTH_PASSMAXLEN];
    uint32_t ptr = 0;
    memset(reply, 0, 1+AUTH_PASSMAXLEN);

    reply[ptr++] = RET_OK;
    memcpy(reply+1, auth_password, auth_password_len);
    ptr+=auth_password_len;

    bslink_write(reply, ptr);
}
#endif


disp_cmdfn_t auth_cmdfn[] = {
    auth_getVersions,
    auth_getPeriphPubKey,
    auth_setCentralPubKey,
    auth_authenticate,
    auth_setPassword,
    device_getSerial,
    auth_cmd_getSeed,
    auth_cmd_resetPassword,
#if DEBUG_GETPASSWORD
    auth_getPassword,
#endif
};
#define AUTH_CMDFN_CNT (sizeof(auth_cmdfn)/sizeof(disp_cmdfn_t))

void auth_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= AUTH_CMDFN_CNT) {
        uint8_t reply[1];
        uint32_t ptr = 0;
        reply[ptr++] = RET_UNIMPLEMENTED;
        bslink_write(reply, ptr);
    } else {
        auth_cmdfn[buf[0]](buf+1, len-1);
    }
}

void auth_evt_cb( ble_evt_t * p_ble_evt ) {
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GAP_EVT_CONNECTED:
            disp_setPermissionMask(0);
            sch_unique_oneshot(auth_do_kickout, KICKOUT_DELAY);
            auth_seed_generate = true;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            isConnected = false;
            break;
        default:
            break;
    }

}

void auth_init() {

    uint32_t err;

    auth_printVersion();

    memset(auth_password, 0, AUTH_PASSMAXLEN);

    auth_password_len = 0;
    err = rec_read(KEY_PASSWORD, auth_password, AUTH_PASSMAXLEN, &auth_password_len);
    if(err == FDS_ERR_NOT_FOUND) {
        printf("No password\r\n");
    } else {
        ERR_CHECK("KEY_PASSWORD", err);
    }

    disp_register(0, 0, auth_dispatch);
    bleapp_evt_register(auth_evt_cb);

}

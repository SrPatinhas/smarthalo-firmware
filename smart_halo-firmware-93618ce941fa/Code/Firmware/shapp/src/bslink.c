
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "aes.h"
#include "scheduler.h"
#include "bleapp.h"
#include "device.h"

#include "bslink.h"

#define BSL_UP_PAYLOAD_CNT 4
#define BSL_PAYLOAD_CNT 4
#define BSL_MTU 20

#define BLE_UUID_SH1_CTRL_UUID      0x0001
#define BLE_UUID_SH1_PAYLOAD_UUID_BASE      0x0010

#define BLE_UUID_SH1_UP_CTRL_UUID      0x0101
#define BLE_UUID_SH1_UP_PAYLOAD_UUID_BASE      0x0110

uint8_t *bslink_secret;
bool bslink_cryptData = false;

uint32_t bslink_payload_cnt;
uint32_t bslink_payload_crypted;
uint8_t bslink_ctrl[4];
uint8_t bslink_payload[BSL_PAYLOAD_CNT*BSL_MTU];
uint8_t bslink_payload_decrypted[BSL_PAYLOAD_CNT*BSL_MTU];
uint8_t bslink_payload_toencrypt[BSL_PAYLOAD_CNT*BSL_MTU];

uint8_t bslink_up_ctrl[4];
uint8_t bslink_up_payload[BSL_UP_PAYLOAD_CNT*BSL_MTU];
uint8_t bslink_up_payload_toencrypt[BSL_UP_PAYLOAD_CNT*BSL_MTU];

uint16_t    bslink_conn_handle;

ble_gatts_char_handles_t    bslink_ctrl_handles;
ble_gatts_char_handles_t    bslink_payload_handles[BSL_PAYLOAD_CNT];

ble_gatts_char_handles_t    bslink_up_ctrl_handles;
ble_gatts_char_handles_t    bslink_up_payload_handles[BSL_UP_PAYLOAD_CNT];

bool bslink_reply_crypted;
uint32_t bslink_reply_len;
uint32_t bslink_reply_step;
bool bslink_reply_pending;
bool bslink_reply_done;

bool bslink_up_reply_crypted;
uint32_t bslink_up_reply_len;
uint32_t bslink_up_reply_step;
bool bslink_up_reply_pending;
bool bslink_up_reply_done;

bslink_listen_cb_t bslink_listen_cb;

bool bslink_auth = false;

void bslink_onAuth() {
    bslink_auth = true;
}

void bslink_setSharedSecret(uint8_t *secret) {
    bslink_cryptData = true;
    bslink_secret = secret;
}

void bslink_listen(bslink_listen_cb_t cb) {
    bslink_listen_cb = cb;
}

#ifdef OVERRIDE_GAP_PARAM
bool bslink_first_rx = false;
#endif

void bslink_recv(void *ctx) {
    uint32_t len;

#ifdef OVERRIDE_GAP_PARAM
    if(bslink_first_rx) {
        bslink_first_rx = false;
        bleapp_gap_params_update();
    }
#endif

    if(bslink_payload_crypted) {
        AES128_CBC_decrypt_buffer(bslink_payload_decrypted, bslink_payload, bslink_payload_cnt, bslink_secret, bslink_secret+16);
        //remove padding
        len = bslink_payload_cnt - MIN(bslink_payload_cnt, bslink_payload_decrypted[bslink_payload_cnt-1]);
    } else {
        memcpy(bslink_payload_decrypted, bslink_payload, bslink_payload_cnt);
        len = bslink_payload_cnt;
    }

    printf("Recv: (%s) ", (bslink_payload_crypted) ? "crypted" : "plain");
    for(int i = 0; i < len; i++) {
        printf("%02X", bslink_payload_decrypted[i]);
    }
    printf("\r\n");

    if(bslink_listen_cb) {
        bslink_listen_cb(bslink_payload_decrypted, len);
    }    
}

#define BSLINK_TXTIMEOUT 3000

void bslink_tx_process(bool);

void bslink_tx_timeout(void *ctx) {
    bslink_tx_process(true);
}

void bslink_write(uint8_t *buf, uint32_t len) {
    uint32_t sendLen;

    if (bslink_conn_handle == BLE_CONN_HANDLE_INVALID) {
        return;
    }

    if(bslink_cryptData) {
        uint32_t padding = 16 - (len % 16);
        uint32_t lenPad = len+padding;
        if(lenPad > BSL_PAYLOAD_CNT*BSL_MTU) {
            APP_ERROR_CHECK(-1);
            return;
        }
        memcpy(bslink_payload_toencrypt, buf, len);
        for(int i = len; i < lenPad; i++) {
            bslink_payload_toencrypt[i] = padding;
        }

        AES128_CBC_encrypt_buffer(bslink_payload, bslink_payload_toencrypt, lenPad, bslink_secret, bslink_secret+16);
        sendLen = lenPad;
    } else {
        memcpy(bslink_payload, buf, len);
        sendLen = len;
    }   

    //send
    bslink_reply_pending = true;
    bslink_reply_len = sendLen;
    bslink_reply_crypted = bslink_cryptData;
    bslink_reply_step = 0;

    if(bslink_up_reply_done) { //no current transaction by notify

        bslink_reply_pending = false;
        bslink_reply_done = false;

        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.handle = bslink_payload_handles[bslink_reply_step].value_handle;
        bslink_reply_step++;
        int err = sd_ble_gatts_hvx(bslink_conn_handle, &hvx_params);
        if(err) {
            printf("bslink_write %d\r\n", err);
            bslink_reply_done = true;
            return;
        }
        sch_unique_oneshot(bslink_tx_timeout, BSLINK_TXTIMEOUT);
    } else {
        printf("bslink_write up not done\r\n");
    }

}

void bslink_up_write(uint8_t *buf, uint32_t len) {
    uint32_t sendLen;

    if (bslink_conn_handle == BLE_CONN_HANDLE_INVALID || !bslink_auth) {
        return;
    }

    if(!bslink_up_reply_done) {
        return; //Discard
    }

    if(dev_isTravelMode()) {
        return; //Discard
    }

    if(bslink_cryptData) {
        uint32_t padding = 16 - (len % 16);
        uint32_t lenPad = len+padding;
        if(lenPad > BSL_UP_PAYLOAD_CNT*BSL_MTU) {
            APP_ERROR_CHECK(-1);
            return;
        }
        memcpy(bslink_up_payload_toencrypt, buf, len);
        for(int i = len; i < lenPad; i++) {
            bslink_up_payload_toencrypt[i] = padding;
        }

        AES128_CBC_encrypt_buffer(bslink_up_payload, bslink_up_payload_toencrypt, lenPad, bslink_secret, bslink_secret+16);
        sendLen = lenPad;
    } else {
        memcpy(bslink_up_payload, buf, len);
        sendLen = len;
    }   

    //send
    bslink_up_reply_pending = true;
    bslink_up_reply_len = sendLen;
    bslink_up_reply_crypted = bslink_cryptData;
    bslink_up_reply_step = 0;

    if(bslink_reply_done) { //no current transaction by regular reply

        bslink_up_reply_pending = false;
        bslink_up_reply_done = false;

        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.handle = bslink_up_payload_handles[bslink_up_reply_step].value_handle;
        bslink_up_reply_step++;
        int err = sd_ble_gatts_hvx(bslink_conn_handle, &hvx_params);
        if(err) {
            printf("bslink_up_write %d\r\n", err);
            bslink_up_reply_done = true;
            return;
        }
        sch_unique_oneshot(bslink_tx_timeout, BSLINK_TXTIMEOUT);
    } else {
        printf("bslink_up_write not done\r\n");
    }

}

static void bslink_on_ble_write(ble_evt_t * p_ble_evt)
{
    //printf("bslink_on_ble_write %d\r\n", p_ble_evt->evt.gatts_evt.params.write.handle);
    if(p_ble_evt->evt.gatts_evt.params.write.handle == bslink_ctrl_handles.value_handle)
    {
        bslink_payload_cnt = MIN(bslink_ctrl[0], BSL_PAYLOAD_CNT*BSL_MTU);
        bslink_payload_crypted = bslink_ctrl[1];
        sch_unique_oneshot(bslink_recv, 0);
        //bslink_recv(NULL);
    }
}

void bslink_tx_process(bool timeout)
{
    int err;
    ble_gatts_hvx_params_t hvx_params;
    memset(&hvx_params, 0, sizeof(hvx_params));
    hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

    if(timeout) {
        printf("bslink_tx_process timeout! %d %d\r\n", bslink_reply_done, bslink_up_reply_done);
        bslink_reply_done = bslink_up_reply_done = true;
    }

    if(!bslink_reply_done) {
        if((bslink_reply_step*BSL_MTU) >= bslink_reply_len) {
            bslink_reply_done = true;
            bslink_ctrl[0] = bslink_reply_len;
            bslink_ctrl[1] = (bslink_reply_crypted) ? 1 : 0;
            hvx_params.handle = bslink_ctrl_handles.value_handle;
        } else {
            hvx_params.handle = bslink_payload_handles[bslink_reply_step].value_handle;
            bslink_reply_step++;
        }
        err = sd_ble_gatts_hvx(bslink_conn_handle, &hvx_params);
        if(err) {
            printf("!bslink_reply_done %d\r\n", err);
            bslink_reply_done = true;
            return;
        }
        sch_unique_oneshot(bslink_tx_timeout, BSLINK_TXTIMEOUT);
        return;
    }

    if(!bslink_up_reply_done) {
        if((bslink_up_reply_step*BSL_MTU) >= bslink_up_reply_len) {
            bslink_up_reply_done = true;
            bslink_up_ctrl[0] = bslink_up_reply_len;
            bslink_up_ctrl[1] = (bslink_up_reply_crypted) ? 1 : 0;
            hvx_params.handle = bslink_up_ctrl_handles.value_handle;
#ifdef BSLINK_INDICATE
            hvx_params.type = BLE_GATT_HVX_INDICATION;
#endif            
        } else {
            hvx_params.handle = bslink_up_payload_handles[bslink_up_reply_step].value_handle;
            bslink_up_reply_step++;
        }
        err = sd_ble_gatts_hvx(bslink_conn_handle, &hvx_params);
        if(err) {
            printf("!bslink_up_reply_done %d\r\n", err);
            bslink_up_reply_done = true;
            return;
        }
        sch_unique_oneshot(bslink_tx_timeout, BSLINK_TXTIMEOUT);
        return;
    }

    if(bslink_reply_pending) {

        bslink_reply_pending = false;
        bslink_reply_done = false;

        hvx_params.handle = bslink_payload_handles[bslink_reply_step].value_handle;
        bslink_reply_step++;
        err = sd_ble_gatts_hvx(bslink_conn_handle, &hvx_params);
        if(err) {
            printf("bslink_reply_pending %d\r\n", err);
            bslink_reply_done = true;
            return;
        }
        sch_unique_oneshot(bslink_tx_timeout, BSLINK_TXTIMEOUT);
        return;
    }

    if(bslink_up_reply_pending) {

        bslink_up_reply_pending = false;
        bslink_up_reply_done = false;

        hvx_params.handle = bslink_up_payload_handles[bslink_up_reply_step].value_handle;
        bslink_up_reply_step++;
        err = sd_ble_gatts_hvx(bslink_conn_handle, &hvx_params);
        if(err) {
            printf("bslink_up_reply_pending %d\r\n", err);
            bslink_up_reply_done = true;
            return;
        }
        sch_unique_oneshot(bslink_tx_timeout, BSLINK_TXTIMEOUT);
        return;
    }

    if(!timeout) {
        sch_unique_cancel(bslink_tx_timeout);
    }

}

/*
void bslink_test(void *ctx) {
    static int step = 0;
    bslink_ctrl[0] = step++;
    ble_gatts_hvx_params_t hvx_params;
    memset(&hvx_params, 0, sizeof(hvx_params));
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.handle = bslink_ctrl_handles.value_handle;
    sd_ble_gatts_hvx(bslink_conn_handle, &hvx_params);

    if(bslink_conn_handle != BLE_CONN_HANDLE_INVALID) {
        sch_unique_oneshot(bslink_test, 1000);
    }
}
*/
/*
void bslink_test(void *ctx) {
    uint8_t buf[] = "12345";
    bslink_up_write(buf, 5);
    sch_unique_oneshot(bslink_test, 1000);
}*/

void bslink_on_ble_evt(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GATTS_EVT_WRITE:
            bslink_on_ble_write(p_ble_evt);
            break;
        case BLE_GAP_EVT_CONNECTED:
            bslink_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            bslink_cryptData = false;
            bslink_auth = false;
#ifdef OVERRIDE_GAP_PARAM
            bslink_first_rx = true;
#endif
            //sch_unique_oneshot(bslink_test, 2000);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            bslink_conn_handle = BLE_CONN_HANDLE_INVALID;
            //sch_unique_cancel(bslink_test);
            break;
#ifdef BSLINK_INDICATE
        case BLE_GATTS_EVT_HVC:
            printf("Indicate received\r\n");
#endif
        case BLE_EVT_TX_COMPLETE:
            //bslink_on_tx_complete(p_ble_evt);
            //sch_unique_oneshot_ctx((sch_cb_ctx_t)bslink_on_tx_complete, 0, (void *)p_ble_evt);
            sch_unique_oneshot_ctx((sch_cb_ctx_t)bslink_tx_process, 0, (void *)false);
            break;
        default:
            // No implementation needed.
            break;
    }
}

void bslink_init()
{

    bslink_reply_done = true;
    bslink_reply_pending = false;
    bslink_up_reply_done = true;
    bslink_up_reply_pending = false;

    bslink_conn_handle = BLE_CONN_HANDLE_INVALID;

    APP_ERROR_CHECK(bleapp_char_add(BLE_UUID_SH1_CTRL_UUID, bslink_ctrl, 4, CHAR_PERM_READ | CHAR_PERM_WRITE | CHAR_FEAT_NOTIFY, &bslink_ctrl_handles));
    for(int i = 0; i < BSL_PAYLOAD_CNT; i++) {
        APP_ERROR_CHECK(bleapp_char_add(BLE_UUID_SH1_PAYLOAD_UUID_BASE+i, bslink_payload+(i*BSL_MTU), BSL_MTU, CHAR_PERM_READ | CHAR_PERM_WRITE | CHAR_FEAT_NOTIFY, &bslink_payload_handles[i]));    
    }

#ifdef BSLINK_INDICATE
    APP_ERROR_CHECK(bleapp_char_add(BLE_UUID_SH1_UP_CTRL_UUID, bslink_up_ctrl, 4, CHAR_PERM_READ | CHAR_PERM_WRITE | CHAR_FEAT_NOTIFY | CHAR_FEAT_INDICATE, &bslink_up_ctrl_handles));
#else
    APP_ERROR_CHECK(bleapp_char_add(BLE_UUID_SH1_UP_CTRL_UUID, bslink_up_ctrl, 4, CHAR_PERM_READ | CHAR_PERM_WRITE | CHAR_FEAT_NOTIFY, &bslink_up_ctrl_handles));
#endif
    for(int i = 0; i < BSL_UP_PAYLOAD_CNT; i++) {
        APP_ERROR_CHECK(bleapp_char_add(BLE_UUID_SH1_UP_PAYLOAD_UUID_BASE+i, bslink_up_payload+(i*BSL_MTU), BSL_MTU, CHAR_PERM_READ | CHAR_PERM_WRITE | CHAR_FEAT_NOTIFY, &bslink_up_payload_handles[i]));    
    }

    bleapp_evt_register(bslink_on_ble_evt);
}

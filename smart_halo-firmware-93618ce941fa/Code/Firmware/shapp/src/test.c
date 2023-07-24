
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "scheduler.h"
#include "sha256.h"
#include "record.h"
#include "bslink.h"
#include "dispatch.h"
#include "leds.h"
#include "sound.h"
#include "meters.h"
#include "touch.h"
#include "bleapp.h"
#include "battery.h"
#include "device.h"
#include "power.h"
#include "nrf_sdm.h"

#include "test.h"

#define TST_FACTORYLEN 4
uint8_t tst_factory[TST_FACTORYLEN] __attribute__ ((aligned (4)));

uint8_t tst_serial[SERIALMAXLEN] __attribute__ ((aligned (4)));

bool tst_isFactoryMode() {
    return tst_factory[0] == 0;    
}


/*
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_UICR->PSELRESET[0] = 21;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_UICR->PSELRESET[1] = 21;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NVIC_SystemReset();
*/

volatile uint32_t tst_protect_enable = 0;

void tst_protect_do(void *ctx) {
    sd_softdevice_disable();

    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_UICR->APPROTECT = 0;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

    NVIC_SystemReset();
}

void tst_protect_kickout(void *ctx) {
    bleapp_kickout();
    sch_unique_oneshot(tst_protect_do, 100);
}

//============================================================================
//

void tst_exitFactory_afterwrite(ret_code_t err) {
    sch_unique_oneshot(tst_protect_kickout, 500);
}

void tst_cmd_exitFactory(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_exitFactory\r\n");

    memset((char*)tst_factory, 0xFF, TST_FACTORYLEN);
    if(buf[0]) {
        rec_write(KEY_FACTORY, tst_factory, TST_FACTORYLEN, tst_exitFactory_afterwrite);
    } else {
        rec_write(KEY_FACTORY, tst_factory, TST_FACTORYLEN, NULL);
    }

    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

void tst_cmd_leds(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_leds\r\n");

    leds_test(buf[0],buf[1],buf[2],buf[3]);

    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

void tst_cmd_loopback(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_loopback\r\n");

    uint8_t reply[64];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;

    for(int i = 0; i < len; i++) {
        reply[ptr++] = buf[i] ^ 0xFF;
    }

    bslink_write(reply, ptr);
}

void tst_cmd_speed(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_speed\r\n");

    uint8_t reply[64];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;

    if(buf[0] == 0) {
        for(int i = 0; i < 62; i++) {
            reply[ptr++] = 0x00;
        }
    }

    bslink_write(reply, ptr);
}


void tst_cmd_setSerial(uint8_t *buf, uint32_t len) {
    uint8_t reply[1];
    uint32_t ptr = 0;

    printf("tst_cmd_setSerial\r\n");

    if(buf[0] < 3 && len >= 1) {
        int key = KEY_SERIAL_PRODUCT + buf[0];

        buf++;
        len--;
        len = MIN(len, SERIALMAXLEN-1);

        if(len) {
            memset(tst_serial, 0, SERIALMAXLEN);
            memcpy((char*)tst_serial, (const char*)buf, len);
            rec_write(key, tst_serial, len, NULL);
        } else {
            rec_delete(key, NULL);
        }
        reply[ptr++] = RET_OK;

    } else {
        reply[ptr++] = RET_FAIL;
    }

    bslink_write(reply, ptr);
}

void tst_cmd_met_do(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_met_do\r\n");

    met_selftest_do();

    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}


void tst_cmd_met_getState(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_met_getState\r\n");

    uint8_t reply[18];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;

    met_selftest_state_t * state = met_selftest_getState();

    reply[ptr++] = state->pending ? 1 : 0;

    reply[ptr++] = state->acc_success ? 1 : 0;

    reply[ptr++] = (uint8_t)((state->res_acc[0] >> 8) & 0xff);
    reply[ptr++] = (uint8_t)((state->res_acc[0]) & 0xff);

    reply[ptr++] = (uint8_t)((state->res_acc[1] >> 8) & 0xff);
    reply[ptr++] = (uint8_t)((state->res_acc[1]) & 0xff);

    reply[ptr++] = (uint8_t)((state->res_acc[2] >> 8) & 0xff);
    reply[ptr++] = (uint8_t)((state->res_acc[2]) & 0xff);

    reply[ptr++] = state->mag_success ? 1 : 0;

    reply[ptr++] = (uint8_t)((state->res_mag[0] >> 8) & 0xff);
    reply[ptr++] = (uint8_t)((state->res_mag[0]) & 0xff);

    reply[ptr++] = (uint8_t)((state->res_mag[1] >> 8) & 0xff);
    reply[ptr++] = (uint8_t)((state->res_mag[1]) & 0xff);

    reply[ptr++] = (uint8_t)((state->res_mag[2] >> 8) & 0xff);
    reply[ptr++] = (uint8_t)((state->res_mag[2]) & 0xff);

    reply[ptr++] = state->int_success ? 1 : 0;

    reply[ptr++] = state->agr ? 1 : 0;

    bslink_write(reply, ptr);
}


void tst_cmd_touch_do(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_touch_do\r\n");

    touch_test_do();

    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}


void tst_cmd_touch_getState(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_touch_getState\r\n");

    uint8_t reply[5];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;

    touch_test_state_t * state = touch_test_getState();
    reply[ptr++] = state->pending ? 1 : 0;
    reply[ptr++] = state->success ? 1 : 0;
    reply[ptr++] = state->selfcnt;
    reply[ptr++] = state->stepcnt;

    bslink_write(reply, ptr);
}


void tst_cmd_battery(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_battery\r\n");

    uint8_t reply[7];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;

    reply[ptr++] = (bat_isCharging()) ? 1 : 0;
    reply[ptr++] = (uint8_t)bat_getSOC();

    int16_t voltage = bat_getVoltage();
    int16_t current = bat_getCurrent_uA() / 1000.;

    reply[ptr++] = (uint8_t)(voltage >> 8);
    reply[ptr++] = (uint8_t)(voltage & 0xFF);
    reply[ptr++] = (uint8_t)(current >> 8);
    reply[ptr++] = (uint8_t)(current & 0xFF);

    bslink_write(reply, ptr);
}

void tst_cmd_power(uint8_t *buf, uint32_t len) {
    printf("tst_cmd_power\r\n");

    pwr_test(buf[0], buf[1]);

    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

disp_cmdfn_t tst_cmdfn[] = {
    tst_cmd_exitFactory,
    tst_cmd_leds,
    tst_cmd_loopback,
    tst_cmd_speed,
    tst_cmd_setSerial,
    sound_cmd_play,
    sound_cmd_stop,
    tst_cmd_met_do,
    tst_cmd_met_getState,
    tst_cmd_touch_do,
    tst_cmd_touch_getState,
    tst_cmd_battery,
    device_getSerial,
    tst_cmd_power,
};
#define TST_CMDFN_CNT (sizeof(tst_cmdfn)/sizeof(disp_cmdfn_t))

void tst_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= TST_CMDFN_CNT) {
        uint8_t reply[1];
        uint32_t ptr = 0;
        reply[ptr++] = RET_UNIMPLEMENTED;
        bslink_write(reply, ptr);
    } else {
        tst_cmdfn[buf[0]](buf+1, len-1);
    }
}

//============================================================================
//

uint8_t tst_auth_seed[4] = {0x11,0x22,0x33,0x44};

uint8_t tst_salt[] = {0x3f, 0x62, 0x38, 0xc4, 0xe3, 0x54, 0x87, 0x3a, 0x4b, 0xed, 0x12, 0x3a, 0xd7, 0x9e, 0xdf, 0x5d};

void tst_auth_cmd_getSeed(uint8_t *buf, uint32_t len) {
    uint8_t reply[1+4];
    uint32_t ptr = 0;

    printf("tst_auth_cmd_getSeed\r\n");
    nrf_drv_rng_block_rand(tst_auth_seed, 4);

    reply[ptr++] = RET_OK;

    memcpy(reply+ptr, tst_auth_seed, 4);
    ptr += 4;

    bslink_write(reply, ptr);
}


void tst_auth_cmd_enterFactory(uint8_t *buf, uint32_t len) {
    printf("tst_auth_cmd_enterFactory\r\n");

    uint8_t challenge[32];

    sha256_context_t ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, tst_auth_seed, 4);
    sha256_update(&ctx, tst_salt, 16);
    sha256_final(&ctx, challenge, 0);

    uint8_t reply[1];
    uint32_t ptr = 0;

    if(memcmp(challenge, buf, 32) == 0) {
        reply[ptr++] = RET_OK;
        memset((char*)tst_factory, 0x00, TST_FACTORYLEN);
        rec_write(KEY_FACTORY, tst_factory, TST_FACTORYLEN, NULL);
    } else {
        reply[ptr++] = RET_FAIL;
    }

    bslink_write(reply, ptr);
}


disp_cmdfn_t tst_auth_cmdfn[] = {
    tst_auth_cmd_getSeed,
    tst_auth_cmd_enterFactory,
};

#define TST_AUTH_CMDFN_CNT (sizeof(tst_auth_cmdfn)/sizeof(disp_cmdfn_t))

void tst_auth_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= TST_AUTH_CMDFN_CNT) {
        uint8_t reply[1];
        uint32_t ptr = 0;
        reply[ptr++] = RET_UNIMPLEMENTED;
        bslink_write(reply, ptr);
    } else {
        tst_auth_cmdfn[buf[0]](buf+1, len-1);
    }
}

//============================================================================
//

void tst_init() {

    memset(tst_factory, 0, TST_FACTORYLEN);
    int err = rec_read(KEY_FACTORY, tst_factory, TST_FACTORYLEN, NULL);
    if(err == FDS_ERR_NOT_FOUND) {
        //
    } else {
        ERR_CHECK("KEY_FACTORY", err);
    }

    disp_register(0xF0, 2, tst_dispatch);
    disp_register(0xF1, 1, tst_auth_dispatch);

}

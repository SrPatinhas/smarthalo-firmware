
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"
#include "nrf_dfu_settings.h"

#include "scheduler.h"
#include "record.h"
#include "bleapp.h"
#include "bslink.h"
#include "dispatch.h"
#include "bootinfo.h"
#include "nrf_soc.h"
#include "nrf_bootloader_info.h"
#include "sdk_macros.h"

#include "device.h"

#define DEVICE_NAME                      "smarthalo"                          /**< Name of device. Will be included in the advertising data. */

bool dev_travelMode = false;

bool dev_isTravelMode() {
    return dev_travelMode;
}

//==============================================================================
// Dispatch

void device_jumptoBootloader(void *ctx) {
    printf("device_jumptoBootloader\r\n");
    bleapp_jumptoBootloader();
}

void device_flashDone() {
    sch_unique_oneshot(device_jumptoBootloader, 500);
}

bool device_enterBootloader() {
    printf("device_enterBootloader\r\n");

    uint32_t err_code;
    err_code = sd_power_gpregret_clr(0, 0xffffffff);
    VERIFY_SUCCESS(err_code);

    err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
    VERIFY_SUCCESS(err_code);

    sch_unique_oneshot(device_jumptoBootloader, 1000);

    return true;
}

void device_getState(uint8_t *buf, uint32_t len) {
}

void device_compass_calibrate(uint8_t *buf, uint32_t len) {
}

#define NAMEMAXLEN 19
uint8_t device_name[NAMEMAXLEN+1] __attribute__ ((aligned (4)));

bool device_setName(uint8_t *buf, uint32_t len) {
    printf("device_setName\r\n");
    memset(device_name, 0, NAMEMAXLEN+1);
    if(!len) {
        strncpy((char *)device_name, DEVICE_NAME, NAMEMAXLEN);
    } else {
        strncpy((char *)device_name, (char *)buf, MIN(len, NAMEMAXLEN));
    }
    bleapp_setName(device_name);
    rec_write(KEY_DEVNAME, device_name, NAMEMAXLEN+1, NULL);

    return true;
}


void device_getSerial(uint8_t *buf, uint32_t len) {
    uint8_t reply[1+SERIALMAXLEN];
    uint32_t ptr = 0;
    memset(reply, 0, 1+SERIALMAXLEN);

    printf("auth_getSerial\r\n");

    if(buf[0] < 3) {
        int key = KEY_SERIAL_PRODUCT + buf[0];
        int err = rec_read(key, reply+1, SERIALMAXLEN, NULL);
        if(!err) {
            reply[ptr++] = RET_OK;
            ptr+= strlen((char*)reply+1);
        } else {
            reply[ptr++] = RET_FAIL;
        }
    } else {
        reply[ptr++] = RET_FAIL;
    }

    bslink_write(reply, ptr);
}

void device_setTravelMode(uint8_t *buf, uint32_t len) {
    printf("device_setTravelMode\r\n");
    dev_travelMode = (buf[0] == 1);
    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

//==============================================================================
//

void device_onPasswordClear() {
    memset(device_name, 0, NAMEMAXLEN+1);
    strncpy((char *)device_name, DEVICE_NAME, NAMEMAXLEN);
    bleapp_setName(device_name);
    rec_write(KEY_DEVNAME, device_name, NAMEMAXLEN+1, NULL);
}


void device_init() {
    uint32_t err;

    memset(device_name, 0, NAMEMAXLEN+1);
    err = rec_read(KEY_DEVNAME, device_name, NAMEMAXLEN+1, NULL);
    printf("device_init %d %s\r\n", err, device_name);

    if(device_name[0]) {
        bleapp_setName(device_name);
    } else {
        bleapp_setName((uint8_t *)DEVICE_NAME);
    }

    binfo_init();
}

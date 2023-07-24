
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
#include "meters.h"
#include "battery.h"
#include "leds.h"
#include "bootinfo.h"

#include "device.h"

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

void device_enterBootloader(uint8_t *buf, uint32_t len) {
    printf("device_enterBootloader\r\n");
    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);

    uint32_t binfo = binfo_getWord();
    binfo = BINFO_STAY_BL(binfo, true);
    binfo_setWord(binfo, device_flashDone);
}

void device_do_shutdown(void *ctx) {
    bleapp_shutdown();
}

void device_shutdown(uint8_t *buf, uint32_t len) {
    printf("device_shutdown\r\n");
    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
    sch_unique_oneshot(device_do_shutdown, 500);
}

void device_getState(uint8_t *buf, uint32_t len) {
    printf("device_getState\r\n");
    uint8_t reply[10];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;

    //Battery
    reply[ptr++] = (uint8_t)bat_getSOC();
    reply[ptr++] = (bat_isCharging()) ? 1 : 0;

    //Compass
    int32_t compass = met_getCompass();
    reply[ptr++] = (compass >> 8) & 0xff;
    reply[ptr++] = compass & 0xff;

    //Frontlight
    reply[ptr++] = leds_frontlight_get() ? 1 : 0;
    reply[ptr++] = leds_frontlight_getState() ? 1 : 0;

    //Temperature
    reply[ptr++] = (int8_t)met_getTemperature();

    //Battery is plugged
    reply[ptr++] = bat_isUSBPlugged() ? 1 : 0;

    //Travel mode
    reply[ptr++] = dev_isTravelMode() ? 1 : 0;

    bslink_write(reply, ptr);
}

void device_compass_calibrate(uint8_t *buf, uint32_t len) {
    printf("device_compass_calibrate\r\n");
    met_compass_calibrate();
    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

#define NAMEMAXLEN 19
uint8_t device_name[NAMEMAXLEN+1] __attribute__ ((aligned (4)));

void device_setName(uint8_t *buf, uint32_t len) {
    printf("device_setName\r\n");
    memset(device_name, 0, NAMEMAXLEN+1);
    if(!len) {
        strncpy((char *)device_name, "smarthalo", NAMEMAXLEN);
    } else {
        strncpy((char *)device_name, (char *)buf, MIN(len, NAMEMAXLEN));
    }
    bleapp_setName(device_name);
    rec_write(KEY_DEVNAME, device_name, NAMEMAXLEN+1, NULL);

    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
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
    if(dev_travelMode) {
        leds_turnOff_lights();
    }
    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

disp_cmdfn_t device_cmdfn[] = {
    device_enterBootloader,
    device_getState,
    device_setName,
    device_compass_calibrate,
    device_shutdown,
    device_getSerial,
    device_setTravelMode,
};
#define DEVICE_CMDFN_CNT (sizeof(device_cmdfn)/sizeof(disp_cmdfn_t))

void device_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= DEVICE_CMDFN_CNT) {
        uint8_t reply[1];
        uint32_t ptr = 0;
        reply[ptr++] = RET_UNIMPLEMENTED;
        bslink_write(reply, ptr);
    } else {
        device_cmdfn[buf[0]](buf+1, len-1);
    }
}

//==============================================================================
//

void device_onPasswordClear() {
    memset(device_name, 0, NAMEMAXLEN+1);
    strncpy((char *)device_name, "smarthalo", NAMEMAXLEN);
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
        bleapp_setName((uint8_t *)"smarthalo");
    }

    //nrf_dfu_flash_init(true);
    //nrf_dfu_settings_init();

    disp_register(4, 1, device_dispatch);

}


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"
#include "nrf_dfu_settings.h"

#include "scheduler.h"
#include "bleapp.h"
#include "bslink.h"
#include "dispatch.h"
#include "meters.h"

#include "device.h"


//==============================================================================
// Dispatch

void device_jumptoBootloader(void *ctx) {

    printf("device_jumptoBootloader\r\n");
    bleapp_jumptoBootloader();

}

void device_flash_callback(fs_evt_t const * const evt, fs_ret_t result)
{
    uint8_t reply[1];
    uint32_t ptr = 0;
    printf("device_flash_callback\r\n");

    if (result == FS_SUCCESS)
    {

        sch_unique_oneshot(device_jumptoBootloader, 500);
        reply[ptr++] = RET_OK;

    } else {
        reply[ptr++] = RET_FAIL;
    }

    bslink_write(reply, ptr);

}


void device_enterBootloader(uint8_t *buf, uint32_t len) {
    uint32_t err;
    s_dfu_settings.config |= 1;
    err = nrf_dfu_settings_write(device_flash_callback);
    printf("device_enterBootloader %d\r\n", err);
}

void device_capture(uint8_t *buf, uint32_t len) {
    printf("device_capture\r\n");
    met_capture_enable = (buf[0] == 1);
    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

disp_cmdfn_t device_cmdfn[] = {
    device_enterBootloader,
    device_capture,
    (disp_cmdfn_t)NULL
};
uint8_t device_cmdfn_len;

void device_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= device_cmdfn_len)
        return; //drop
    device_cmdfn[buf[0]](buf+1, len-1);
}

//==============================================================================
//

void device_init() {

    device_cmdfn_len = 0;
    while(device_cmdfn_len < 0xff && device_cmdfn[device_cmdfn_len] != NULL) {
        device_cmdfn_len++;
    }
    disp_register(3, 1, device_dispatch);

    nrf_dfu_flash_init(true);
    nrf_dfu_settings_init();
}

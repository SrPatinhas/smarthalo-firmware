
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "touch.h"
#include "sound.h"
#include "twi.h"

#include "power.h"

extern bool sound_active;
extern bool touch_active;
extern bool bleapp_connected;

bool pwr_testmode = false;

platform_hw_t pwr_hw;

bool pwr_s2_8_getState() {
	return (sound_active || touch_active || bleapp_connected);
}

void pwr_update() {
    if(pwr_testmode) {
        return;
    }
    if(pwr_hw == HW_V12) {
        nrf_gpio_pin_write(V12_EN_QTOUCH, (pwr_s2_8_getState()) ? 1 : 0);
        nrf_gpio_pin_write(V12_EN_VPIEZO_LDO, (pwr_s2_8_getState()) ? 1 : 0);
    } else {
        nrf_gpio_pin_write(V10_EN_2_8V, (pwr_s2_8_getState()) ? 1 : 0);
    }
    nrf_gpio_pin_write(TOUCH_MODE_PIN, (pwr_s2_8_getState()) ? 1 : 0);
    nrf_gpio_pin_write(V12_EN_ACCEL, 1);
}

void pwr_shutdown() {
    if(pwr_hw == HW_V12) {
    	nrf_gpio_pin_write(V12_EN_QTOUCH, 0);
    	nrf_gpio_pin_write(V12_EN_VPIEZO_LDO, 0);
        nrf_gpio_pin_write(V12_EN_ACCEL, 0);
    } else {
        nrf_gpio_pin_write(V10_EN_2_8V, 0);
    }
    nrf_gpio_pin_write(TOUCH_MODE_PIN, 0);
}

void pwr_test(int reset, int enable) {
    if(reset) {
        pwr_testmode = false;
        pwr_update();
    } else {
        twi_shutdown();
        pwr_testmode = true;
        int i = (enable) ? 1 : 0;
        if(pwr_hw == HW_V12) {
            nrf_gpio_pin_write(V12_EN_QTOUCH, i);
            nrf_gpio_pin_write(V12_EN_VPIEZO_LDO, i);
            nrf_gpio_pin_write(V12_EN_ACCEL, i);
        } else {
            nrf_gpio_pin_write(V10_EN_2_8V, i);
        }
        nrf_gpio_pin_write(TOUCH_MODE_PIN, i);
        nrf_gpio_pin_write(EN_VLED, i);
    }
}

void pwr_init() {
    pwr_hw = platform_getHW();

    if(pwr_hw == HW_V12) {
        nrf_gpio_cfg_output(V12_EN_QTOUCH);
        nrf_gpio_pin_write(V12_EN_QTOUCH, 0);
        nrf_gpio_cfg_output(V12_EN_VPIEZO_LDO);
        nrf_gpio_pin_write(V12_EN_VPIEZO_LDO, 0);

        nrf_gpio_cfg_output(V12_EN_ACCEL);
        nrf_gpio_pin_write(V12_EN_ACCEL, 1);
    } else {
        nrf_gpio_cfg_output(V10_EN_2_8V);
        nrf_gpio_pin_write(V10_EN_2_8V, 0);
    }

    nrf_gpio_cfg_output(TOUCH_MODE_PIN);
    nrf_gpio_pin_write(TOUCH_MODE_PIN, 0);

}

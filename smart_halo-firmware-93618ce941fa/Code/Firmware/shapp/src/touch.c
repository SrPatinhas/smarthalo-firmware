
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "scheduler.h"
#include "bslink.h"
#include "leds.h"
#include "alarm.h"
#include "sound.h"
#include "power.h"
#include "battery.h"
#include "ui.h"
#include "device.h"

#include "touch.h"

bool touch_active = false;

#if !defined(PLATFORM_pca10040) && !defined(PLATFORM_shmp)

uint32_t touch_code;
uint32_t touch_codeLen;
uint8_t touch_volume = 40;
bool touch_sound_enabled = true;

void touch_offDampener(void *ctx) {
    touch_active = false;
    pwr_update();
}

void touch_enable(){
    touch_active = true;
    pwr_update();
}

void touch_movement_on() {
    sch_unique_cancel(touch_offDampener);    
    sch_unique_oneshot(touch_enable, 1);
}

void touch_movement_off() {
    sch_unique_oneshot(touch_offDampener, 15000);
}

void touch_sound(uint8_t volume, bool isEnabled){
    touch_volume = volume;
    touch_sound_enabled = isEnabled;
}

void touch_handleCode(void *ctx) {
    leds_touch_complete();
    if(!alarm_codeHandle(touch_code, touch_codeLen)) {
        if(!leds_codeHandle(touch_code, touch_codeLen) && !sound_codeHandle(touch_code, touch_codeLen)) {
            if(touch_codeLen) {
                uint8_t buf[3];
                uint32_t ptr = 0;
                buf[ptr++] = NOTIFY_TOUCH;
                buf[ptr++] = touch_code;
                buf[ptr++] = touch_codeLen;
                bslink_up_write(buf, ptr);
            }
            char codeString[touch_codeLen];
            ui_analyze_touch_sequence(codeString, touch_code, touch_codeLen);

            if(!strcmp(codeString, "SSSLSSSL") &&
                !alarm_isEnabled()) {
                sound_easteregg();
            }
        }
    }

    touch_code = 0;
    touch_codeLen = 0;
}

#define TOUCH_LIMIT 8

void touch_event(bool active, touch_type_t type) {
    //printf("touch_event!\r\n");
    sch_unique_cancel(touch_handleCode);
    if(!active) {
        sch_unique_oneshot(touch_handleCode, 500);
        if(touch_codeLen < TOUCH_LIMIT) {
            touch_code += (type << touch_codeLen);
            touch_codeLen++;
        }
    } else {
        sch_unique_oneshot(touch_handleCode, 5000);
    }
    leds_touch(active, type, touch_code, touch_codeLen);
    if(ui_isAuth() && touch_sound_enabled){
        if(!type && active){
            sch_unique_oneshot(sound_touch, 0);
        }else if(type && active){
            sch_unique_oneshot(sound_touch_long, 0);
        }
    }
    touch_movement_on();
}

void touch_read(void *ctx) {
    static int val = 0;
    static bool touch = false;
    static touch_type_t type;
    static int touchTime = 0;
    static int onCnt = 0;

    val = nrf_gpio_pin_read(TOUCH_OUT_PIN);

    if(!touch && val) {
        touch = true;
        type = TOUCH_SHORT;
        touchTime = 0;
        printf("touch!\r\n");
        touch_event(true, type);
        onCnt=5*100;
    }

    if(touch && type == TOUCH_SHORT) {
        if(touchTime > 300) {
            type = TOUCH_LONG;
            touch_event(true, type);
        }
    }

    if(touch && !val) {
        touch = false;
        printf("untouch! %d ms\r\n", touchTime);
        touch_event(false, type);
        onCnt=5*100;
        touch_movement_off();
    }

    if(onCnt) {
        onCnt--;
        sch_unique_oneshot(touch_read, 10);
        touchTime+=10;
    } else {
        touch = false;
    }
}

//==============================================================================
//

bool touch_test_active = false;
bool touch_test_self = false;
touch_test_state_t touch_test_state;


void touch_test_done(void *ctx) {
    touch_test_active = false;
    touch_test_state.success = (touch_test_state.stepcnt==4) && (touch_test_state.selfcnt>1);
}

void touch_test_read(void *ctx) {
    static int val = 0;
    static bool touch = false;

    val = nrf_gpio_pin_read(TOUCH_OUT_PIN);

    if(!touch && val) {
        touch = true;
        sch_unique_oneshot(touch_test_read, 10);
        leds_test_touch(true, touch_test_state.stepcnt);
    }

    if(touch && !val) {
        touch = false;
        touch_test_state.stepcnt++;
        if(touch_test_state.stepcnt==4) {
            leds_anim_off(ANIM_TEST_TOUCH);
            sch_unique_oneshot(touch_test_done, 0);
        } else {
            leds_test_touch(false, touch_test_state.stepcnt);
        }
    }
}

touch_test_state_t *touch_test_getState() {
    touch_test_state.pending = touch_test_active;
    return &touch_test_state;
}

void touch_test_self_done(void *ctx) {
    touch_test_self = false;
    NRF_GPIO->PIN_CNF[TOUCH_OUT_PIN] &= ~(GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos);
    sch_unique_oneshot(touch_test_done, 20000);
    leds_test_touch(false, 0);
}

void touch_test_do() {
    touch_test_active = true;
    touch_test_self = true;
    touch_test_state.selfcnt = 0;
    touch_test_state.stepcnt = 0;
    touch_test_state.success = false;
    printf("touch: %08X\r\n", NRF_GPIO->PIN_CNF[TOUCH_OUT_PIN]);
    NRF_GPIO->PIN_CNF[TOUCH_OUT_PIN] |= (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);
    sch_unique_oneshot(touch_test_self_done, 1000);
}

//==============================================================================
//

static void touch_int_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    if(pwr_s2_8_getState() && !dev_isTravelMode()) {
        if(touch_test_active) {
            if(touch_test_self) {
                touch_test_state.selfcnt++;
            } else {
                sch_unique_oneshot(touch_test_read, 0);
            }
        } else {
            sch_unique_oneshot(touch_read, 0);
        }
    }
}

//==============================================================================
//

void touch_init() {
    ret_code_t err;

    if(!nrf_drv_gpiote_is_init())
    {
        err = nrf_drv_gpiote_init();
        ERR_CHECK("nrf_drv_gpiote_init", err);
    }

    //nrf_gpio_cfg_input(TOUCH_OUT_PIN, NRF_GPIO_PIN_NOPULL);
    //sch_unique_oneshot(touch_read, 1000);
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    config.pull = NRF_GPIO_PIN_NOPULL;//NRF_GPIO_PIN_PULLUP; //NRF_GPIO_PIN_NOPULL
    //config.pull = NRF_GPIO_PIN_PULLUP;//NRF_GPIO_PIN_PULLUP; //NRF_GPIO_PIN_NOPULL
    err = nrf_drv_gpiote_in_init(TOUCH_OUT_PIN, &config, touch_int_handler);
    ERR_CHECK("touch int init", err);
    nrf_drv_gpiote_in_event_enable(TOUCH_OUT_PIN, true);

    touch_active = true;
    pwr_update();

    sch_unique_oneshot(touch_offDampener, 30000);

}

#endif

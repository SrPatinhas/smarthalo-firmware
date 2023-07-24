
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "scheduler.h"
#include "sha256.h"
#include "record.h"
#include "dispatch.h"
#include "leds.h"
#include "sound.h"
#include "meters.h"
#include "touch.h"
#include "bleapp.h"
#include "battery.h"
#include "device.h"
#include "ui.h"
#include "nrf_sdm.h"
#include "animscript.h"

#include "exp.h"

//============================================================================

uint32_t exp_disableTime;

void exp_ble_disable(void *ctx) {
    uint32_t time_left = exp_disableTime;
    sd_softdevice_disable();
    __disable_irq();
    while (time_left >= 1){
        printf("time left : %d \r\n", time_left);
        if(time_left > (0xFFFFFFFF/1000)){
            nrf_delay_ms((uint32_t)(0xFFFFFFFF/1000));
            time_left = time_left - (0xFFFFFFFF/1000);
        }
        else{
            nrf_delay_ms((uint32_t)time_left);
            time_left = 0;
        }
    }
    NVIC_SystemReset();
}

void exp_cmd_ble_disable(uint8_t *buf, uint32_t len) {
    printf("exp_cmd_ble_disable\r\n");

    exp_disableTime = ((uint32_t)buf[0]<<24) + ((uint32_t)buf[1] << 16) + ((uint32_t)buf[2] << 8) + ((uint32_t)buf[3]); 
    printf("disable time : %d \r\n", exp_disableTime);

    disp_respond(RET_OK);

    sch_unique_oneshot(exp_ble_disable, 500);
}

void exp_cmd_cadence(uint8_t *buf, uint32_t len) {
    
    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t v = buf[2];
    float cadence = ((int16_t)(((uint32_t)buf[3] << 8) + (uint32_t)buf[4]));
    leds_cadence(h,s,v,cadence);

    printf("ui_cadence %d\r\n",(int32_t)cadence);

    disp_respond(RET_OK);
}

void exp_cmd_cadence_off(uint8_t *buf, uint32_t len) {

    leds_cadence_off();

    printf("ui_cadence_off\r\n");

    disp_respond(RET_OK);
}

void exp_ui_cmd_nav_destination_angle(uint8_t *buf, uint32_t len) {
    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t l = buf[2];
    float heading = (int16_t)(((uint32_t)buf[3] << 8) + (uint32_t)buf[4]);
    uint32_t progress = buf[5];

    leds_nav_destination_angle(h, s, l, heading, progress);

    disp_respond(RET_OK);
}

void exp_ui_cmd_clock(uint8_t *buf, uint32_t len){
    if(len < 13){
        disp_respond(RET_FAIL);
        return;
    }
    uint8_t hour = buf[0];
    uint8_t hour_h = buf[1];
    uint8_t hour_s = buf[2];
    uint8_t hour_l = buf[3];
    uint8_t minute = buf[4];
    uint8_t minute_h = buf[5];
    uint8_t minute_s = buf[6];
    uint8_t minute_l = buf[7];
    uint8_t center_h = len == 16 ? buf[8] : minute_h;
    uint8_t center_s = len == 16 ? buf[9] : minute_s;
    uint8_t center_l = len == 16 ? buf[10] : minute_l;
    uint16_t duration = len == 16 
        ? (uint16_t)(((uint16_t) buf[11] << 8) + (uint16_t) buf[12]) 
        : (uint16_t)(((uint16_t) buf[8] << 8) + (uint16_t) buf[9]);
    bool fade = len == 16 ? buf[13] : buf[10];
    bool intro = len == 16 ? buf[14] : buf[11];
    bool pulse = len == 16 ? buf[15] : buf[12];

    leds_clock(hour, hour_h, hour_s, hour_l, minute, minute_h, minute_s, minute_l, center_h, center_s, center_l, duration, fade, intro, pulse);

    disp_respond(RET_OK);
}

void exp_ui_cmd_clock_off(){
    leds_clock_off();

    disp_respond(RET_OK);
}

void exp_ui_cmd_touch_sound(uint8_t *buf, uint32_t len){
    uint8_t volume = buf[0];
    bool isEnabled = buf[1];    
    touch_sound(volume,isEnabled);

    disp_respond(RET_OK);
}

void exp_ui_cmd_front_external_toggle(uint8_t *buf, uint32_t len){
    bool isRequired = buf[0];      
    leds_front_external_toggle(isRequired);

    disp_respond(RET_OK);
}

void exp_ui_cmd_show_state_of_charge(uint8_t *buf, uint32_t len){
    leds_batlevel_show();

    disp_respond(RET_OK);
}

void exp_ui_cmd_turn_by_turn_intro(uint8_t *buf, uint32_t len){
    if(len < 1){
        disp_respond(RET_DENIED);
        return;
    }
    uint8_t mode = buf[0];
    leds_turn_intro_play(mode);

    disp_respond(RET_OK);
}

void exp_ui_cmd_pointer_intro(uint8_t *buf, uint32_t len){
    if(len < 4){
        disp_respond(RET_DENIED);
        return;
    }

    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t v = buf[2];
    uint8_t intro_mode = len == 6 ? buf[5] : buf[3];
    float heading = len == 6 ? (int16_t)(((uint32_t)buf[3] << 8) + (uint32_t)buf[4]) : 0;
    uint8_t standby = len == 6 ? 0 : 1;

    leds_pointer(h,s,v,heading,standby,intro_mode);

    disp_respond(RET_OK);
}

//============================================================================

disp_cmdfn_t exp_cmdfn[] = {
    exp_cmd_ble_disable,
    ui_cmd_pointer, //exp_cmd_pointer,
    ui_cmd_pointer_turnOff, //exp_cmd_pointer_turnOff,
    ui_cmd_nav_angle_2,
    ui_cmd_pointer_standby, //exp_cmd_pointer_standby,         
    exp_cmd_cadence,
    exp_cmd_cadence_off,
    ui_cmd_speedometer_intro,
    ui_cmd_fitness_intro,
    ascr_cmd_load,
    ascr_cmd_run,
    ascr_cmd_stop,
    ui_cmd_leds_demo,
    exp_ui_cmd_nav_destination_angle,
    exp_ui_cmd_clock,
    exp_ui_cmd_clock_off,
    exp_ui_cmd_touch_sound,
    exp_ui_cmd_front_external_toggle,
    exp_ui_cmd_show_state_of_charge,
    exp_ui_cmd_turn_by_turn_intro,
    exp_ui_cmd_pointer_intro,
};
#define TST_CMDFN_CNT (sizeof(exp_cmdfn)/sizeof(disp_cmdfn_t))

void exp_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= TST_CMDFN_CNT) {
        disp_respond(RET_UNIMPLEMENTED);
    } else {
        exp_cmdfn[buf[0]](buf+1, len-1);
    }
}

//============================================================================
//

void exp_init() {

    disp_register(0xF8, 1, exp_dispatch);

}

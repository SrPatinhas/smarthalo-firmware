
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "scheduler.h"
#include "bleapp.h"
#include "dispatch.h"
#include "leds.h"
#include "meters.h"
#include "alarm.h"
#include "battery.h"
#include "sound.h"
#include "touch.h"
#include "device.h"

#include "ui.h"

bool ui_demoMode = false;
bool ui_isDemoMode() {
    return ui_demoMode;
}

//============================================================================
// Comm


//This function will return the code in the form of short(S) and long(L) taps
void ui_analyze_touch_sequence(char *codeString, uint8_t code, uint8_t length){
    for(uint8_t i=0; i<length; i++){
        codeString[i] = code >> i & 0x1 ? 'L' : 'S';
    }
}

void ui_cmd_nav_show(uint8_t *buf, uint32_t len) {
    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t l = buf[2];
    leds_nav_heading_t heading = buf[3];
    uint32_t progress = buf[4];

    if (heading <= 8 && heading >= 0) {
        leds_nav_show(h, s, l, heading, progress);
        disp_respond(RET_OK);
    } else {
        disp_respond(RET_FAIL);
    }  
}

void ui_cmd_nav_roundAbout(uint8_t *buf, uint32_t len) {
    uint32_t exitNow = buf[0];
    uint32_t driveRight = buf[1];
    uint8_t h1,s1,l1,h2,s2,l2;

    h1 = buf[2];
    s1 = buf[3];
    l1 = buf[4];
    h2 = buf[5];
    s2 = buf[6];
    l2 = buf[7];

    uint32_t number_exits = MIN((len - 8)>>1, 10);
    uint8_t j = 8;
    float angle_exits[10];
    
    for (uint8_t i = 0; i < number_exits; i++) {
        angle_exits[i] = (int16_t)(((uint32_t)buf[j] << 8) + (uint32_t)buf[j+1]);
        j+=2;
    }

    if (number_exits > 0) {
        leds_nav_roundAbout(exitNow,driveRight,h1,s1,l1,h2,s2,l2,number_exits,angle_exits);
        disp_respond(RET_OK);
    } else {
        disp_respond(RET_FAIL);
    }
}

void ui_nav_angle(bool compass, uint8_t *buf, uint32_t len) {
    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t l = buf[2];
    float heading = (int16_t)(((uint32_t)buf[3] << 8) + (uint32_t)buf[4]);
    uint32_t progress = buf[5];
    uint8_t nt_h = 0;
    uint8_t nt_s = 0;
    uint8_t nt_l = l;
    uint32_t compass_mode = 0;

    float nt_heading;
    uint32_t nt_progress;

    if(len == 6) {
        nt_heading = 0;
        nt_progress = 0;
    } else if(len == 9 || len == 10) {
        nt_heading = ((int16_t)(((uint32_t)buf[6] << 8) + (uint32_t)buf[7]));
        nt_progress = buf[8];
    } else {
        nt_h = buf[6];
        nt_s = buf[7];
        nt_l = buf[8];

        nt_heading = ((int16_t)(((uint32_t)buf[9] << 8) + (uint32_t)buf[10]));
        nt_progress = buf[11];

        compass_mode = (compass) ? buf[12] : 0;
    }

    leds_nav_angle(h, s, l, heading, progress, nt_h, nt_s, nt_l, nt_heading, nt_progress, compass_mode);

    disp_respond(RET_OK);
}

void ui_cmd_nav_angle(uint8_t *buf, uint32_t len) {
    ui_nav_angle(false, buf, len);
}
void ui_cmd_nav_angle_2(uint8_t *buf, uint32_t len) {
    ui_nav_angle(true, buf, len);
}

void ui_cmd_nav_reroute(uint8_t *buf, uint32_t len) {
    leds_nav_reroute();

    disp_respond(RET_OK);
}

void ui_cmd_nav_off(uint8_t *buf, uint32_t len) {
    leds_nav_off();
    leds_nav_roundAbout_off();

    disp_respond(RET_OK);
}

void ui_cmd_frontlight(uint8_t *buf, uint32_t len) {
    if ((buf[0] >= 0 && buf[0] <= 1) && (len < 2 || (buf[1] >= 0 && buf[1] <= 1))){    
        uint32_t on = buf[0];
        uint32_t bg = (len == 2) ? buf[1] : 0;
        leds_frontlight(on, bg);
        disp_respond(RET_OK);
    }

    else{
        disp_respond(RET_FAIL);
    }    
}

void ui_cmd_frontlight_settings(uint8_t *buf, uint32_t len) {
    if (buf[0] >= 0 && buf[0] <= 2){
        leds_front_mode_t mode = buf[0];
        uint32_t brightness = buf[1];
        leds_frontlight_settings(mode, brightness);
        disp_respond(RET_OK);
    }
    else{
        disp_respond(RET_FAIL);
    }
}

void ui_cmd_progress(uint8_t *buf, uint32_t len) {
    uint8_t h1 = buf[0];
    uint8_t s1 = buf[1];
    uint8_t l1 = buf[2];
    uint8_t h2 = buf[3];
    uint8_t s2 = buf[4];
    uint8_t l2 = buf[5];
    uint32_t cycle = ((uint32_t)buf[6] << 8) + (uint32_t)buf[7];
    uint32_t progress = buf[8];
    uint32_t fitness = (len > 9) ? buf[9] : 1;

    leds_progress(h1,s1,l1,h2,s2,l2,cycle,progress,fitness);

    disp_respond(RET_OK);
}

void ui_cmd_progress_off(uint8_t *buf, uint32_t len) {
    leds_progress_off();

    disp_respond(RET_OK);
}

void ui_cmd_speedometer(uint8_t *buf, uint32_t len) {
    uint32_t speed = buf[0];
    leds_speedometer(speed,false);

    disp_respond(RET_OK);
}

void ui_cmd_speedometer_off(uint8_t *buf, uint32_t len) {
    leds_speedometer_off();

    disp_respond(RET_OK);
}

void ui_cmd_notif(uint8_t *buf, uint32_t len) {
    uint8_t h1 = buf[0];
    uint8_t s1 = buf[1];
    uint8_t l1 = buf[2];
    uint8_t h2 = buf[3];
    uint8_t s2 = buf[4];
    uint8_t l2 = buf[5];
    uint32_t fadein = ((uint32_t)buf[6] << 8) + (uint32_t)buf[7];
    uint32_t on = ((uint32_t)buf[8] << 8) + (uint32_t)buf[9];
    uint32_t fadeout = ((uint32_t)buf[10] << 8) + (uint32_t)buf[11];
    uint32_t off = ((uint32_t)buf[12] << 8) + (uint32_t)buf[13];
    uint32_t repeat = buf[14];
    leds_notif(h1,s1,l1,h2,s2,l2,fadein,on,fadeout,off,repeat);

    disp_respond(RET_OK);
}

void ui_cmd_notif_off(uint8_t *buf, uint32_t len) {

    leds_notif_off();

    disp_respond(RET_OK);
}

void ui_cmd_hb(uint8_t *buf, uint32_t len) {

    uint8_t h1 = buf[0];
    uint8_t s1 = buf[1];
    uint8_t l1 = buf[2];
    uint8_t h2 = buf[3];
    uint8_t s2 = buf[4];
    uint8_t l2 = buf[5];
    uint32_t fadein = ((uint32_t)buf[6] << 8) + (uint32_t)buf[7];
    uint32_t on = ((uint32_t)buf[8] << 8) + (uint32_t)buf[9];
    uint32_t fadeout = ((uint32_t)buf[10] << 8) + (uint32_t)buf[11];
    uint32_t off = ((uint32_t)buf[12] << 8) + (uint32_t)buf[13];
    leds_hb(h1,s1,l1,h2,s2,l2,fadein,on,fadeout,off);

    disp_respond(RET_OK);
}

void ui_cmd_hb_off(uint8_t *buf, uint32_t len) {

    leds_hb_off();

    disp_respond(RET_OK);
}

void ui_cmd_compass(uint8_t *buf, uint32_t len) {

    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t l = buf[2];
    float heading = (int16_t)(((uint32_t)buf[3] << 8) + (uint32_t)buf[4]);
    leds_compass(h,s,l,heading);

    disp_respond(RET_OK);
}

void ui_cmd_compass_off(uint8_t *buf, uint32_t len) {

    leds_compass_off();

    disp_respond(RET_OK);
}

void ui_cmd_pointer_standby(uint8_t *buf, uint32_t len) {
    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t v = buf[2];
    uint8_t standby = 1;
    uint8_t heading = 0;

    leds_pointer(h,s,v,heading,standby,false);

    disp_respond(RET_OK);
}


void ui_cmd_pointer_turnOff(uint8_t *buf, uint32_t len) {
    leds_pointer_off();

    disp_respond(RET_OK);
}

void ui_cmd_pointer(uint8_t *buf, uint32_t len) {
    uint8_t h = buf[0];
    uint8_t s = buf[1];
    uint8_t v = buf[2];
    float heading = (int16_t)(((uint32_t)buf[3] << 8) + (uint32_t)buf[4]);
    uint8_t standby = 0;

    leds_pointer(h,s,v,heading,standby,false);

    disp_respond(RET_OK);
}

void ui_cmd_logo(uint8_t *buf, uint32_t len) {

    leds_logo();
    disp_respond(RET_OK);
}

void ui_cmd_disconnect(uint8_t *buf, uint32_t len) {

    leds_disconnect();
    disp_respond(RET_OK);
}

void ui_cmd_lowBat(uint8_t *buf, uint32_t len) {
    leds_battery_low();
    sound_battery_low(buf[0]);
    disp_respond(RET_OK);
}

void ui_cmd_leds_off(uint8_t *buf, uint32_t len) {

    leds_anim_allOff();

    disp_respond(RET_OK);
}


void ui_cmd_setBrightness(uint8_t *buf, uint32_t len) {

    uint32_t brightness = buf[0];
    leds_setBrightness(brightness);

    disp_respond(RET_OK);
}


void ui_cmd_leds_demo(uint8_t *buf, uint32_t len) {
    uint8_t id = buf[0];
    uint8_t arg1 = buf[1];
    uint8_t arg2 = buf[2];

    ui_demoMode = true;

    int testcase = 0;
    if(id == testcase++) {
        leds_anim_off(ANIM_ALARM);
        leds_anim_off(ANIM_ALARM_ARM);
        leds_anim_off(ANIM_ALARM_DISARM);
    } else 
    if(id == testcase++) {
        leds_alarm_arm();
    } else 
    if(id == testcase++) {
        leds_alarm_disarm();
    } else 
    if(id == testcase++) {
        leds_alarm_trigger(false, (float)MIN(arg1,100)/100.f );
    }
    if(id == testcase++) {
        leds_alarm_trigger(true, 0);
    }
    if(id == testcase++) {
        leds_touch(true, 0, arg1, arg2);
    }
    if(id == testcase++) {
        leds_touch(false, 0, 0, 0);
    }
    if(id == testcase++) {
        leds_battery(arg1);
    }
    if(id == testcase++) {
        leds_demo_intro();
    }
    if(id == testcase++) {
        leds_demo_intro_off();
    }

    disp_respond(RET_OK);
}   

void ui_cmd_speedometer_intro(uint8_t *buf, uint32_t len) {
    leds_speedometer(0,true);

    disp_respond(RET_OK);
}

void ui_cmd_fitness_intro(uint8_t *buf, uint32_t len) {
    uint8_t h1 = buf[0];
    uint8_t s1 = buf[1];
    uint8_t v1 = buf[2];
    uint8_t h2 = buf[3];
    uint8_t s2 = buf[4];
    uint8_t v2 = buf[5];
    uint32_t period = ((uint32_t)buf[6] << 8) + (uint32_t)buf[7];
    leds_fitness_intro(h1,s1,v1,h2,s2,v2,period);

    disp_respond(RET_OK);
}   

void ui_cmd_clock(uint8_t *buf, uint32_t len){
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

void ui_cmd_clock_off(){
    leds_clock_off();

    disp_respond(RET_OK);
}

void ui_cmd_front_external_toggle(uint8_t *buf, uint32_t len){
    bool isRequired = buf[0];      
    leds_front_external_toggle(isRequired);

    disp_respond(RET_OK);
}

void ui_cmd_show_state_of_charge(uint8_t *buf, uint32_t len){
    leds_batlevel_show();

    disp_respond(RET_OK);
}

void ui_cmd_turn_by_turn_intro(uint8_t *buf, uint32_t len){
    if(len < 1){
        disp_respond(RET_FAIL);
        return;
    }
    uint8_t mode = buf[0];
    leds_turn_intro_play(mode);

    disp_respond(RET_OK);
}

void ui_cmd_pointer_intro(uint8_t *buf, uint32_t len){
    if(len < 4){
        disp_respond(RET_FAIL);
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

disp_cmdfn_t ui_cmdfn[] = {
    ui_cmd_logo,
    ui_cmd_nav_show,
    ui_cmd_nav_reroute,
    ui_cmd_nav_off,
    ui_cmd_frontlight,
    ui_cmd_progress,
    ui_cmd_progress_off,
    ui_cmd_notif,
    ui_cmd_notif_off,
    ui_cmd_hb,
    ui_cmd_hb_off,
    ui_cmd_compass,
    ui_cmd_compass_off,
    ui_cmd_disconnect,
    ui_cmd_leds_off,
    ui_cmd_setBrightness,
    ui_cmd_frontlight_settings,
    ui_cmd_nav_angle,
    ui_cmd_speedometer,
	ui_cmd_speedometer_off,
    ui_cmd_nav_roundAbout,
    ui_cmd_lowBat,
    ui_cmd_pointer,
    ui_cmd_pointer_turnOff,
    ui_cmd_pointer_standby,
    ui_cmd_leds_demo,
    ui_cmd_speedometer_intro,
    ui_cmd_fitness_intro,
    ui_cmd_clock,
    ui_cmd_clock_off,
    ui_cmd_front_external_toggle,
    ui_cmd_show_state_of_charge,
    ui_cmd_turn_by_turn_intro,
    ui_cmd_pointer_intro,
};
#define UI_CMDFN_CNT (sizeof(ui_cmdfn)/sizeof(disp_cmdfn_t))

void ui_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= UI_CMDFN_CNT) {
        disp_respond(RET_UNIMPLEMENTED);
    } else {
        ui_cmdfn[buf[0]](buf+1, len-1);
    }
}

//============================================================================
//

bool ui_pending_logo = false;
bool ui_authentificated = false;

bool ui_isAuth() {
    return ui_authentificated;    
}

void ui_onLogoAnimation() {
    ui_pending_logo = false;
}

void ui_trigger_logo(void *ctx) {
    leds_logo();
}

void ui_onAuth() {
    if(met_isMoving() && !alarm_isEnabled()) {
        sch_unique_oneshot(ui_trigger_logo, 0);
    } else {
        ui_pending_logo = true;
    }
    ui_authentificated = true;
}

void ui_onAlarm_disarm() {
    if(met_isMoving()) {
        sch_unique_oneshot(ui_trigger_logo, 0);
    } else {
        ui_pending_logo = true;
    }
}

void ui_onMovement() {
    if(ui_pending_logo && !alarm_isEnabled()) {
        ui_pending_logo = false;
        sch_unique_oneshot(ui_trigger_logo, 0);
    }
}

void ui_onDisconnect(void *ctx) {
    if(!dev_isTravelMode()) {
        leds_disconnect();
    }
    leds_turnOff_lights();
}

//============================================================================
//

void ui_evt_cb( ble_evt_t * p_ble_evt ) {
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GAP_EVT_CONNECTED:
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            ui_demoMode = false;
            ui_pending_logo = false;
            if(ui_authentificated) {
                ui_authentificated = false;
                sch_unique_oneshot(ui_onDisconnect, 0);
            }
            leds_setBrightness(50);
            break;
        default:
            break;
    }

}

//============================================================================
//

void ui_init() {

    touch_init();
    leds_init();
    met_init();

    disp_register(1, 1, ui_dispatch);
    bleapp_evt_register(ui_evt_cb);

}

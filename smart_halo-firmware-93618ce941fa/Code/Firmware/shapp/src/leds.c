
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "platform.h"

#include "scheduler.h"
#include "bleapp.h"
#include "bslink.h"
#include "dispatch.h"
#include "twi.h"
#include "meters.h"
#include "record.h"
#include "battery.h"
#include "ui.h"
#include "alarm.h"
#include "sound.h"
#include "animscript.h"

#include "leds.h"

#define DRAWMS 16

#define LEDS_CNT 24

#define TWIENABLE
#define PWMENABLE

#define FRONTLIGHT_ON_CYCLE     30

#define LEDS_MAXCURRENT_DIV     2

#define LEDS_PERC_TURN_BY_TURN 0.3

#define MOD(a,b) ((((a)%(b))+(b))%(b))

#define ABS(a,b) ((a>=b)? a-b : MOD(a-b,LEDS_CNT))

#define ABS_LEDS(a,b) ((MOD((b-a),LEDS_CNT) >= MOD((a-b),LEDS_CNT))? MOD((a-b),LEDS_CNT) : MOD((b-a),LEDS_CNT))

#define ABS_LEDS_LOOP(a,b) ((MOD((b-a),LEDS_CNT) >= MOD((a-b),LEDS_CNT))? (MOD((a-b),LEDS_CNT)): -1 * (MOD((b-a),LEDS_CNT)))

typedef void (*leds_anim_fn_t)(bool);

typedef struct {
    uint8_t brightness;
    uint8_t frontlight_mode;
    uint8_t frontlight_brightness;
    //uint8_t frontlight_on;
} leds_settings_t;

leds_settings_t leds_settings __attribute__ ((aligned (4)));

bool leds_movement_active = false;

//============================================================================
// lookups & gamma
const uint8_t leds_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

uint8_t leds_pixel_lut[] = { LEDS_LUT };

uint32_t leds_toPWMAddr(uint32_t out) {
    //out index already 1 based
    if(out > 36) {
        return out+1;
    }
    return out;
}

//============================================================================
// Buffers & utils

void leds_HSVtoRGB(uint8_t *in_h, uint8_t *in_s, uint8_t *in_v, uint8_t *rgb)
{
    uint8_t region, fpart, p, q, t, r, g, b;
    uint8_t h = *in_h;
    uint8_t s = *in_s;
    uint8_t v = *in_v;
    
    if(s == 0) {
        /* color is grayscale */
        r = g = b = v;
    } else {    
        /* make hue 0-5 */
        region = h / 43;
        /* find remainder part, make it from 0-255 */
        fpart = (h - (region * 43)) * 6;
        
        /* calculate temp vars, doing integer multiplication */
        p = (v * (255 - s)) >> 8;
        q = (v * (255 - ((s * fpart) >> 8))) >> 8;
        t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;
            
        /* assign temp vars based on color cone region */
        switch(region) {
            case 0:
                r = v; g = t; b = p; break;
            case 1:
                r = q; g = v; b = p; break;
            case 2:
                r = p; g = v; b = t; break;
            case 3:
                r = p; g = q; b = v; break;
            case 4:
                r = t; g = p; b = v; break;
            default:
                r = v; g = p; b = q; break;
        }
    }
    
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
    
    return;
}


uint8_t leds_buffer_rgb[LEDS_CNT*3];

uint8_t leds_central_rgb[3];

uint8_t leds_front_lum;

void leds_util_buf_init() {
    leds_front_lum = 0;
    memset(leds_central_rgb, 0, 3);
    memset(leds_buffer_rgb, 0, LEDS_CNT*3);
}

void leds_util_central_copyHSVtoRGB(uint8_t h, uint8_t s, uint8_t v) {
    leds_HSVtoRGB(&h, &s, &v, leds_central_rgb);
}

void leds_util_copyHSVtoRGB(uint8_t *h, uint8_t *s, uint8_t *v) {
    for(int i = 0; i < LEDS_CNT; i++) {
        leds_HSVtoRGB(h+i, s+i, v+i, leds_buffer_rgb+(3*i));
    }
  
}

//============================================================================
// State animations

float leds_anim_brightness;
float leds_anim_goal_brightness;
float leds_anim_start_brightness;

#define LEDS_ANIM_NEXT_FRAME() do{ frame++; step = 0; }while(0)

uint8_t leds_logo_hues[LEDS_CNT] = {167, 151, 144, 139, 131, 121, 111, 101, 91, 77, 65, 56, 49, 43, 38, 32, 24, 14, 2, 245, 237, 232, 221, 196};
uint8_t leds_logo_sats[LEDS_CNT] = {125, 187, 237, 225, 210, 196, 188, 181, 175, 167, 182, 195, 208, 220, 233, 219, 203, 187, 171, 200, 237, 209, 168, 133};

void leds_anim_step_still(bool reset) {
    uint8_t val[LEDS_CNT];
    memset(val, 255, LEDS_CNT);
    leds_util_copyHSVtoRGB(leds_logo_hues,leds_logo_sats,val);
}

#define FADE_STEP_LOGO      30
#define LEDS_LOGO_OFFSET    6

void leds_anim_step_logo(bool reset) {
    static uint8_t hue[LEDS_CNT];
    static uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    uint32_t maxlum = leds_anim_brightness * 255.f;

    if(reset) {
        frame = 0;
        step = 0;
    }

    switch(frame) {
        case 0: {
            memset(hue, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            val[0] = maxlum>>4;
            val[1] = maxlum>>3;
            val[2] = maxlum>>2;
            val[3] = maxlum>>1;
            val[4] = maxlum;
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            val[step] = 0;
            val[step+1] = maxlum>>4;
            val[step+2] = maxlum>>3;
            val[step+3] = maxlum>>2;
            val[step+4] = maxlum>>1;
            val[step+5] = maxlum;
            step++;
            if(step == 19) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            val[step] = maxlum;
            step++;
            if(step == LEDS_CNT) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            float lvl = (1.f/((float) FADE_STEP_LOGO))*(float)step;
            memset(val, maxlum-((float)maxlum*lvl), LEDS_CNT);
            step++;
            if(step > FADE_STEP_LOGO) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 4: {
            for(int i = 0; i < LEDS_CNT; i++) {
                hue[i] = leds_logo_hues[(i+LEDS_LOGO_OFFSET)%LEDS_CNT];
                sat[i] = leds_logo_sats[(i+LEDS_LOGO_OFFSET)%LEDS_CNT];
            }
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 5: {
            float lvl = (1.f/((float) FADE_STEP_LOGO*2))*(float)step;
            memset(val, ((float)maxlum*lvl), LEDS_CNT);
            step++;
            if(step > FADE_STEP_LOGO*2) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 6: {
            float lvl = (1.f/((float) FADE_STEP_LOGO*2))*(float)step;
            memset(val, maxlum-((float)maxlum*lvl), LEDS_CNT);
            step++;
            if(step > FADE_STEP_LOGO*2) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 7: {
            leds_anim_off(ANIM_LOGO);
            break;
        }
    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define DEMO_INTRO_DIV1 1
#define DEMO_INTRO_DIV2 2
#define DEMO_INTRO_FADE 2
bool leds_param_demo_intro_turnOff = false;
void leds_anim_step_demo_intro(bool reset) {
    static uint8_t hue[LEDS_CNT];
    static uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t lum = 0;
    uint32_t maxlum = leds_anim_brightness * 255.f;

    if(reset) {
        frame = 0;
        step = 0;
        lum = maxlum;
    }

    switch(frame) {
        case 0: {
            memset(val, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {

            if(leds_param_demo_intro_turnOff) {
                lum = (lum>DEMO_INTRO_FADE) ? lum-DEMO_INTRO_FADE : 0;
            }
            for(int i = 0; i < LEDS_CNT; i++) {
                hue[i] = leds_logo_hues[(LEDS_CNT-1) - (i+(step>>DEMO_INTRO_DIV2))%LEDS_CNT];
                sat[i] = leds_logo_sats[(LEDS_CNT-1) - (i+(step>>DEMO_INTRO_DIV2))%LEDS_CNT];
                val[i] = (val[i]>DEMO_INTRO_FADE) ? val[i]-DEMO_INTRO_FADE : 0;
            }

            val[(step>>DEMO_INTRO_DIV1)%LEDS_CNT] = lum;
            step++;
            step%=LEDS_CNT<<DEMO_INTRO_DIV2;

            if(!lum) {
                leds_anim_off(ANIM_DEMO_INTRO);
            }
            break;
        }
    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}



#define FADE_IN_STEP_DISCONNECT     30
#define FADE_OUT_STEP_DISCONNECT    60
#define LEDS_DISCONNECT_OFFSET      6

void leds_anim_step_disconnect(bool reset) {
    static uint8_t hue[LEDS_CNT];
    static uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    uint32_t maxlum = leds_anim_brightness * 255.f;

    if(reset) {
        frame = 0;
        step = 0;
    }

    switch(frame) {
        case 0: {
            memset(val, 0, LEDS_CNT);
            for(int i = 0; i < LEDS_CNT; i++) {
                hue[i] = leds_logo_hues[(i+LEDS_DISCONNECT_OFFSET)%LEDS_CNT];
                sat[i] = leds_logo_sats[(i+LEDS_DISCONNECT_OFFSET)%LEDS_CNT];
            }
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            float lvl = (1.f/((float)FADE_IN_STEP_DISCONNECT))*(float)step;
            memset(val, maxlum*lvl, LEDS_CNT);
            step++;
            if(step > FADE_IN_STEP_DISCONNECT) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            float lvl = (1.f/((float)FADE_OUT_STEP_DISCONNECT))*(float)step;
            memset(val, 255*(leds_anim_brightness*(1.f-lvl)), LEDS_CNT);
            for(int i = 0; i < LEDS_CNT; i++) {
                sat[i] = leds_logo_sats[(i+LEDS_DISCONNECT_OFFSET)%LEDS_CNT] - ((float)(leds_logo_sats[(i+LEDS_DISCONNECT_OFFSET)%LEDS_CNT]) * lvl);
            }
            step++;
            if(step > FADE_OUT_STEP_DISCONNECT) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            leds_anim_off(ANIM_DISCONNECT);
            break;
        }
    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define CLOCK_MINUTE_BLINK_STEPS    10
#define CLOCK_FADE_IN_STEPS  38
#define CLOCK_FADE_OUT_STEPS 38
uint8_t leds_clock_hour;
uint8_t leds_clock_hour_h;
uint8_t leds_clock_hour_s;
uint8_t leds_clock_hour_l;
uint8_t leds_clock_minute;
uint8_t leds_clock_minute_h;
uint8_t leds_clock_minute_s;
uint8_t leds_clock_minute_l;
uint8_t leds_clock_center_s;
uint8_t leds_clock_center_l;
uint8_t leds_clock_center_h;
uint16_t leds_clock_duration_steps;
bool leds_clock_fade;
bool leds_clock_intro;
bool leds_clock_pulse;
bool leds_clock_turn_off; 

void leds_animation_step_clock(bool reset){

    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];
    
    memset(hue, 0, LEDS_CNT);
    memset(sat, 0, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    static int32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t fade_step = 0;
    static uint32_t duration_steps = 0;

    float hour_lum = leds_anim_brightness * leds_clock_hour_l;
    float minute_lum = leds_anim_brightness * leds_clock_minute_l;
    uint8_t central_hue = leds_clock_center_h;
    uint8_t central_sat = leds_clock_center_s;
    uint8_t central_val = leds_anim_brightness * leds_clock_center_l;
    
    if(reset) {
        step = 0;
        duration_steps = 0;
        fade_step = 0;
        if(leds_clock_intro){
            frame = -2;
        }else{
            frame = 0;    
        }
    }

    //if the clock is turned off, fade everything off.
    if(leds_clock_turn_off){
        frame = 5;
    }

    uint32_t hour_final_position = (leds_clock_hour*2 + leds_clock_minute/32)%24;         
    hue[hour_final_position] = leds_clock_hour_h;
    sat[hour_final_position] = leds_clock_hour_s;
    val[hour_final_position] = hour_lum;
    uint32_t minute_final_position = leds_clock_minute*2/5;
    uint8_t final_blink_steps = CLOCK_MINUTE_BLINK_STEPS + CLOCK_MINUTE_BLINK_STEPS*(hour_final_position==minute_final_position);
    if(minute_final_position == hour_final_position){
        leds_clock_fade = false;
        leds_clock_pulse = false;
    }

    if(leds_clock_pulse){
        if(frame == 2){
            frame++;
        }
    }else if((frame == 1 || frame  == 3) && !leds_clock_fade) {
        frame++;
    }

    if(fade_step < CLOCK_FADE_IN_STEPS){
        float fade = (float) fade_step/CLOCK_FADE_IN_STEPS;
        central_val = (float) central_val*fade;
        hour_lum = (float) hour_lum*fade;
        minute_lum = (float) minute_lum*fade;
        fade_step++;
    }

    switch(frame){
        case -2:{
            memset(hue, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);

            float hour_speed = 1.0f;
            uint32_t hour_position = step*hour_speed;
            hue[hour_position%24] = leds_clock_hour_h;
            sat[hour_position%24] = (float)leds_clock_hour_s;
            val[hour_position%24] = hour_lum;
            step++;
            if(hour_position == (leds_clock_hour*2 + leds_clock_minute/32)){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;     
        }
        case -1:{
            memset(hue, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);

            float minute_speed = 1.0f;
            uint32_t minute_position = leds_clock_hour*2 + leds_clock_minute/32 + step*minute_speed;
            hue[hour_final_position] = leds_clock_hour_h;
            sat[hour_final_position] = leds_clock_hour_s;
            val[hour_final_position] = hour_lum;
            hue[minute_position%24] = leds_clock_minute_h;
            sat[minute_position%24] = (float)leds_clock_minute_s;
            val[minute_position%24] = (float)minute_lum;
            step++;
            if(minute_position%24 == (leds_clock_minute*2/5 == 0 ? 23 : leds_clock_minute*2/5-1)){
                LEDS_ANIM_NEXT_FRAME();
                frame = 4;
            }
            break;     
        }
        case 0:{
            float fade = (float) step/CLOCK_FADE_IN_STEPS;
            hue[hour_final_position] = leds_clock_hour_h;
            sat[hour_final_position] = (float)leds_clock_hour_s*fade;
            val[hour_final_position] = hour_lum;
            hue[minute_final_position] = leds_clock_minute_h;
            sat[minute_final_position] = (float)leds_clock_minute_s*fade;
            val[minute_final_position] = (float)minute_lum;     
            step++;       
            if(step == CLOCK_FADE_IN_STEPS){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 1:{
            float fade = 1.0 - (float) step/CLOCK_FADE_OUT_STEPS;
            hue[minute_final_position] = leds_clock_minute_h;
            sat[minute_final_position] = (float)leds_clock_minute_s;
            val[minute_final_position] = (float)minute_lum*fade;            
            step++;
            if(step == CLOCK_FADE_OUT_STEPS/2){
                LEDS_ANIM_NEXT_FRAME();
            }    
            break;  
        }
        case 2:{
            step++;
            if(step == final_blink_steps){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3:{
            float fade = 0.5f + (float)step/CLOCK_FADE_IN_STEPS;
            hue[minute_final_position] = leds_clock_minute_h;
            sat[minute_final_position] = (float)leds_clock_minute_s;
            val[minute_final_position] = (float)minute_lum*fade;            
            step++;
            if(step == CLOCK_FADE_IN_STEPS/2){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 4:{
            hue[minute_final_position] = leds_clock_minute_h;
            sat[minute_final_position] = leds_clock_minute_s;
            val[minute_final_position] = minute_lum;    
            step++;
            if(step == final_blink_steps){
                LEDS_ANIM_NEXT_FRAME();
                if(duration_steps < leds_clock_duration_steps){
                    frame = 1;
                }
            }
            break;
        }
        case 5:{
            float fade = 1.0 - (float) step/CLOCK_FADE_OUT_STEPS;
            hue[hour_final_position] = (float)leds_clock_hour_h*fade;
            sat[hour_final_position] = (float)leds_clock_hour_s;
            val[hour_final_position] = (float)hour_lum*fade;
            hue[minute_final_position] = leds_clock_minute_h;
            sat[minute_final_position] = (float)leds_clock_minute_s;
            val[minute_final_position] = (float)minute_lum*fade;     
            central_val = (float)central_val*fade;
            
            step++;
            if(step == CLOCK_FADE_OUT_STEPS){
                leds_anim_off(CLOCK_ANIMATION);
                memset(hue, 0, LEDS_CNT);
                memset(sat, 0, LEDS_CNT);
                memset(val, 0, LEDS_CNT);
            }
            break;
        }
    }
    duration_steps++;

    leds_util_copyHSVtoRGB(hue, sat, val);
    leds_util_central_copyHSVtoRGB(central_hue, central_sat, central_val);
}

#define LEDS_BATTERY_OFFSET     LEDS_CNT/2
#define FADE_IN_STEP_BATTERY    10
#define FADE_OUT_STEP_BATTERY   30
#define WAIT_STEP_BATTERY       120
#define WAIT_STEP_BATTERY_FULL  3
#define BATTERY_FULL_STEPS      30
#define BATTERY_REVERSE_STEPS   30

uint8_t leds_is_charged = false;
float leds_bat_param_soc = 0.5;

void leds_anim_step_battery(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    uint32_t maxlum = 50.0f;

    if(reset) {
        frame = 0;
        step = 0;
    }

    for(int i = 0; i < LEDS_CNT>>1; i++) {
    	hue[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 85.f * ((float)i/(float)(LEDS_CNT>>1));
    }
    for(int i = LEDS_CNT>>1; i < LEDS_CNT; i++) {
        hue[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 85.f;
    }
    memset(sat, 255, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    uint32_t gauge = (leds_bat_param_soc * (LEDS_CNT-1)) + 1;

    switch(frame) {
        case 0: {
            float totalFillStep = 1.f/BATTERY_FULL_STEPS*(float)step;
            for(int i = 0; i < LEDS_CNT; i++) {
                float fillPercentage = totalFillStep * LEDS_CNT - i;
                val[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = fillPercentage > 1 ? maxlum : fillPercentage * maxlum;
                if(fillPercentage < 0){
                    break;
                }
            }            

            step++;
            if(totalFillStep == 1) {
                LEDS_ANIM_NEXT_FRAME();    
            }
            break;
        }
        case 1:{
            for (int i = 0; i <= LEDS_CNT; i++){
                val[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = maxlum;
            }
            step++;
            if(step == WAIT_STEP_BATTERY_FULL) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            float totalReverseStep = 1.f/BATTERY_REVERSE_STEPS*(float)step;
            float finishLED = (float) gauge+1 + (1.f - totalReverseStep) * (uint8_t)(LEDS_CNT-gauge-1);
            for(int i = 0; i < finishLED; i++) {
                float fillPercentage = finishLED - i;
                val[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = fillPercentage > 1 ? maxlum : fillPercentage * maxlum;
            }            

            step++;
            if(gauge == LEDS_CNT || totalReverseStep == 1){
                if(bat_isUSBPlugged()){
                    if(leds_is_charged){
                        sch_unique_oneshot(leds_charged,0);                        
                    }else{
                        sch_unique_oneshot(leds_charging,0);                        
                    }
                    leds_anim_off(ANIM_BATTERY);
                }else{
                   LEDS_ANIM_NEXT_FRAME();    
                }
            }
            break;
        }
        case 3:{
            for (int i = 0; i <= gauge; i++){
                val[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = maxlum;
            }
            step++;
            if(step == WAIT_STEP_BATTERY) {
                leds_anim_off(ANIM_BATTERY);
            }
            break;
        }
    }   
    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define CHARGING_SHADOW_PULSE_STEPS 5*60
uint8_t leds_charging_mode = 4;
uint8_t leds_charging_shadow_base = 0;

void leds_animation_charging_shadow(bool reset){
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t cycles = 0;

    if(reset){
        step = 0;
        frame = 0;
        cycles = 0;
    }

    if(!bat_isUSBPlugged()){
        leds_anim_off(CHARGE_SHADOW);
    }

    for(int i = 0; i < LEDS_CNT>>1; i++) {
        hue[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 85.f * ((float)i/(float)(LEDS_CNT>>1));
    }
    for(int i = LEDS_CNT>>1; i < LEDS_CNT; i++) {
        hue[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 85.f;
    }
    memset(sat, 255, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    uint32_t gauge = (leds_bat_param_soc * (LEDS_CNT-1)) + 1;

    switch(frame) {
        case 0: {
            for (int i = 0; i <= gauge; i++){
                val[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 50;
            }
            step++;
            if(leds_charging_mode == 4 && step == CHARGING_SHADOW_PULSE_STEPS){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 1:{
            for (int i = 0; i <= gauge; i++){
                val[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 50;
            }
            float increment = (float)step/2.75;
            if(increment > 2 && increment < gauge + 3 && increment < LEDS_CNT + 2){
                val[((uint8_t)increment-2+LEDS_BATTERY_OFFSET)%24] = 65;     
            }
            if(increment > 1 && increment < gauge + 2 && increment < LEDS_CNT + 1){
                val[((uint8_t)increment-1+LEDS_BATTERY_OFFSET)%24] = 85;      
            }
            if(increment < gauge + 1  && increment && increment < LEDS_CNT - 0){
                val[((uint8_t)increment+LEDS_BATTERY_OFFSET)%24] = 65;
            }
            step++;
            if(increment >= gauge+3 || increment >= LEDS_CNT + 2){
                step = 0;
                frame = 0;
                cycles++;
            }
            if(cycles == 20){
                sch_unique_oneshot(leds_charging, 500);            
            }
        }
    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define CHARGING_TRACE_WAIT_STEPS 3*60

void leds_animation_charging_trace(bool reset){
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t cycles = 0;

    if(reset){
        step = 0;
        frame = 0;
        cycles = 0;
    }

    if(!bat_isUSBPlugged()){
        leds_anim_off(CHARGE_SHADOW);
    }

    for(int i = 0; i < LEDS_CNT>>1; i++) {
        hue[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 85.f * ((float)i/(float)(LEDS_CNT>>1));
    }
    for(int i = LEDS_CNT>>1; i < LEDS_CNT; i++) {
        hue[(i+LEDS_BATTERY_OFFSET)%LEDS_CNT] = 85.f;
    }
    memset(sat, 255, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    uint32_t gauge = (leds_bat_param_soc * (LEDS_CNT-1)) + 1;
    if(leds_charging_mode == 5){
       val[(gauge+LEDS_BATTERY_OFFSET)%24] = 100;
    }
    switch(frame){
        case 0:{
            step++;
            if(step >= CHARGING_TRACE_WAIT_STEPS){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 1:{
            float increment = (float)step/2.0;
            val[((uint8_t)increment+LEDS_BATTERY_OFFSET)%24] = 100;
            step++;
            if(increment >= gauge){
                step = 0;
                frame = 0;
                cycles++;
                if(cycles == 20){
                    sch_unique_oneshot(leds_charging, 50);            
                }
            }
            break;
        }    
    }    
    leds_util_copyHSVtoRGB(hue,sat,val);
}


float leds_speedometer_param_speed = 0.0;
bool leds_speedometer_turnOff;
bool leds_speedometer_intro = false;

#define LEDS_SPEEDO_OFFSET              16
#define LEDS_SPEEDO_RANGE               17
#define MOVE_STEP_SPEEDO                30
#define FADE_OUT_STEP_SPEEDO            30
#define LEDS_SPEEDO_INTRO_PAUSE_STEPS   10

void leds_anim_step_speedometer(bool reset) {

    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static float current_gauge = 0;
    static float goal_gauge = 0;
    uint32_t maxlum = leds_anim_brightness * 255.f;

    for(int i = 0; i < LEDS_SPEEDO_RANGE; i++) {
        hue[(i + LEDS_SPEEDO_OFFSET)%LEDS_CNT] = 90.f * ((float)((-1 * i) + (LEDS_SPEEDO_RANGE-1))/(float)(LEDS_SPEEDO_RANGE-1));
    }

    memset(sat, 255, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    float gauge = (leds_speedometer_param_speed * (float)LEDS_SPEEDO_RANGE);

    if(reset) {
        frame = leds_speedometer_intro ? -3 : 0;
        step = 0;
        leds_speedometer_turnOff = false;
        current_gauge = gauge;
        goal_gauge = gauge;
    }
    
    uint32_t lum_last_led =  ((current_gauge - (int32_t) current_gauge) * maxlum);

    
    if (goal_gauge != gauge && !leds_speedometer_turnOff){
        if (frame == 2){
            frame = 5;
            step = 1;
            goal_gauge = gauge;
        }
        else if (frame == 3){
            frame = 0;
            step = 0;
            goal_gauge = gauge;
        }
        else if (frame == 5){
            current_gauge = (float)current_gauge + ((float)(goal_gauge - (float)current_gauge) * ((float)(1.f/MOVE_STEP_SPEEDO) * ((float)step -1)));
            frame = 5;
            step = 1;
            goal_gauge = gauge;
        }
    }

    if(leds_speedometer_turnOff) {
        if (frame == 2){
            leds_speedometer_turnOff = false;
            frame = 3;
            step = 0;
        }
    }

    switch(frame) {

        case -3: {
            float lvl = ((float)(1.f/MOVE_STEP_SPEEDO) * (float)step);
            float temp_gauge = (float)current_gauge + ((float)(1.0 * LEDS_SPEEDO_RANGE - (float)current_gauge) * lvl);
            float lum_last_led_temp = temp_gauge - (int32_t)temp_gauge;
            memset(val, 0, LEDS_CNT);

            for(int i = 0; i < temp_gauge; i++) {
                val[(i+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = maxlum;
            }
            val[((int)temp_gauge+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = lum_last_led_temp * maxlum;

            step++;
            
            if (step == MOVE_STEP_SPEEDO){
                current_gauge = temp_gauge;
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }

        case -2: {
            for(int i = 0; i < LEDS_SPEEDO_RANGE; i++) {
                val[(i+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = maxlum;
            }

            step++;
            if (step == LEDS_SPEEDO_INTRO_PAUSE_STEPS) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }

        case -1: {
            float lvl = ((float)(1.f/MOVE_STEP_SPEEDO) * (float)step);
            float temp_gauge = (float)current_gauge + ((float)(0.0 - (float)current_gauge) * lvl);
            float lum_last_led_temp = temp_gauge - (int32_t)temp_gauge;
            memset(val, 0, LEDS_CNT);

            for(int i = 0; i < temp_gauge; i++) {
                val[(i+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = maxlum;
            }
            val[((int)temp_gauge+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = lum_last_led_temp * maxlum;

            step++;
            
            if (step == MOVE_STEP_SPEEDO){
                LEDS_ANIM_NEXT_FRAME();
                current_gauge = gauge;
            }
            break;
        }

        case 0: {
            memset(val, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            break;
            }
        case 1: {
            memset(val, 0, LEDS_CNT);
		    float lvl = (1.f/MOVE_STEP_SPEEDO)*(float)step;
		    float prog = ((float)current_gauge)*lvl;
            float lvl_last_led = prog - (int)prog;
		    for(int i = 0; i < prog; i++) {
		        val[(i+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = maxlum;
			}
            val[((int)prog+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = maxlum * lvl_last_led;

		    if (step < MOVE_STEP_SPEEDO){
               step++;  
			}
		    else{
                //current_gauge = gauge;
		        LEDS_ANIM_NEXT_FRAME();
		    }

		    break;
		}
        case 2: {
            memset(val, 0, LEDS_CNT);

            for(int i = 0; i < current_gauge; i++) {
                val[(i+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = maxlum;
            }

            val[((int)current_gauge+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = lum_last_led;

            break;
        }

        case 3: {
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - (1.f/FADE_OUT_STEP_SPEEDO)*(float)step;
    
            //Fade off of the leds that are lighten at a 100%
            for(int i = 0; i < current_gauge; i++) {
                val[(i+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = lvl * maxlum;
            }

            //Fade off of the last led that may not be lighten at a 100%
            val[((int)current_gauge+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = lvl * lum_last_led;
            step++;

            if(step < FADE_OUT_STEP_SPEEDO) {
                step++;
            }
            else{
                current_gauge = 0;
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }

        case 4: {
            memset(val, 0, LEDS_CNT);
            leds_anim_off(ANIM_SPEEDOMETER);
            break;
                            
        }
        case 5: {
            float lvl = ((float)(1.f/MOVE_STEP_SPEEDO) * (float)step);
            float temp_gauge = (float)current_gauge + ((float)(goal_gauge - (float)current_gauge) * lvl);
            float lum_last_led_temp = temp_gauge - (int32_t)temp_gauge;
            memset(val, 0, LEDS_CNT);

            for(int i = 0; i < temp_gauge; i++) {
                val[(i+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = maxlum;
            }
            val[((int)temp_gauge+LEDS_SPEEDO_OFFSET)%LEDS_CNT] = lum_last_led_temp * maxlum;

            step++;
            
            if (step == MOVE_STEP_SPEEDO){
                frame = 2;
                step = 0;
                current_gauge = goal_gauge;
            }
            break;
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

float leds_cadence_param_cad = 0.0;
int32_t leds_param_h;
int32_t leds_param_s;
int32_t leds_param_l;
bool leds_cadence_turnOff;

#define LEDS_CADENCE_OFFSET     16
#define LEDS_CADENCE_RANGE      11
#define MOVE_STEP_CADENCE       50
#define FADE_STEP_CADENCE       30

void leds_anim_step_cadence(bool reset) {

    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static float current_gauge = 0;
    static float goal_gauge = 0;
    static float previous_sat_green_goal = 0;
    uint32_t maxlum = leds_anim_brightness * leds_param_l;
    

    memset(sat, 0, LEDS_CNT);
    memset(hue, 0, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    float gauge = (leds_cadence_param_cad * (float)LEDS_CADENCE_RANGE);
    float lum_last_led =  ((gauge - (int32_t) gauge));
    if(lum_last_led < 0){
        lum_last_led = -1.f * lum_last_led;
    }
    float cadence = ((leds_cadence_param_cad < 0)? (-1.f * leds_cadence_param_cad) : leds_cadence_param_cad);
    float sat_green_goal = leds_param_s * (1.f - (cadence/0.2f));

    if(reset) {
        frame = 0;
        step = 0;
        leds_cadence_turnOff = false;
        current_gauge = gauge;
        goal_gauge = gauge;
        previous_sat_green_goal = sat_green_goal;
    }

    
    if (goal_gauge != gauge && !leds_cadence_turnOff){
        if (frame == 2){
            frame = 4;
            step = 1;
            goal_gauge = gauge;
        }
    }

    if(leds_cadence_turnOff) {
        if (frame == 2){
            leds_cadence_turnOff = false;
            frame = 3;
            step = 0;
        }
    }

    switch(frame) {

        case 0: {
            memset(val, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(hue, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            memset(val, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(hue, leds_param_h, LEDS_CNT);
            float lvl = (1.f/FADE_STEP_CADENCE)*(float)step;

            uint32_t ptr1 = MOD((int32_t)gauge,LEDS_CNT);
            uint32_t ptr2 = ptr1;
            uint32_t ptr4 = ptr1; 
            uint32_t ptr3 = ptr1;

            if (gauge < 0){
                ptr2 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1-2), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
            }
            else{
                ptr2 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1+2), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
            }
       
            val[ptr1] = (float)maxlum * lvl;
            val[ptr2] = (float)maxlum * lvl;
            val[ptr3] = (float)((1.f - lum_last_led) * maxlum) * lvl;
            val[ptr4] = (float)(lum_last_led * maxlum) * lvl;
            val[0] = (float)maxlum * lvl;
            sat[0] = 255;

            if ((int32_t)gauge < 1 && (int32_t)gauge > -1){
                sat[ptr1] = (float) sat_green_goal * lvl;
                sat[ptr2] = (float) sat_green_goal * lvl;
                sat[ptr4] = (float) sat_green_goal * lvl;
                sat[ptr3] = (float) sat_green_goal * lvl;
            }
            sat[0] = 255;

            step++;

            if (step >= FADE_STEP_CADENCE){
                LEDS_ANIM_NEXT_FRAME();
            }

		    break;
		}
        case 2: {
            memset(val, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(hue, leds_param_h, LEDS_CNT);

            if (lum_last_led < 0){
                lum_last_led = -1.f * lum_last_led;
            }
            uint32_t ptr1 = MOD((int32_t)gauge,LEDS_CNT);
            uint32_t ptr2 = ptr1;
            uint32_t ptr4 = ptr1; 
            uint32_t ptr3 = ptr1;

            if (gauge < 0){
                ptr2 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1-2), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
            }
            else{
                ptr2 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1+2), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
            }
        
            val[ptr1] = (float)maxlum;
            val[ptr2] = (float)maxlum;
            val[ptr3] = (float)((1.f - lum_last_led) * maxlum);
            val[ptr4] = (float)(lum_last_led * maxlum);
            val[0] = (float)maxlum;
            
            if ((int32_t)gauge < 1 && (int32_t)gauge > -1){
                sat[ptr1] = (float) sat_green_goal;
                sat[ptr2] = (float) sat_green_goal;
                sat[ptr4] = (float) sat_green_goal;
                sat[ptr3] = (float) sat_green_goal;
            }
            sat[0] = 255;

		    break;
        }

        case 3: {
            memset(val, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(hue, leds_param_h, LEDS_CNT);
            float lvl = 1.f - (1.f/FADE_STEP_CADENCE)*(float)step;
            uint32_t ptr1 = MOD((int32_t)gauge,LEDS_CNT);
            uint32_t ptr2 = ptr1;
            uint32_t ptr3 = ptr1;
            uint32_t ptr4 = ptr1;

            if (gauge < 0){
                ptr2 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1-2), LEDS_CNT);

            }
            else{
                ptr2 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1+2), LEDS_CNT);
            }     
            val[ptr1] = (float)maxlum * lvl;
            val[ptr2] = (float)maxlum * lvl;
            val[ptr3] = (float)((1.f - lum_last_led) * maxlum * lvl);
            val[ptr4] = (float)(lum_last_led * maxlum * lvl);
            val[0] = (float)maxlum * lvl;
            
            if ((int32_t)gauge < 1 && (int32_t)gauge > -1){
                sat[ptr1] = (float) sat_green_goal;
                sat[ptr2] = (float) sat_green_goal;
                sat[ptr4] = (float) sat_green_goal;
                sat[ptr3] = (float) sat_green_goal;
            }
            sat[0] = 255;

            step++;
            
            if (step > FADE_STEP_CADENCE){
                current_gauge = 0;
                leds_anim_off(ANIM_CADENCE);
            }
            break;
        }

        case 4: {
            memset(val, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(hue, leds_param_h, LEDS_CNT);
            float lvl = ((float)(1.f/MOVE_STEP_CADENCE) * (float)step);
            float move = (float)goal_gauge - (float)current_gauge;
            float temp_gauge = (float)current_gauge + ((float)move * lvl);
            float temp_sat_green = (float) previous_sat_green_goal + ((sat_green_goal - previous_sat_green_goal) * lvl);
            lum_last_led =  ((float)temp_gauge - (int32_t) temp_gauge);
            if (lum_last_led < 0){
                lum_last_led = -1.f * lum_last_led;
            }
            uint32_t ptr1 = MOD((int32_t)temp_gauge,LEDS_CNT);
            uint32_t ptr2 = ptr1;
            uint32_t ptr4 = ptr1; 
            uint32_t ptr3 = ptr1;

            if (temp_gauge < 0){
                ptr2 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1-2), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
            }
            else{
                ptr2 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1+2), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
            } 
            val[ptr1] = (float)maxlum;
            val[ptr2] = (float)maxlum;
            val[ptr3] = (float)((1.f - lum_last_led) * maxlum);
            val[ptr4] = (float)(lum_last_led * maxlum);
            val[0] = (float)maxlum;
            step ++;
            
            if ((int32_t)current_gauge < 1 && (int32_t)current_gauge > -1){
                if (goal_gauge < -1 || goal_gauge > 1){
                    sat[ptr1] = (float) previous_sat_green_goal * (1.f-lvl);
                    sat[ptr2] = (float) previous_sat_green_goal * (1.f-lvl);
                    sat[ptr4] = (float) previous_sat_green_goal * (1.f-lvl);
                    sat[ptr3] = (float) previous_sat_green_goal * (1.f-lvl);
                }
                else{
                    sat[ptr1] = (float) temp_sat_green;
                    sat[ptr2] = (float) temp_sat_green;
                    sat[ptr4] = (float) temp_sat_green;
                    sat[ptr3] = (float) temp_sat_green;
                }
            }
            sat[0] = 255;

            step++;

            if (step > MOVE_STEP_CADENCE){
                if ((int32_t)goal_gauge < 1 && (int32_t)goal_gauge > -1 && (current_gauge < -1 || current_gauge  > 1)){
                    frame = 5;
                    step = 0;
                }
                else{
                    frame = 2;
                    step = 0;
                }
                previous_sat_green_goal = sat_green_goal;
                current_gauge = goal_gauge;
            }
            break;
        }
        case 5 : {
            memset(val, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(hue, leds_param_h, LEDS_CNT);
            float lvl = (1.f/FADE_STEP_CADENCE)*(float)step;
		    if (lum_last_led < 0){
                lum_last_led = -1.f * lum_last_led;
            }
            uint32_t ptr1 = MOD((int32_t)gauge,LEDS_CNT);
            uint32_t ptr2 = ptr1;
            uint32_t ptr3 = ptr1;
            uint32_t ptr4 = ptr1;
            
            if (gauge <= 0){
                ptr2 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1-2), LEDS_CNT);
            }
            else if (gauge >= 0){
                ptr2 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                ptr3 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                ptr4 = MOD((int32_t)((int32_t)ptr1+2), LEDS_CNT);
            }    
            val[ptr1] = (float)maxlum;
            val[ptr2] = (float)maxlum;
            val[ptr3] = (float)((1.f - lum_last_led) * maxlum);
            val[ptr4] = (float)(lum_last_led * maxlum);
            val[0] = (float)maxlum;

            if ((int32_t)current_gauge < 1 && (int32_t)current_gauge > -1){
                sat[ptr1] = (float) sat_green_goal * lvl;
                sat[ptr2] = (float) sat_green_goal * lvl;
                sat[ptr4] = (float) sat_green_goal * lvl;
                sat[ptr3] = (float) sat_green_goal * lvl;
            }
            sat[0] = 255;

            step++;

            if (step >= FADE_STEP_CADENCE){
                frame = 2;
                step = 0;
            }

		    break;
            
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define NAV_BREATH_TICK (30)
#define NAV_BREATH_TICK_DIV2 (NAV_BREATH_TICK>>1)

#define FADE_STEP_NAV           5
#define MOVE_STEP_NAV           30

bool leds_nav_param_turnOff = false;
bool leds_nav_param_reroute = false;
int32_t leds_nav_param_h;
int32_t leds_nav_param_s;
int32_t leds_nav_param_l;
int32_t leds_nav_param_nextTurn_h;
int32_t leds_nav_param_nextTurn_s;
int32_t leds_nav_param_nextTurn_l;
uint32_t leds_nav_progress;
int32_t leds_nav_param_next_turn;
uint32_t leds_nav_progress_next_turn = 0;
uint32_t leds_nav_param_compass_mode;
float leds_nav_param_head;
uint32_t leds_nav_param_number_of_leds = ((uint32_t)(LEDS_CNT*LEDS_PERC_TURN_BY_TURN));
float leds_nav_param_head_next_turn;
void leds_anim_step_nav(bool reset) {
    uint8_t hue[LEDS_CNT];
    static uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t lvl_lum = 0;
    static uint32_t adj_progress = 0;
    static bool reroute = false;
    static bool reroute_firstloop = false;
    static uint32_t current_progress = 0;
    static uint32_t current_progress_nextTurn = 0;
    static int32_t current_direction = 0;
    static uint32_t current_compass_mode = 0;
    static uint32_t goal_progress = 0;
    static uint32_t goal_progress_nextTurn = 0;
    uint32_t ptr;
    static uint32_t ptr_cur1 = 0;
    static uint32_t ptr_cur2 = 0;
    uint32_t maxlum = leds_anim_brightness * 255.f;
    static uint32_t previous_next_turn = 0;

    if (leds_nav_param_compass_mode){
        leds_nav_param_next_turn = MOD((int32_t)round(-((met_getCompass() - leds_nav_param_head_next_turn) / 360.f) * ((float)LEDS_CNT)),LEDS_CNT);
        float head = MOD((int32_t)round(-((met_getCompass() - leds_nav_param_head) / 360.f) * ((float)LEDS_CNT)),LEDS_CNT);
        ptr = ((uint32_t)head % LEDS_CNT);
        uint32_t ptr1 = ptr - ((uint32_t)leds_nav_param_number_of_leds/2);
        if(ptr1<0) {
            ptr1 += LEDS_CNT;
        }
        uint32_t ptr2 = ptr + ((uint32_t)leds_nav_param_number_of_leds/2);
        ptr2 %= LEDS_CNT;
    }

    else{
        float head = MOD((int32_t)round((leds_nav_param_head / 360.f) * ((float)LEDS_CNT)),LEDS_CNT);
        ptr = ((uint32_t)head % LEDS_CNT);
        uint32_t ptr1 = ptr - ((uint32_t)leds_nav_param_number_of_leds/2);;
        if(ptr1<0) {
            ptr1 += LEDS_CNT;
        }
        uint32_t ptr2 = ptr + ((uint32_t)leds_nav_param_number_of_leds/2);;
        ptr2 %= LEDS_CNT;
    }

    if(reset) {
        leds_nav_param_turnOff = false;
        if(leds_nav_param_reroute) {
            reroute = true;
            reroute_firstloop = false;
            frame = 3;
        } else {
            frame = 0;
        }
        current_progress = leds_nav_progress;
        goal_progress = leds_nav_progress;
        goal_progress_nextTurn = leds_nav_progress_next_turn;
        current_progress_nextTurn = leds_nav_progress_next_turn;
        current_direction = ptr;
        current_compass_mode = leds_nav_param_compass_mode;
        adj_progress = 0;
        previous_next_turn = leds_nav_param_next_turn;
        ptr_cur1 = ptr;
        ptr_cur2 = ptr;
        step = 0;
        lvl_lum = 0;
    }

    if (current_direction != ptr){
        lvl_lum = 0;
        step = 0;
        if (previous_next_turn == ptr && !leds_nav_param_compass_mode){
            ptr_cur1 = MOD((int)((int)ptr-2), LEDS_CNT);
            ptr_cur2 = MOD((int)((int)ptr+2), LEDS_CNT);
            int32_t range;
            range = leds_nav_param_number_of_leds>>1;
            range += ((leds_nav_param_number_of_leds)%2) ? 1 : 0;
            range += 2;
            current_progress = (float)(2.f * (float)current_progress_nextTurn/range);
            current_progress_nextTurn = 0;
            frame = 6;
        }
        else {
            current_progress_nextTurn = leds_nav_progress_next_turn;
            current_progress = leds_nav_progress;
            goal_progress = leds_nav_progress;
            goal_progress_nextTurn = leds_nav_progress_next_turn;
        }
        current_direction = ptr;
    }
    
    else if (previous_next_turn != leds_nav_param_next_turn && leds_nav_progress_next_turn > 0 && leds_nav_progress < 100 && !leds_nav_param_compass_mode){
        ptr_cur1 = MOD(((int)(ptr-leds_nav_param_number_of_leds/2)-1), LEDS_CNT);
        ptr_cur2 = MOD(((int)(ptr+leds_nav_param_number_of_leds/2)+1), LEDS_CNT);
        current_progress_nextTurn = 0;
        lvl_lum = 0;
        step = 0;
        frame = 6;
    }

    else if ((goal_progress != leds_nav_progress || leds_nav_progress_next_turn != goal_progress_nextTurn) && leds_nav_progress < 100 && frame != 6){ 
        frame = 5;
        step = 0;
        if (current_progress != goal_progress && adj_progress > 0){
            current_progress = ((float) current_progress + (goal_progress - current_progress) * ((adj_progress - 1)/((float)MOVE_STEP_NAV)));
        }
        if (current_progress_nextTurn != goal_progress_nextTurn && adj_progress > 0){
            current_progress_nextTurn = ((float) current_progress_nextTurn + (goal_progress_nextTurn - current_progress_nextTurn) * ((adj_progress - 1)/((float)MOVE_STEP_NAV)));
        }
        adj_progress = 1;
        goal_progress = leds_nav_progress;
        goal_progress_nextTurn = leds_nav_progress_next_turn;
    }

    if (leds_nav_progress_next_turn > 0){
        previous_next_turn = leds_nav_param_next_turn;
    }
    else {
        previous_next_turn = -1;
    }
    if (current_compass_mode != leds_nav_param_compass_mode){
        current_compass_mode = leds_nav_param_compass_mode;
    }

    if(!reroute && leds_nav_param_reroute) {
        reroute = true;
        reroute_firstloop = false;
        frame = 3;
    }

    if(reroute && !leds_nav_param_reroute) {
        reroute = false;
    }

    switch(frame) {
        case 0: {
            memset(val, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            memset(hue, leds_nav_param_h, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = ((float)(1.f/FADE_STEP_NAV) * lvl_lum);

            int32_t cur1min, cur2max;
            cur1min = (int)MOD((int)(ptr_cur1 - step),LEDS_CNT);
            cur2max = (int)MOD((int)(ptr_cur2 + step),LEDS_CNT);
            

            for(int i = 0; i <= leds_nav_param_number_of_leds/2; i++) {
                int cur1 = MOD((int)((int)ptr-i),LEDS_CNT);
                int cur2 = MOD((int)((int)ptr+i),LEDS_CNT);
                if(cur1 == cur1min && cur2 == cur2max) {
                    val[cur1] = ((float)leds_anim_brightness * leds_nav_param_l * lvl);
                    val[cur2] = ((float)leds_anim_brightness * leds_nav_param_l * lvl);
                }
                else if(i < (step + ABS(((int)ptr_cur2), ((int)ptr))))  {
                    val[cur1] = ((float)leds_anim_brightness * leds_nav_param_l);
                    val[cur2] = ((float)leds_anim_brightness * leds_nav_param_l);
                }
                else{
                    val[cur1] = 0;
                    val[cur2] = 0;
                }
            }

            if (leds_nav_progress_next_turn > 0){
                for (int i = 0 ; i <= 1; i++){
                    int cur1 = MOD((int)(int)(leds_nav_param_next_turn+i),LEDS_CNT);
                    int cur2 = MOD((int)(int)(leds_nav_param_next_turn-i),LEDS_CNT);
                    hue[cur1] = leds_nav_param_nextTurn_h;
                    hue[cur2] = leds_nav_param_nextTurn_h;
                    if (step == i && lvl > 0.2){
                        val[cur1] = leds_anim_brightness * leds_nav_param_nextTurn_l * lvl;
                        val[cur2] = leds_anim_brightness * leds_nav_param_nextTurn_l * lvl;
                    }
                    else if (i < step){
                        val[cur1] = leds_anim_brightness * leds_nav_param_nextTurn_l;
                        val[cur2] = leds_anim_brightness * leds_nav_param_nextTurn_l;
                    }
                }
            }

            lvl_lum++;

            if (lvl_lum > FADE_STEP_NAV){
                step++;
                lvl_lum = 0;
            }
            
            if(ABS(((int)cur2max),((int)ptr)) > leds_nav_param_number_of_leds/2) {
                LEDS_ANIM_NEXT_FRAME();
                lvl_lum = 0;
                break;
            }

            

            break;
        }
        case 2: {
            memset(hue, leds_nav_param_h, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            int32_t  iprg, iprg_nextTurn, range;
            float lum, prg, prg_nextTurn;

            range = leds_nav_param_number_of_leds>>1;
            range += 1;
            prg = (float)range * (((float)leds_nav_progress)/100.f);
            iprg = prg;
            prg = (prg - (float)iprg) * (float)leds_nav_param_s;


            float lvl;
            if(step < NAV_BREATH_TICK_DIV2) {
                lvl = 1.f - (1.f/(float)NAV_BREATH_TICK_DIV2)*((float)step);
            } else {
                lvl = (1.f/(float)NAV_BREATH_TICK_DIV2)*((float)step-NAV_BREATH_TICK_DIV2);
            }
            if(leds_nav_progress == 100 || leds_nav_param_turnOff) {
                lum = lvl*leds_anim_brightness;
            } else {
                lum = leds_anim_brightness;
            }
            step++;
            step%=NAV_BREATH_TICK;

            for(int i = 0; i <= leds_nav_param_number_of_leds/2; i++) {
                int cur1 = MOD((int)((int)ptr-i),LEDS_CNT);
                int cur2 = MOD((int)((int)ptr+i),LEDS_CNT);
                val[cur1] = lum*leds_nav_param_l;
                val[cur2] = lum*leds_nav_param_l;
                if(i==iprg) {
                    sat[cur1] = prg;
                    sat[cur2] = prg;
                }
                else if(i < iprg){
                    sat[cur1] = leds_nav_param_s;
                    sat[cur2] = leds_nav_param_s;
                }
                else{
                    sat[cur1] = 0;
                    sat[cur2] = 0;
                 }
            }

            if (leds_nav_progress_next_turn > 0){
                prg_nextTurn = (float)2.0 * (((float)leds_nav_progress_next_turn)/100.f);
                iprg_nextTurn = prg_nextTurn;
                prg_nextTurn = (prg_nextTurn - (float)iprg_nextTurn) * (float)leds_nav_param_nextTurn_s;
                for(int i = 0; i <= 1; i++) {
                    int cur1 = MOD((int)((int)leds_nav_param_next_turn-i),LEDS_CNT);
                    int cur2 = MOD((int)((int)leds_nav_param_next_turn+i),LEDS_CNT);
                    hue[cur1] = leds_nav_param_nextTurn_h;
                    hue[cur2] = leds_nav_param_nextTurn_h;
                    if (leds_nav_param_turnOff){
                        val[cur2] = lum * leds_nav_param_nextTurn_l;
                        val[cur1] = lum * leds_nav_param_nextTurn_l;
                    }
                    else{
                        val[cur2] = leds_anim_brightness * leds_nav_param_nextTurn_l;
                        val[cur1] = leds_anim_brightness * leds_nav_param_nextTurn_l;  
                    }
                    if(i==iprg_nextTurn) {
                        sat[cur1] = prg_nextTurn;
                        sat[cur2] = prg_nextTurn;
                    }
                    else if(i < iprg_nextTurn){
                        sat[cur1] = leds_nav_param_nextTurn_s;
                        sat[cur2] = leds_nav_param_nextTurn_s;
                    }
                    else{
                        sat[cur1] = 0;
                        sat[cur2] = 0;
                    }
                }
            }

            if(step == NAV_BREATH_TICK_DIV2 && leds_nav_param_turnOff) {
                lum = 0;
                leds_anim_off(ANIM_NAV);
                leds_nav_param_turnOff = false;
            }

            break;
        }
        //------------------------------------
        // Reroute
        case 3: {
            memset(hue, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            val[20] = maxlum>>4;
            val[21] = maxlum>>3;
            val[22] = maxlum>>2;
            val[23] = maxlum>>1;
            val[0] = maxlum;
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 4: {
            memset(hue, 0, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            uint32_t ptr = 20 + (step >> 1);
            val[(ptr)%LEDS_CNT] = 0;
            val[(ptr+1)%LEDS_CNT] = maxlum>>4;
            val[(ptr+2)%LEDS_CNT] = maxlum>>3;
            val[(ptr+3)%LEDS_CNT] = maxlum>>2;
            val[(ptr+4)%LEDS_CNT] = maxlum>>1;
            val[(ptr+5)%LEDS_CNT] = maxlum;
            step++;
            if(step == 48) {
                reroute_firstloop = true;
                step=0;
            }
            if(!reroute && reroute_firstloop) {
                frame = 0;
            }
            if(leds_nav_param_turnOff && reroute_firstloop) {
                leds_nav_param_turnOff = false;
                leds_anim_off(ANIM_NAV);
            }
            break;
        }

        case 5: {
            memset(hue, leds_nav_param_h, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            int32_t  iprg, iprg_nextTurn, range;
            float lum, prg, prg_nextTurn, lvl_prog;

            lum = leds_anim_brightness;

            lvl_prog = (float)((1.f/MOVE_STEP_NAV) * adj_progress);

            range = leds_nav_param_number_of_leds>>1;
            range += 1;
            prg = (float)range * (((float)current_progress + ((int32_t)(goal_progress - current_progress) * lvl_prog))/100.f);
            iprg = prg;
            prg = (prg - (float)iprg) * (float)leds_nav_param_s;

            for(int i = 0; i <= leds_nav_param_number_of_leds/2; i++) {
                int cur1 = MOD((int)((int)ptr-i),LEDS_CNT);
                int cur2 = MOD((int)((int)ptr+i),LEDS_CNT);
                val[cur1] = lum*leds_nav_param_l;
                val[cur2] = lum*leds_nav_param_l;
                if(i==iprg) {
                    sat[cur1] = prg;
                    sat[cur2] = prg;
                }
                else if(i < iprg){
                    sat[cur1] = leds_nav_param_s;
                    sat[cur2] = leds_nav_param_s;
                }
                else{
                    sat[cur1] = 0;
                    sat[cur2] = 0;
                }
            }

            if (leds_nav_progress_next_turn > 0){
                prg_nextTurn = (float)2.0 * (((float)current_progress_nextTurn + ((int32_t)(goal_progress_nextTurn - current_progress_nextTurn) * lvl_prog))/100.f);
                iprg_nextTurn = prg_nextTurn;
                prg_nextTurn = (prg_nextTurn - (float)iprg_nextTurn) * (float)leds_nav_param_nextTurn_s;
                for(int i = 0; i <= 1; i++) {
                    int cur1 = MOD((int)((int)leds_nav_param_next_turn-i),LEDS_CNT);
                    int cur2 = MOD((int)((int)leds_nav_param_next_turn+i),LEDS_CNT);
                    val[cur1] = lum*leds_nav_param_nextTurn_l;
                    val[cur2] = lum*leds_nav_param_nextTurn_l;
                    hue[cur1] = leds_nav_param_nextTurn_h;
                    hue[cur2] = leds_nav_param_nextTurn_h;
                    if(i == iprg_nextTurn) {
                        sat[cur1] = prg_nextTurn;
                        sat[cur2] = prg_nextTurn;
                    }
                    else if(i < iprg_nextTurn){
                        sat[cur1] = leds_nav_param_nextTurn_s;
                        sat[cur2] = leds_nav_param_nextTurn_s;
                    }
                    else{
                        sat[cur1] = 0;
                        sat[cur2] = 0;
                    }
                }
            }

            adj_progress++;
            
            if (adj_progress >= MOVE_STEP_NAV){
                current_progress = goal_progress;
                current_progress_nextTurn = goal_progress_nextTurn;
                frame = 2;
                step = 0;
                adj_progress = 0;
            }

            break;
        }
        case 6: {
            memset(hue, leds_nav_param_h, LEDS_CNT);
            memset(val, 0, LEDS_CNT);

            float lvl = ((float)(1.f/FADE_STEP_NAV) * lvl_lum);
            int32_t cur1min, cur2max;

            for (int i = 0; i <= (LEDS_CNT - ABS(((int)ptr_cur2),((int)ptr_cur1))); i ++){
                int cur = MOD((int)((int)ptr_cur2 + i), LEDS_CNT);
                sat[cur] = 0;
            }

            cur1min = (int)MOD((int)(ptr_cur1 - step),LEDS_CNT);
            cur2max = (int)MOD((int)(ptr_cur2 + step),LEDS_CNT);

            for(int i = 0; i <= leds_nav_param_number_of_leds/2; i++) {
                int cur1 = MOD((int)((int)ptr-i),LEDS_CNT);
                int cur2 = MOD((int)((int)ptr+i),LEDS_CNT);
                if(cur1 == cur1min && cur2 == cur2max) {
                    val[cur1] = ((float)leds_anim_brightness * leds_nav_param_l * lvl);
                    val[cur2] = ((float)leds_anim_brightness * leds_nav_param_l * lvl);
                }
                else if(i < (step + ABS(((int)ptr_cur2), ((int)ptr))))  {
                    val[cur1] = ((float)leds_anim_brightness * leds_nav_param_l);
                    val[cur2] = ((float)leds_anim_brightness * leds_nav_param_l);
                }
                else{
                    val[cur1] = 0;
                    val[cur2] = 0;
                }
            }

            if (leds_nav_progress_next_turn > 0){
                for (int i = 0 ; i <= 1; i++){
                    int cur1 = MOD((int)((int)leds_nav_param_next_turn-i),LEDS_CNT);
                    int cur2 = MOD((int)((int)leds_nav_param_next_turn+i),LEDS_CNT);
                    if (step == i){
                        if (lvl > 0.2){
                            sat[cur1] = 0;
                            sat[cur2] = 0;
                            hue[cur1] = leds_nav_param_nextTurn_h;
                            hue[cur2] = leds_nav_param_nextTurn_h;
                            val[cur1] = leds_anim_brightness * leds_nav_param_nextTurn_l * lvl;
                            val[cur2] = leds_anim_brightness * leds_nav_param_nextTurn_l * lvl;
                        }
                    }
                    else if (i < step){
                        sat[cur1] = 0;
                        sat[cur2] = 0;
                        hue[cur1] = leds_nav_param_nextTurn_h;
                        hue[cur2] = leds_nav_param_nextTurn_h;
                        val[cur1] = leds_anim_brightness * leds_nav_param_nextTurn_l;
                        val[cur2] = leds_anim_brightness * leds_nav_param_nextTurn_l;
                    }
                }
            }

            lvl_lum++;

            if (lvl_lum > FADE_STEP_NAV){
                step++;
                lvl_lum = 0;
            }
            
            if((ABS(((int)cur2max),((int)ptr)) > (leds_nav_param_number_of_leds/2)) && step > 1) {
                step = 0;
                lvl_lum = 0;
                frame = 5;
                adj_progress = 1;
                goal_progress = leds_nav_progress;
                goal_progress_nextTurn = leds_nav_progress_next_turn;
                break;
            }

            break;
        }


    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}


#define ALARM_HUE           13
#define ALARM_SAT           207
#define STEP_ALARM_ARM      30

void leds_anim_step_alarm_arm(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    uint32_t maxlum = leds_anim_brightness * 255.f;

    if(reset) {
        frame = 0;
        step = 0;
    }

    switch(frame) {
        case 0: {
            memset(hue, ALARM_HUE, LEDS_CNT);
            memset(sat, ALARM_SAT, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/STEP_ALARM_ARM)*(float)step;
            uint32_t prog = 8.f*lvl;
            for (int i = 0; i < prog; i++) {
                val[(20+i)%LEDS_CNT] = maxlum;
                val[(4+i)%LEDS_CNT] = maxlum;
                val[(12+i)%LEDS_CNT] = maxlum;
            }
            step++;
            if(step == STEP_ALARM_ARM) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 1: {
            memset(hue, ALARM_HUE, LEDS_CNT);
            memset(sat, ALARM_SAT, LEDS_CNT);
            float lvl = (1.f/STEP_ALARM_ARM)*(float)step;
            memset(val, maxlum-((float)maxlum*lvl), LEDS_CNT);
            step++;
            if(step > STEP_ALARM_ARM) {
                leds_anim_off(ANIM_ALARM_ARM);
            }
            break;
        }
    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define MODLOOP(x, m)  ((((x) > 0) ? (x) : (x)+(m)) % (m))
#define STEP_ALARM_DISARM       15
#define MOVE_STEP_ALARM_ARM     30

void leds_anim_step_alarm_disarm(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    uint32_t maxlum = leds_anim_brightness * 255.f;

    if(reset) {
        frame = 0;
        step = 0;
    }

    switch(frame) {
        case 0: {
            memset(hue, ALARM_HUE, LEDS_CNT);
            memset(sat, ALARM_SAT, LEDS_CNT);
            float lvl = (1.f/STEP_ALARM_DISARM)*(float)step;
            memset(val, ((float)maxlum*lvl), LEDS_CNT);
            step++;
            if(step == STEP_ALARM_DISARM) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 1: {
            memset(val, maxlum, LEDS_CNT);
            memset(sat, ALARM_SAT, LEDS_CNT);
            float lvl = (1.f/STEP_ALARM_DISARM)*(float)step;
            memset(hue, ALARM_HUE+((float)(80-ALARM_HUE)*lvl), LEDS_CNT);
            step++;
            if(step == STEP_ALARM_DISARM) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            memset(hue, 80, LEDS_CNT);
            memset(sat, 255, LEDS_CNT);
            float lvl = (1.f/MOVE_STEP_ALARM_ARM)*(float)step;
            int32_t prog = 8.f*lvl;
            val[MODLOOP(3-prog, LEDS_CNT)] = 0;
            val[(11-prog)%LEDS_CNT] = 0;
            val[(19-prog)%LEDS_CNT] = 0;
            step++;
            if(step == MOVE_STEP_ALARM_ARM) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            leds_anim_off(ANIM_ALARM_DISARM);
            break;
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define MOVE_STEP_ALARM       30

bool leds_anim_alarm_alert;
float leds_anim_alarm_level;
int leds_anim_alarm_repeat;

void leds_anim_step_alarm(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static float level;

    if(reset) {
        step = 0;
    }

    if(leds_anim_alarm_alert) {
        frame = 1;
    } else {
        frame = 0;
    }

    memset(hue, 0, LEDS_CNT);
    memset(sat, 0, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    switch(frame) {
        case 0: {
            memset(hue, ALARM_HUE, LEDS_CNT);
            memset(sat, ALARM_SAT, LEDS_CNT);
            memset(val, 0, LEDS_CNT);

            float lvl;
            if(step < MOVE_STEP_ALARM) {
                lvl = (1.f/MOVE_STEP_ALARM)*((float)step);
            } else {
                lvl = 1.f - (1.f/MOVE_STEP_ALARM)*((float)step-MOVE_STEP_ALARM);
            }
            uint8_t lum = (0.1f + lvl*0.9f) * leds_anim_brightness * 255.f;

            if(leds_anim_alarm_repeat) {
                if(step == 0) {
                    leds_anim_alarm_repeat--;
                    level = leds_anim_alarm_level;
                }
                uint32_t prog = ceil(8.f*level);

                for (int i = 0; i < prog; i++) {
                    val[(20+i)%LEDS_CNT] = lum;
                    val[(4+i)%LEDS_CNT] = lum;
                    val[(12+i)%LEDS_CNT] = lum;
                }
            }            

            step++;
            step%=(2 * MOVE_STEP_ALARM);
            break;
        }
        case 1: {
            if(step==0) {
                memset(hue, ALARM_HUE, LEDS_CNT);
                memset(sat, 0, LEDS_CNT);
                memset(val, 128, LEDS_CNT);
            }
            if(step >=1 && step <= 3) {
                memset(hue, ALARM_HUE, LEDS_CNT);
                memset(sat, ALARM_SAT, LEDS_CNT);
                memset(val, 128, LEDS_CNT);
            }
            if(step==15) {
                leds_front_lum = 99;
            }
            if(step==18) {
                leds_front_lum = 0;
            }
            step++;
            step%=MOVE_STEP_ALARM;
            break;
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define ROUNDABOUT_OFFSET           LEDS_CNT/2

#define RIGHTLEFT(i)  (((leds_roundAbout_driveRight) == 1) ? MOD((ROUNDABOUT_OFFSET - i),LEDS_CNT) : MOD((ROUNDABOUT_OFFSET + i),LEDS_CNT))
#define SHOWEXIT(e,nb_e)  (((leds_roundAbout_driveRight) == 1) ? MOD(11-e,LEDS_CNT): MOD(10+e+nb_e,LEDS_CNT))


#define MOVE_STEP_ROUNDABOUT        5
#define FADE_IN_STEP_ROUNDABOUT     7
#define FADE_OUT_STEP_ROUNDABOUT    20
#define TURN_OFF_STEP_ROUNDABOUT    10
#define FADE_STEP_ROUNDABOUT        30

bool leds_roundAbout_param_destination;
int32_t leds_roundAbout_param_angle_exits[10];
int32_t leds_roundAbout_param_number_exits = 0;
int32_t leds_roundAbout_driveRight = 1;
uint32_t leds_roundAbout_withDirection = 0;
int32_t leds_nav_param_exit_h;
int32_t leds_nav_param_exit_s;
int32_t leds_nav_param_exit_l;
int32_t leds_nav_param_noExit_h;
int32_t leds_nav_param_noExit_s;
int32_t leds_nav_param_noExit_l;

bool leds_roundAbout_with_direction_param_turnOff = false;
void leds_anim_step_roundAbout(bool reset) {

    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t led_lum = 0;
    static uint32_t previousExit = 0;
    static uint32_t previousDirection = 1;
    static uint32_t previousNumberLedsExits = 0;

    uint32_t maxlum = 255 * leds_anim_brightness;
    uint32_t maxlumExit = leds_nav_param_exit_l * leds_anim_brightness;
    uint32_t number_leds_exits = ((leds_roundAbout_param_number_exits >= 5) ?  1:2);
    int32_t exit = (int32_t)(round((float)(leds_roundAbout_param_angle_exits[0] / 360.f) * LEDS_CNT));
    
    if (exit == 12  && leds_roundAbout_driveRight){
        exit = 11;
    }
    if (!leds_roundAbout_driveRight && exit == 12){
        exit = 13;
    }

    if (reset){
        frame = 0;
        step = 0;
        led_lum = 0;
        leds_roundAbout_with_direction_param_turnOff = false;
        previousExit = exit;
        previousDirection = leds_roundAbout_driveRight;
        previousNumberLedsExits = number_leds_exits;
    }

    if ((previousExit != exit || previousDirection != leds_roundAbout_driveRight || previousNumberLedsExits != number_leds_exits)
        && !leds_roundAbout_with_direction_param_turnOff) {
        if (leds_roundAbout_withDirection && previousDirection != 2){        
            frame = 3;
            step = 0;
            led_lum = 0;
            
        }
        else if (!leds_roundAbout_withDirection){
            frame = 10;
            step = 0;
            led_lum = 0;
        }
        previousExit = exit;
        previousDirection = leds_roundAbout_driveRight;
        previousNumberLedsExits = number_leds_exits;
    }
    

    if(leds_roundAbout_with_direction_param_turnOff){
        if (frame == 3 && step == 0){
            leds_roundAbout_with_direction_param_turnOff = false;
            frame = 4;
            step = 0;
        }
        else if (frame == 6){
            frame = 9;
            step = 0;
        }
    }
    
    memset(hue, 0, LEDS_CNT);
    memset(sat, 0, LEDS_CNT);
    

    for (uint8_t i = 1; i < leds_roundAbout_param_number_exits; i++){
        int32_t noExit = (int32_t)(round((float)(leds_roundAbout_param_angle_exits[i] / 360.f) * LEDS_CNT));
        for (uint8_t j=0; j<number_leds_exits; j++){
            hue[MOD(noExit + j,LEDS_CNT)] = leds_nav_param_noExit_h;
            sat[MOD(noExit + j,LEDS_CNT)] = leds_nav_param_noExit_s;
        }
    }

    for (uint8_t j=0; j<number_leds_exits; j++){
            hue[MOD((exit + j),LEDS_CNT)] = leds_nav_param_exit_h;
            sat[MOD((exit + j),LEDS_CNT)] = leds_nav_param_exit_s;
    }
    
    switch(frame) {
        case 0: {
            memset(val, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            if (!leds_roundAbout_withDirection){
                frame = 5;
            }
            break;
        }
        case 1: {
            memset(val, 0, LEDS_CNT);
            float lvl_lum = (0.7/FADE_IN_STEP_ROUNDABOUT)*(float)led_lum;
            float lvl_lum_2 = (0.7/FADE_IN_STEP_ROUNDABOUT)*(float)(led_lum + round(FADE_IN_STEP_ROUNDABOUT/2));
            uint32_t lum = maxlum;
	        for(uint8_t i = 0; i < step; i++) {
                lum = maxlum;
                for (uint8_t j=0; j<number_leds_exits; j++){
                    if (i == SHOWEXIT(exit + j,number_leds_exits)){
                        lum = maxlumExit;
                    }
                }
	            if (i == (step - 1)){
                    val[MOD((RIGHTLEFT(i)),LEDS_CNT)] = lum * lvl_lum; 
                }
                else if (i == (step - 2)){
                    val[MOD(RIGHTLEFT(i),LEDS_CNT)] = MIN((lum * lvl_lum_2), lum * 0.7);
                }
                else {
                    val[MOD(RIGHTLEFT(i),LEDS_CNT)] = lum * 0.7;
                }
		    }
           
            led_lum++;
            if (led_lum > (round(FADE_IN_STEP_ROUNDABOUT/2) - 1) && step < LEDS_CNT){
                step++;
                led_lum = 0;
            }
            else if(led_lum == FADE_IN_STEP_ROUNDABOUT && step == LEDS_CNT){
                step++;
                led_lum = 0;
            }
            if(step > LEDS_CNT && led_lum == 0) {
                frame = 3;
                step = 0;
            }

            break;
        } 
        case 2: {
            memset(val, 0.7 * maxlum, LEDS_CNT);
            float lvl = 1.f - (0.3/FADE_OUT_STEP_ROUNDABOUT)*(float)step;
            uint32_t lum = maxlum;
            for (uint8_t i = 0; i <= MIN(SHOWEXIT(exit,number_leds_exits)+1,LEDS_CNT - 1); i++){ 
                lum = maxlum;
                for (uint8_t j=0; j<number_leds_exits; j++){
                    if (i == SHOWEXIT(exit + j,number_leds_exits)){
                        lum = maxlumExit;
                    }
                }
                val[MOD(RIGHTLEFT(i),LEDS_CNT)] = lum * lvl;
            }        
            step++;
            if(step >= FADE_OUT_STEP_ROUNDABOUT) {
                LEDS_ANIM_NEXT_FRAME();
                if (!leds_roundAbout_withDirection){
                    frame = 10;
                }
                led_lum = 0;
            }
            break;
        }
        case 3: {
            memset(val, 0.7 * maxlum, LEDS_CNT);
            float lvl_lum = 0.7 + (0.3/MOVE_STEP_ROUNDABOUT)*(float)led_lum;
            float lvl_lum_2 = 0.7 + (0.3/MOVE_STEP_ROUNDABOUT)*(float)(led_lum + round(MOVE_STEP_ROUNDABOUT/2));
            uint32_t lum = maxlum;
            step = MIN(step, LEDS_CNT);
            for (uint8_t i = step; i < LEDS_CNT ; i++){
                lum = maxlum;
                for (uint8_t j=0; j<number_leds_exits; j++){
                    if (i == SHOWEXIT(exit + j,number_leds_exits)){
                        lum = maxlumExit;
                    }
                }
                val[MOD(RIGHTLEFT(i),LEDS_CNT)] = lum * 0.7;
            }   
	        for(uint8_t i = 0; i < step; i++) {
                lum = maxlum;
                for (uint8_t j=0; j<number_leds_exits; j++){
                    if (i == SHOWEXIT(exit + j,number_leds_exits)){
                        lum = maxlumExit;
                    }
                }
	            if (i == (step - 1)){
                    val[MOD((RIGHTLEFT(i)),LEDS_CNT)] = lum * lvl_lum; 
                }
                else if (i == (step - 2)){
                    val[MOD(RIGHTLEFT(i),LEDS_CNT)] = MIN((lum * lvl_lum_2), lum);
                }
                else {
                    val[MOD(RIGHTLEFT(i),LEDS_CNT)] = lum;
                }
		    }
           
            led_lum++;
            if (led_lum > (round(MOVE_STEP_ROUNDABOUT/2) - 1) && step <= MIN(SHOWEXIT(exit,number_leds_exits)+1,LEDS_CNT)){
                step++;
                led_lum = 0;
            }
            else if(led_lum == MOVE_STEP_ROUNDABOUT && step == MIN(SHOWEXIT(exit,number_leds_exits)+2,LEDS_CNT)){
                step++;
                led_lum = 0;
            }
            if(step > MIN(SHOWEXIT(exit,number_leds_exits)+2,LEDS_CNT) && led_lum == 0) {
                frame = 2;
                step = 0;
            }

            break;
        }
        case 4: {
            memset(val, 0, LEDS_CNT);
            float lvl = 0.7 - (0.7/TURN_OFF_STEP_ROUNDABOUT)*(float)step;
            memset(val, maxlum * lvl, LEDS_CNT);
            for (uint8_t j=0; j<number_leds_exits; j++){
                val[MOD((exit + j),LEDS_CNT)] = maxlumExit * lvl;
            }
            step++;
            if(step >= TURN_OFF_STEP_ROUNDABOUT) {
                leds_anim_off(ANIM_ROUNDABOUT);
            }
            break;
        }
        case 5 : {
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/FADE_OUT_STEP_ROUNDABOUT)*(float)step;
            memset(val, maxlum * lvl, LEDS_CNT);
            for (uint8_t j=0; j<number_leds_exits; j++){
                val[MOD((exit + j),LEDS_CNT)] = maxlumExit * lvl;
            }
            step++;
            if(step >= FADE_OUT_STEP_ROUNDABOUT) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }

        case 6: {
            memset(val, maxlum, LEDS_CNT);
            for (uint8_t j=0; j<number_leds_exits; j++){
                val[MOD((exit + j),LEDS_CNT)] = maxlumExit;
            }
            if (leds_roundAbout_withDirection){
                frame = 9;
                step = 0;
            }    
            else if(leds_roundAbout_param_destination) {
                LEDS_ANIM_NEXT_FRAME();
            }
            
            break;
        }

        case 7: {
            float lvl = 1.f - (1.f/FADE_STEP_ROUNDABOUT)*(float)step;
            memset(val, maxlum, LEDS_CNT);
            for (uint8_t j=0; j<number_leds_exits; j++){
                val[MOD((exit + j),LEDS_CNT)] = maxlumExit * lvl;
                sat[MOD((exit + j),LEDS_CNT)] = 175 * lvl;
            }
            step++;
            if(step > FADE_STEP_ROUNDABOUT) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }

        case 8: {
            float lvl = (1.f/FADE_STEP_ROUNDABOUT)*(float)step;
            memset(val, maxlum, LEDS_CNT);
            for (uint8_t j=0; j<number_leds_exits; j++){
                val[MOD((exit + j),LEDS_CNT)] = maxlumExit * lvl;
                sat[MOD((exit + j),LEDS_CNT)] = 175 * lvl;
            }
            step++;
            if(step >= FADE_STEP_ROUNDABOUT) {
                frame = 6;
                step = 0;
            }
            break;
        }
        case 9 : {
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - (0.3/TURN_OFF_STEP_ROUNDABOUT)*(float)step;
            memset(val, maxlum * lvl, LEDS_CNT);
            for (uint8_t j=0; j<number_leds_exits; j++){
                val[MOD((exit + j),LEDS_CNT)] = maxlumExit * lvl;
            }
            step++;
            if(step >= TURN_OFF_STEP_ROUNDABOUT) {
                frame = 3;
                step = 0;
                led_lum = 0;
            }
            break;
        }
        case 10 : {
            memset(val, 0, LEDS_CNT);
            float lvl = 0.7 + (0.3/FADE_OUT_STEP_ROUNDABOUT)*(float)step;
            memset(val, maxlum * lvl, LEDS_CNT);
            for (uint8_t j=0; j<number_leds_exits; j++){
                val[MOD((exit + j),LEDS_CNT)] = maxlumExit * lvl;
            }
            step++;
            if(step >= FADE_OUT_STEP_ROUNDABOUT) {
                frame = 6;
                step = 0;
            }
            break;
        }
    }
    leds_util_copyHSVtoRGB(hue,sat,val);          
}

int32_t leds_notif_param_h1;
int32_t leds_notif_param_s1;
int32_t leds_notif_param_l1;
int32_t leds_notif_param_h2;
int32_t leds_notif_param_s2;
int32_t leds_notif_param_l2;
uint32_t leds_notif_param_fadein;
uint32_t leds_notif_param_on;
uint32_t leds_notif_param_fadeout;
uint32_t leds_notif_param_off;
uint32_t leds_notif_param_repeat;

void leds_anim_step_notif(bool reset) {
    uint8_t central_hue = 0;
    uint8_t central_sat = 0;
    uint8_t central_val = 0;

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t repeat = 0;

    if(reset) {
        frame = 0;
        step = 0;
        repeat = leds_notif_param_repeat;
    }

    uint32_t fadeinTick = leds_notif_param_fadein >> 4; // /16
    uint32_t onTick = leds_notif_param_on >> 4; // /16
    uint32_t fadeoutTick = leds_notif_param_fadeout >> 4; // /16
    uint32_t offTick = leds_notif_param_off >> 4; // /16

    switch(frame) {
        case 0: {
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            if(!fadeinTick) {
                LEDS_ANIM_NEXT_FRAME();
                break;
            }
            float lvl = (1.f/(float)fadeinTick)*(float)step;
            central_hue = (uint8_t)(leds_notif_param_h2 + ((float)(leds_notif_param_h1 - leds_notif_param_h2))*lvl) % 256;
            central_sat = leds_notif_param_s2 + ((float)(leds_notif_param_s1 - leds_notif_param_s2))*lvl;
            central_val = (leds_notif_param_l2 + ((float)(leds_notif_param_l1 - leds_notif_param_l2))*lvl) * leds_anim_brightness;
            step++;
            if(step >= fadeinTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            central_hue = leds_notif_param_h1;
            central_sat = leds_notif_param_s1;
            central_val = leds_notif_param_l1 * leds_anim_brightness;
            step++;
            if(step >= onTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            if(!fadeoutTick) {
                LEDS_ANIM_NEXT_FRAME();
                break;
            }
            float lvl = (1.f/(float)fadeoutTick)*(float)step;
            central_hue = (uint8_t)(leds_notif_param_h1 + ((float)(leds_notif_param_h2 - leds_notif_param_h1))*lvl) % 256;
            central_sat = leds_notif_param_s1 + ((float)(leds_notif_param_s2 - leds_notif_param_s1))*lvl;
            central_val = (leds_notif_param_l1 + ((float)(leds_notif_param_l2 - leds_notif_param_l1))*lvl) * leds_anim_brightness;
            step++;
            if(step >= fadeoutTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 4: {
            central_hue = leds_notif_param_h2;
            central_sat = leds_notif_param_s2;
            central_val = leds_notif_param_l2 * leds_anim_brightness;
            step++;
            if(step >= offTick) {
                if(repeat) {
                    repeat--;
                    frame = 1;
                    step = 0;
                } else {
                    leds_anim_off(ANIM_NOTIF);
                }
            }
            break;
        }

    }

    leds_util_central_copyHSVtoRGB(central_hue, central_sat, central_val);
}


int32_t leds_hb_param_h1;
int32_t leds_hb_param_s1;
int32_t leds_hb_param_l1;
int32_t leds_hb_param_h2;
int32_t leds_hb_param_s2;
int32_t leds_hb_param_l2;
uint32_t leds_hb_param_fadein;
uint32_t leds_hb_param_on;
uint32_t leds_hb_param_fadeout;
uint32_t leds_hb_param_off;

void leds_anim_step_hb(bool reset) {
    uint8_t central_hue = 0;
    uint8_t central_sat = 0;
    uint8_t central_val = 0;

    static uint32_t frame = 0;
    static uint32_t step = 0;

    if(reset) {
        frame = 0;
        step = 0;
    }

    uint32_t fadeinTick = leds_hb_param_fadein >> 4; // /16
    uint32_t onTick = leds_hb_param_on >> 4; // /16
    uint32_t fadeoutTick = leds_hb_param_fadeout >> 4; // /16
    uint32_t offTick = leds_hb_param_off >> 4; // /16

    switch(frame) {
        case 0: {
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            if(!fadeinTick) {
                LEDS_ANIM_NEXT_FRAME();
                break;
            }
            float lvl = (1.f/(float)fadeinTick)*(float)step;
            central_hue = (uint8_t)(leds_hb_param_h2 + ((float)(leds_hb_param_h1 - leds_hb_param_h2))*lvl) % 256;
            central_sat = leds_hb_param_s2 + ((float)(leds_hb_param_s1 - leds_hb_param_s2))*lvl;
            central_val = (leds_hb_param_l2 + ((float)(leds_hb_param_l1 - leds_hb_param_l2))*lvl) * leds_anim_brightness;
            step++;
            if(step >= fadeinTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            central_hue = leds_hb_param_h1;
            central_sat = leds_hb_param_s1;
            central_val = leds_hb_param_l1 * leds_anim_brightness;
            step++;
            if(step >= onTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            if(!fadeoutTick) {
                LEDS_ANIM_NEXT_FRAME();
                break;
            }
            float lvl = (1.f/(float)fadeoutTick)*(float)step;
            central_hue = (uint8_t)(leds_hb_param_h1 + ((float)(leds_hb_param_h2 - leds_hb_param_h1))*lvl) % 256;
            central_sat = leds_hb_param_s1 + ((float)(leds_hb_param_s2 - leds_hb_param_s1))*lvl;
            central_val = (leds_hb_param_l1 + ((float)(leds_hb_param_l2 - leds_hb_param_l1))*lvl) * leds_anim_brightness;
            step++;
            if(step >= fadeoutTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 4: {
            central_hue = leds_hb_param_h2;
            central_sat = leds_hb_param_s2;
            central_val = leds_hb_param_l2 * leds_anim_brightness;
            step++;
            if(step >= offTick) {
                frame = 1;
                step = 0;
            }
            break;
        }

    }

    leds_util_central_copyHSVtoRGB(central_hue, central_sat, central_val);
}

void leds_anim_step_lowbat(bool reset) {

    int32_t param_h1 = 253;
    int32_t param_s1 = 226;
    int32_t param_l1 = 255;
    int32_t param_h2 = 253;
    int32_t param_s2 = 226;
    int32_t param_l2 = 0;

    uint8_t central_hue = 0;
    uint8_t central_sat = 0;
    uint8_t central_val = 0;

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t repeat = 0;

    if(reset) {
        frame = 0;
        step = 0;
        repeat = 1;
    }
    uint32_t fadeinTick = 68 >> 4;
    uint32_t onTick = 53 >> 4;
    uint32_t fadeoutTick = 500 >> 4;
    uint32_t offTick = 0 >> 4;

    if (repeat == 0){
        fadeinTick = 68 >> 4;
        onTick = 53 >> 4;
        fadeoutTick = 1000 >> 4;
        offTick = 0 >> 4;
    }

    switch(frame) {
        case 0: {
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            if(!fadeinTick) {
                LEDS_ANIM_NEXT_FRAME();
                break;
            }
            float lvl = (1.f/(float)fadeinTick)*(float)step;
            central_hue = (uint8_t)(param_h2 + ((float)(param_h1 - param_h2))*lvl) % 256;
            central_sat = param_s2 + ((float)(param_s1 - param_s2))*lvl;
            central_val = (param_l2 + ((float)(param_l1 - param_l2))*lvl) * leds_anim_brightness;
            step++;
            if(step >= fadeinTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            central_hue = param_h1;
            central_sat = param_s1;
            central_val = param_l1 * leds_anim_brightness;
            step++;
            if(step >= onTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            if(!fadeoutTick) {
                LEDS_ANIM_NEXT_FRAME();
                break;
            }
            float lvl = (1.f/(float)fadeoutTick)*(float)step;
            central_hue = (uint8_t)(param_h1 + ((float)(param_h2 - param_h1))*lvl) % 256;
            central_sat = param_s1 + ((float)(param_s2 - param_s1))*lvl;
            central_val = (param_l1 + ((float)(param_l2 - param_l1))*lvl) * leds_anim_brightness;
            step++;
            if(step >= fadeoutTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 4: {
            central_hue = param_h2;
            central_sat = param_s2;
            central_val = param_l2 * leds_anim_brightness;
            step++;
            if(step >= offTick) {
                if(repeat) {
                    repeat--;
                    frame = 1;
                    step = 0;
                } else {
                    leds_anim_off(ANIM_LOWBAT);
                }
            }
            break;
        }

    }

    leds_util_central_copyHSVtoRGB(central_hue, central_sat, central_val);
}

#define FADE_STEP_FRONT         60
#define FADE_STEP_FRONT_BLINK   30    

bool leds_front_setting = false;
bool leds_front_resetFade = false;
bool leds_front_state = false;
bool frontlight_animation_on = true;
bool leds_front_external_toggle_required = false;

void leds_animation_step_front(bool reset) {
    static uint32_t frame = 0;
    static uint32_t step = 0;

    static uint8_t hue[LEDS_CNT];
    static uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];

    uint32_t maxlum = leds_anim_brightness * 255.f;

    static uint32_t fadestep = 0;
    static float curBright = 0;
    static int32_t bright = 0;
    static uint32_t lum = 0;
    static float dim = 0;

    if(reset) {
        frame = 0;
        step = 0;
    }

    switch(frame) {
        case 0: {
            if(!leds_front_setting){
                leds_anim_off(FRONT_LIGHT_ANIMATION);
                return;
            }
            memset(hue, 38, LEDS_CNT);
            memset(sat, 234, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            val[(10)%24] = maxlum;
            val[(11)%24] = maxlum>>1;
            val[(12)%24] = maxlum>>2;
            val[(13)%24] = maxlum>>1;
            val[(14)%24] = maxlum;
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            int32_t increment = step/2;
            val[(12-increment)%24] = 0;
            val[(11-increment)%24] = maxlum>>2;
            val[(10-increment)%24] = maxlum>>1;
            val[(9- increment)%24] = maxlum;
            val[(increment+12)%24] = 0;
            val[(increment+13)%24] = maxlum>>2;
            val[(increment+14)%24] = maxlum>>1;
            val[(increment+15)%24] = maxlum;
            step++;
            if(step == LEDS_CNT - 4) {
                LEDS_ANIM_NEXT_FRAME();
            }
           break;
        }
        case 2: {
            memset(val, 0, LEDS_CNT);   
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 3: {
            int32_t brightSetting = 20 + ((leds_settings.frontlight_brightness * 80) / 100);

            int32_t trgBright = (leds_front_setting && leds_front_state) ? brightSetting : 0;

            if(leds_front_resetFade) {
                leds_front_resetFade = false;
                fadestep = 0;
                dim = trgBright - curBright;
            }

            if(fadestep < FADE_STEP_FRONT) {
                curBright += dim*(1.f/FADE_STEP_FRONT);
                bright = round(curBright);
                fadestep++;
                if(fadestep == FADE_STEP_FRONT) {
                    bright = trgBright;
                    if(bright == 0) {
                        LEDS_ANIM_NEXT_FRAME();
                    }
                }
            }

            switch(leds_settings.frontlight_mode) {
                case FRONT_ON: {
                    lum = bright;
                    break;
                }
                case FRONT_BLINK: {
                    if(step<(FADE_STEP_FRONT_BLINK/2)) {
                        lum = bright;
                    } else {
                        lum = 0;
                    }
                    step++;
                    step%=FADE_STEP_FRONT_BLINK;
                    break;
                }
                case FRONT_STROBE: {
                    if(step==0) {
                        lum = bright;
                    }
                    if(step==1) {
                        lum = 0;
                    }
                    step++;
                    step%=(FADE_STEP_FRONT_BLINK/2);
                    break;
                }
            }

            leds_front_lum = lum;
            break;
        }
        case 4: {
            memset(hue, 38, LEDS_CNT);
            memset(sat, 234, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            val[(10+12)%24] = maxlum;
            val[(11+12)%24] = maxlum>>1;
            val[(12+12)%24] = maxlum>>2;
            val[(13+12)%24] = maxlum>>1;
            val[(14+12)%24] = maxlum;
            LEDS_ANIM_NEXT_FRAME();
           break;
        }
        case 5: {
            int32_t increment = step/2;
            val[(12-increment+12)%24] = 0;
            val[(11-increment+12)%24] = maxlum>>2;
            val[(10-increment+12)%24] = maxlum>>1;
            val[(9- increment+12)%24] = maxlum;
            val[(   increment+12+12)%24] = 0;
            val[(   increment+13+12)%24] = maxlum>>2;
            val[(   increment+14+12)%24] = maxlum>>1;
            val[(   increment+15+12)%24] = maxlum;
            step++;
            if(step == LEDS_CNT - 4) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 6: {
            leds_anim_off(FRONT_LIGHT_ANIMATION);
        }
    }

    if (frame != 3) {
        leds_util_copyHSVtoRGB(hue,sat,val);
    }   
}

void leds_front_external_toggle(bool isRequired){
    leds_front_external_toggle_required = isRequired;
}

#define PROGRESS_OFFSET LEDS_CNT/2

#define FADE_STEP_PROGRESS      7
#define MOVE_STEP_PROGRESS      30
#define TURN_OFF_STEP_PROGRESS  30

int32_t leds_progress_h1;
int32_t leds_progress_s1;
int32_t leds_progress_l1;
int32_t leds_progress_h2;
int32_t leds_progress_s2;
int32_t leds_progress_l2;
uint32_t leds_progress_period;
uint32_t leds_progress_perc;
bool leds_progress_turnOff;

void leds_anim_step_progress(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t led_lum = 0;
    static uint32_t current_progress = 0;
    static uint32_t goal_progress = 0;
    
    uint32_t progress = 1 + round(((float)(LEDS_CNT-1)) * ((float)leds_progress_perc / 100.f));
    uint32_t duty = leds_progress_period >> 5; // /16 /2

    if(reset) {
        frame = 0;
        step = 0;
        leds_progress_turnOff = false;
        led_lum = 0;
        current_progress = progress;
        goal_progress = progress;
    }

    if(leds_progress_turnOff) {
        if (frame == 2 && step == 0){
            leds_progress_turnOff = false;
            frame = 4;
            step = 0;
            led_lum = 0;
        }
    }
    
    if (goal_progress != progress && !leds_progress_turnOff){
        if (frame == 2 && step == 0){
            goal_progress = progress;
            frame = 5;
            step = 1;
            led_lum = 0;
        }
        else if (frame == 5 && step > 0){
            current_progress = (float)current_progress + ((float)(goal_progress - (float)current_progress) * ((float)(1.f/MOVE_STEP_PROGRESS) * (float)(step-1)));
            step = 1;
            goal_progress = progress;
        }
        else if (frame == 4){
            frame = 0;
            step = 0;
            goal_progress = progress;
        }
    }

    switch(frame) {
        case 0: {
            memset(val, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            memset(hue, leds_progress_h1%256, LEDS_CNT);
            memset(sat, leds_progress_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/FADE_STEP_PROGRESS)*(float)led_lum;
            float lvl_2 = (1.f/FADE_STEP_PROGRESS)*(float) (led_lum + (round(FADE_STEP_PROGRESS/2)));
            for(int i = 0; i < step; i++) {
                if (i == step - 1){
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_progress_l1 * leds_anim_brightness * lvl;
                }
                else if (i == step - 2){
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = MIN(leds_progress_l1 * leds_anim_brightness * lvl_2, leds_progress_l1 * leds_anim_brightness);
                }
                else{
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_progress_l1 * leds_anim_brightness;
                }
            }

            led_lum++;
            if(step < current_progress && led_lum > (round(FADE_STEP_PROGRESS/2) - 1)){
                step ++;
                led_lum = 0;
            }
            else if(step == current_progress && led_lum == FADE_STEP_PROGRESS){
                step++;
                led_lum = 0;
            }
            if(step > current_progress && led_lum == 0) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            float lvl = 0;
            if(duty>1) {
                lvl = (1.f/(float)duty)*(float)step;
            }
            uint32_t effh = leds_progress_h1 + ((float)(leds_progress_h2 - leds_progress_h1))*lvl;
            uint32_t effs = leds_progress_s1 + ((float)(leds_progress_s2 - leds_progress_s1))*lvl;
            uint32_t effl = (leds_progress_l1 + ((float)(leds_progress_l2 - leds_progress_l1))*lvl) * leds_anim_brightness;

            memset(hue, effh%256, LEDS_CNT);
            memset(sat, effs, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            for(int i = 0; i < current_progress; i++) {
                val[(i+PROGRESS_OFFSET)%LEDS_CNT] = effl;
            }

            if(duty > 0) {
                step++;
            }
            if(duty && (step == duty)) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            float lvl = 0;
            if(duty>1) {
                lvl = (1.f/(float)duty)*(float)step;
            }
            uint32_t effh = leds_progress_h2 + ((float)(leds_progress_h1 - leds_progress_h2))*lvl;
            uint32_t effs = leds_progress_s2 + ((float)(leds_progress_s1 - leds_progress_s2))*lvl;
            uint32_t effl = (leds_progress_l2 + ((float)(leds_progress_l1 - leds_progress_l2))*lvl) * leds_anim_brightness;

            memset(hue, effh%256, LEDS_CNT);
            memset(sat, effs, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            for(int i = 0; i < current_progress; i++) {
                val[(i+PROGRESS_OFFSET)%LEDS_CNT] = effl;
            }

            if(duty > 0) {
                step++;
            }
            if(!duty || (step == duty)) {
                frame = 2;
                step = 0;
            }
            break;
        }
        case 4: {
            memset(hue, leds_progress_h1%256, LEDS_CNT);
            memset(sat, leds_progress_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - (1.f/TURN_OFF_STEP_PROGRESS)*(float)step;
            
            for(int i = 0; i < current_progress; i++) {
                val[(i+PROGRESS_OFFSET)%LEDS_CNT] = ((float) leds_progress_l1 * leds_anim_brightness * lvl);
            }
            step++;
            if(step == TURN_OFF_STEP_PROGRESS) {
                leds_anim_off(ANIM_PROGRESS);
            }
            break;
        }
        case 5: {
            float lvl = ((float)(1.f/MOVE_STEP_PROGRESS) * (float)step);
            float temp_progress = (float)current_progress + ((float)(goal_progress - (float)current_progress) * lvl);
            float lum_last_led_temp = temp_progress - (int32_t)temp_progress;
            memset(val, 0, LEDS_CNT);
            uint32_t effh = leds_progress_h1; 
            uint32_t effs = leds_progress_s1;
            uint8_t i = 0;
            memset(hue, effh%256, LEDS_CNT);
            memset(sat, effs, LEDS_CNT);

            for(i = 0; i < temp_progress; i++) {
                val[(i+PROGRESS_OFFSET)%LEDS_CNT] = ((float) leds_progress_l1 * leds_anim_brightness);
            }
             val[(((uint8_t)temp_progress)+PROGRESS_OFFSET)%LEDS_CNT] = ((float) leds_progress_l1 * leds_anim_brightness * lum_last_led_temp);


            step++;
            
            if (step == MOVE_STEP_PROGRESS){
                frame = 2;
                step = 0;
                current_progress = goal_progress;
            }
            break;
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define FADE_IN_STEP_FITNESS   7
#define FADE_OFF_STEP_FITNESS  10
#define FADE_STEP_FITNESS      4
#define TURN_OFF_STEP_FITNESS  30

int32_t leds_fitness_h1;
int32_t leds_fitness_s1;
int32_t leds_fitness_l1;
int32_t leds_fitness_h2;
int32_t leds_fitness_s2;
int32_t leds_fitness_l2;
uint32_t leds_fitness_period;

uint32_t leds_fitness_perc;
bool leds_fitness_turnOff;

void leds_anim_step_fitness(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t current_progress = 0;
    static uint32_t current_led = 0;
    static uint32_t led_lum = 0;
    static uint32_t lum_change_color = 0;

    uint32_t progress = 1 + round(((float)(LEDS_CNT-1)) * ((float)leds_fitness_perc / 100.f));
    uint32_t duty = ((float)(leds_fitness_period >> 5)); 

    if(reset) {
        frame = 0;
        step = 0;
        leds_fitness_turnOff = false;
        current_progress = 0;
        led_lum = 0;
        lum_change_color = 0;
        current_led = 0;        
    }
    
    if(current_progress != progress && !leds_fitness_turnOff){
        if ((frame == 3 || frame == 4 || frame == 7 || frame == 8) && (progress > current_progress) && (duty > 1)){
            if (frame == 4 && step == 0 && led_lum == 0){
                frame = 7;
                current_led = current_progress - 1;
                step = 0;
                led_lum = 0;
                lum_change_color = 0;
                current_progress = progress;
            }
        }
        else {
            frame = 1;
            step = 0;
            current_progress = progress;
        }
    }

    if(leds_fitness_turnOff) {
        if (frame == 2 && step == 0 && led_lum == 0){
            leds_fitness_turnOff = false;
            frame = 5;           
        }
        if (frame == 3 && step == 0 && led_lum == 0){
            leds_fitness_turnOff = false;
            frame = 6;
        }
    }

    switch(frame) {
        case 0: {
            memset(val, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/FADE_IN_STEP_FITNESS)*(float)led_lum;
            float lvl_2 = (1.f/FADE_IN_STEP_FITNESS)*(float) (led_lum + round(FADE_IN_STEP_FITNESS/2));
            for(int i = 0; i < step; i++) {
                uint8_t cur = MOD((i+PROGRESS_OFFSET),LEDS_CNT);
                if (i == step - 1){
                    val[cur] = leds_fitness_l1 * leds_anim_brightness * lvl;
                }
                else if (i == step - 2){
                    val[cur] = MIN(leds_fitness_l1 * leds_anim_brightness * lvl_2, leds_fitness_l1 * leds_anim_brightness);
                }
                else{
                    val[cur] = leds_fitness_l1 * leds_anim_brightness;
                }
            }

            led_lum++;
            if(step < current_progress && led_lum > (round(FADE_IN_STEP_FITNESS/2) - 1)){
                step ++;
                led_lum = 0;
            }
            else if(step == current_progress && led_lum == FADE_IN_STEP_FITNESS){
                step++;
                led_lum = 0;
            }
            if(step > current_progress && led_lum == 0) {
                LEDS_ANIM_NEXT_FRAME();
                current_progress = progress;
            }
            break;
        }
        case 2: {
            float lvl = ((1.f/(float)FADE_OFF_STEP_FITNESS)*(float)led_lum);
            float lvl_2 = ((1.f/(float)FADE_OFF_STEP_FITNESS)*(float) (led_lum + round(FADE_OFF_STEP_FITNESS/2)));
            float lum = leds_fitness_l1 * leds_anim_brightness;
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            for(int i = 0; i < current_progress; i++) {
                if (i == current_progress - 1){
                    lum = 0;
                }
                else if (i == current_progress - 2){
                    lum = ((float)leds_fitness_l1 * (float)leds_anim_brightness)*(1.f/8.f);
                }
                else if (i == current_progress - 3){
                    lum = ((float)leds_fitness_l1 * (float)leds_anim_brightness)*(1.f/4.f);
                }
                else if (i == current_progress - 4){
                    lum = ((float)leds_fitness_l1 * (float)leds_anim_brightness)*(3.f/8.f);
                }
                else if (i == current_progress - 5){
                    lum = ((float)leds_fitness_l1 * (float)leds_anim_brightness)*(1.f/2.f);
                }
                else {
                    lum = (float) (leds_fitness_l1 * leds_anim_brightness);
                }

                uint8_t cur = MOD((i+PROGRESS_OFFSET),LEDS_CNT);
                if (i == ((int)step - 1)){
                    val[cur] = ((float)leds_fitness_l1 * leds_anim_brightness) - ((float)lum * lvl);
                }
                else if (i == ((int)step - 2)){
                    val[cur] = MAX((float)leds_fitness_l1 * leds_anim_brightness - ((float)lum * lvl_2), ((float)leds_fitness_l1 * leds_anim_brightness)- (float)lum);
                }
                else if (i < ((int)step - 2)){
                    val[cur] = ((float)leds_fitness_l1 * leds_anim_brightness) - lum;
                }
                else {
                    val[cur] = ((float)leds_fitness_l1 * leds_anim_brightness);
                }
            }
            led_lum++;
            if(step < current_progress - 1 && led_lum > (round(FADE_OFF_STEP_FITNESS/2) - 1)){
                step ++;
                led_lum = 0;
            }
            else if(step == current_progress - 1 && led_lum == FADE_OFF_STEP_FITNESS){
                step++;
                led_lum = 0;
            }
            if(step > current_progress - 1 && led_lum == 0) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 3: {
            float lvl = 0;
            if(duty>1) {
                lvl = ((1.f/(float)duty)*(float)led_lum);
            }
            uint32_t effh = ((float)leds_fitness_h1 + (float)(((float)leds_fitness_h2 - (float)leds_fitness_h1)*lvl));
            uint32_t effs = leds_fitness_s1 + ((float)(leds_fitness_s2 - leds_fitness_s1)*lvl);

            memset(hue, effh%256, LEDS_CNT);
            memset(sat, effs, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lum = (float)1.0f;

            for(int i = 0; i < 5; i++) {
                if (i == ((int)4)){
                    lum = (float)1.f;
                }
                else if (i == ((int)3)){
                    lum = (float)(7.f/8.f);
                }
                else if (i == ((int)2)){
                    lum = (float)(6.f/8.f);
                }
                else if (i == ((int)1)){
                    lum = (float)(5.f/8.f);
                }
                else if (i == ((int)0)){
                    lum = (float)(1.f/2.f);
                }
                else {
                    lum = 0;
                }
                
                val[MOD(((int)i+PROGRESS_OFFSET + current_progress - 5),LEDS_CNT)] = ((float) leds_anim_brightness * ((lum * leds_fitness_l1) + ((float)(leds_fitness_l2 - (lum * leds_fitness_l1)))*lvl));
                if (((int)i + (int)current_progress - 5) < 0){
                    val[MOD(((int)i+PROGRESS_OFFSET + current_progress - 5),LEDS_CNT)] = 0; 
                }
            }

            if (duty > 0){
                led_lum++;
            }

            if(led_lum > duty && duty){
                LEDS_ANIM_NEXT_FRAME();
                led_lum = 0;
                lum_change_color = 0;
            }
            break;

        }
        case 4: {
            float lvl = 0;
            float lvl_2 = 0;
            float duty_flash = (float)duty/5.f;

            if(duty>1) {
                lvl = ((1.f/(float)duty_flash)*(float)led_lum);
                lvl_2 = ((1.f/(float)duty_flash)*(float) (led_lum + round(duty_flash/2)));
                if (lvl_2 > 1){
                    lvl_2 = 1.f;
                }
            }
            float lum = (float)1.f;
            memset(val, 0, LEDS_CNT);          

            for(int i = 0; i < 5; i++) {
                if (i == ((int)4)){
                    lum = (float)1.f;
                }
                else if (i == ((int)3)){
                    lum = (float)(7.f/8.f);
                }
                else if (i == ((int)2)){
                    lum = (float)(3.f/4.f);
                }
                else if (i == ((int)1)){
                    lum = (float)(5.f/8.f);
                }
                else if (i == ((int)0)){
                    lum = (float)(1.f/2.f);
                }
                else {
                    lum = 0;
                }

                uint8_t cur = MOD(((int)i+PROGRESS_OFFSET+ current_progress - 5),LEDS_CNT);
                if (i == ((int) step - 1)){
                    val[cur] = ((float)leds_anim_brightness * (leds_fitness_l2 + ((float)(lum * leds_fitness_l1 - leds_fitness_l2))*lvl));
                    hue[cur] = ((float)leds_fitness_h2 + (float)((float)lvl*((float)leds_fitness_h1 - (float)leds_fitness_h2)));
                    sat[cur] = leds_fitness_s2 + ((float)(leds_fitness_s1 - leds_fitness_s2)*lvl);
                }
                else if (i == ((int) step - 2)){
                    val[cur] = (float)(leds_anim_brightness * (leds_fitness_l2 + ((float)(lum * leds_fitness_l1 - leds_fitness_l2))*lvl_2));
                    hue[cur] = ((float)leds_fitness_h2 + (float)((float)lvl_2*((float)leds_fitness_h1 - (float)leds_fitness_h2)));
                    sat[cur] = leds_fitness_s2 + ((float)(leds_fitness_s1 - leds_fitness_s2)*lvl_2);
                }
                else if (i < ((int)step - 2)){
                    val[cur] = ((float)leds_anim_brightness * leds_fitness_l1 * lum);
                    hue[cur] = (float)leds_fitness_h1;
                    sat[cur] = (float)leds_fitness_s1;
                }
                else {
                    val[cur] = ((float)leds_anim_brightness * leds_fitness_l2);
                    hue[cur] = (float)leds_fitness_h2;
                    sat[cur] = (float)leds_fitness_s2;
                }
                if (((int)i + (int)current_progress - 5) < 0){
                    val[cur] = 0;
                }
            }
            
            if (duty > 0){
                led_lum++;
                lum_change_color++;
            }

            if(step < 5 && led_lum > (round(duty_flash/2) - 1) && duty){
                step ++;
                led_lum = 0;
            }
            else if(step == 5 && led_lum >= duty_flash && duty){
                step++;
                led_lum = 0;
            }
            if(step > 5 && led_lum == 0 && duty) {
                lum_change_color = 0;
                frame = 3;
                step = 0;
            }
            break;
        }
        case 5: {
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - (1.f/TURN_OFF_STEP_FITNESS)*(float)step;
            for(int i = 0; i < current_progress; i++) {
                val[(i+PROGRESS_OFFSET)%LEDS_CNT] = ((float) leds_fitness_l1 * leds_anim_brightness * lvl);
            }
            step++;
            if(step == TURN_OFF_STEP_FITNESS) {
                current_progress = 0;
                leds_anim_off(ANIM_FITNESS);
            }
            break;
        }
        case 6: {
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - (1.f/TURN_OFF_STEP_FITNESS)*(float)step;
            float lum = (float)leds_anim_brightness;
            for(int i = 0; i < 5; i++) {
                if (i == ((int)4)){
                    lum = (float)leds_anim_brightness * leds_fitness_l1;
                }
                else if (i == ((int)3)){
                    lum = (float)leds_anim_brightness*(7.f/8.f) * (float)leds_fitness_l1;
                }
                else if (i == ((int)2)){
                    lum = (float)leds_anim_brightness* (3.f/4.f) * (float)leds_fitness_l1;
                }
                else if (i == ((int)1)){
                    lum = (float)leds_anim_brightness*(5.f/8.f) * (float)leds_fitness_l1;
                }
                else if (i == ((int)0)){

                    lum = (float)leds_anim_brightness*(1.f/2.f) * (float)leds_fitness_l1;
                }
                else {
                    lum = 0;
                }

                uint8_t cur = MOD(((int)i+PROGRESS_OFFSET + current_progress - 5),LEDS_CNT);
                val[cur] = ((float)lum * lvl);

                if (((int)i + (int)current_progress - 5) < 0){
                    val[cur] = 0;
                }
            }
            step++;
            if(step == TURN_OFF_STEP_FITNESS) {
                current_progress = 0;
                leds_anim_off(ANIM_FITNESS);
            }
            break;
        }
        case 7: {
            float lvl = 0;
            float lvl_2 = 0;
            int32_t start_l = 0;
            int32_t finish_l = 0;
            uint32_t adj_progress = current_progress;

            if(duty>1) {
                lvl = ((1.f/(float)FADE_IN_STEP_FITNESS)*(float)led_lum);
                lvl_2 = ((1.f/(float)FADE_IN_STEP_FITNESS)*(float) (led_lum + round(FADE_IN_STEP_FITNESS/2)));
                if (lvl_2 > 1){
                    lvl_2 = 1.f;
                }
            }

            memset(val, 0, LEDS_CNT);
            float lum = (float)leds_anim_brightness;;

            for(int i = 0; i < current_progress; i++) {
                start_l = leds_fitness_l2;
                finish_l = leds_fitness_l1;
                if (i == ((int)current_led)){
                    //lum = (float)1.f;
                }
                else if (i == ((int)current_led - 1)){
                    //lum = (float)(7.f/8.f);
                }
                else if (i == ((int)current_led - 2)){
                    //lum = (float)(3.f/4.f);
                }
                else if (i == ((int)current_led - 3)){
                    //lum = (float)(5.f/8.f);
                }
                else if (i == ((int)current_led - 4)){
                    //lum = (float)(1.f/2.f);
                }
                else {
                    lum = 1.f;
                    start_l = 0;
                }
                /*if (((int)i + (int)current_led - 4) < 0){
                    lum = 0;
                    start_l = 0;
                    finish_l = 0;
                }*/

                uint8_t cur = MOD(((int)i+PROGRESS_OFFSET),LEDS_CNT);
                if (i == ((int) step - 1)){
                    val[cur] = ((float)leds_anim_brightness * (start_l + ((float)((lum * finish_l) - start_l))*lvl));
                    hue[cur] = ((float)leds_fitness_h2 + (float)((float)lvl*((float)leds_fitness_h1 - (float)leds_fitness_h2)));
                    sat[cur] = leds_fitness_s2 + ((float)(leds_fitness_s1 - leds_fitness_s2)*lvl);
                }
                else if (i == ((int) step - 2)){
                    val[cur] = (float)(leds_anim_brightness * (start_l + ((float)((lum * finish_l) - start_l))*lvl_2));
                    hue[cur] = ((float)leds_fitness_h2 + (float)((float)lvl_2*((float)leds_fitness_h1 - (float)leds_fitness_h2)));
                    sat[cur] = leds_fitness_s2 + ((float)(leds_fitness_s1 - leds_fitness_s2)*lvl_2);
                }
                else if (i < ((int)step - 2)){
                    val[cur] = ((float)leds_anim_brightness * finish_l * lum);
                    hue[cur] = (float)leds_fitness_h1;
                    sat[cur] = (float)leds_fitness_s1;
                }
                else {
                    val[cur] = ((float)leds_anim_brightness * start_l);
                    hue[cur] = (float)leds_fitness_h2;
                    sat[cur] = (float)leds_fitness_s2;
                }
            }

            led_lum++;
            lum_change_color++;

            if(step < adj_progress && led_lum > (round(FADE_IN_STEP_FITNESS/2) - 1)){
                step ++;
                led_lum = 0;
            }
            else if(step == adj_progress && led_lum >= FADE_IN_STEP_FITNESS){
                step++;
                led_lum = 0;
            }
            if(step > adj_progress && led_lum == 0) {
                LEDS_ANIM_NEXT_FRAME();
                lum_change_color = 0;
                step = 0;
            }
            break;
        }
        case 8: {
            float lvl = ((1.f/(float)FADE_STEP_FITNESS)*(float)led_lum);
            float lvl_2 = ((1.f/(float)FADE_STEP_FITNESS)*(float) (led_lum + round(FADE_STEP_FITNESS/2)));
            if (lvl_2 > 1){
                    lvl_2 = 1.f;
            }
            int32_t start_l = 0;
            int32_t finish_l = 0;

            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);

            for(int i = 0; i < current_progress; i++) {
                start_l = (float)leds_fitness_l1;
                finish_l = 0;
                if (i == ((int)current_progress - 1)){
                    finish_l = (float)leds_fitness_l1;
                }
                else if (i == ((int)current_progress - 2)){
                    finish_l = (float)leds_fitness_l1*(7.f/8.f);
                }
                else if (i == ((int)current_progress - 3)){
                    finish_l = (float)leds_fitness_l1*(3.f/4.f);
                }
                else if (i == ((int)current_progress - 4)){
                    finish_l = (float)leds_fitness_l1*(5.f/8.f);
                }
                else if (i == ((int)current_progress - 5)){
                    finish_l = (float)leds_fitness_l1*(1.f/2.f);
                }
                /*if (i == current_led){
                    start_l = (float)leds_fitness_l1;
                }
                else if (i == current_led  - 1){
                    start_l = (float)leds_fitness_l1*(7.f/8.f);
                }
                else if (i == current_led - 2){
                    start_l = (float)leds_fitness_l1*(3.f/4.f);
                }
                else if (i == current_led - 3){
                    start_l = (float)leds_fitness_l1*(5.f/8.f);
                }
                else if (i == current_led - 4){
                    start_l = (float)leds_fitness_l1*(1.f/2.f);
                }*/
   
                /*if (((int)i + (int)current_led - 4) < 0){
                    start_l = 0;
                    finish_l = 0;
                }*/
                
                int cur = MOD((((int)i)+ PROGRESS_OFFSET),LEDS_CNT);
                if (i == ((int)step - 1)){
                    val[cur] = ((float)(leds_anim_brightness) * (start_l + ((float)(finish_l - start_l))*lvl));
                }
                else if (i == ((int)step - 2)){
                    val[cur] = (float)(leds_anim_brightness * (start_l + ((float)(finish_l - start_l))*lvl_2));
                }
                else if (i < ((int)step - 2)){
                    val[cur] = ((float)finish_l * leds_anim_brightness);
                }
                else {
                    val[cur] = ((float)start_l * leds_anim_brightness);
                }
            }

            led_lum++;

            if(step < current_progress - 1 && led_lum > (round(FADE_STEP_FITNESS/2) - 1)){
                step ++;
                led_lum = 0;
            }

            else if(step == current_progress - 1 && led_lum == FADE_STEP_FITNESS){
                step++;
                led_lum = 0;
            }

            if(step > current_progress - 1 && led_lum == 0) {
                frame = 3;
                step = 0;
            }

            break;

        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

void leds_anim_step_old_fitness(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t current_progress = 0;
    static uint32_t current_led = 0;
    static uint32_t led_lum = 0;
    static uint32_t led_lum_current_progress = 0;
    static uint32_t currentLed_fadeIn = 1;

    uint32_t progress = 1 + round(((float)(LEDS_CNT-1)) * ((float)leds_fitness_perc / 100.f));
    uint32_t duty = leds_fitness_period >> 5; 

    if(reset) {
        frame = 0;
        step = 0;
        leds_fitness_turnOff = false;
        current_progress = 0;
        led_lum = 0;
        led_lum_current_progress = 0;
        current_led = 0;        
    }
    
    if(current_progress != progress && !leds_fitness_turnOff){
        if (frame == 3 || frame == 4){
            frame = 7;
            led_lum_current_progress = step;
            current_led = current_progress - 1;
            step = 0;
            led_lum = 0;
            current_progress = progress;
        }
        else {
            frame = 1;
            step = 0;
            current_progress = progress;
        }
    }

    if(leds_fitness_turnOff) {
        if ((frame == 2 && step == 0) || (frame == 3 && step == 0)){
            leds_fitness_turnOff = false;
            if (frame == 2){
                frame = 5;           
            }
            else{
                frame = 6;
            }
            step = 0;
        }
    }

    

    switch(frame) {
        case 0: {
            memset(val, 0, LEDS_CNT);
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/FADE_IN_STEP_FITNESS)*(float)led_lum;
            float lvl_2 = (1.f/FADE_IN_STEP_FITNESS)*(float) (led_lum + round(FADE_IN_STEP_FITNESS/2));
            for(int i = 0; i < step; i++) {
                if (i == step - 1){
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_fitness_l1 * leds_anim_brightness * lvl;
                }
                else if (i == step - 2){
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = MIN(leds_fitness_l1 * leds_anim_brightness * lvl_2, leds_fitness_l1 * leds_anim_brightness);
                }
                else{
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_fitness_l1 * leds_anim_brightness;
                }
            }

            led_lum++;
            if(step < current_progress && led_lum > (round(FADE_IN_STEP_FITNESS/2) - 1)){
                step ++;
                led_lum = 0;
            }
            else if(step == current_progress && led_lum == FADE_IN_STEP_FITNESS){
                step++;
                led_lum = 0;
            }
            if(step > current_progress && led_lum == 0) {
                LEDS_ANIM_NEXT_FRAME();
                current_progress = progress;
            }
            break;
        }
        case 2: {
            float lvl = 0;
            lvl = 1.f - ((1.f/(float)FADE_OFF_STEP_FITNESS)*(float)step);
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            for(int i = 0; i < current_progress; i++) {
                if (i == current_progress - 1){
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_fitness_l1 * leds_anim_brightness;
                }
                else {
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_fitness_l1 * lvl * leds_anim_brightness;
                }
            }
            step++;
            if(step > FADE_OFF_STEP_FITNESS) {
                LEDS_ANIM_NEXT_FRAME();
                currentLed_fadeIn = 0;
            }
            break;
        }
        case 3: {
            float lvl = 0;
            if(duty>1) {
                lvl = (1.f/(float)duty)*(float)step;
            }
            uint32_t effh = leds_fitness_h1 + ((float)(leds_fitness_h2 - leds_fitness_h1))*lvl;
            uint32_t effs = leds_fitness_s1 + ((float)(leds_fitness_s2 - leds_fitness_s1))*lvl;
            uint32_t effl = (leds_fitness_l1 + ((float)(leds_fitness_l2 - leds_fitness_l1))*lvl) * leds_anim_brightness;

            memset(hue, effh%256, LEDS_CNT);
            memset(sat, effs, LEDS_CNT);
            memset(val, 0, LEDS_CNT);

            val[(current_progress - 1 +PROGRESS_OFFSET)%LEDS_CNT] = effl;

            if(duty > 0) {
                step++;
            }
            if(duty && (step == duty)) {
                currentLed_fadeIn = 1;
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 4: {
            float lvl = 0;
            if(duty>1) {
                lvl = (1.f/(float)duty)*(float)step;
            }
            uint32_t effh = leds_fitness_h2 + ((float)(leds_fitness_h1 - leds_fitness_h2))*lvl;
            uint32_t effs = leds_fitness_s2 + ((float)(leds_fitness_s1 - leds_fitness_s2))*lvl;
            uint32_t effl = (leds_fitness_l2 + ((float)(leds_fitness_l1 - leds_fitness_l2))*lvl) * leds_anim_brightness;

            memset(hue, effh%256, LEDS_CNT);
            memset(sat, effs, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            val[(current_progress - 1 +PROGRESS_OFFSET)%LEDS_CNT] = effl;

            if(duty > 0) {
                step++;
            }
            if(!duty || (step == duty)) {
                frame = 3;
                currentLed_fadeIn = 0;
                step = 0;
            }
            break;
        }
        case 5: {
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - (1.f/TURN_OFF_STEP_FITNESS)*(float)step;
            for(int i = 0; i < current_progress; i++) {
                val[(i+PROGRESS_OFFSET)%LEDS_CNT] = ((float) leds_fitness_l1 * leds_anim_brightness * lvl);
            }
            step++;
            if(step == TURN_OFF_STEP_FITNESS) {
                current_progress = 0;
                leds_anim_off(ANIM_FITNESS);
            }
            break;
        }
        case 6: {
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - (1.f/TURN_OFF_STEP_FITNESS)*(float)step;
            val[(current_progress - 1+PROGRESS_OFFSET)%LEDS_CNT] = ((float) leds_fitness_l1 * leds_anim_brightness * lvl);
            step++;
            if(step == TURN_OFF_STEP_FITNESS) {
                current_progress = 0;
                leds_anim_off(ANIM_FITNESS);
            }
            break;
        }
        case 7: {
            memset(hue, leds_fitness_h1%256, LEDS_CNT);
            memset(sat, leds_fitness_s1, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/FADE_IN_STEP_FITNESS)*(float)led_lum;
            float lvl_2 = (1.f/FADE_IN_STEP_FITNESS)*(float) (led_lum + round(FADE_IN_STEP_FITNESS/2));

            //So that the led that is currently flashing continue flashing
            float lvl_current_progress = 0;
            if(duty>1) {
                lvl_current_progress = (1.f/(float)duty)*(float)led_lum_current_progress;
            }
            
            if (currentLed_fadeIn){
                val[(current_led + PROGRESS_OFFSET)%LEDS_CNT] = (leds_fitness_l2 + ((float)(leds_fitness_l1 - leds_fitness_l2))*lvl_current_progress) * leds_anim_brightness;
            }
            else{
                val[(current_led + PROGRESS_OFFSET)%LEDS_CNT] = (leds_fitness_l1 + ((float)(leds_fitness_l2 - leds_fitness_l1))*lvl_current_progress) * leds_anim_brightness;
            }
            
            //When the progress catches up with the led that is flashing we increase the lightning of this LED until we reach the desired value
            for(int i = 0; i < step; i++) {
                if (i == step - 1){
                    if (i == current_led){
                        val[(i+PROGRESS_OFFSET)%LEDS_CNT] = MIN((((float)val[(i+PROGRESS_OFFSET)%LEDS_CNT]/leds_anim_brightness) + ((float) (leds_fitness_l1 - ((float)val[(i+PROGRESS_OFFSET)%LEDS_CNT]/leds_anim_brightness))) * lvl), leds_fitness_l1) *      leds_anim_brightness;
                    }
                    else{
                        val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_fitness_l1 * leds_anim_brightness * lvl;
                    }
                }
                else if (i == step - 2){
                    if (i == current_led){
                        val[(i+PROGRESS_OFFSET)%LEDS_CNT] = MIN((((float)val[(i+PROGRESS_OFFSET)%LEDS_CNT]/leds_anim_brightness) + ((float)(leds_fitness_l1 - ((float)val[(i+PROGRESS_OFFSET)%LEDS_CNT]/leds_anim_brightness))) * lvl_2), leds_fitness_l1) * leds_anim_brightness;
                    }
                    else{
                        val[(i+PROGRESS_OFFSET)%LEDS_CNT] = MIN((leds_fitness_l1 * lvl_2), leds_fitness_l1) * leds_anim_brightness ;
                    }
                }
                else{
                    val[(i+PROGRESS_OFFSET)%LEDS_CNT] = leds_fitness_l1 * leds_anim_brightness;
                }
            }

            led_lum++;

            if (step - 1 < current_led){
                led_lum_current_progress++;
                if (led_lum_current_progress >= duty){
                    currentLed_fadeIn = !currentLed_fadeIn;
                    led_lum_current_progress = 0;
                }
            }

            if(step < current_progress && led_lum > (round(FADE_IN_STEP_FITNESS/2) - 1)){
                step ++;
                led_lum = 0;
            }
            else if(step == current_progress && led_lum == FADE_IN_STEP_FITNESS){
                step++;
                led_lum = 0;
            }
            if(step > current_progress && led_lum == 0) {
                frame = 2;
                step = 0;
            }
            break;
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define TOUCH_MAX 5

bool leds_touch_param_state;
touch_type_t leds_touch_param_type;
uint32_t leds_touch_param_code;
uint32_t leds_touch_param_codeLen;

void leds_anim_step_touch(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    uint32_t maxlum = 255 * leds_anim_brightness;
    uint32_t timeOutTick = 250;
    static uint32_t step = 0;

    if(reset) {
        step = 0;
    }

    memset(val, 0, LEDS_CNT);
    if(ui_isAuth()){
        memset(sat, 0, LEDS_CNT);
        memset(hue, 0, LEDS_CNT);
    }else{
        memset(sat, 209, LEDS_CNT);
        memset(hue, 21, LEDS_CNT);
    }

    int maxLen = MIN(TOUCH_MAX, leds_touch_param_codeLen);
    int i, val1;
    for(i = 0; i < maxLen; i++) {
        if((leds_touch_param_code >> i) & 1) {
            val1 = maxlum;
            
        } else {
            val1 = 0;
        }
        val[(18+i*3+0)%LEDS_CNT] = maxlum;
        val[(18+i*3+1)%LEDS_CNT] = val1;
        val[(18+i*3+2)%LEDS_CNT] = 0;
    }
    if(leds_touch_param_state && i < TOUCH_MAX) {
        if(leds_touch_param_type & 1) {
            val1 = maxlum;
        } else {
            val1 = 0;
        }
        val[(18+i*3+0)%LEDS_CNT] = maxlum;
        val[(18+i*3+1)%LEDS_CNT] = val1;
        val[(18+i*3+2)%LEDS_CNT] = 0;
    }

    step++;
    if(step == timeOutTick) {
        if(!leds_touch_param_state){
            leds_anim_off(ANIM_TOUCH);
        }else{
            step--;
        }
    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

void leds_touch_complete(){
    leds_anim_off(ANIM_TOUCH);    
}

#define FADE_STEP_COMPASS   10
#define MOVE_STEP_COMPASS   50

int32_t leds_compass_param_h;
int32_t leds_compass_param_s;
int32_t leds_compass_param_l;
float leds_compass_param_heading;
bool leds_compass_turnOff = false; 

void leds_anim_step_compass(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];
    
    uint32_t maxlum = 255 * leds_anim_brightness;

    static uint32_t frame = 0;
    static uint32_t step = 0;
    (void)step;
    
    if(reset) {
        frame = 0;
        step = 0;
        leds_compass_turnOff = false;
    }

    if (leds_compass_turnOff && frame == 4){
        leds_compass_turnOff = false;
        frame = 5;
        step = 0;
    }

    switch(frame) {
        case 0: {
            memset(hue, leds_compass_param_h, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/(float)FADE_STEP_COMPASS)*(float)step;
            float head = 0;
            float decimal = head - (int32_t)head;
            uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
            uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
            uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
            uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
            val[ptr1] = (float)maxlum * lvl;
            val[ptr2] = (float)maxlum * lvl;
            val[ptr3] = (float)((1.f - decimal) * maxlum * lvl);
            val[ptr4] = (float)(decimal * maxlum * lvl);
            step ++;
            if (step >= FADE_STEP_COMPASS){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 1: {
            memset(hue, leds_compass_param_h, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/(float)MOVE_STEP_COMPASS)*(float)step;
            float head = ((float)LEDS_CNT * lvl);
            float decimal = head - (int32_t)head;
            uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
            uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
            uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
            uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
            val[ptr1] = (float)maxlum;
            val[ptr2] = (float)maxlum;
            val[ptr3] = (float)((1.f - decimal) * maxlum);
            val[ptr4] = (float)(decimal * maxlum);
            step ++;
            if (step >= MOVE_STEP_COMPASS){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            memset(hue, leds_compass_param_h, LEDS_CNT);
            memset(sat, 0, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float head_goal = (-((met_getCompass() - leds_compass_param_heading) / 360.f) * ((float)LEDS_CNT));
            if(head_goal<0) {
                head_goal += ((float)LEDS_CNT);
            }
            float lvl = (1.f/(float)MOVE_STEP_COMPASS)*(float)step;
            float head = (head_goal * lvl);
            float decimal = head - (int32_t)head;
            uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
            uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
            uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
            uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
            val[ptr1] = (float)leds_anim_brightness * leds_compass_param_l;
            val[ptr2] = (float)leds_anim_brightness * leds_compass_param_l;
            val[ptr3] = (float)((1.f - decimal) * leds_compass_param_l * leds_anim_brightness);
            val[ptr4] = (float)(decimal * leds_compass_param_l * leds_anim_brightness);
            memset(sat, (float) lvl * (leds_compass_param_s), LEDS_CNT);
            step ++;
            if (step >= MOVE_STEP_COMPASS){
                frame = 4;
                step = 0;
            }
            break;
        }
        case 3: {
            memset(hue, leds_compass_param_h, LEDS_CNT);
            memset(sat, leds_compass_param_s, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = (1.f/(float)FADE_STEP_COMPASS)*(float)step;
            float head = (-((met_getCompass() - leds_compass_param_heading) / 360.f) * ((float)LEDS_CNT));
            if(head<0) {
                head += ((float)LEDS_CNT);
            }
            float decimal = head - (int32_t)head;
            uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
            uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
            uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
            uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
            val[ptr1] = (float)leds_anim_brightness * leds_compass_param_l * lvl;
            val[ptr2] = (float)leds_anim_brightness * leds_compass_param_l * lvl;
            val[ptr3] = (float)((1.f - decimal) * leds_compass_param_l * leds_anim_brightness * lvl);
            val[ptr4] = (float)(decimal * leds_compass_param_l * leds_anim_brightness * lvl);
            step++;
            if (step >= FADE_STEP_COMPASS){
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 4: {
            memset(hue, leds_compass_param_h, LEDS_CNT);
            memset(sat, leds_compass_param_s, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float head = (-((met_getCompass() - leds_compass_param_heading) / 360.f) * ((float)LEDS_CNT));
            if(head<0) {
                head += ((float)LEDS_CNT);
            }
            float decimal = head - (int32_t)head;
            uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
            uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
            uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
            uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
            val[ptr1] = (float)leds_anim_brightness * leds_compass_param_l;
            val[ptr2] = (float)leds_anim_brightness * leds_compass_param_l;
            val[ptr3] = (float)((1.f - decimal) * leds_compass_param_l * leds_anim_brightness);
            val[ptr4] = (float)(decimal * leds_compass_param_l * leds_anim_brightness);
            break;
        }

        case 5: {
            memset(hue, leds_compass_param_h, LEDS_CNT);
            memset(sat, leds_compass_param_s, LEDS_CNT);
            memset(val, 0, LEDS_CNT);
            float lvl = 1.f - ((1.f/(float)FADE_STEP_COMPASS)*(float)step);
            float head = (-((met_getCompass() - leds_compass_param_heading) / 360.f) * ((float)LEDS_CNT));
            if(head<0) {
                head += ((float)LEDS_CNT);
            }
            float decimal = head - (int32_t)head;
            uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
            uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
            uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
            uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
            val[ptr1] = (float)leds_anim_brightness * leds_compass_param_l * lvl;
            val[ptr2] = (float)leds_anim_brightness * leds_compass_param_l * lvl;
            val[ptr3] = (float)((1.f - decimal) * leds_compass_param_l * leds_anim_brightness * lvl);
            val[ptr4] = (float)(decimal * leds_compass_param_l * leds_anim_brightness * lvl);  
            step ++;
            if (step > FADE_STEP_COMPASS){
                leds_anim_off(ANIM_COMPASS);
            }
            break;
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}

#define FADE_STEP_POINTER   10
#define MOVE_STEP_POINTER   50
#define COMPASS_INTRO_HUE_SHIFT_STEPS 30
#define NAV_INTRO_FADE 30
uint8_t leds_pointer_intro_mode;

int32_t leds_pointer_param_h;
int32_t leds_pointer_param_s;
int32_t leds_pointer_param_l;
float leds_pointer_param_heading;
uint32_t leds_pointer_param_standby = 0;
bool leds_pointer_turnOff = false; 
bool leds_pointer_intro = false;

void leds_anim_step_pointer(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];
    
    uint32_t maxlum = leds_pointer_param_l * leds_anim_brightness;
    static float current_heading = 0;
    static float goal_heading = 0;
    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t led_lum = 0;
    static uint32_t standby = 0;
    static uint32_t transition_step = 0;
    static uint8_t stage = 0;
    
    if(reset) {
        frame = 0;
        step = 0;
        leds_pointer_turnOff = false;
        current_heading = leds_pointer_param_heading;
        goal_heading = leds_pointer_param_heading;
        standby = 0;
        led_lum = 0;
        transition_step = 0;
        stage = leds_pointer_intro ? 0 : 1;
    }
    
    switch(stage){
        case 0:{
            memset(hue,0,LEDS_CNT);
            memset(sat,0,LEDS_CNT);
            memset(val,0,LEDS_CNT);
            switch(leds_pointer_intro_mode){
                case 0:{
                    uint8_t hues[LEDS_CNT] = {91, 91, 91, 77, 
                                            77, 77, 56, 56,
                                            56, 43, 43, 43, 
                                            38, 38, 38, 28, 
                                            28, 28, 24, 24, 
                                            24, 14, 14, 14};
                    uint8_t saturations[LEDS_CNT] = {175, 175, 175, 167, 
                                                    167, 167, 195, 195, 
                                                    195, 220, 220, 220, 
                                                    233, 233, 233, 219, 
                                                    219, 219, 203, 203, 
                                                    203, 187, 187, 187};
                    for(int i = 0; i<LEDS_CNT; i++){
                        hue[i] = hues[i];
                        sat[i] = saturations[i];
                    }
                    switch(frame) {
                        case 0:{
                            memset(val, 0, LEDS_CNT);
                            uint32_t curLum = (float)leds_anim_brightness * 255.f;
                            uint32_t position = step/2;
                            val[position%24] = curLum;
                            val[(position+1)%24] = curLum;
                            val[(position+2)%24] = curLum;
                            step++;
                            if(step == LEDS_CNT*2-4){
                                LEDS_ANIM_NEXT_FRAME();
                                stage++;
                                frame = 0;
                            }
                            break;
                        }
                        
                    }
                    leds_util_copyHSVtoRGB(hue,sat,val);
                    break;
                }
                case 1:{
                    uint8_t hues[LEDS_CNT] = {2, 14, 24, 28, 
                                                32, 38, 43, 49, 
                                                56, 65, 77, 84, 
                                                91, 84, 77, 65, 
                                                65, 49, 43, 38, 
                                                32, 28, 24, 14};
                    uint8_t saturations[LEDS_CNT] = {171, 187, 203, 219, 
                                                    226, 233, 220, 208, 
                                                    195, 182, 167, 171, 
                                                    175, 171, 167, 182, 
                                                    195, 208, 220, 233, 
                                                    226, 203, 187, 171};
                    for(int i = 0; i<LEDS_CNT; i++){
                        hue[i] = hues[i];
                        sat[i] = saturations[i];
                    }
                    switch(frame) {
                        case 0: {
                            // memset(val, 0, LEDS_CNT);
                            val[(10)%24] = maxlum;
                            val[(11)%24] = maxlum>>1;
                            val[(12)%24] = maxlum>>2;
                            val[(13)%24] = maxlum>>1;
                            val[(14)%24] = maxlum;
                            LEDS_ANIM_NEXT_FRAME();
                            break;
                        }
                        case 1: {
                            float lvl = (float)step/NAV_INTRO_FADE;
                            int32_t increment = (float)NAV_INTRO_FADE*lvl/2.5f;
                            val[(12-increment)%24] = 0;
                            val[(11-increment)%24] = maxlum>>2;
                            val[(10-increment)%24] = maxlum>>1;
                            val[(9- increment)%24] = maxlum;
                            val[(increment+12)%24] = 0;
                            val[(increment+13)%24] = maxlum>>2;
                            val[(increment+14)%24] = maxlum>>1;
                            val[(increment+15)%24] = maxlum;
                            step++;
                            if(step == LEDS_CNT + 1) {
                                LEDS_ANIM_NEXT_FRAME();
                                val[1] = 0;
                                val[2] = 0;
                            }
                           break;
                        }
                        case 2:{
                            float lvl = (float)step*2/COMPASS_INTRO_HUE_SHIFT_STEPS;
                            uint8_t new_hue = step*2 < COMPASS_INTRO_HUE_SHIFT_STEPS ? 2 + (12*lvl) : 14;
                            memset(hue, new_hue, LEDS_CNT);
                            val[22] = lvl*maxlum;
                            val[23] = lvl*maxlum;
                            val[0] = lvl*maxlum;
                            if(step == COMPASS_INTRO_HUE_SHIFT_STEPS/2){
                                LEDS_ANIM_NEXT_FRAME();
                                stage++;
                                frame = 0;    
                            }
                            step++;
                            break;
                        }
                    }   
                    leds_util_copyHSVtoRGB(hue,sat,val);
                    break;
                }
                case 2:{
                    uint8_t hues[LEDS_CNT] = {2, 14, 24, 28, 
                                                32, 38, 43, 49, 
                                                56, 65, 77, 84, 
                                                91, 84, 77, 65, 
                                                65, 49, 43, 38, 
                                                32, 28, 24, 14};
                    uint8_t saturations[LEDS_CNT] = {171, 187, 203, 219, 
                                                    226, 233, 220, 208, 
                                                    195, 182, 167, 171, 
                                                    175, 171, 167, 182, 
                                                    195, 208, 220, 233, 
                                                    226, 203, 187, 171};
                    for(int i = 0; i<LEDS_CNT; i++){
                        hue[i] = hues[i];
                        sat[i] = saturations[i];
                    }
                    switch(frame) {
                        case 0: {
                            float lvl = (float)step/NAV_INTRO_FADE;
                            memset(val, (float)maxlum*lvl, LEDS_CNT);
                            if(step == NAV_INTRO_FADE){
                                LEDS_ANIM_NEXT_FRAME();
                            }
                            step++;
                            break;
                        }
                        case 1: {
                            memset(val, maxlum, LEDS_CNT);
                            step++;
                            if(step == NAV_INTRO_FADE) {
                                LEDS_ANIM_NEXT_FRAME();
                            }
                            break;
                        }
                        case 2: {
                            memset(val, maxlum, LEDS_CNT);
                            for(int j = 0; j<=step && j<=11; j++){                                    
                                if(j<10){
                                    val[(j+12)%24] = 0;
                                }
                                val[(12-j)%24] = 0;
                            } 
                            step++;
                            if(step == NAV_INTRO_FADE/2){
                                LEDS_ANIM_NEXT_FRAME();
                            }
                            break;
                        }
                        case 3: {
                            if(step>COMPASS_INTRO_HUE_SHIFT_STEPS/8){
                                val[2] = 0;
                            }
                            if(step>COMPASS_INTRO_HUE_SHIFT_STEPS/4){
                                val[1] = 0;
                            }
                            float lvl = (float)step*2/COMPASS_INTRO_HUE_SHIFT_STEPS;
                            uint8_t new_hue = step*2 < COMPASS_INTRO_HUE_SHIFT_STEPS ? 2 + (12*lvl) : 14;
                            memset(hue, new_hue, LEDS_CNT);
                            val[22] = maxlum;
                            val[23] = maxlum;
                            val[0] = maxlum; 
                            step++;
                            if(step == COMPASS_INTRO_HUE_SHIFT_STEPS/2){
                                LEDS_ANIM_NEXT_FRAME();
                                stage++;
                                frame = 0;
                            }
                            break;
                        }
                    }   
                    leds_util_copyHSVtoRGB(hue,sat,val);
                    break;    
                }
                case 3:{
                    uint8_t hues[LEDS_CNT] = {14, 91, 91, 77, 
                                            77, 77, 56, 56,
                                            56, 43, 43, 43, 
                                            38, 38, 38, 28, 
                                            28, 28, 24, 24, 
                                            24, 14, 14, 14};
                    uint8_t saturations[LEDS_CNT] = {171, 175, 175, 167, 
                                                    167, 167, 195, 195, 
                                                    195, 220, 220, 220, 
                                                    233, 233, 233, 219, 
                                                    219, 219, 203, 203, 
                                                    203, 187, 171, 171};
                    for(int i = 0; i<LEDS_CNT; i++){
                        hue[i] = hues[i];
                        sat[i] = saturations[i];
                    }
                    switch(frame) {
                        case 0: {
                            float lvl = (float)step/NAV_INTRO_FADE;
                            memset(val, (float)maxlum*lvl, LEDS_CNT);
                            if(step == NAV_INTRO_FADE){
                                LEDS_ANIM_NEXT_FRAME();
                            }
                            step++;
                            break;
                        }
                        case 1: {
                            memset(val, maxlum, LEDS_CNT);
                            step++;
                            if(step == NAV_INTRO_FADE) {
                                LEDS_ANIM_NEXT_FRAME();
                            }
                            break;
                        }
                        case 2: {
                            memset(val, maxlum, LEDS_CNT);
                            for(int j = 1; j<=step && j<22; j++){                                    
                                val[j] = 0;
                            } 
                            step++;
                            if(step == 22){
                                LEDS_ANIM_NEXT_FRAME();
                                stage++;
                                frame = 0;
                            }
                            break;
                        }
                        case 3: {
                            if(step>COMPASS_INTRO_HUE_SHIFT_STEPS/8){
                                val[2] = 0;
                            }
                            if(step>COMPASS_INTRO_HUE_SHIFT_STEPS/4){
                                val[1] = 0;
                            }
                            float lvl = (float)step*2/COMPASS_INTRO_HUE_SHIFT_STEPS;
                            uint8_t new_hue = step*2 < COMPASS_INTRO_HUE_SHIFT_STEPS ? 2 + (12*lvl) : 14;
                            memset(hue, new_hue, LEDS_CNT);
                            val[22] = maxlum;
                            val[23] = maxlum;
                            val[0] = maxlum; 
                            step++;
                            if(step == COMPASS_INTRO_HUE_SHIFT_STEPS/2){
                                LEDS_ANIM_NEXT_FRAME();
                                stage++;
                                frame = 0;
                            }
                            break;
                        }
                    }   
                    leds_util_copyHSVtoRGB(hue,sat,val);
                    break;    
                }
                break;
            }
            break;
        }
        case 1:{
            if (goal_heading != leds_pointer_param_heading && !leds_pointer_turnOff && !leds_pointer_param_standby && !standby){
                frame = 3;
                step = 1;
                goal_heading = leds_pointer_param_heading;
            }
            
            if (leds_pointer_param_standby && !standby && !leds_pointer_turnOff){
                if (frame == 0){
                    frame = 4;
                    step = 0;
                    standby = leds_pointer_param_standby;
                }
                else if (frame == 1){
                    frame = 5;
                    step = ((float)goal_heading/LEDS_CNT * MOVE_STEP_POINTER);
                    standby = leds_pointer_param_standby;
                }
            }
            
            if (!leds_pointer_param_standby && standby){
                standby = leds_pointer_param_standby;
                goal_heading = leds_pointer_param_heading;
            }

            if (leds_pointer_turnOff && frame == 1){
                leds_pointer_turnOff = false;
                frame = 2;
                step = 0;
            }
            
            if (leds_pointer_turnOff && frame == 5){
                leds_pointer_turnOff = false;
                frame = 7;
                step = MAX(0, ((int32_t)step - 1));
                led_lum = 0;
            }

            uint8_t transitioning_hue = transition_step < COMPASS_INTRO_HUE_SHIFT_STEPS ? 14 + (float)(leds_pointer_param_h - 14)*(float)transition_step/(COMPASS_INTRO_HUE_SHIFT_STEPS) : leds_pointer_param_h;
            transition_step++;
            switch(frame){
                case 0: {
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);                    
                    memset(val, 0, LEDS_CNT);
                    uint32_t curLum = (float)leds_anim_brightness * leds_pointer_param_l;
                    uint32_t position = step/2;
                    val[(position+22)%24] = curLum;
                    val[(position+23)%24] = curLum;
                    val[(position+00)%24] = curLum;
                    step++;
                    if(position == (uint8_t)leds_pointer_param_heading + 2){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    break;
                }
                case 1: {
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);
                    memset(val, 0, LEDS_CNT);
                    float decimal = leds_pointer_param_heading - (int32_t)leds_pointer_param_heading;
                    uint32_t ptr1 = ((uint32_t)leds_pointer_param_heading % LEDS_CNT);
                    uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
                    uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
                    uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
                    val[ptr1] = (float)maxlum;
                    val[ptr2] = (float)maxlum;
                    val[ptr3] = (float)((1.f - decimal) * maxlum);
                    val[ptr4] = (float)(decimal * maxlum);
                    break;
                }

                case 2: {
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);
                    memset(val, 0, LEDS_CNT);
                    float lvl = 1.f - ((1.f/(float)FADE_STEP_POINTER)*(float)step);
                    float decimal = leds_pointer_param_heading - (int32_t)leds_pointer_param_heading;
                    uint32_t ptr1 = ((uint32_t)leds_pointer_param_heading % LEDS_CNT);
                    uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
                    uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
                    uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
                    val[ptr1] = (float)maxlum * lvl;
                    val[ptr2] = (float)maxlum * lvl;
                    val[ptr3] = (float)((1.f - decimal) * maxlum * lvl);
                    val[ptr4] = (float)(decimal * maxlum * lvl);  
                    step ++;
                    if (step > FADE_STEP_POINTER){
                        leds_anim_off(POINTER_ANIMATION);
                    }
                    break;
                }
                case 3 :{
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);
                    memset(val, 0, LEDS_CNT);
                    float lvl = (1.f/(float)MOVE_STEP_POINTER)*(float)step;
                    float move = (int32_t)ABS_LEDS_LOOP((int32_t)goal_heading,(int32_t)current_heading);
                    float head = ((float)current_heading + ((float) move * lvl));
                    float decimal = head - (int32_t)head;
                    if (decimal < 0)
                    {
                        decimal = decimal * -1;
                    }

                    uint32_t ptr1 = MOD((int32_t)head,LEDS_CNT);
                    uint32_t ptr2 = ptr1;
                    uint32_t ptr4 = ptr1; 
                    uint32_t ptr3 = ptr1;

                    if (head < 0){
                        ptr2 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                        ptr4 = MOD((int32_t)((int32_t)ptr1-2), LEDS_CNT);
                        ptr3 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                    }
                    else{
                        ptr2 = MOD((int32_t)((int32_t)ptr1+1), LEDS_CNT);
                        ptr4 = MOD((int32_t)((int32_t)ptr1+2), LEDS_CNT);
                        ptr3 = MOD((int32_t)((int32_t)ptr1-1), LEDS_CNT);
                    }         
                    val[ptr1] = (float)maxlum;
                    val[ptr2] = (float)maxlum;
                    val[ptr3] = (float)((1.f - decimal) * maxlum);
                    val[ptr4] = (float)(decimal * maxlum); 
                    step ++;

                    if (step >= MOVE_STEP_POINTER){
                        frame = 1;
                        step = 0;
                        current_heading = goal_heading;
                    }
                    break;
                }
                case 4: {
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);
                    memset(val, 0, LEDS_CNT);
                    float lvl = (1.f/(float)MOVE_STEP_POINTER)*(float)step;
                    float head = ((float)LEDS_CNT * lvl);
                    float decimal = head - (int32_t)head;
                    uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
                    uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
                    uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
                    uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
                    val[ptr1] = (float)maxlum;
                    val[ptr2] = (float)maxlum;
                    val[ptr3] = (float)((1.f - decimal) * maxlum);
                    val[ptr4] = (float)(decimal * maxlum);
                    if (step >= MOVE_STEP_POINTER){
                        LEDS_ANIM_NEXT_FRAME();
                        step = 1;
                    }
                    step ++;
                    break;
                }
                case 5: {
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);
                    memset(val, 0, LEDS_CNT);
                    float lvl = (1.f/(float)MOVE_STEP_POINTER)*(float)step;
                    float head = ((float)LEDS_CNT * lvl)-1;
                    float decimal = head - (int32_t)head;
                    uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
                    uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
                    uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
                    uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
                    val[ptr1] = (float)maxlum;
                    val[ptr2] = (float)maxlum;
                    val[ptr3] = (float)((1.f - decimal) * maxlum);
                    val[ptr4] = (float)(decimal * maxlum);
                    if (step >= MOVE_STEP_POINTER && standby){
                        step = 1;
                    }
                    else if (step >= MOVE_STEP_POINTER && !standby){
                        frame = 6;
                        step = 1;
                    }
                    step ++;
                    break;
                }
                case 6: {
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);
                    memset(val, 0, LEDS_CNT);
                    float head_goal = leds_pointer_param_heading;
                    float lvl = (1.f/(float)MOVE_STEP_POINTER)*(float)step;
                    float head = (head_goal * lvl);
                    float decimal = head - (int32_t)head;
                    uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
                    uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
                    uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
                    uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
                    val[ptr1] = (float)leds_anim_brightness * leds_pointer_param_l;
                    val[ptr2] = (float)leds_anim_brightness * leds_pointer_param_l;
                    val[ptr3] = (float)((1.f - decimal) * leds_pointer_param_l * leds_anim_brightness);
                    val[ptr4] = (float)(decimal * leds_pointer_param_l * leds_anim_brightness);
                    step ++;
                    if (step >= MOVE_STEP_POINTER){
                        frame = 1;
                        step = 0;
                    }
                    break;
                }
                case 7:{
                    memset(hue, transitioning_hue, LEDS_CNT);
                    memset(sat, leds_pointer_param_s, LEDS_CNT);
                    memset(val, 0, LEDS_CNT);
                    float leds_to_light = ((1.f/(float)MOVE_STEP_POINTER)*(float)step);
                    float lvl = 1.f - ((1.f/(float)FADE_STEP_POINTER)*(float)led_lum);
                    float head = ((float)LEDS_CNT * leds_to_light);
                    float decimal = head - (int32_t)head;
                    uint32_t ptr1 = ((uint32_t)head % LEDS_CNT);
                    uint32_t ptr2 = MOD((ptr1+1), LEDS_CNT);
                    uint32_t ptr4 = MOD((ptr1+2), LEDS_CNT);
                    uint32_t ptr3 = MOD((int32_t)(ptr1-1), LEDS_CNT);
                    val[ptr1] = (float)maxlum * lvl;
                    val[ptr2] = (float)maxlum * lvl;
                    val[ptr3] = (float)((1.f - decimal) * maxlum * lvl);
                    val[ptr4] = (float)(decimal * maxlum * lvl);
                    led_lum++;
                    if (led_lum > FADE_STEP_POINTER){
                        leds_anim_off(POINTER_ANIMATION);
                    }
                    break;
                }

            }
            leds_util_copyHSVtoRGB(hue,sat,val);
            break;
        }
    }
}

uint8_t leds_turn_intro_mode;

void leds_turn_intro_play(uint8_t mode){
    leds_turn_intro_mode = mode;
    leds_anim_on(TURN_INTRO_ANIMATION, 10);
}

#define COMPASS_INTRO_HUE_SHIFT_STEPS 30
#define NAV_INTRO_FADE 30

void leds_turn_intro_animation(bool reset){
    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t count = 0;

    uint32_t maxlum = leds_anim_brightness * 255.f;

    if(reset) {
        frame = 0;
        step = 0;
        count = 0;
    }

    static uint8_t hue[LEDS_CNT];
    static uint8_t sat[LEDS_CNT];
    static uint8_t val[LEDS_CNT];                    

    switch(leds_turn_intro_mode){
        case 0:{
            if(reset){
                memset(hue, 67, LEDS_CNT);
                memset(sat, 255, LEDS_CNT);
            }
            switch(frame) {
                case 0: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    memset(val, (float)maxlum*lvl, LEDS_CNT);
                    if(step == NAV_INTRO_FADE){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    step++;
                    break;
                }
                case 1: {
                    step++;
                    if(step == NAV_INTRO_FADE) {
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    break;
                }
                case 2: {
                    float lvl = 1.0f - (float)step/NAV_INTRO_FADE;
                    memset(val, (float)maxlum*lvl, LEDS_CNT);
                    step++;
                    if(step == NAV_INTRO_FADE){
                        leds_anim_off(TURN_INTRO_ANIMATION);
                    }
                    break;
                }
            }   
            leds_util_copyHSVtoRGB(hue,sat,val);
            break;    
        }
        case 1:{
            if(reset){
                memset(hue, 67, LEDS_CNT);
                memset(sat, 255, LEDS_CNT);
            }
            switch(frame) {
                case 0: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    memset(val, (float)maxlum*lvl, LEDS_CNT);
                    if(step == NAV_INTRO_FADE){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    step++;
                    break;
                }
                case 1: {
                    step++;
                    if(step == NAV_INTRO_FADE) {
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    break;
                }
                case 2: {
                    sat[step/5] = sat[step/5]>>1;
                    sat[(24-step/5)%24] = sat[step/5]>>1;
                    step++;
                    if(step == LEDS_CNT*6/2){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    break;
                }
                case 3: {
                    float lvl = 1.0f - (float)step/NAV_INTRO_FADE;
                    memset(val, (float)maxlum*lvl, LEDS_CNT);
                    step++;
                    if(step == NAV_INTRO_FADE){
                        leds_anim_off(TURN_INTRO_ANIMATION);
                    }
                    break;
                }
            }   
            leds_util_copyHSVtoRGB(hue,sat,val);
            break;    
        }
        case 2:{
            if(reset){
                memset(hue, 67, LEDS_CNT);
                memset(sat, 255, LEDS_CNT);
            }
            leds_anim_off(TURN_INTRO_ANIMATION);
            break;
            switch(frame) {
                case 0: {
                    memset(val, 0, LEDS_CNT);
                    float lvl = (float)step/NAV_INTRO_FADE;
                    int32_t increment = (float)NAV_INTRO_FADE*lvl/3.0f;
                    val[(6-increment)%24] =  increment == 0 ? 0.33f*maxlum : increment == 1 ? maxlum>>1 : maxlum;
                    val[(18+increment)%24] = increment == 0 ? 0.33f*maxlum : increment == 1 ? maxlum>>1 : maxlum;
                    val[(9-increment)%24] = 0;
                    val[(15+increment)%24] = 0;
                    if(increment > 0){
                        val[(7-increment)%24] = increment == 1 ? 0.33f*maxlum : maxlum>>1;
                        val[(17+increment)%24] = increment == 1 ? 0.33f*maxlum : maxlum>>1;
                    }
                    if(increment > 1){
                        val[(8-increment)%24] = 0.33f*maxlum;
                        val[(16+increment)%24] = 0.33f*maxlum;
                    }
                    step++;
                    if(increment > LEDS_CNT/4){
                        count++; 
                        if(count != 3){
                            step = 0;
                        }else{
                            step = 0;
                            count = 0;
                            frame = 0;
                            leds_anim_off(TURN_INTRO_ANIMATION);
                        }
                    }else{
                        leds_util_copyHSVtoRGB(hue,sat,val);
                    }
                    break;
                }
            }   
            break;
        }
        case 3:{
            if(reset){
                memset(hue, 67, LEDS_CNT);
                memset(sat, 255, LEDS_CNT);
                memset(val, 0, LEDS_CNT);
            }
            switch(frame) {
                case 0: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    int32_t increment = (float)NAV_INTRO_FADE*lvl/5.0f;
                    val[0] = maxlum;
                    val[1] = maxlum;
                    val[2] = maxlum;
                    val[3] = maxlum;
                    val[4] = maxlum;
                    val[5] = maxlum;
                    val[6] = maxlum;
                    sat[0] = increment < 4 ? 0 : increment == 4 ? 0.33f*171 : increment == 5 ? 171>>1 : 171;
                    sat[1] = increment < 3 ? 0 : increment == 3 ? 0.33f*171 : increment == 4 ? 171>>1 : 171;
                    sat[2] = increment < 2 ? 0 : increment == 2 ? 0.33f*171 : increment == 3 ? 171>>1 : 171;
                    sat[3] = increment < 1 ? 0 : increment == 1 ? 0.33f*171 : increment == 2 ? 171>>1 : 171;
                    sat[4] = increment < 2 ? 0 : increment == 2 ? 0.33f*171 : increment == 3 ? 171>>1 : 171;
                    sat[5] = increment < 3 ? 0 : increment == 3 ? 0.33f*171 : increment == 4 ? 171>>1 : 171;
                    sat[6] = increment < 4 ? 0 : increment == 4 ? 0.33f*171 : increment == 5 ? 171>>1 : 171;
                    if(increment == 7){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    step++;
                    break;
                }
                case 1: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    int32_t increment = (float)NAV_INTRO_FADE*lvl/10.0f;
                    val[0] = maxlum * (increment%2) ;
                    val[1] = maxlum * (increment%2) ;
                    val[2] = maxlum * (increment%2) ;
                    val[3] = maxlum * (increment%2) ;
                    val[4] = maxlum * (increment%2) ;
                    val[5] = maxlum * (increment%2) ;
                    val[6] = maxlum * (increment%2) ;
                    if(increment == 4){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    step++;
                    break;
                }
                case 2: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    int32_t increment = (float)NAV_INTRO_FADE*lvl/5.0f;
                    val[15] = maxlum;
                    val[16] = maxlum;
                    val[17] = maxlum;
                    val[18] = maxlum;
                    val[19] = maxlum;
                    val[20] = maxlum;
                    val[21] = maxlum;
                    sat[15] = increment < 4 ? 0 : increment == 4 ? 0.33f*171 : increment == 5 ? 171>>1 : 171;
                    sat[16] = increment < 3 ? 0 : increment == 3 ? 0.33f*171 : increment == 4 ? 171>>1 : 171;
                    sat[17] = increment < 2 ? 0 : increment == 2 ? 0.33f*171 : increment == 3 ? 171>>1 : 171;
                    sat[18] = increment < 1 ? 0 : increment == 1 ? 0.33f*171 : increment == 2 ? 171>>1 : 171;
                    sat[19] = increment < 2 ? 0 : increment == 2 ? 0.33f*171 : increment == 3 ? 171>>1 : 171;
                    sat[20] = increment < 3 ? 0 : increment == 3 ? 0.33f*171 : increment == 4 ? 171>>1 : 171;
                    sat[21] = increment < 4 ? 0 : increment == 4 ? 0.33f*171 : increment == 5 ? 171>>1 : 171;
                    if(increment == 7){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    step++;
                    break;
                }
                case 3: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    int32_t increment = (float)NAV_INTRO_FADE*lvl/10.0f;
                    val[15] = maxlum * (increment%2) ;
                    val[16] = maxlum * (increment%2) ;
                    val[17] = maxlum * (increment%2) ;
                    val[18] = maxlum * (increment%2) ;
                    val[19] = maxlum * (increment%2) ;
                    val[20] = maxlum * (increment%2) ;
                    val[21] = maxlum * (increment%2) ;
                    if(increment == 4){
                        LEDS_ANIM_NEXT_FRAME();
                        memset(sat, 0, LEDS_CNT);
                    }
                    step++;
                    break;
                }
                case 4: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    int32_t increment = (float)NAV_INTRO_FADE*lvl/5.0f;
                    sat[0] = increment < 1 ? 0 : increment == 1 ? 0.33f*171 : increment == 2 ? 171>>1 : 171;
                    sat[1] =  increment < 2 ? 0 : increment == 2 ? 0.33f*171 : increment == 3 ? 171>>1 : 171;
                    sat[23] = increment < 2 ? 0 : increment == 2 ? 0.33f*171 : increment == 3 ? 171>>1 : 171;
                    for(int i=0; i<LEDS_CNT; i++){
                        val[i] = maxlum;
                    }
                    if(increment > 2 && increment < 12){    
                        sat[(24+increment)%24] = 0.33f*171;
                        sat[(24-increment)%24] = 0.33f*171;
                        sat[(24+increment-1)%24] = 171>>1;
                        sat[(24-increment+1)%24] = 171>>1;
                        sat[(24+increment-2)%24] = 171;
                        sat[(24-increment+2)%24] = 171;
                    }
                    sat[14] = increment < 10 ? 0 : increment == 10 ? 0.33f*171 : increment == 11 ? 171>>1 : 171;
                    sat[10] = increment < 10 ? 0 : increment == 10 ? 0.33f*171 : increment == 11 ? 171>>1 : 171;
                    sat[13] = increment < 11 ? 0 : increment == 11 ? 0.33f*171 : increment == 12 ? 171>>1 : 171;
                    sat[11] = increment < 11 ? 0 : increment == 11 ? 0.33f*171 : increment == 12 ? 171>>1 : 171;
                    sat[12] = increment < 12 ? 0 : increment == 12 ? 0.33f*171 : increment == 13 ? 171>>1 : 171;
                    
                    if(increment == LEDS_CNT/2+2){
                        LEDS_ANIM_NEXT_FRAME();
                    }
                    step++;
                    break;
                }
                case 5: {
                    float lvl = (float)step/NAV_INTRO_FADE;
                    int32_t increment = (float)NAV_INTRO_FADE*lvl/10.0f;
                    for(int i=0; i<LEDS_CNT; i++){
                        val[i] = maxlum * (increment%2);
                    }
                    if(increment == 4){
                        LEDS_ANIM_NEXT_FRAME();
                        leds_anim_off(TURN_INTRO_ANIMATION);
                    }
                    step++;
                    break;
                }
                
            }   
            leds_util_copyHSVtoRGB(hue,sat,val);
            break;    
        }
        break;
    }
}


#define FADE_STEP_CALIBRATE 30

float leds_anim_calibrate_level;

void leds_anim_step_calibrate(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    static uint32_t frame = 0;
    static uint32_t step = 0;

    if(reset) {
        step = 0;
    }

    memset(hue, 0, LEDS_CNT);
    memset(sat, 0, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    switch(frame) {
        case 0: {
            memset(hue, 200, LEDS_CNT);
            memset(sat, 255, LEDS_CNT);
            memset(val, 0, LEDS_CNT);

            float lvl;
            if(step < FADE_STEP_CALIBRATE) {
                lvl = 1.f - (1.f/FADE_STEP_CALIBRATE)*((float)step);
            } else {
                lvl = (1.f/FADE_STEP_CALIBRATE)*((float)step-FADE_STEP_CALIBRATE);
            }
            uint8_t lum = (0.5f*leds_anim_brightness + lvl*0.5f*leds_anim_brightness) * 255.f;
            
            uint32_t prog = (5.f*leds_anim_calibrate_level)+1;
            for (int i = 0; i < prog; i++) {
                val[(i)%LEDS_CNT] = lum;
                val[(6+i)%LEDS_CNT] = lum;
                val[(12+i)%LEDS_CNT] = lum;
                val[(18+i)%LEDS_CNT] = lum;
            }
            
            step++;
            step%=(FADE_STEP_CALIBRATE * 2);
            break;
        }

    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}


bool leds_anim_test_touch_active;
uint32_t leds_anim_test_touch_step;

void leds_anim_step_test_touch(bool reset) {
    uint8_t hue[LEDS_CNT];
    uint8_t sat[LEDS_CNT];
    uint8_t val[LEDS_CNT];

    memset(hue, (leds_anim_test_touch_active) ? 85 : 170, LEDS_CNT);
    memset(sat, 255, LEDS_CNT);
    memset(val, 0, LEDS_CNT);

    for(int i = 0; i < 7; i++) {
        int ptr = 21 + (6*leds_anim_test_touch_step) + i;
        val[ptr%LEDS_CNT] = 128;
    }

    leds_util_copyHSVtoRGB(hue,sat,val);
}


uint8_t leds_test_num;
uint8_t leds_test_r;
uint8_t leds_test_g;
uint8_t leds_test_b;

void leds_anim_step_test(bool reset) {
    if(leds_test_num == 0) {
        for(int i = 0; i < LEDS_CNT; i++) {
            leds_buffer_rgb[i*3] = leds_test_r;
            leds_buffer_rgb[(i*3)+1] = leds_test_g;
            leds_buffer_rgb[(i*3)+2] = leds_test_b;
        }
        leds_central_rgb[0] = leds_test_r;
        leds_central_rgb[1] = leds_test_g;
        leds_central_rgb[2] = leds_test_b;
        leds_front_lum = MAX(leds_test_r, MAX(leds_test_g, leds_test_b));
    } else {
        if(leds_test_num < 25) {
            leds_buffer_rgb[(leds_test_num-1)*3] = leds_test_r;
            leds_buffer_rgb[((leds_test_num-1)*3)+1] = leds_test_g;
            leds_buffer_rgb[((leds_test_num-1)*3)+2] = leds_test_b;
        } else {
            if(leds_test_num == 25) {
                leds_central_rgb[0] = leds_test_r;
                leds_central_rgb[1] = leds_test_g;
                leds_central_rgb[2] = leds_test_b;
            } else {
                leds_front_lum = MAX(leds_test_r, MAX(leds_test_g, leds_test_b));
            }
        }
    }
}

//============================================================================
// twi

#define ISSI1_ADDR          (0x78 >> 1)
#define ISSI2_ADDR          (0x7E >> 1)

//'volatile' garantees data in ram so easydma could uses it
volatile uint8_t leds_data_reset[] = { 0x4f, 0 };
volatile  uint8_t leds_data_shutdownmode[] = { 0x00, 0 };
volatile  uint8_t leds_data_normalmode[] = { 0x00, 1 }; 
volatile  uint8_t leds_data_highfrequency[] = { 0x4b, 1 }; 
volatile  uint8_t leds_data_ctrl[37];

uint8_t leds_data_pwms[74];
volatile  uint8_t leds_data_update[] = { 0x25, 0x0 };

static app_twi_transfer_t const leds_trans_pwm[] =
{
    APP_TWI_WRITE(ISSI1_ADDR, leds_data_pwms, 37, 0),
    APP_TWI_WRITE(ISSI2_ADDR, leds_data_pwms+37, 37, 0),
    APP_TWI_WRITE(ISSI1_ADDR, leds_data_update, sizeof(leds_data_update), 0),
    APP_TWI_WRITE(ISSI2_ADDR, leds_data_update, sizeof(leds_data_update), 0),
};

static app_twi_transfer_t const leds_trans_on[] =
{
    APP_TWI_WRITE(ISSI1_ADDR, leds_data_reset, sizeof(leds_data_reset), 0),
    APP_TWI_WRITE(ISSI1_ADDR, leds_data_ctrl, sizeof(leds_data_ctrl), 0),
    APP_TWI_WRITE(ISSI2_ADDR, leds_data_reset, sizeof(leds_data_reset), 0),
    APP_TWI_WRITE(ISSI2_ADDR, leds_data_ctrl, sizeof(leds_data_ctrl), 0),
    APP_TWI_WRITE(ISSI1_ADDR, leds_data_highfrequency, sizeof(leds_data_highfrequency), 0),
    APP_TWI_WRITE(ISSI2_ADDR, leds_data_highfrequency, sizeof(leds_data_highfrequency), 0),
    APP_TWI_WRITE(ISSI1_ADDR, leds_data_normalmode, sizeof(leds_data_normalmode), 0),
    APP_TWI_WRITE(ISSI2_ADDR, leds_data_normalmode, sizeof(leds_data_normalmode), 0),
};
static app_twi_transfer_t const leds_trans_off[] =
{
    APP_TWI_WRITE(ISSI1_ADDR, leds_data_shutdownmode, sizeof(leds_data_shutdownmode), 0),
    APP_TWI_WRITE(ISSI2_ADDR, leds_data_shutdownmode, sizeof(leds_data_shutdownmode), 0),
};

void leds_twi_init() {

    leds_data_pwms[0] = 0x1; //pwm start addr
    leds_data_pwms[37] = 0x1; //pwm start addr

    uint32_t outCurrent = LEDS_MAXCURRENT_DIV-1;
    uint32_t setting = ((outCurrent & 0x3) << 1) | 0x1;
    leds_data_ctrl[0] = 0x26; //ctrl start addr
    for(int i = 1; i < 37; i++) {
        leds_data_ctrl[i] = setting;
    }

}

//============================================================================
// PWM

bool leds_pwm_active_front;

#define LEDS_PWM_PINEN(x,v) x = ((v) ? ((x) & 0x7fffffff) : ((x) | 0x80000000))
#define LEDS_FRONT_PSEL NRF_PWM0->PSEL.OUT[0]
#define LEDS_RED_PSEL NRF_PWM0->PSEL.OUT[1]
#define LEDS_GREEN_PSEL NRF_PWM0->PSEL.OUT[2]
#define LEDS_BLUE_PSEL NRF_PWM0->PSEL.OUT[3]

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static uint16_t leds_pwm_values[] =
{
    0x8000, 0x8000, 0x8000, 0x8000
};

nrf_pwm_sequence_t const leds_pwm_seq0 =
{
    .values.p_individual = (nrf_pwm_values_individual_t*)leds_pwm_values,
    .length          = NRF_PWM_VALUES_LENGTH(leds_pwm_values),
    .repeats         = 0,
    .end_delay       = 0
};

void leds_pwmInit()
{
    ret_code_t err;

    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            FRONTLED_PIN,
            CENTRAL_RED_PIN,
            CENTRAL_GREEN_PIN,
            CENTRAL_BLUE_PIN,
        },
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .base_clock   = NRF_PWM_CLK_16MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 1000,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    err = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    ERR_CHECK("nrf_drv_pwm_init0", err);

    leds_pwm_values[0] = 0x8000;
    leds_pwm_values[1] = 0x8000;
    leds_pwm_values[2] = 0x8000;
    leds_pwm_values[3] = 0x8000;

    leds_pwm_active_front = false;
    LEDS_PWM_PINEN(LEDS_FRONT_PSEL, leds_pwm_active_front);
}


//============================================================================
// Animation engine

typedef struct {
    leds_animId_t id;
    leds_anim_fn_t fn;
    uint32_t shutdownCnt;
    bool reset;
    bool running;
} leds_animation_t;

leds_animation_t leds_anim_stack[] = {
    {CHARGE_SHADOW, leds_animation_charging_shadow, 0, false, false},
    {CHARGE_TRACE, leds_animation_charging_trace, 0, false, false},
    {ANIM_SPEEDOMETER, leds_anim_step_speedometer, 0, false, false},
    {ANIM_CADENCE, leds_anim_step_cadence, 0, false, false},
    {POINTER_ANIMATION, leds_anim_step_pointer, 0, false, false},
    {ANIM_COMPASS, leds_anim_step_compass, 0, false, false},
    {ANIM_CALIBRATE, leds_anim_step_calibrate, 0, false, false},
    {FRONT_LIGHT_ANIMATION, leds_animation_step_front, 0, false, false},
    {ANIM_FITNESS, leds_anim_step_fitness, 0, false, false},
    {ANIM_PROGRESS, leds_anim_step_progress, 0, false, false},  
    {ANIM_ROUNDABOUT, leds_anim_step_roundAbout, 0, false, false},
    {ANIM_NAV, leds_anim_step_nav, 0, false, false},
    {TURN_INTRO_ANIMATION, leds_turn_intro_animation, 0, false, false},
    {ANIM_HB, leds_anim_step_hb, 0, false, false},
    {CLOCK_ANIMATION, leds_animation_step_clock, 0, false, false},
    {ANIM_NOTIF, leds_anim_step_notif, 0, false, false},
    {ANIM_LOWBAT, leds_anim_step_lowbat, 0, false, false},
    {ANIM_BATTERY, leds_anim_step_battery, 0, false, false},
    {ANIM_ANIMSCRIPT, ascr_anim_step_render, 0, false, false},
    {ANIM_ALARM, leds_anim_step_alarm, 0, false, false},
    {ANIM_STILL, leds_anim_step_still, 0, false, false},
    {ANIM_TOUCH, leds_anim_step_touch, 0, false, false},
    {ANIM_DEMO_INTRO, leds_anim_step_demo_intro, 0, false, false},
    {ANIM_LOGO, leds_anim_step_logo, 0, false, false},
    {ANIM_DISCONNECT, leds_anim_step_disconnect, 0, false, false},
    {ANIM_ALARM_DISARM, leds_anim_step_alarm_disarm, 0, false, false},
    {ANIM_ALARM_ARM, leds_anim_step_alarm_arm, 0, false, false},
    {ANIM_TEST, leds_anim_step_test, 0, false, false},
    {ANIM_TEST_TOUCH, leds_anim_step_test_touch, 0, false, false},   
};

#define LEDS_ANIM_STACK_CNT (sizeof(leds_anim_stack)/sizeof(leds_animation_t))

void leds_anim_draw(void *ctx);

bool leds_anim_pwr_state = false;

void leds_anim_pwr_on_done(ret_code_t result, void *ctx) {
    //printf("leds_anim_pwr_on_done %d\r\n", result);
    sch_unique_oneshot(leds_anim_draw, DRAWMS);
}


void leds_anim_pwr_on_twi(void *ctx) {
    //printf("leds_anim_pwr_on_twi\r\n");
    twi_transaction_do(leds_trans_on, sizeof(leds_trans_on)/sizeof(app_twi_transfer_t), leds_anim_pwr_on_done, NULL);
}

void leds_anim_pwr_on() {
    if(!leds_anim_pwr_state) {
        leds_anim_brightness = leds_anim_goal_brightness;
        leds_anim_pwr_state = true;
        for(int i = 0; i < LEDS_CNT*3; i++) {
            leds_data_pwms[leds_toPWMAddr(leds_pixel_lut[i])] = 0;
        }
        //printf("leds_anim_pwr_on\r\n");
#ifndef DBG_MOTION_VISUAL
        nrf_gpio_pin_write(EN_VLED, 1);
#endif
#ifdef TWIENABLE
        nrf_gpio_pin_write(I2C_SDB_PIN, 1);
        sch_unique_oneshot(leds_anim_pwr_on_twi, 50);
#else
        sch_unique_oneshot(leds_anim_draw, DRAWMS);
#endif
#ifndef DBG_MOTION_VISUAL
#ifdef PWMENABLE
        nrf_drv_pwm_simple_playback(&m_pwm0, &leds_pwm_seq0, 1, NRF_DRV_PWM_FLAG_LOOP);
        //nrf_drv_pwm_simple_playback(&m_pwm1, &leds_pwm_seq1, 1, NRF_DRV_PWM_FLAG_LOOP);
#endif
#endif
        //printf("FRONTLED_PIN %08X\r\n", NRF_GPIO->PIN_CNF[FRONTLED_PIN]);

    }
}

void leds_anim_pwr_off_done(ret_code_t result, void *ctx) {
    if(!leds_anim_pwr_state) {
        //printf("leds_anim_pwr_off_done\r\n");
        nrf_gpio_pin_write(I2C_SDB_PIN, 0);
#ifndef DBG_MOTION_VISUAL
        nrf_gpio_pin_write(EN_VLED, 0);
#endif
    }
}

void leds_anim_pwr_off() {
    if(leds_anim_pwr_state) {
        leds_anim_pwr_state = false;
        //printf("leds_anim_pwr_off\r\n");
#ifdef TWIENABLE
        twi_transaction_do(leds_trans_off, sizeof(leds_trans_off)/sizeof(app_twi_transfer_t), leds_anim_pwr_off_done, NULL);
#else
        //printf("leds_anim_pwr_off\r\n");
        nrf_gpio_pin_write(I2C_SDB_PIN, 0);
#ifndef DBG_MOTION_VISUAL
        nrf_gpio_pin_write(EN_VLED, 0);
#endif
#endif
#ifndef DBG_MOTION_VISUAL
#ifdef PWMENABLE
        nrf_drv_pwm_stop(&m_pwm0, true);
        //nrf_drv_pwm_stop(&m_pwm1, true);
#endif
#endif
        leds_pwm_values[0] = 0x8000;
        leds_pwm_values[1] = 0x8000;
        leds_pwm_values[2] = 0x8000;
        leds_pwm_values[3] = 0x8000;
    }
}

void leds_anim_draw_done(ret_code_t result, void *ctx) {
    sch_unique_oneshot(leds_anim_draw, DRAWMS);
}

#define CENTRALMAX 800

#define TOPWM_R(x) MIN(CENTRALMAX-200, (uint16_t)((((float)(x))/255.f)*((float)CENTRALMAX-200)))
#define TOPWM_GB(x) MIN(CENTRALMAX, (uint16_t)((((float)(x))/255.f)*((float)CENTRALMAX)))
#define FADE_STEP_MASTER_BRIGHTNESS 30
void leds_anim_draw(void *ctx) {
    //printf("A\r\n");
    leds_front_lum = 0;
    memset(leds_central_rgb, 0, 3);
    memset(leds_buffer_rgb, 0, LEDS_CNT*3);
    static uint8_t fadestep = 0;

    bool running = false;
    leds_animation_t *anim = NULL;
    for(int i = 0; i < LEDS_ANIM_STACK_CNT; i++) {
        anim = &leds_anim_stack[i];

        if(!anim->shutdownCnt) {
            anim->running = false;
        } else {
            anim->shutdownCnt--;
        }

        running |= anim->running;

        if(anim->running && anim->fn) {
            anim->fn(anim->reset);
            anim->reset = false;
        }
    }

    if(running) {
        for(int i = 0; i < LEDS_CNT*3; i++) {
            leds_data_pwms[leds_toPWMAddr(leds_pixel_lut[i])] = leds_gamma[leds_buffer_rgb[i]];
        }
#ifdef TWIENABLE
        twi_transaction_do(leds_trans_pwm, sizeof(leds_trans_pwm)/sizeof(app_twi_transfer_t), leds_anim_draw_done, NULL);
#else
        sch_unique_oneshot(leds_anim_draw, DRAWMS);
#endif
        leds_pwm_values[0] = 0x8000 | (uint16_t)(10 * MIN(100, leds_front_lum));
        leds_pwm_values[1] = 0x8000 | TOPWM_R(leds_gamma[leds_central_rgb[0]]);
        leds_pwm_values[2] = 0x8000 | TOPWM_GB(leds_gamma[leds_central_rgb[1]]);
        leds_pwm_values[3] = 0x8000 | TOPWM_GB(leds_gamma[leds_central_rgb[2]]);

        bool active = (leds_front_lum != 0);
        if(active != leds_pwm_active_front) {
            leds_pwm_active_front = active;
            LEDS_PWM_PINEN(LEDS_FRONT_PSEL, leds_pwm_active_front);
        }
        
        // master dim fading
        if(fadestep < FADE_STEP_MASTER_BRIGHTNESS && ((round(leds_anim_brightness * 100.f)) != round(leds_anim_goal_brightness * 100.f))) {
            float diff = (leds_anim_goal_brightness - leds_anim_start_brightness);
            leds_anim_brightness += diff*(1.f/(float)FADE_STEP_MASTER_BRIGHTNESS);
            fadestep++;
            if(fadestep == FADE_STEP_MASTER_BRIGHTNESS) {
                leds_anim_brightness = leds_anim_goal_brightness;
                fadestep = 0;
            }
        }
        //

    } else {
        leds_anim_pwr_off();
    }

}

bool leds_state_shutdown = false;

void leds_anim_on(leds_animId_t id, uint32_t timeout) {
    if(leds_state_shutdown) {
        return;
    }
    leds_animation_t *anim = NULL;
    for(int i = 0; i < LEDS_ANIM_STACK_CNT; i++) {
        if(leds_anim_stack[i].id == id) {
            anim = &leds_anim_stack[i];
            break;
        }
    }
    if(!anim) {
        return;
    }

    anim->shutdownCnt = 60 * timeout;
    if(!anim->running) {
        anim->reset = true;
    }
    anim->running = true;

    leds_anim_pwr_on();

}

void leds_anim_off(leds_animId_t id) {
    leds_animation_t *anim = NULL;
    for(int i = 0; i < LEDS_ANIM_STACK_CNT; i++) {
        if(leds_anim_stack[i].id == id) {
            anim = &leds_anim_stack[i];
            anim->running = false;
            break;
        }
    }
}
void leds_anim_allOff() {
    leds_animation_t *anim = NULL;
    for(int i = 0; i < LEDS_ANIM_STACK_CNT; i++) {
        anim = &leds_anim_stack[i];
        anim->running = false;
    }
}

void leds_shutdown() {
    leds_state_shutdown = true;
    leds_anim_allOff();
}

//============================================================================
// Frontlight management

uint8_t leds_front_notify_buf[4];

void leds_front_notify_do(void *ctx) {
    bslink_up_write(leds_front_notify_buf, 4);
}

void leds_front_notify(bool touch) {
    static bool on;
    static bool active;

    if(on != leds_front_setting || active != leds_front_state) {
        on = leds_front_setting;
        active = leds_front_state;
        
        uint32_t ptr = 0;
        leds_front_notify_buf[ptr++] = NOTIFY_FRONTLIGHT;
        leds_front_notify_buf[ptr++] = (on) ? 1 : 0;
        leds_front_notify_buf[ptr++] = (active) ? 1 : 0;
        leds_front_notify_buf[ptr++] = (touch) ? 1 : 0;

        sch_unique_oneshot(leds_front_notify_do, 100);
    }
}

void leds_front_turnOn(void);
void leds_front_turnOff(void);

void leds_movement_monitor(void *ctx) {
    if(leds_movement_active || ui_isDemoMode()) {
        leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);
        sch_unique_oneshot(leds_movement_monitor, (FRONTLIGHT_ON_CYCLE)*1000);
    } else {
        leds_front_turnOff();
        leds_front_notify(false);
    }
}

void leds_front_turnOff() {
    if(leds_front_state) {
        leds_front_resetFade = true;
        leds_front_state = false;
        leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);
        sch_unique_cancel(leds_movement_monitor);
    }
}

void leds_front_turnOn() {
    if(!leds_front_state) {
        leds_front_resetFade = true;
        leds_front_state = true;
        leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);
        sch_unique_oneshot(leds_movement_monitor, (FRONTLIGHT_ON_CYCLE)*1000);
    }
}

void leds_movement_on() {
#ifdef DBG_MOTION_VISUAL
    leds_pwm_values[1] = 0x8000 | TOPWM_R(leds_gamma[192]);
    leds_pwm_values[2] = 0x8000 | TOPWM_GB(leds_gamma[128]);
    leds_pwm_values[3] = 0x8000 | TOPWM_GB(leds_gamma[64]);
#endif
    leds_movement_active = true;
    if(leds_front_setting && !alarm_isEnabled()) {
        leds_front_turnOn();
        //
        leds_front_notify(false);
    }
}

void leds_movement_off() {
#ifdef DBG_MOTION_VISUAL
    leds_pwm_values[1] = 0x8000 | TOPWM_R(leds_gamma[0]);
    leds_pwm_values[2] = 0x8000 | TOPWM_GB(leds_gamma[0]);
    leds_pwm_values[3] = 0x8000 | TOPWM_GB(leds_gamma[0]);
#endif
    leds_movement_active = false;
    if(leds_front_state) {
       leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);
    }
    sch_unique_oneshot(leds_movement_monitor, (FRONTLIGHT_ON_CYCLE)*1000);
}

bool leds_frontlight_get() {
    return leds_front_setting;
}

void leds_frontlight_set(bool on) {
    leds_front_setting = on;
}

bool leds_frontlight_getState() {
    return leds_front_state;
}

//============================================================================

bool leds_demoMode_active = false;
int leds_demoMode_step = 0;

void leds_demoMode(void *ctx) {
    static int progress;
    if(!leds_demoMode_active) {
        return;
    }

    int testcase = 0;
    uint8_t speedometer_demo_values[] = {20, 50, 70, 75, 90, 100, 60, 70, 50, 40, 60, 50, 40, 60, 20, 1};

    if(leds_demoMode_step == testcase++) {
        leds_logo();
        sch_unique_oneshot(leds_demoMode, 4000);
        leds_demoMode_step++;
        progress = 0;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_angle(0x5A, 0xAF, 0xB5, -95, progress, 0x5A, 0xAF, 0xB5, 0, 0, false);
        sch_unique_oneshot(leds_demoMode, (progress == 0) ? 2000 : (progress == 100) ? 2500 : 250);
        progress+=10;
        if(progress > 110) {
            leds_nav_off();
            leds_demoMode_step++;
            progress = 0;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_off();
        sch_unique_oneshot(leds_demoMode, 1000);
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_angle(0x5A, 0xAF, 0xB5, 85, progress, 0x5A, 0xAF, 0xB5, -30, progress > 60 ? progress - 60 : 0, false);
        sch_unique_oneshot(leds_demoMode, (progress == 0) ? 2000 : (progress == 100) ? 2500 : 250);
        progress+=10;
        if(progress > 110) {
            leds_demoMode_step++;
            progress = 60;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_angle(0x5A, 0xAF, 0xB5, -30, progress, 0x5A, 0xAF, 0xB5, 0, 0, false);
        sch_unique_oneshot(leds_demoMode,  (progress == 0) ? 2000 : (progress == 100) ? 2500 : 250);
        progress+=10;
        if(progress > 110) {
            leds_nav_off();
            leds_demoMode_step++;
            progress = 0;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_off();
        sch_unique_oneshot(leds_demoMode, 1000);
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_pointer(67*progress/400, 255, 255, progress < 100 ? 90 
            : progress < 200 ? 0 
            : progress < 300 ? -103 
            : -8
            , progress < 10,
            false);
        sch_unique_oneshot(leds_demoMode, ((progress == 0) || (progress == 400)) ? 2000 : 250);
        progress+=10;
        if(progress > 400) {
            leds_pointer_off();
            leds_demoMode_step++;
            progress = 0;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_off();
        sch_unique_oneshot(leds_demoMode, 1000);
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_show(0x5A, 0xAF, 0xB5, 0x00, progress);
        sch_unique_oneshot(leds_demoMode, progress != 0 && progress < 100 ? 250 : 1400);
        progress+=5;
        if(progress > 110) {
            leds_nav_off();
            leds_demoMode_step++;
            progress = 0;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_nav_off();
        sch_unique_oneshot(leds_demoMode, 1000);
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_front_resetFade = true;
        leds_front_setting = true;
        leds_front_state = true;
        leds_settings.frontlight_mode = FRONT_BLINK;
        leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);

        sch_unique_oneshot(leds_demoMode, ((progress == 0) || (progress == 100)) ? 2000 : 500);
        progress+=25;
        if(progress > 100) {
            leds_settings.frontlight_mode = FRONT_ON;
            leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);
            leds_demoMode_step++;
            progress = 0;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_front_resetFade = true;
        leds_front_setting = true;
        leds_front_state = true;
        leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);

        sch_unique_oneshot(leds_demoMode, ((progress == 0) || (progress == 100)) ? 1000 : 500);
        progress+=25;
        if(progress > 100) {
            leds_front_setting = false;
            leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);
            leds_demoMode_step++;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        sch_unique_oneshot(leds_demoMode, 2000);
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_alarm_arm();
        sch_unique_oneshot(leds_demoMode, 3000);
        leds_demoMode_step++;
        progress = 0;
    } else 
    if(leds_demoMode_step == testcase++) {
        progress++;
        leds_alarm_trigger(false, progress/8.0f);
        sch_unique_oneshot(leds_demoMode, 1000);
        if(progress == 9){
            leds_alarm_trigger(true, 1);
            sch_unique_oneshot(leds_demoMode, 5000);
            leds_demoMode_step++;
            progress = 0;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_alarm_disarm();
        sch_unique_oneshot(leds_demoMode, 3000);
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_progress(0xED, 0xED, 0xE8, 0xED, 0xED, 0x55,2000,65,1);
        sch_unique_oneshot(leds_demoMode, 1000);
        progress = 65;
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        progress+=2;
        leds_progress(0xED, 0xED, 0xE8, 0xED, 0xED, 0x55,2000,progress,1);
        sch_unique_oneshot(leds_demoMode, 2000);
        if(progress == 75){
            leds_demoMode_step++;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_progress_off();
        sch_unique_oneshot(leds_demoMode, 2000);
        progress = 0;
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_speedometer(speedometer_demo_values[progress],false);
        sch_unique_oneshot(leds_demoMode, 500);
        progress++;
        if(progress == sizeof(speedometer_demo_values)){
            leds_demoMode_step++;
        }
    } else 
    if(leds_demoMode_step == testcase++) {
        leds_speedometer_off();
        sch_unique_oneshot(leds_demoMode, 1000);
        leds_demoMode_step++;
    } else
    if(leds_demoMode_step == testcase++) {
        leds_notif(0x8F,0xDB,0xC6,0x8F,0xDB,0x00,100,25,100,0,11);
        sch_unique_oneshot(leds_demoMode, 4000);
        leds_demoMode_step++;
    } else 
    if(leds_demoMode_step == testcase++) {
            leds_disconnect();
            sch_unique_oneshot(leds_demoMode, 4000);
            leds_demoMode_step = 0;
    } else 
    {}
}

//============================================================================

bool leds_stillMode_active = false;

void leds_stillMode(void *ctx) {
    if(!leds_stillMode_active) {
        leds_anim_off(ANIM_STILL);
        return;
    }

    leds_still();

    sch_unique_oneshot(leds_stillMode, 10000);

}

//============================================================================

void leds_batlevel_show(void *ctx) {
    leds_battery(bat_getSOC());
}

void leds_lowBat_notify_do(void *ctx) {
    leds_battery_low();
    sound_battery_low(0);
}

void leds_lowBat_notify() {
    if (is_lowBat() && !bleapp_isConnected()){
        sch_unique_oneshot(leds_lowBat_notify_do, 5000);
    }
}

void leds_front_turnOn_from_tap(void *ctx){
    leds_front_turnOn();
}

bool leds_codeHandle(uint32_t code, uint32_t len) {
    char codeString[len];
    ui_analyze_touch_sequence(codeString, code, len);

    //Disconnected only tap codes
    if(!ui_isAuth() && !alarm_isEnabled()){
        //toggle light on
        if(!strcmp(codeString,"SS")) {
            leds_front_resetFade = true;
            leds_front_setting = (leds_front_setting) ? 0 : 1;
            leds_lowBat_notify();
            //
            if(leds_front_setting) {
                sch_unique_oneshot(leds_front_turnOn_from_tap, 50);
            } else {
                leds_front_turnOff();
            }
            leds_front_notify(true);

            return true;
        }
    }

    if(!strcmp(codeString,"SS") &&
        !leds_front_external_toggle_required &&
        ui_isAuth() &&
        !alarm_isEnabled()
    ) {
        leds_front_resetFade = true;
        leds_front_setting = (leds_front_setting) ? 0 : 1;
        //rec_write(KEY_LEDSETTINGS, &leds_settings, sizeof(leds_settings_t), NULL);
        //
        leds_lowBat_notify();
        //
        if(leds_front_setting) {
            sch_unique_oneshot(leds_front_turnOn_from_tap, 50);
        } else {
            leds_front_turnOff();
        }
        //
        leds_front_notify(true);
        //
        return false;
    }
    if(!strcmp(codeString,"LLLSSSS") && 
        !alarm_isEnabled()) {
        if(leds_demoMode_active) {
            leds_demoMode_active = false;
            leds_anim_allOff();
            leds_front_resetFade = true;
            leds_front_state = false;
            leds_front_setting = false;
            leds_anim_on(FRONT_LIGHT_ANIMATION, FRONTLIGHT_ON_CYCLE<<1);
        } else {
            leds_setBrightness(100);
            leds_demoMode_active = true;
            sch_unique_oneshot(leds_demoMode, 0);
        }
        //
        return true;
    }
    if(!strcmp(codeString,"LLLLSSS") && 
        !alarm_isEnabled()) {
        leds_stillMode_active = !leds_stillMode_active;
        if(leds_stillMode_active) {
            sch_unique_oneshot(leds_stillMode, 0);
        } else {
            leds_anim_off(ANIM_STILL);
            sch_unique_cancel(leds_stillMode);
        }
        //
        return true;
    }
    return false;
}

//============================================================================
// Public fns

void leds_turnOff_lights(){
    leds_anim_off(ANIM_HB);
    leds_anim_off(ANIM_PROGRESS);
    leds_anim_off(ANIM_FITNESS);
    leds_anim_off(ANIM_NAV);
    leds_anim_off(ANIM_COMPASS);
    leds_anim_off(ANIM_SPEEDOMETER);
    leds_anim_off(ANIM_ROUNDABOUT);
    leds_anim_off(POINTER_ANIMATION);
    leds_anim_off(ANIM_CADENCE);
    leds_frontlight_set(false);
    leds_front_turnOff();
}

void leds_nav_show(int32_t h, int32_t s, int32_t l, leds_nav_heading_t heading, uint32_t progress) {
    leds_nav_param_reroute = false;
    leds_nav_param_h = h;
    leds_nav_param_s = s;
    leds_nav_param_l = l;
    leds_nav_progress = MAX(0,MIN(100, progress));
    leds_nav_progress_next_turn = 0;
    leds_nav_param_compass_mode = 0;
    switch(heading) {
        case NAV_DESTINATION:
            leds_nav_param_head = 0;
            leds_nav_param_number_of_leds = LEDS_CNT;
            break;
        case NAV_FRONT:
            leds_nav_param_head = 0;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.5);
            break;
        case NAV_FRONT_RIGHT:
            leds_nav_param_head = 45;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.25);
            break;
        case NAV_RIGHT:
            leds_nav_param_head = 90;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.5);
            break;
        case NAV_BACK_RIGHT:
            leds_nav_param_head = 135;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.25);
            break;
        case NAV_FRONT_LEFT:
            leds_nav_param_head = 315;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.25);
            break;
        case NAV_LEFT:
            leds_nav_param_head = 270;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.5);
            break;
        case NAV_BACK_LEFT:
            leds_nav_param_head = 225;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.25);
            break;
        case NAV_BACK:
            leds_nav_param_head = -180;
            leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * 0.5);
            break;
    }
    leds_anim_on(ANIM_NAV, 30);
}

void leds_nav_destination_angle(int32_t h, int32_t s, int32_t l, float heading, uint32_t progress){
    leds_nav_param_reroute = false;
    leds_nav_param_h = h;
    leds_nav_param_s = s;
    leds_nav_param_l = l;
    leds_nav_progress = MAX(0,MIN(100, progress));
    leds_nav_param_head = heading;
    leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT);
    
    leds_anim_on(ANIM_NAV, 30);
}

void leds_nav_angle(int32_t h, int32_t s, int32_t l, float heading, uint32_t progress, int32_t nextTurnh, int32_t nextTurns, int32_t nextTurnl,float heading_next_turn, uint32_t progress_next_turn, uint32_t compass_mode) {
    leds_nav_param_reroute = false;
    leds_nav_param_h = h;
    leds_nav_param_s = s;
    leds_nav_param_l = l;
    leds_nav_param_nextTurn_h = nextTurnh;
    leds_nav_param_nextTurn_s = nextTurns;
    leds_nav_param_nextTurn_l = nextTurnl;
    leds_nav_progress = MAX(0,MIN(100, progress));
    leds_nav_param_compass_mode = compass_mode;
    leds_nav_param_head = heading;
    leds_nav_param_number_of_leds = ((uint32_t)LEDS_CNT * LEDS_PERC_TURN_BY_TURN);
    leds_nav_param_head_next_turn = heading_next_turn;

    if (progress_next_turn > 0){
        if (!compass_mode){
            leds_nav_param_next_turn = MOD((int32_t)round((float)(heading_next_turn / 360.f) * ((float)LEDS_CNT)),LEDS_CNT);
        }
        else{
            leds_nav_param_next_turn = MOD((int32_t)round(-((met_getCompass() - leds_nav_param_head) / 360.f) * ((float)LEDS_CNT)),LEDS_CNT);
        }
        leds_nav_progress_next_turn = MAX(0,MIN(100, progress_next_turn));
    }
    else{
        leds_nav_progress_next_turn = 0;
    }

    leds_anim_on(ANIM_NAV, 30);
}

void leds_nav_reroute() {
    leds_nav_param_reroute = true;
    leds_anim_on(ANIM_NAV, 5);
}

void leds_nav_off() {
    leds_nav_param_turnOff = true;
}

void leds_nav_roundAbout(uint32_t exitNow, uint32_t driveRight, int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t number_exits, float * angle_exits){
    
    leds_roundAbout_param_destination = (exitNow==1);
    leds_roundAbout_param_number_exits = number_exits;
    leds_roundAbout_driveRight = driveRight;

    leds_nav_param_exit_h = h1;
    leds_nav_param_exit_s = s1;
    leds_nav_param_exit_l = l1;

    leds_nav_param_noExit_h = h2;
    leds_nav_param_noExit_s = s2;
    leds_nav_param_noExit_l = l2;

    for(uint8_t i = 0; i < number_exits; i++){
        leds_roundAbout_param_angle_exits[i] = angle_exits[i];
    }
    
    leds_roundAbout_withDirection = (driveRight == 2) ? 0 : 1;
    leds_anim_on(ANIM_ROUNDABOUT, 60);
}

void leds_nav_roundAbout_off() {
    leds_roundAbout_with_direction_param_turnOff = true;
}

void leds_nav_angle_test(void * ctx){
    leds_nav_angle(90,181,175,90,100,90,181,175,-45,50,0);
}

void leds_roundAbout_test(void * ctx){
    float angle_exits [4] = {180,-100,45};
    leds_nav_roundAbout(1,0,90,175,181,253,226,255,3,angle_exits);
}

void leds_frontlight_settings(leds_front_mode_t mode, uint32_t brightness) {
    leds_front_resetFade = true;
    leds_settings.frontlight_mode = mode;
    uint8_t bright = MAX(0, MIN(100, brightness));
    leds_settings.frontlight_brightness = bright;
    rec_write(KEY_LEDSETTINGS, &leds_settings, sizeof(leds_settings_t), NULL);    
}

void leds_frontlight(uint32_t on, uint32_t bg) {
    leds_front_setting = on;
    //rec_write(KEY_LEDSETTINGS, &leds_settings, sizeof(leds_settings_t), NULL);
    if(leds_front_setting) {
        if(bg) {
            if(leds_movement_active) {
                leds_front_turnOn();
            }
        } else {
            leds_front_turnOn();
        }
    } else {
        leds_front_turnOff();
    }
    leds_front_notify(false);
}

void leds_progress(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t period, uint32_t progress, uint32_t fitness_on) {
    if(h1 > h2) {
        h2+=256;
    }
    if((h2 - h1) > 128) {
        h1 += 256;
    }
    if (fitness_on) {
        leds_fitness_h1 = h1;
        leds_fitness_s1 = s1;
        leds_fitness_l1 = l1;
        leds_fitness_h2 = h2;
        leds_fitness_s2 = s2;
        leds_fitness_l2 = l2;
        leds_fitness_period = period;
        leds_fitness_perc = MAX(0,MIN(100,progress));
        leds_anim_on(ANIM_FITNESS, 60);
        leds_progress_turnOff = true;
    }
    else{
        leds_progress_h1 = h1;
        leds_progress_s1 = s1;
        leds_progress_l1 = l1;
        leds_progress_h2 = h2;
        leds_progress_s2 = s2;
        leds_progress_l2 = l2;
        leds_progress_period = period;
        leds_progress_perc = MAX(0,MIN(100,progress));
        leds_anim_on(ANIM_PROGRESS, 60);
        leds_fitness_turnOff = true;
    }
}

void leds_play_fitness_intro(void * ctx)
{
    int32_t progress = (int32_t) ctx;
    if (progress < 0){	
        leds_fitness_perc = 0;
	    leds_anim_on(ANIM_FITNESS, 60);
        leds_progress_turnOff = true;
    }
    else{
        leds_progress_perc = MAX(0,MIN(100,progress));
	    leds_anim_on(ANIM_PROGRESS, 60);
        leds_fitness_turnOff = true;
    }
}


void leds_fitness_intro(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t period){
    
    uint8_t i = 0;
    if(h1 > h2) {
        h2+=256;
    }
    if((h2 - h1) > 128) {
        h1 += 256;
    }

    leds_fitness_h1 = h1;
    leds_fitness_s1 = s1;
    leds_fitness_l1 = l1;
    leds_fitness_h2 = h2;
    leds_fitness_s2 = s2;
    leds_fitness_l2 = l2;
    leds_fitness_period = period;

    leds_progress_h1 = h1;
    leds_progress_s1 = s1;
    leds_progress_l1 = l1;
    leds_progress_h2 = h2;
    leds_progress_s2 = s2;
    leds_progress_l2 = l2;
    leds_progress_period = period;

    sch_oneshot_ctx(leds_play_fitness_intro,0, (void *) 0);
    sch_oneshot_ctx(leds_play_fitness_intro,200, (void *) 100);
    i++;
    sch_oneshot_ctx(leds_play_fitness_intro,800 * i, (void *) 0);
    i++;
    sch_oneshot_ctx(leds_play_fitness_intro,1000 * i, (void *)-1);
}

void leds_progress_off() {
    leds_progress_turnOff = true;
    leds_fitness_turnOff = true;
}

void leds_speedometer(uint32_t speed, bool introEnabled)
{
    leds_speedometer_intro = introEnabled;
    leds_speedometer_param_speed = (((float)MAX(0,MIN(100,speed)))/100.f);
    leds_anim_on(ANIM_SPEEDOMETER, 60);
}

void leds_speedometer_off() {
    leds_speedometer_turnOff = true;
}

void leds_cadence(int32_t h, int32_t s, int32_t l, float cadence){
    leds_param_h = h;
    leds_param_s = s;
    leds_param_l = l;
	leds_cadence_param_cad = (((float)MAX(-100.f,MIN(100.f,cadence)))/100.f);
	leds_anim_on(ANIM_CADENCE, 60);
}

void leds_cadence_off() {
    leds_cadence_turnOff = true;
}

void leds_notif(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t fadein, uint32_t on, uint32_t fadeout, uint32_t off, uint32_t repeat) {
    if(h1 > h2) {
        h2+=256;
    }
    if((h2 - h1) > 128) {
        h1 += 256;
    }
    leds_notif_param_h1 = h1;
    leds_notif_param_s1 = s1;
    leds_notif_param_l1 = l1;
    leds_notif_param_h2 = h2;
    leds_notif_param_s2 = s2;
    leds_notif_param_l2 = l2;
    leds_notif_param_fadein = fadein;
    leds_notif_param_on = on;
    leds_notif_param_fadeout = fadeout;
    leds_notif_param_off = off;
    leds_notif_param_repeat = repeat;
    leds_anim_on(ANIM_NOTIF, 15);
}

void leds_notif_off() {
    leds_anim_off(ANIM_NOTIF);
}

void leds_hb(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t fadein, uint32_t on, uint32_t fadeout, uint32_t off) {
    if(h1 > h2) {
        h2+=256;
    }
    if((h2 - h1) > 128) {
        h1 += 256;
    }
    leds_hb_param_h1 = h1;
    leds_hb_param_s1 = s1;
    leds_hb_param_l1 = l1;
    leds_hb_param_h2 = h2;
    leds_hb_param_s2 = s2;
    leds_hb_param_l2 = l2;
    leds_hb_param_fadein = fadein;
    leds_hb_param_on = on;
    leds_hb_param_fadeout = fadeout;
    leds_hb_param_off = off;
    leds_anim_on(ANIM_HB, 5*60);
}

void leds_hb_off() {
    leds_anim_off(ANIM_HB);
}

void leds_touch(bool state, touch_type_t type, uint32_t code, uint32_t codeLen) {
    leds_touch_param_state = state;
    leds_touch_param_type = type;
    leds_touch_param_code = code;
    leds_touch_param_codeLen = codeLen;
    leds_anim_on(ANIM_TOUCH, 5);
}

void leds_alarm_arm() {
    leds_anim_on(ANIM_ALARM_ARM, 5);
}

void leds_alarm_disarm() {
    leds_anim_off(ANIM_ALARM);
    leds_anim_on(ANIM_ALARM_DISARM, 5);
}

void leds_alarm_trigger(bool alert, float level) {
    leds_anim_alarm_alert = alert;
    leds_anim_alarm_level = level;
    leds_anim_alarm_repeat = 4;
    if(alert) {
        leds_anim_on(ANIM_ALARM, 15);
    } else {
        leds_anim_on(ANIM_ALARM, 4);
    }
}

void leds_logo() {
    ui_onLogoAnimation();
    leds_anim_on(ANIM_LOGO, 5);
}

void leds_disconnect() {
    leds_front_external_toggle_required = false;
    leds_anim_on(ANIM_DISCONNECT, 5);
}

void leds_compass(int32_t h, int32_t s, int32_t l, float heading) {
    leds_compass_param_h = h;
    leds_compass_param_s = s;
    leds_compass_param_l = l;
    leds_compass_param_heading = heading;
    leds_anim_on(ANIM_COMPASS, 2*60);
}

void leds_compass_off() {
    leds_compass_turnOff = true;
}

void leds_pointer(int32_t h, int32_t s, int32_t l, float heading, uint32_t standby, uint8_t intro_mode) {
    leds_pointer_param_h = h;
    leds_pointer_param_s = s;
    leds_pointer_param_l = l;
    float head = (heading / 360.f) * ((float)LEDS_CNT);
    if(head<0) {
        head += ((float)LEDS_CNT);
    }
    leds_pointer_param_heading = head;
    leds_pointer_param_standby = standby;
    leds_pointer_intro_mode = intro_mode - 1;
    leds_pointer_intro = intro_mode ? true : false;
    leds_anim_on(POINTER_ANIMATION, 2*60);
}

void leds_pointer_off() {
    leds_pointer_turnOff = true;
}

void leds_clock(uint8_t hour, uint8_t hour_hue, uint8_t hour_sat, uint8_t hour_lum, uint8_t minute, uint8_t minute_hue, uint8_t minute_sat, uint8_t minute_lum, uint8_t center_hue, uint8_t center_sat, uint8_t center_lum,
 uint16_t duration, bool fade, bool intro, bool pulse){
    leds_clock_hour = hour;
    leds_clock_hour_h = hour_hue;
    leds_clock_hour_s = hour_sat;
    leds_clock_hour_l = hour_lum;
    leds_clock_minute = minute;
    leds_clock_minute_h = minute_hue;
    leds_clock_minute_s = minute_sat;
    leds_clock_minute_l = minute_lum;
    leds_clock_center_h = center_hue;
    leds_clock_center_s = center_sat;
    leds_clock_center_l = center_lum;
    leds_clock_duration_steps = (uint16_t) (60.0f * duration / 1000.f);
    leds_clock_fade = fade;
    leds_clock_intro = intro;
    leds_clock_pulse = pulse;
    leds_clock_turn_off = false;
    leds_anim_on(CLOCK_ANIMATION, 65);
}

void leds_clock_off(){
    leds_clock_turn_off = true;
}

void leds_battery(float soc) {
    leds_bat_param_soc = MAX(0.f, MIN(1.f, soc/100.f));
    leds_anim_on(ANIM_BATTERY, 5);
}

void leds_battery_low() {
    leds_anim_on(ANIM_LOWBAT, 5);
}

void leds_calibrate(float lvl) {
    leds_anim_calibrate_level = lvl;
    leds_anim_on(ANIM_CALIBRATE, 10);
}

void leds_demo_intro() {
    leds_param_demo_intro_turnOff = false;
    leds_anim_on(ANIM_DEMO_INTRO, 60);
}
void leds_demo_intro_off() {
    leds_param_demo_intro_turnOff = true;
}

void leds_setBrightness(uint32_t brightness) {
    uint8_t bright = MAX(0, MIN(100, brightness));
    leds_anim_goal_brightness = 0.3f + 0.7f * (((float)bright)/100.f);
    leds_anim_start_brightness = leds_anim_brightness;
}

void leds_inner_test(void *ctx) {
    //leds_notif(43,255,128,43,255,0, 200, 100, 200, 1000, 0);
    //leds_anim_on(ANIM_DEMO_INTRO, 60);
}

void leds_test(uint8_t num, uint8_t r, uint8_t g, uint8_t b) {
    leds_test_num = num;
    leds_test_r = r;
    leds_test_g = g;
    leds_test_b = b;
    leds_anim_on(ANIM_TEST, 5);
}

void leds_test_touch(bool active, uint32_t step) {
    leds_anim_test_touch_active = active;
    leds_anim_test_touch_step = step;
    leds_anim_on(ANIM_TEST_TOUCH, 20);
}

void leds_still() {
    leds_anim_on(ANIM_STILL, 15);
}

void leds_charged(){
    leds_hb_off();
    sch_unique_cancel(leds_charging);
    leds_anim_off(CHARGE_TRACE);
    leds_bat_param_soc = bat_getSOC()/100.f;  
    leds_is_charged = true;
    leds_charging_mode = 3;
    leds_anim_on(CHARGE_SHADOW, 80);
}

void leds_charging(){
    switch(leds_charging_mode){
        case 0:{
            leds_hb(143,234,142,143,234,100,500,1500,200,250);
            sch_unique_oneshot(leds_charging, 3*60*1000);            
            break;
        }
        case 1:{
            leds_hb(24,203,142,24,203,100,500,1500,200,250);
            sch_unique_oneshot(leds_charging, 3*60*1000);            
            break;    
        }
        case 2:{
            leds_hb(67,125,142,24,203,100,500,1500,200,250);
            sch_unique_oneshot(leds_charging, 3*60*1000);       
            break;                 
        }
        case 3:{
            leds_bat_param_soc = bat_getSOC()/100.f;
            leds_anim_on(CHARGE_SHADOW, 80);
            break;    
        }
        case 4:{
            leds_anim_off(CHARGE_SHADOW);
            leds_bat_param_soc = bat_getSOC()/100.f;
            leds_anim_on(CHARGE_SHADOW, 140);
            break;    
        }
        case 5:
        case 6:{
            leds_anim_off(CHARGE_TRACE);
            leds_bat_param_soc = bat_getSOC()/100.f;
            leds_anim_on(CHARGE_TRACE, 100);
            break;    
        }
    }
}

void leds_charging_off(){
    switch(leds_charging_mode){
        case 0:
        case 1:
        case 2:{
            leds_hb_off();
            break;
        }
        case 3:
        case 4:{
            leds_anim_off(CHARGE_SHADOW);
            break;
        }
        case 5:
        case 6:{
            leds_anim_off(CHARGE_TRACE);
            break;
        }
    }
    sch_unique_cancel(leds_charging);
    if(leds_is_charged){
        leds_hb_off();
    }
}
//============================================================================
//


void leds_init() {

    memset(&leds_settings, 0, sizeof(leds_settings));
    rec_read(KEY_LEDSETTINGS, &leds_settings, sizeof(leds_settings), NULL);
    leds_anim_brightness = 0.3f + 0.7f * 0.5f;
    leds_anim_goal_brightness = leds_anim_brightness;
    leds_anim_start_brightness = leds_anim_brightness;

    leds_util_buf_init();

#ifdef DBG_MOTION_VISUAL
    nrf_gpio_pin_write(EN_VLED, 1);
#else
    nrf_gpio_pin_write(EN_VLED, 0);
#endif
    nrf_gpio_cfg_output(EN_VLED);
    nrf_gpio_pin_write(I2C_SDB_PIN, 0);
    nrf_gpio_cfg_output(I2C_SDB_PIN);

    leds_pwmInit();
    leds_twi_init();

#ifdef DBG_MOTION_VISUAL
    nrf_drv_pwm_simple_playback(&m_pwm0, &leds_pwm_seq0, 1, NRF_DRV_PWM_FLAG_LOOP);
#endif

    //sch_unique_oneshot(leds_inner_test, 3000);
}

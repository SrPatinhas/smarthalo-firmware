
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "scheduler.h"
#include "dispatch.h"
#include "power.h"

#include "sound.h"
#include "alarm.h"
#include "battery.h"
#include "ui.h"
#include "touch.h"

extern uint8_t touch_volume;

sound_bit_t sound_test_track[] = {
{3000,2000},
{0,0}
};

sound_bit_t sound_easteregg_track[] = {
{104,125},
{139,125},
{165,125},
{208,125},
{139,125},
{165,125},
{208,125},
{277,125},
{165,125},
{208,125},
{277,125},
{330,125},
{208,125},
{277,125},
{330,125},
{415,125},
{277,125},
{330,125},
{415,125},
{554,125},
{330,125},
{415,125},
{554,125},
{659,125},
{415,125},
{554,125},
{659,125},
{831,125},
{659,125},
{831,125},
{0,125},
{104,125},
{131,125},
{156,125},
{208,125},
{131,125},
{156,125},
{208,125},
{262,125},
{156,125},
{208,125},
{262,125},
{311,125},
{208,125},
{262,125},
{311,125},
{415,125},
{262,125},
{311,125},
{415,125},
{523,125},
{311,125},
{415,125},
{523,125},
{622,125},
{415,125},
{523,125},
{622,125},
{831,125},
{622,125},
{831,125},
{0,125},
{139,125},
{175,125},
{208,125},
{277,125},
{175,125},
{208,125},
{277,125},
{349,125},
{208,125},
{277,125},
{349,125},
{415,125},
{277,125},
{349,125},
{415,125},
{554,125},
{349,125},
{415,125},
{554,125},
{698,125},
{415,125},
{554,125},
{698,125},
{831,125},
{554,125},
{698,125},
{831,125},
{1109,125},
{831,125},
{1109,125},
{0,125},
{139,125},
{185,125},
{220,125},
{277,125},
{277,125},
{370,125},
{440,125},
{554,125},
{554,125},
{740,125},
{880,125},
{1109,125},
{880,125},
{1109,125},
{0,125},
{139,125},
{165,125},
{196,125},
{277,125},
{277,125},
{330,125},
{392,125},
{554,125},
{554,125},
{659,125},
{784,125},
{1109,125},
{784,125},
{1109,125},
{831,125},
{0,125},
{415,125},
{831,125},
{415,125},
{831,125},
{466,125},
{831,125},
{523,125},
{831,125},
{554,125},
{831,125},
{622,125},
{831,125},
{523,125},
{831,125},
{622,125},
{831,125},
{554,125},
{831,125},
{740,125},
{831,125},
{659,125},
{831,125},
{622,125},
{831,125},
{554,125},
{831,125},
{523,125},
{831,125},
{440,125},
{784,125},
{415,125},
{831,125},
{415,125},
{831,125},
{415,125},
{831,125},
{466,125},
{831,125},
{523,125},
{831,125},
{554,125},
{831,125},
{622,125},
{831,125},
{523,125},
{831,125},
{622,125},
{831,125},
{554,125},
{831,125},
{740,125},
{831,125},
{659,125},
{831,125},
{622,125},
{831,125},
{554,125},
{831,125},
{523,125},
{831,125},
{440,125},
{784,125},
{415,125},
{831,125},
{440,125},
{784,125},
{415,125},
{831,125},
{440,125},
{784,125},
{415,125},
{831,125},
{440,125},
{784,125},
{415,125},
{831,125},
{440,125},
{784,125},
{415,125},
{831,125},
{0,125},
{208,1500},
{0,0}
};

sound_bit_t low_battery_track[] = {
{493,55},
{349,82},
{138,106},
{0,0}
};

sound_bit_t touch_track[] = {
{58,10},
{0,0}
};

sound_bit_t long_touch_track[] = {
{880,2},
{0,0}
};

sound_bit_t horn_track[] = {
    {466,250},
    {0,100},
    {466,500},
    {0,0}
};

bool sound_active = false;
bool sound_delayed_active = false;

bool sound_isActive() {
    return sound_delayed_active;
}

void sound_delayed_off(void *ctx) {
    sound_delayed_active = false;
}

void sound_pwr_set(bool active) {
    sound_active = active;
    if(active) {
        sound_delayed_active = true;
        sch_unique_cancel(sound_delayed_off);
    } else {
        sch_unique_oneshot(sound_delayed_off, 500);
    }
    pwr_update();
}

nrf_ppi_channel_t sound_ppi_chVol, sound_ppi_chOut;

sound_bit_t *sound_track = NULL;
uint32_t sound_track_ptr = 0;
uint32_t sound_track_repeat = 0;
float sound_out_period = 0;
uint32_t sound_out_period_i = 0;
float sound_out_delta = 0;

#define TIMERFREQ (16000000UL)

#define USTOTICK(x) ((double)(x)*0.000001)/(1./(double)TIMERFREQ)

#define VOLSETLEN 2
uint8_t sound_vol_buf[VOLSETLEN];
uint32_t sound_vol_byte_ptr;
uint32_t sound_vol_bit_ptr;

sch_cb_ctx_t sound_int_cb = NULL;

void sound_int_easyscale_start(void *ctx);
void sound_volume_done(void *ctx);

void sound_int_easyscale_done(void *ctx) {
    nrf_drv_gpiote_set_task_trigger(PIEZO_VOLUME_PIN);
    sound_int_cb = NULL;
    sch_unique_oneshot(sound_volume_done, 0);
}

void sound_int_easyscale_end(void *ctx) {
    nrf_drv_gpiote_clr_task_trigger(PIEZO_VOLUME_PIN);
    sound_vol_bit_ptr = 0;
    sound_vol_byte_ptr++;
    if(sound_vol_byte_ptr == VOLSETLEN) {
        sound_int_cb = sound_int_easyscale_done;    
    } else {
        sound_int_cb = sound_int_easyscale_start;
    }
    NRF_TIMER3->CC[0] = 0xffff;
    NRF_TIMER3->CC[1] = USTOTICK(10);
    NRF_TIMER3->TASKS_CLEAR = 1;
    NRF_TIMER3->TASKS_START = 1;
}


void sound_int_easyscale_bit(void *ctx) {
    nrf_drv_gpiote_clr_task_trigger(PIEZO_VOLUME_PIN);
    uint32_t bitTime = (sound_vol_buf[sound_vol_byte_ptr] & (1 << (7 - sound_vol_bit_ptr))) ? USTOTICK(100) : USTOTICK(200);
    sound_vol_bit_ptr++;
    if(sound_vol_bit_ptr == 8) {
        sound_int_cb = sound_int_easyscale_end;    
    } else {
        sound_int_cb = sound_int_easyscale_bit;
    }
    NRF_TIMER3->CC[0] = bitTime;
    NRF_TIMER3->CC[1] = USTOTICK(300);
    NRF_TIMER3->TASKS_CLEAR = 1;
    NRF_TIMER3->TASKS_START = 1;
}

void sound_int_easyscale_start(void *ctx) {
    nrf_drv_gpiote_set_task_trigger(PIEZO_VOLUME_PIN);
    sound_int_cb = sound_int_easyscale_bit;
    NRF_TIMER3->CC[0] = 0xffff;
    NRF_TIMER3->CC[1] = USTOTICK(10);
    NRF_TIMER3->TASKS_CLEAR = 1;
    NRF_TIMER3->TASKS_START = 1;
}

void sound_int_easyscale_en1(void *ctx) {
    nrf_drv_gpiote_clr_task_trigger(PIEZO_VOLUME_PIN);
    sound_int_cb = sound_int_easyscale_start;
    NRF_TIMER3->CC[0] = 0xffff;
    NRF_TIMER3->CC[1] = USTOTICK(300);
    NRF_TIMER3->TASKS_CLEAR = 1;
    NRF_TIMER3->TASKS_START = 1;
}

void sound_int_easyscale_en0(void *ctx) {
    nrf_drv_gpiote_set_task_trigger(PIEZO_VOLUME_PIN);
    sound_int_cb = sound_int_easyscale_en1;
    NRF_TIMER3->CC[0] = 0xffff;
    NRF_TIMER3->CC[1] = USTOTICK(150);
    NRF_TIMER3->TASKS_CLEAR = 1;
    NRF_TIMER3->TASKS_START = 1;
}

bool sound_mode_isVol = false;

void TIMER3_IRQHandler(void)
{
    if(NRF_TIMER3->EVENTS_COMPARE[0]) {
        NRF_TIMER3->EVENTS_COMPARE[0] = 0;
        (void)NRF_TIMER3->EVENTS_COMPARE[0];
    }
    if(NRF_TIMER3->EVENTS_COMPARE[1]) {
        NRF_TIMER3->EVENTS_COMPARE[1] = 0;
        (void)NRF_TIMER3->EVENTS_COMPARE[1];
    }
    if(sound_mode_isVol) {
        if(sound_int_cb) {
            sch_unique_oneshot(sound_int_cb, 0);
        }
    } else {
        NRF_TIMER3->CC[0] = sound_out_period_i;
    }
}

void TIMER4_IRQHandler(void)
{
    if(NRF_TIMER4->EVENTS_COMPARE[0]) {
        NRF_TIMER4->EVENTS_COMPARE[0] = 0;
        (void)NRF_TIMER4->EVENTS_COMPARE[0];
    }
    sound_out_period += sound_out_delta;
    sound_out_period_i = sound_out_period;
}


void sound_mode_vol() {
    uint32_t err;

    sound_mode_isVol = true;

    NRF_TIMER3->TASKS_STOP = 1;
    NRF_TIMER4->TASKS_STOP = 1;

    NRF_TIMER3->SHORTS = TIMER_SHORTS_COMPARE1_STOP_Msk;
    NRF_TIMER3->INTENSET = TIMER_INTENSET_COMPARE1_Msk;
    NRF_TIMER3->INTENCLR = TIMER_INTENSET_COMPARE0_Msk;

    err = nrf_drv_ppi_channel_disable(sound_ppi_chOut);
    ERR_CHECK("ppi_channel out dis", err);
    err = nrf_drv_ppi_channel_enable(sound_ppi_chVol);
    ERR_CHECK("ppi_channel vol en", err);

}

void sound_mode_out() {
    uint32_t err;

    sound_mode_isVol = false;

    NRF_TIMER3->TASKS_STOP = 1;
    NRF_TIMER4->TASKS_STOP = 1;

    NRF_TIMER3->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    NRF_TIMER3->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    NRF_TIMER3->INTENCLR = TIMER_INTENSET_COMPARE1_Msk;

    err = nrf_drv_ppi_channel_disable(sound_ppi_chVol);
    ERR_CHECK("ppi_channel vol dis", err);
    err = nrf_drv_ppi_channel_enable(sound_ppi_chOut);
    ERR_CHECK("ppi_channel out en", err);

}

void sound_play_note(void *ctx) {

    NRF_TIMER3->TASKS_STOP = 1;
    NRF_TIMER4->TASKS_STOP = 1;

    if(sound_track[sound_track_ptr].freq == 0 && sound_track[sound_track_ptr].ms == 0) {
        if(sound_track_repeat) {
            sound_track_repeat--;
            sound_track_ptr=0;
        } else {
            sound_pwr_set(false);
            nrf_drv_gpiote_clr_task_trigger(PIEZO_VOLUME_PIN);
            return;
        }
    }

    float freq = sound_track[sound_track_ptr].freq;
    uint32_t ms = sound_track[sound_track_ptr].ms & 0x7fff;
    bool sweep = ((sound_track[sound_track_ptr].ms & 0x8000) != 0);
    sound_track_ptr++;

    uint32_t ahead = sound_track_ptr;
    if(sound_track[ahead].freq == 0 && sound_track[ahead].ms == 0) {
        ahead = 0;
    }
    float swpfreq = sound_track[ahead].freq;

    if(freq != 0) {
        sound_out_period = ((float)(TIMERFREQ>>1) / freq);
        sound_out_period_i = sound_out_period;
        NRF_TIMER3->CC[0] = sound_out_period_i;
        if(sweep) {
            float swpPeriod = ((float)(TIMERFREQ>>1) / swpfreq);
            float dp = swpPeriod - sound_out_period;
            float dt = (float)MAX(10, ms) / 10.f;
            sound_out_delta = dp/dt;
            NRF_TIMER4->TASKS_CLEAR = 1;
            NRF_TIMER4->TASKS_START = 1;
        }
        NRF_TIMER3->TASKS_CLEAR = 1;
        NRF_TIMER3->TASKS_START = 1;
    }
    sch_unique_oneshot(sound_play_note, ms);
}

void sound_volume_done(void *ctx) {

    sound_mode_out();
    sch_unique_oneshot(sound_play_note, 0);

}

/*

p = polyfit(ynorm,x,3);

x1 = linspace(0,1);
y1 = polyval(p,x1);
figure
plot(ynorm,x,'o')
hold on
plot(x1,y1)
hold off

*/

double sound_volP[] = {31.24954,  -74.06017,   74.47257,   -0.54565};

void sound_stop() {
    sound_pwr_set(false);
    NRF_TIMER3->TASKS_STOP = 1;
    NRF_TIMER4->TASKS_STOP = 1;
    nrf_drv_gpiote_clr_task_trigger(PIEZO_VOLUME_PIN);
    sch_unique_cancel(sound_play_note);
}

void sound_play(uint32_t volume, uint32_t repeat, sound_bit_t *track) {

    sound_pwr_set(true);

    double x = MIN(100,MAX(0,volume));
    x/=100;
    uint32_t vol = 
        sound_volP[0]*x*x*x +
        sound_volP[1]*x*x +
        sound_volP[2]*x +
        sound_volP[3];
    //printf("Vol:%d\r\n",vol);
    sound_track = track;
    sound_track_ptr = 0;
    sound_track_repeat = repeat;

    sound_vol_byte_ptr = sound_vol_bit_ptr = 0;
    sound_vol_buf[0] = 0x72;
    sound_vol_buf[1] = (uint8_t)vol & 0x1f;

    sound_mode_vol();

    nrf_drv_gpiote_clr_task_trigger(PIEZO_VOLUME_PIN);
    sound_int_cb = sound_int_easyscale_en0;
    NRF_TIMER3->CC[0] = 0xffff;
    NRF_TIMER3->CC[1] = USTOTICK(2600);
    NRF_TIMER3->TASKS_CLEAR = 1;
    NRF_TIMER3->TASKS_START = 1;

}

void sound_test() {
    sound_play(100, 0, sound_test_track);
}

void sound_easteregg() {
    sound_play(14, 0, sound_easteregg_track);
}

void sound_touch(void *ctx) {
    sound_play(touch_volume, 0, touch_track);
}

void sound_touch_long(void *ctx) {
    sound_play(touch_volume, 0, long_touch_track);
}

void sound_battery_low(uint32_t volume){
    sound_play(volume, 0,low_battery_track);
}

void sound_horn(){
    sound_play(100, 0, horn_track);
}

//============================================================================
//

#define SOUNDMAXTRACK 16

sound_bit_t sound_disp_track[SOUNDMAXTRACK+1];

void sound_cmd_stop(uint8_t *buf, uint32_t len) {
    printf("sound_cmd_stop\r\n");
    sound_stop();
    disp_respond(RET_OK);
}

void sound_cmd_play(uint8_t *buf, uint32_t len) {
    uint8_t vol = buf[0];
    uint8_t repeat = buf[1];
    buf+=2;len-=2;

    int efflen = MIN(SOUNDMAXTRACK*4, len);
    if(efflen%4) {
        disp_respond(RET_FAIL);
    } else {
        int i;
        for(i = 0; i < efflen; i+=4) {
            sound_disp_track[i>>2].freq = ((uint16_t)buf[i] << 8) + (uint16_t)buf[i+1];
            sound_disp_track[i>>2].ms = ((uint16_t)buf[i+2] << 8) + (uint16_t)buf[i+3];
        }
        sound_disp_track[i>>2].freq = 0;
        sound_disp_track[i>>2].ms = 0;
        sound_play(vol, repeat, sound_disp_track);
        disp_respond(RET_OK);
    }
}

void sound_cmd_touch_state(uint8_t *buf, uint32_t len){
    uint8_t volume = buf[0];
    bool isEnabled = buf[1];    
    touch_sound(volume,isEnabled);

    disp_respond(RET_OK);
}

disp_cmdfn_t sound_cmdfn[] = {
    sound_cmd_play,
    sound_cmd_stop,
    sound_cmd_touch_state
};
#define SOUND_CMDFN_CNT (sizeof(sound_cmdfn)/sizeof(disp_cmdfn_t))

void sound_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= SOUND_CMDFN_CNT) {
        disp_respond(RET_UNIMPLEMENTED);
    } else {
        sound_cmdfn[buf[0]](buf+1, len-1);
    }
}

//============================================================================
//

bool sound_codeHandle(uint32_t code, uint32_t len){
    char codeString[len];
    ui_analyze_touch_sequence(codeString, code, len);

    if(!ui_isAuth() && !alarm_isEnabled()){
        return true;
    }
    return false;
}

void sound_init() {

    uint32_t err;

    disp_register(2, 1, sound_dispatch);

    if(!nrf_drv_gpiote_is_init())
    {
        err = nrf_drv_gpiote_init();
        ERR_CHECK("nrf_drv_gpiote_init", err);
    }

    nrf_drv_gpiote_out_config_t cfg_toggle = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);
    err = nrf_drv_gpiote_out_init(PIEZO_DRIVE_PIN, &cfg_toggle);
    ERR_CHECK("nrf_drv_gpiote_out_init", err);
    nrf_drv_gpiote_out_task_enable(PIEZO_DRIVE_PIN);

    err = nrf_drv_ppi_channel_alloc(&sound_ppi_chOut);
    ERR_CHECK("nrf_drv_ppi_channel_alloc", err);
    err = nrf_drv_ppi_channel_assign(sound_ppi_chOut,
                                          (uint32_t)&NRF_TIMER3->EVENTS_COMPARE[0],
                                          nrf_drv_gpiote_out_task_addr_get(PIEZO_DRIVE_PIN));
    ERR_CHECK("nrf_drv_ppi_channel_assign", err);


    nrf_drv_gpiote_out_config_t cfg_set = GPIOTE_CONFIG_OUT_TASK_HIGH;
    err = nrf_drv_gpiote_out_init(PIEZO_VOLUME_PIN, &cfg_set);
    ERR_CHECK("nrf_drv_gpiote_out_init", err);
    nrf_drv_gpiote_out_task_enable(PIEZO_VOLUME_PIN);
    nrf_drv_gpiote_clr_task_trigger(PIEZO_VOLUME_PIN);

    err = nrf_drv_ppi_channel_alloc(&sound_ppi_chVol);
    ERR_CHECK("nrf_drv_ppi_channel_alloc", err);
    err = nrf_drv_ppi_channel_assign(sound_ppi_chVol,
                                          (uint32_t)&NRF_TIMER3->EVENTS_COMPARE[0],
                                          nrf_drv_gpiote_out_task_addr_get(PIEZO_VOLUME_PIN));
    ERR_CHECK("nrf_drv_ppi_channel_assign", err);


    // Timer init
    NRF_TIMER3->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER3->TASKS_STOP = 1;
    NRF_TIMER3->TASKS_CLEAR = 1;
    NRF_TIMER3->PRESCALER = 0;
    NRF_TIMER3->BITMODE = TIMER_BITMODE_BITMODE_32Bit;

    nrf_drv_common_irq_enable(nrf_drv_get_IRQn(NRF_TIMER3), TIMER_DEFAULT_CONFIG_IRQ_PRIORITY);

    // Timer init
    NRF_TIMER4->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER4->TASKS_STOP = 1;
    NRF_TIMER4->TASKS_CLEAR = 1;
    NRF_TIMER4->PRESCALER = 0;
    NRF_TIMER4->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
    NRF_TIMER4->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    NRF_TIMER4->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    NRF_TIMER4->CC[0] = 160000; //100Hz
    nrf_drv_common_irq_enable(nrf_drv_get_IRQn(NRF_TIMER4), TIMER_DEFAULT_CONFIG_IRQ_PRIORITY);

}


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "platform.h"

#include "scheduler.h"

#include "blleds.h"

#define LEDS_MAXCURRENT_DIV     4

typedef void (*leds_anim_fn_t)(bool);


//============================================================================
// Buffers & utils


float leds_hue2rgb(float p, float q, float t){
    if(t < 0.f) t += 1.f;
    if(t > 1.f) t -= 1.f;
    if(t < 1.f/6.f) return p + (q - p) * 6.f * t;
    if(t < 1.f/2.f) return q;
    if(t < 2.f/3.f) return p + (q - p) * (2.f/3.f - t) * 6.f;
    return p;
}
void leds_hslToRgb(uint8_t *in_h, uint8_t *in_s, uint8_t *in_l, uint8_t *rgb){
    float r, g, b, h, s, l;
    h = (float)(*in_h) / 255.f;
    s = (float)(*in_s) / 255.f;
    l = (float)(*in_l) / 255.f;

    if(s == 0.f){
        r = g = b = l; // achromatic
    }else{
        float q = l < 0.5f ? l * (1.f + s) : l + s - l * s;
        float p = 2.f * l - q;
        r = leds_hue2rgb(p, q, h + 1.f/3.f);
        g = leds_hue2rgb(p, q, h);
        b = leds_hue2rgb(p, q, h - 1.f/3.f);
    }
    rgb[0] = r * 255.f;
    rgb[1] = g * 255.f;
    rgb[2] = b * 255.f;
}


uint8_t leds_central_rgb[3];
uint8_t leds_central_hue;
uint8_t leds_central_sat;
uint8_t leds_central_lum;

uint8_t leds_front_lum;

void leds_util_buf_init() {
    leds_central_lum = 0;
    leds_central_sat = 0;
    leds_central_hue = 0;
    leds_front_lum = 0;
    memset(leds_central_rgb, 0, 3);
}

void leds_util_central_copyHSLtoRGB() {
    leds_hslToRgb(&leds_central_hue, &leds_central_sat, &leds_central_lum, leds_central_rgb);
}


//============================================================================
// State animations

uint8_t leds_logo_hues[24] = {167, 151, 144, 139, 131, 121, 111, 101, 91, 77, 65, 56, 49, 43, 38, 32, 24, 14, 2, 245, 237, 232, 221, 196};
uint8_t leds_logo_sats[24] = {125, 187, 237, 225, 210, 196, 188, 181, 175, 167, 182, 195, 208, 220, 233, 219, 203, 187, 171, 200, 237, 209, 168, 133};

uint32_t leds_anim_maxLum = 20;

#define LEDS_CNT 24
#define LEDS_ANIM_NEXT_FRAME() do{ frame++; step = 0; }while(0)
#define MODLOOP(x, y, m)  ((((x-y) > 0) ? (y-x) : (-1 * ((x-y)+(m)))))

bool leds_bl_connected = false;
uint32_t leds_bl_progress;

void leds_anim_step_bl(bool reset) {
    static uint32_t frame = 0;
    static uint32_t step = 0;
    static uint32_t logo_color = 0;
    static bool showProgress = false;
    uint32_t maxlum = 5;

    static int32_t leds_bootloader_h1 = 0;
    static int32_t leds_bootloader_s1 = 0;
    static int32_t leds_bootloader_h2 = 0;
    static int32_t leds_bootloader_s2 = 0;

    if(reset) {
        frame = 0;
        step = 0;
        showProgress = false;
        logo_color = 0;
    }

    uint32_t fadeTick, onTick, offTick;

    if(showProgress) {
        fadeTick = 300 >> 4;
        onTick = 1000 >> 4;
        offTick = 500 >> 4;
        leds_central_hue = 85.f * ((float)leds_bl_progress / 100.f);
        leds_central_sat = 255;
    } else {
        if(leds_bl_connected) {
            fadeTick = 500 >> 4;
            onTick = 50 >> 4;
            offTick = 50 >> 4;
        } else {
            fadeTick = 500 >> 4;
            onTick = 150 >> 4;
            offTick = 50 >> 4;
        }
    }

    switch(frame) {
        case 0: {
            leds_central_lum = 0;
            if (!showProgress){
                leds_bootloader_h1 = 0;
                 leds_bootloader_h2 = leds_logo_hues[logo_color];
                 leds_bootloader_s1 = 0;
                 leds_bootloader_s2 = leds_logo_sats[logo_color];
                 leds_central_hue = leds_bootloader_h2;
                 leds_central_sat = leds_bootloader_s2;
            }
            LEDS_ANIM_NEXT_FRAME();
            break;
        }
        case 1: {
            float lvl = (1.f/(float)fadeTick)*(float)step;
            leds_central_lum = (float)maxlum*lvl;
            step++;
            if(step >= fadeTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 2: {
            leds_central_lum = maxlum;
            step++;
            if(!showProgress && step >= onTick){
                logo_color++;
                LEDS_ANIM_NEXT_FRAME();
            }
            else if(step >= onTick && showProgress ) {
                frame = 6;
                step = 0;
            }
            break;
        }
        case 3: {
            leds_central_hue = leds_bootloader_h2;
            leds_central_sat = leds_bootloader_s2;
            leds_central_lum = maxlum;
            leds_bootloader_h1 = leds_bootloader_h2;
            leds_bootloader_s1 = leds_bootloader_s2;
            leds_bootloader_h2 = leds_logo_hues[logo_color];
            leds_bootloader_s2 = leds_logo_sats[logo_color];
            LEDS_ANIM_NEXT_FRAME();
        }
        case 4: {
            float lvl = (1.f/(float)onTick)*(float)step;
            leds_central_hue = (uint8_t)(leds_bootloader_h1 + ((float)MODLOOP(leds_bootloader_h1, leds_bootloader_h2, 256))*lvl) % 256;
            leds_central_sat = leds_bootloader_s1 + ((float)(leds_bootloader_s2 - leds_bootloader_s1))*lvl;
            leds_central_lum = maxlum;
            step++;
            if(step >= onTick) {
                logo_color++;
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 5: {
            leds_central_hue = leds_bootloader_h2;
            leds_central_sat = leds_bootloader_s2;
            leds_central_lum = maxlum;
            if (logo_color < LEDS_CNT){
                frame = 3;
                step = 0;
            }
            if(logo_color >= LEDS_CNT) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 6: {
            if(!fadeTick) {
                LEDS_ANIM_NEXT_FRAME();
                break;
            }
            float lvl = (1.f/(float)fadeTick)*(float)step;
            leds_central_lum =  maxlum-(float)maxlum*lvl;
            step++;
            if(step >= fadeTick) {
                LEDS_ANIM_NEXT_FRAME();
            }
            break;
        }
        case 7: {
            leds_central_lum = 0;
            step++;
            if(step >= offTick) {
                frame = 0;
                step = 0;
                logo_color = 0;
                if (leds_bl_progress > 0 && !showProgress){
                    showProgress = true;
                }
                else if (leds_bl_progress == 0 && showProgress){
                    showProgress = false;
                }
                else if (leds_bl_progress >= 100 && showProgress){
                    showProgress = false;
                }
            }
            break;
        }

    }

    leds_util_central_copyHSLtoRGB();
}


//============================================================================
// PWM

#define LEDS_PWM_LISTMAX 4

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static uint16_t leds_pwm_values[] =
{
    0x8000, 0x8000, 0x8000, 0x8000
};
nrf_pwm_sequence_t const seq =
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
            NRF_DRV_PWM_PIN_NOT_USED,
            CENTRAL_RED_PIN,
            CENTRAL_GREEN_PIN,
            CENTRAL_BLUE_PIN,
        },
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .base_clock   = NRF_PWM_CLK_500kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 1000,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    err = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    ERR_CHECK("nrf_drv_pwm_init", err);

    for(int i = 0; i < LEDS_PWM_LISTMAX; i++) {
        leds_pwm_values[i] = 0x8000;
    }

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
    {ANIM_BL, leds_anim_step_bl, 0, false, false},
};

#define LEDS_ANIM_STACK_CNT (sizeof(leds_anim_stack)/sizeof(leds_animation_t))

void leds_anim_draw(void *ctx);

bool leds_anim_pwr_state = false;

void leds_anim_pwr_on() {
    if(!leds_anim_pwr_state) {
        leds_anim_pwr_state = true;
        //printf("leds_anim_pwr_on\r\n");

        nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);

    }
}

void leds_anim_pwr_off() {
    if(leds_anim_pwr_state) {
        leds_anim_pwr_state = false;
        //printf("leds_anim_pwr_off\r\n");

        nrf_drv_pwm_stop(&m_pwm0, true);
        leds_pwm_values[0] = 0x8000;
        leds_pwm_values[1] = 0x8000;
        leds_pwm_values[2] = 0x8000;
        leds_pwm_values[3] = 0x8000;
    }
}

void leds_anim_draw(void *ctx) {

    leds_front_lum = 0;
    memset(leds_central_rgb, 0, 3);

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
        leds_pwm_values[1] = 0x8000 | 10 * MIN(100, leds_central_rgb[0]);
        leds_pwm_values[2] = 0x8000 | 10 * MIN(100, leds_central_rgb[1]);
        leds_pwm_values[3] = 0x8000 | 10 * MIN(100, leds_central_rgb[2]);

        sch_unique_oneshot(leds_anim_draw, 16);

    } else {
        leds_anim_pwr_off();
    }

}

void leds_direct_set(uint8_t r, uint8_t g, uint8_t b) {
    leds_pwm_values[1] = 0x8000 | 10 * MIN(100, r);
    leds_pwm_values[2] = 0x8000 | 10 * MIN(100, g);
    leds_pwm_values[3] = 0x8000 | 10 * MIN(100, b);
}

void leds_anim_on(leds_animId_t id, uint32_t timeout) {
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

    sch_unique_oneshot(leds_anim_draw, 16);

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


//============================================================================
// Public fns

void leds_bl(bool connected, uint32_t progress) {
    leds_bl_connected = connected;
    leds_bl_progress = progress;
    leds_anim_on(ANIM_BL, 60);
}

void leds_bl_off() {
    leds_anim_off(ANIM_BL);
}

//============================================================================
//

void leds_test(void *ctx) {
    //leds_bl(false, 0);
}

void leds_init() {

    leds_util_buf_init();

    nrf_gpio_cfg_output(EN_VLED);
    nrf_gpio_pin_write(EN_VLED, 1);

    leds_pwmInit();

    //sch_unique_oneshot(leds_test, 1000);
}

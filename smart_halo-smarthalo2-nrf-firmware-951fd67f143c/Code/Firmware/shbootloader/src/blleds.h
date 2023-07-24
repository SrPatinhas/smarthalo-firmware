#ifndef _BLLEDS_H
#define _BLLEDS_H


typedef enum {
	FRONT_ON=0,
	FRONT_BLINK,
	FRONT_STROBE,
} leds_front_mode_t;

void leds_front_set(leds_front_mode_t mode, uint32_t brightness);

void leds_bl(bool connected, uint32_t progress);
void leds_bl_off();

typedef enum {
    ANIM_BL,
    ANIM_FRONT,
} leds_animId_t;

void leds_anim_on(leds_animId_t id, uint32_t timeout);
void leds_anim_off(leds_animId_t id);
void leds_anim_allOff();

void leds_anim_pwr_on();
void leds_anim_pwr_off();

void leds_direct_set(uint8_t r, uint8_t g, uint8_t b);

void leds_init();

#endif

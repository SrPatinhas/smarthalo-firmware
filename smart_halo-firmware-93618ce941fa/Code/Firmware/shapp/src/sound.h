#ifndef _SOUND_H
#define _SOUND_H

#define SOUND_SWEEP 0x8000

typedef struct {
	uint16_t freq;
	uint16_t ms;
} sound_bit_t;

void sound_test(void);
void sound_easteregg(void);
void sound_touch(void *ctx);
void sound_touch_long(void *ctx);

bool sound_isActive(void);

void sound_cmd_play(uint8_t *buf, uint32_t len);
void sound_cmd_stop(uint8_t *buf, uint32_t len);

void sound_stop();
void sound_play(uint32_t volume, uint32_t repeat, sound_bit_t *track);
void sound_battery_low(uint32_t volume);
void sound_horn();
bool sound_codeHandle(uint32_t code, uint32_t len);


void sound_init();

#endif

#ifndef _TOUCH_H
#define _TOUCH_H

typedef struct {
	bool pending;
	bool success;
	int selfcnt;
	int stepcnt;
} touch_test_state_t;

#if !defined(PLATFORM_pca10040) && !defined(PLATFORM_shmp)

void touch_test_do(void);
touch_test_state_t *touch_test_getState(void);

void touch_movement_on();
void touch_movement_off();
void touch_sound(uint8_t volume, bool isEnabled);

void touch_init();

#else

#define touch_test_do() (void)0
#define touch_test_getState() (touch_test_state_t*)0

#define touch_movement_on() (void)0
#define touch_movement_off() (void)0
#define touch_init() (void)0

#endif

#endif
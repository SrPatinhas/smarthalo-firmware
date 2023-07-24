#ifndef _LEDS_H
#define _LEDS_H

typedef enum {
	NAV_DESTINATION=0,
	NAV_FRONT,
	NAV_FRONT_RIGHT,
	NAV_RIGHT,
	NAV_BACK_RIGHT,
	NAV_FRONT_LEFT,
	NAV_LEFT,
	NAV_BACK_LEFT,
	NAV_BACK,
} leds_nav_heading_t;
/*
typedef enum {
	NAV_MODE_NORMAL=0,
	NAV_MODE_MISS,
	NAV_MODE_REROUTE,
} leds_nav_mode_t;
*/

void leds_nav_show(int32_t h, int32_t s, int32_t l, leds_nav_heading_t heading, uint32_t progress);
void leds_nav_destination_angle(int32_t h, int32_t s, int32_t l, float heading, uint32_t progress);
void leds_nav_angle(int32_t h, int32_t s, int32_t l, float heading, uint32_t progress, int32_t nextTurnh, int32_t nextTurns, int32_t nextTurnl, float heading_next_turn, uint32_t progress_next_turn, uint32_t compass_mode);
void leds_nav_reroute(void);
void leds_nav_off(void);
void leds_nav_roundAbout(uint32_t exitNow, uint32_t driveRight, int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t number_exits, float * angle_exits);
void leds_nav_roundAbout_off();
void leds_turn_intro_play(uint8_t mode);

void leds_alarm_arm(void);
void leds_alarm_disarm(void);
void leds_alarm_trigger(bool alert, float level);

typedef enum {
	FRONT_ON=0,
	FRONT_BLINK,
	FRONT_STROBE,
} leds_front_mode_t;

void leds_frontlight_settings(leds_front_mode_t mode, uint32_t brightness);
void leds_frontlight(uint32_t on, uint32_t bg);
void leds_front_external_toggle(bool isRequired);

void leds_progress(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t period, uint32_t progress, uint32_t fitness_on);
void leds_progress_off();

void leds_fitness_intro(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t period);

void leds_speedometer(uint32_t speed, bool introEnabled);
void leds_speedometer_off();

void leds_cadence(int32_t h, int32_t s, int32_t l, float cadence);
void leds_cadence_off();

void leds_notif(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t fadein, uint32_t on, uint32_t fadeout, uint32_t off, uint32_t repeat);
void leds_notif_off();
void leds_hb(int32_t h1, int32_t s1, int32_t l1, int32_t h2, int32_t s2, int32_t l2, uint32_t fadein, uint32_t on, uint32_t fadeout, uint32_t off);
void leds_hb_off();

void leds_bl(uint32_t progress);
void leds_bl_off();

void leds_touch(bool active, touch_type_t type, uint32_t code, uint32_t codeLen);
void leds_touch_complete();

void leds_compass(int32_t h, int32_t s, int32_t l, float heading);
void leds_compass_off(void);

void leds_pointer(int32_t h, int32_t s, int32_t l, float heading, uint32_t standby, uint8_t intro_mode);
void leds_pointer_off(void);

void leds_logo(void);
void leds_disconnect(void);

void leds_clock(uint8_t hour, uint8_t hour_hue, uint8_t hour_sat, uint8_t hour_lum, uint8_t minute, uint8_t minute_hue, uint8_t minute_sat, uint8_t minute_lum, uint8_t center_hue, uint8_t center_sat, uint8_t center_lum,
 uint16_t duration, bool fade, bool intro, bool pulse);
void leds_clock_off();
void leds_battery(float soc);
void leds_battery_low();
void leds_batlevel_show();

void leds_calibrate(float lvl);

void leds_demo_intro();
void leds_demo_intro_off();

void leds_still(void);

void leds_charged();
void leds_charging();
void leds_charging_off();

typedef enum {
    ANIM_LOGO,
    ANIM_DISCONNECT,
    ANIM_NAV,
    ANIM_PROGRESS,
    ANIM_HB,
    ANIM_NOTIF,
    ANIM_ALARM,
    ANIM_ALARM_ARM,
    ANIM_ALARM_DISARM,
    FRONT_LIGHT_ANIMATION,
    ANIM_TOUCH,
    ANIM_BL,
    ANIM_COMPASS,
    ANIM_BATTERY,
    ANIM_LOWBAT,
    ANIM_CALIBRATE,
    ANIM_TEST_TOUCH,
    ANIM_TEST,
    ANIM_TEST2,
    ANIM_SPEEDOMETER,
    ANIM_FITNESS,
    ANIM_ROUNDABOUT,
    ANIM_STILL,
    POINTER_ANIMATION,
    ANIM_CADENCE,
    ANIM_ANIMSCRIPT,
    ANIM_DEMO_INTRO,
    CHARGE_SHADOW,
    CHARGE_TRACE,
    CLOCK_ANIMATION,
    TURN_INTRO_ANIMATION
} leds_animId_t;


void leds_anim_on(leds_animId_t id, uint32_t timeout);
void leds_anim_off(leds_animId_t id);
void leds_anim_allOff();
void leds_turnOff_lights();

void leds_setBrightness(uint32_t brightness);

void leds_movement_on(void);
void leds_movement_off(void);
bool leds_codeHandle(uint32_t code, uint32_t len);

void leds_test(uint8_t num, uint8_t r, uint8_t g, uint8_t b);

bool leds_frontlight_get(void);
void leds_frontlight_set(bool on);
bool leds_frontlight_getState(void);

void leds_lowBat_notify();

void leds_inner_test(void *ctx);

void leds_test_touch(bool active, uint32_t step);

void leds_front_turnOff();

void leds_init();

void leds_shutdown();

void leds_util_copyHSVtoRGB(uint8_t *h, uint8_t *s, uint8_t *v);
void leds_util_central_copyHSVtoRGB(uint8_t h, uint8_t s, uint8_t v);

#endif

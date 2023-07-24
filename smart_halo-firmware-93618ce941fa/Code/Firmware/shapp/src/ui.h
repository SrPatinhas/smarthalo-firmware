#ifndef _UI_H
#define _UI_H

void ui_analyze_touch_sequence(char *codeString, uint8_t code, uint8_t length);
void ui_cmd_nav_destination_angle(uint8_t *buf, uint32_t len);
void ui_cmd_nav_angle_2(uint8_t *buf, uint32_t len);
void ui_cmd_pointer_standby(uint8_t *buf, uint32_t len);
void ui_cmd_pointer_turnOff(uint8_t *buf, uint32_t len);
void ui_cmd_pointer(uint8_t *buf, uint32_t len);
void ui_cmd_leds_demo(uint8_t *buf, uint32_t len);

void ui_cmd_speedometer_intro(uint8_t *buf, uint32_t len);
void ui_cmd_fitness_intro(uint8_t *buf, uint32_t len);

bool ui_isDemoMode(void);

bool ui_isAuth(void);
void ui_onLogoAnimation(void);
void ui_onAuth(void);
void ui_onMovement(void);
void ui_onAlarm_disarm(void);
void ui_init(void);

#endif
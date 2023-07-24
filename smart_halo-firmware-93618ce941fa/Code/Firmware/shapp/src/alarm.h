#ifndef _ALARM_H
#define _ALARM_H

void alarm_arm();
void alarm_disarm();

bool alarm_isEnabled(void);
bool alarm_isCodeValid(void);

void alarm_onAuth(void);

void alarm_onPasswordClear(void);

void alarm_movement_on(void);
void alarm_movement_off(void);
bool alarm_codeHandle(uint32_t code, uint32_t len);
void alarm_init(void);

#endif
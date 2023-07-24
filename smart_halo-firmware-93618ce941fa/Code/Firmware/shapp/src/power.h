#ifndef _POWER_H
#define _POWER_H

void pwr_test(int reset, int enable);

bool pwr_s2_8_getState(void);
void pwr_update(void);
void pwr_shutdown(void);
void pwr_init(void);

#endif
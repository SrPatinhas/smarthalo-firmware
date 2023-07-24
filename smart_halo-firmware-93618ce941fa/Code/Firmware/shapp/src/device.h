#ifndef _DEVICE_H
#define _DEVICE_H

bool dev_isTravelMode(void);
void device_onPasswordClear(void);
void device_getSerial(uint8_t *buf, uint32_t len);

void device_init();

#endif
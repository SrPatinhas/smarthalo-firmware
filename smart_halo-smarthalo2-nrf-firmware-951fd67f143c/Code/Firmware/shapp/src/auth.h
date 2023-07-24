#ifndef _AUTH_H
#define _AUTH_H

//uint8_t * auth_getPubKey(void);

uint8_t* get_fwversion();
//void auth_loadKeys();
//void auth_register_dispatch();

void auth_init();
void auth_getVersions(uint8_t *reply, uint32_t len);
bool auth_isConnected();

#endif

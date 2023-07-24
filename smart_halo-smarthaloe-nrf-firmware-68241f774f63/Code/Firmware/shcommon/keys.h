#ifndef _KEYS_H
#define _KEYS_H

ret_code_t keys_shared_secret_compute(uint8_t const * p_le_pk, uint8_t *p_le_ss);
uint8_t *keys_getPubKey(void);

void keys_init(void);

#endif

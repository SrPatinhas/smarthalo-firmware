#ifndef _BSLINK_H_
#define _BSLINK_H_

void bslink_setSharedSecret(uint8_t *secret);

typedef void (*bslink_listen_cb_t)( uint8_t *buf, uint32_t len );

void bslink_listen(bslink_listen_cb_t cb);

void bslink_write(uint8_t *buf, uint32_t len);

void bslink_up_write(uint8_t *buf, uint32_t len);

void bslink_onAuth();

void bslink_init(void);

#endif 

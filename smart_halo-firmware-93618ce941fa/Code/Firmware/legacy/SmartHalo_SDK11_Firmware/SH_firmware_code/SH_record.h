#ifndef _REC_H
#define _REC_H

typedef void (*rec_cb_t)(ret_code_t err);

ret_code_t rec_read(uint16_t key, uint8_t *buf, uint32_t bufLen, uint32_t *readLen);
void rec_write(uint16_t key, uint8_t *buf, uint32_t len, rec_cb_t cb);

void rec_init(rec_cb_t cb);

#endif
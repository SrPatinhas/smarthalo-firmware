#ifndef _TWI_H
#define _TWI_H

typedef void (*twi_cb_t)(uint32_t, void * );

void twi_transaction_do(const app_twi_transfer_t *transfers, uint32_t numTransfers, twi_cb_t cb, void *ctx);

void twi_shutdown(void);

void twi_init(void);

#endif
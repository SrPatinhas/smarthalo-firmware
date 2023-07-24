
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "sha256.h"

#include "keys.h"

__ALIGN(4) uint8_t keys_sk[32];
__ALIGN(4) uint8_t keys_pk[64];
__ALIGN(4) uint8_t keys_pk_published[64];

uint8_t keys_salt[] = {0xbb, 0x60, 0x2c, 0x0d, 0xf4, 0x62, 0x84, 0xf6, 0xd0, 0x48, 0x78, 0xec, 0xaf, 0x76, 0x6f, 0x04};

ret_code_t keys_shared_secret_compute(uint8_t const * p_le_pk, uint8_t *p_le_ss) {
	return ecc_p256_shared_secret_compute(keys_sk, p_le_pk, p_le_ss);
}

uint8_t *keys_getPubKey() {
    return keys_pk_published;
}


void keys_reverse(uint8_t* p_out, uint8_t* p_in, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len ; i++)
    {
        p_out[i] = p_in[len - i - 1];
    }
}


void keys_print(char *name, uint8_t *buf, uint32_t len) {
    printf("%s: ", name);
    for(int i = 0; i < len; i++) {
        printf("%02X", buf[i]);
    }
    printf("\r\n");
}


void keys_init() {
	//keys_print("ER", (uint8_t *)NRF_FICR->ER, 16);
	//keys_print("IR", (uint8_t *)NRF_FICR->IR, 16);
	sha256_context_t ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, (uint8_t *)NRF_FICR->ER, 16);
	sha256_update(&ctx, (uint8_t *)NRF_FICR->IR, 16);
	sha256_update(&ctx, keys_salt, 16);
	sha256_final(&ctx, keys_sk, 0);
	//keys_print("SK", keys_sk, 32);
    ecc_init(false);
	ecc_p256_public_key_compute(keys_sk, keys_pk);
	//keys_print("PK", keys_pk, 64);
    keys_reverse(keys_pk_published, keys_pk, 32);
    keys_reverse(keys_pk_published+32, keys_pk+32, 32);

    printf("Device Id: ");
    for(int i = 0; i < 8; i++) {
        printf("0x%02X,", keys_pk_published[i]);
    }
    printf("\r\n");

}
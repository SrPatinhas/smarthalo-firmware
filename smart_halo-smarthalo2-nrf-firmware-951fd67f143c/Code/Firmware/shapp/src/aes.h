#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>


// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES128 encryption in CBC-mode of operation and handles 0-padding.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef AES_CBC
  #define AES_CBC 1
#endif

#ifndef AES_ECB
  #define AES_ECB 0
#endif



#if defined(AES_ECB) && AES_ECB

void AES128_ECB_encrypt(uint8_t* input, const uint8_t* key, uint8_t *output);
void AES128_ECB_decrypt(uint8_t* input, const uint8_t* key, uint8_t *output);

#endif // #if defined(AES_ECB) && AES_ECB


#if defined(AES_CBC) && AES_CBC

void AES128_getIV(uint8_t* iv);

void AES128_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES128_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

#endif // #if defined(AES_CBC) && AES_CBC



#endif //_AES_H_

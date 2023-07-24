/*
 */

#ifndef __EXTFLASH_H__
#define __EXTFLASH_H__

#include <stdint.h>
#include <stdbool.h>

#define EXTFLASH_SECTOR_SZ      (64 * 1024)
#define EXTFLASH_FW_IMAGE_SZ    (256 * 1024)

#define EXTFLASH_ID             0x1540EF

bool ExtFlashInit();
bool ExtFlashErase(uint32_t addr, size_t len);
bool ExtFlashRead(uint32_t addr, size_t len, uint8_t *buf);
bool ExtFlashCopyGoldenFW();
bool ExtFlashInstallImage(uint32_t u32QSPIAddr);

extern uint8_t     u8Buffer[FLASHMEM_PAGE_SIZE];

#endif // __EXTFLASH_H__

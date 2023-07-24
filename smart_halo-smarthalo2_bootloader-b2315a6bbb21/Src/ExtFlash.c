/*
 * ExtFlash.c
 */

#include "main.h"
#include "log.h"
#include "crc.h"
#include "qspiflash.h"
#include "FlashMem.h"
#include "ExtFlash.h"
#include "memory_map.h"
#include "mcb.h"
#include "Boot.h"

/*! @brief  Utility buffer, used by other things.

            It is defined once here to keep the memory footprint down
            (vs every module defining its own 2KB buffer).
            The extern declaration is in ExtFlash.h
*/
uint8_t u8Buffer[FLASHMEM_PAGE_SIZE];

/*! @brief  Wakeup and validate the external flash.

    Init, wakeup and check the chip ID on the external flash.
*/
bool ExtFlashInit()
{
    SPIFLASH_init();
    SPIFLASH_wake();

    // small delay
    for (int i = 0; i < 1000; i++)
        asm("nop");

    uint32_t jedec_id = 0;

    SPIFLASH_read_jedec_id(&jedec_id);

    if (jedec_id != EXTFLASH_ID) {
        ERR("Incorrect external flash id! Got: %06lx\n, expected: 0x%06x\n", jedec_id, EXTFLASH_ID);
        die();
    }

    return true;
}

/*! @brief  Erase some sectors of the external flash
            (where sector is 64KB)
*/
bool ExtFlashErase(uint32_t addr, size_t len)
{
    if (len % EXTFLASH_SECTOR_SZ) {
        ERR("ExtFlashErase: len should be a multiple of %d\n", EXTFLASH_SECTOR_SZ);
        return false;
    }

    while (len) {
        if (SPIFLASH_erase_SECTOR(addr) != HAL_OK) {
            return false;
        }
        addr += EXTFLASH_SECTOR_SZ;
        len -= EXTFLASH_SECTOR_SZ;
    }

    return true;
}

/*! @brief  Read len bytes from external flash addr into buf

    The reading is performed conservatively in BS sizes chunks.
*/
bool ExtFlashRead(uint32_t addr, size_t len, uint8_t *buf)
{
#define BS 256
    if (len % BS) {
        ERR("ExtFlashRead: len should be a multiple of %d bytes\n", BS);
        return false;
    }

    while (len) {
        if (SPIFLASH_read(addr, BS, buf) != HAL_OK) {
            ERR("problem reading block at %08lx\n", addr);
            return false;
        }
        addr += BS;
        buf += BS;
        len -= BS;
    }

    return true;
}

/*! @brief  Copy FW from internal flash to the golden/backup region of
            the external flash.
*/
bool ExtFlashCopyGoldenFW()
{
    int i;
    bool flag = false;

    // Assume that the image at UC_START_ADDR is golden
    ExtFlashErase(EXT_FLASH_BACKUP_START_OFFSET, EXTFLASH_FW_IMAGE_SZ);
    uint8_t *src = (uint8_t *)UC_START_ADDR;
    uint32_t dst = EXT_FLASH_BACKUP_START_OFFSET;
    LOG("Creating Golden image on external flash\n");
    for (i = 0; i < 1024; i++) {
        SPIFLASH_write(dst, 256, src);
        if (is_block_erased(src, 256)) {
            if (!flag) {
                LOG("\nfirst empty block at %p\n", src);
                flag = true;
            }
        }
        LOG(".");
        src += 256;
        dst += 256;
    }
    LOG("\n");

    LOG("Verifying copy\n");
    src = (uint8_t *)UC_START_ADDR;
    dst = EXT_FLASH_BACKUP_START_OFFSET;
    for (i = 0; i < 1024; i++) {
        memset(u8Buffer, 0xff, 256);
        SPIFLASH_read(dst, 256, u8Buffer);
        if (memcmp(src, u8Buffer, 256)) {
            ERR("Error at %08lx\n", dst);
            break;
        }
        LOG(".");
        src += 256;
        dst += 256;
    }
    LOG("\n");

    return true;
}

/*! @brief  Copy a firmware image from external to internal flash
*/
bool ExtFlashInstallImage(uint32_t u32QSPIAddr)
{
    bool        retval = true, bData;
    uint32_t    u32InternalAddr;

    BF->disableWatchdog = false;

    for (u32InternalAddr = UC_START_ADDR;
         u32InternalAddr < UC_END_ADDR;
         u32QSPIAddr += FLASHMEM_PAGE_SIZE, u32InternalAddr += FLASHMEM_PAGE_SIZE) {

        memset(u8Buffer, 0xff, FLASHMEM_PAGE_SIZE);

        // Slow down the printf
        // for (i = 0; i < 1000000; i++) asm("nop");

        // printf("Reading from: 0x%08x  ", u32QSPIAddr);

        LOG(".");

        if (!ExtFlashRead(u32QSPIAddr, FLASHMEM_PAGE_SIZE, u8Buffer)) {
            LOG("ExtFlashRead failed!\n");
            retval = false;
            break;
        }

        // On the first block, check the "magic number"
        if (u32InternalAddr == UC_START_ADDR) {
            if (!CheckFirmwareMagic(u8Buffer)) {
                retval = false;
                break;
            }
        }

        if (!FlashMemErasePage(u32InternalAddr, 1)) {
            LOG("FlashMemErasePage failed\n");
            retval = false;
            break;
        }
        if (FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE) != HAL_OK) {
            LOG("FLASH_WaitForLastOperation failed\n");
        }

        // printf("Writing to: 0x%08x\n", u32InternalAddr);

        if (!FlashMemWriteFullPage(u32InternalAddr, u8Buffer, &bData)) {
            LOG("FlashMemWriteFullPage failed\n");
            retval = false;
            break;
        }
        if (FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE) != HAL_OK) {
            LOG("FLASH_WaitForLastOperation failed\n");
        }
        if (!bData) {
            LOG("bData from FlashMemWriteFullPage is false!\n");
            retval = false;
            break;
        }
    }

    LOG("\n");

    // on failure, nothing more to do
    if (retval == false) return retval;

    // Check CRC, set boot flag
    // extern uint32_t __region_boot_key__;
    // __region_boot_key__ = 0;

    return retval;
}

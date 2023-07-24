/**
 * @file    memory_map.h
 * @brief   Memory map
 * @author  Georg Nikodym
 * @details Macros defining locations and sizes of various things on the SH2 device
 */

#include <stdint.h>

/*
 * Internal flash map
 *
 *  0x08000000 - 0x08005dff     boot loader
 *  0x08005800 - 0x08005fff     MCB - magic control block (CRCs and stuff)
 *  0x08006000 - 0x08006003     user code CRC
 *  0x08006004 - 0x0803ffff     user code (SmartHalo2 OS)
 *
 */
#define UC_START_ADDR   0x8006000
#define UC_END_ADDR     0x8040000
#define UC_LEN          (UC_END_ADDR - UC_START_ADDR)

/*
 * Magic Control Block (mcb.h)
 */
#define MCB_MAGIC       0x8005800           // magic control block (2KB because flash page size)

/*
 * External flash offsets map
 *
 *  0x00000000 - 0x0003ffff     backup region (golden firmware)
 *  0x00040000 - 0x0007ffff     update region (new FW gets put here first)
 *
 * N.B. These are offsets into the external flash -- NOT addresses
 */
#define EXT_FLASH_BACKUP_START_OFFSET   0
#define EXT_FLASH_UPDATE_START_OFFSET   0x00040000
#define EXT_FLASH_FIRMWARE_END_OFFSET   0x00080000
#define EXT_FLASH_BACKUP_LEN            (EXT_FLASH_UPDATE_START_OFFSET - EXT_FLASH_BACKUP_START_OFFSET)
#define EXT_FLASH_UPDATE_LEN            (EXT_FLASH_FIRMWARE_END_OFFSET - EXT_FLASH_UPDATE_START_OFFSET)

/*
 * Internal RAM
 *
 *  160KB starting at 0x200000000
 *
 * We put some "sticky" things at the end of this region.
 * Sticky, meaning data that we expect to survive a soft-reset.
 *
 * (ie, see __region_boot_key__ in the linker map)
 */
#define SRAM_START          0x20000000
#define SRAM_END            0x20027fff

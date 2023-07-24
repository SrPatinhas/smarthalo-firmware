
#ifndef QSPIFLASH_H_
#define QSPIFLASH_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


#define QSPI_PAGE_SIZE                       256

/* Reset Operations */
#define RESET_ENABLE_CMD                     0x66
#define RESET_MEMORY_CMD                     0x99

/* Identification Operations */
#define READ_ID_CMD                          0x9E
#define READ_ID_CMD2                         0x9F
#define MULTIPLE_IO_READ_ID_CMD              0xAF
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD    0x5A

/* Read Operations */
#define READ_CMD                             0x03
#define READ_4_BYTE_ADDR_CMD                 0x13

#define FAST_READ_CMD                        0x0B
#define FAST_READ_DTR_CMD                    0x0D
#define FAST_READ_4_BYTE_ADDR_CMD            0x0C

#define DUAL_OUT_FAST_READ_CMD               0x3B
#define DUAL_OUT_FAST_READ_DTR_CMD           0x3D
#define DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x3C

#define DUAL_INOUT_FAST_READ_CMD             0xBB
#define DUAL_INOUT_FAST_READ_DTR_CMD         0xBD
#define DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xBC

#define QUAD_OUT_FAST_READ_CMD               0x6B
#define QUAD_OUT_FAST_READ_DTR_CMD           0x6D
#define QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x6C

#define QUAD_INOUT_FAST_READ_CMD             0xEB
#define QUAD_INOUT_FAST_READ_DTR_CMD         0xED
#define QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xEC

/* Write Operations */
#define WRITE_ENABLE_CMD                     0x06
#define WRITE_DISABLE_CMD                    0x04

/* Register Operations */
#define READ_STATUS_REG_CMD                  0x05
#define WRITE_STATUS_REG_CMD                 0x01

#define READ_LOCK_REG_CMD                    0xE8
#define WRITE_LOCK_REG_CMD                   0xE5

#define READ_FLAG_STATUS_REG_CMD             0x70
#define CLEAR_FLAG_STATUS_REG_CMD            0x50

#define READ_NONVOL_CFG_REG_CMD              0xB5
#define WRITE_NONVOL_CFG_REG_CMD             0xB1

#define READ_VOL_CFG_REG_CMD                 0x85
#define WRITE_VOL_CFG_REG_CMD                0x81

#define READ_ENHANCED_VOL_CFG_REG_CMD        0x65
#define WRITE_ENHANCED_VOL_CFG_REG_CMD       0x61

#define READ_EXT_ADDR_REG_CMD                0xC8
#define WRITE_EXT_ADDR_REG_CMD               0xC5

/* Program Operations */
#define PAGE_PROG_CMD                        0x02
#define PAGE_PROG_4_BYTE_ADDR_CMD            0x12

#define DUAL_IN_FAST_PROG_CMD                0xA2
#define EXT_DUAL_IN_FAST_PROG_CMD            0xD2

#define QUAD_IN_FAST_PROG_CMD                0x32
#define EXT_QUAD_IN_FAST_PROG_CMD            0x12 /*0x38*/
#define QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD    0x34

/* Erase Operations */
#define SUBSECTOR_ERASE_CMD                  0x20
#define SUBSECTOR_ERASE_4_BYTE_ADDR_CMD      0x21

#define SECTOR_ERASE_CMD                     0xD8
#define SECTOR_ERASE_4_BYTE_ADDR_CMD         0xDC

#define BULK_ERASE_CMD                       0xC7

#define PROG_ERASE_RESUME_CMD                0x7A
#define PROG_ERASE_SUSPEND_CMD               0x75

/* One-Time Programmable Operations */
#define READ_OTP_ARRAY_CMD                   0x4B
#define PROG_OTP_ARRAY_CMD                   0x42

/* 4-byte Address Mode Operations */
#define ENTER_4_BYTE_ADDR_MODE_CMD           0xB7
#define EXIT_4_BYTE_ADDR_MODE_CMD            0xE9

/* Quad Operations */
#define ENTER_QUAD_CMD                       0x35
#define EXIT_QUAD_CMD                        0xF5

/* Default dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ              8
#define DUMMY_CLOCK_CYCLES_READ_QUAD         8

#define DUMMY_CLOCK_CYCLES_READ_DTR          6
#define DUMMY_CLOCK_CYCLES_READ_QUAD_DTR     8

/**
 * Initiates the spi flash device struct.
 *
 */
void SPIFLASH_init();

/**
 * Writes data to the spi flash.
 *
 * @param addr  the address of the spi flash to write to.
 * @param len   number of bytes to write.
 * @param buf   the data to write.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_write(uint32_t addr, uint32_t len, uint8_t *buf);

/**
 * Erases data in the spi flash. The erase range must be aligned to the
 * smallest erase size a SPIFLASH_ERR_ERASE_UNALIGNED will be returned.
 *
 * @param addr  the address of the spi flash to write to.
 * @param buf   the data to write.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_erase_SUBSECTOR(uint32_t addr);	// 4K
int SPIFLASH_erase_SECTOR(uint32_t addr);	// 64K

/**
 * Erases the entire spi flash chip.
 *
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_chip_erase();

/**
 * Writes to the status register.
 *
 * @param sr   the data to write to sr.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_write_sr(uint8_t sr);

/**
 * Reads from the spi flash.
 *
 * @param addr  the address to read from.
 * @param len   number of bytes to read.
 * @param buf   where to put read data.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_read(uint32_t addr, uint32_t len, uint8_t *buf);

/**
 * Reads from the spi flash, fast mode. Will automatically add one extra
 * dummy byte to address (apart from cfg.addr_dummy_sz). If not supported
 * (0 in cmd_tbl), a normal read will take place.
 *
 * @param addr  the address to read from.
 * @param len   number of bytes to read.
 * @param buf   where to put read data.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_fast_read(uint32_t addr, uint32_t len,
                       uint8_t *buf);

/**
 * Reads the status register.
 *
 * @param sr   where to put the data of the status register.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_read_sr(uint8_t *sr);

/**
 * Reads the status register and parses the busy bit according to
 * cmd_tbl.sr_busy_bit.
 *
 * @param busy  will be set to true if busy, false otherwise.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_read_sr_busy(uint8_t *busy);

/**
 * Reads the jedec id of the device, 3 bytes.
 *
 * @param jedec_id  where to store the jedec id.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_read_jedec_id(uint32_t *jedec_id);

/**
 * Reads some hardware specific register.
 *
 * @param reg      register number
 * @param data     where to store the register contents.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_read_reg(uint8_t reg, uint8_t *data);

/**
 * Writes some hardware specific register.
 *
 * @param reg       register number
 * @param data      what to set the register to.
 * @param write_en  if write enable must be issued before writing to register.
 * @param wait_ms   if write_en is enabled, this states the typical time in
 *                  milliseconds to write to the register.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_write_reg(uint8_t reg, uint8_t data,
    uint8_t write_en, uint32_t wait_ms);

/**
 * Reads the product id of the device, 3 bytes.
 *
 * @param prod_id  where to store the product id.
 * @return error code or SPIFLASH_OK
 */
int SPIFLASH_read_product_id(uint32_t *prod_id);


int SPIFLASH_deep_sleep();
int SPIFLASH_wake();


#ifdef __cplusplus
}
#endif

#endif /*QSPIFLASH_H_*/


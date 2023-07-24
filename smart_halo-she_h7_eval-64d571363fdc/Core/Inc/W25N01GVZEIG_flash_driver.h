/*
 * W25N01GVZEIG_flash_driver.h
 *
 *  Created on: May 10, 2021
 *      Author: Felix Cormier
 */

#ifndef INC_W25N01GVZEIG_FLASH_DRIVER_H_
#define INC_W25N01GVZEIG_FLASH_DRIVER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "stm32h7xx_hal.h"

#define OSPI_TIMEOUT	93333333	// 1 second @ 93.333 MHz
#define N_PAGES         65536         
#define N_COLUMNS       2048
#define N_BLOCKS        1024
#define N_BYTES         (N_PAGES * N_COLUMNS)

typedef enum {
    DEVICE_RESET                    = 0xFF,
    READ_STATUS_REG                 = 0x0F,
    WRITE_STATUS_REG                = 0x1F,
    JEDEC_ID                        = 0x9F,
    WRITE_ENABLE                    = 0x06,
    WRITE_DISABLE                   = 0x04,
    BAD_BLOCK_MANAGEMENT            = 0xA1,
    READ_BBM_LUT                    = 0xA5,
    LAST_ECC_FAILURE_PAGE_ADR       = 0xA9,
    BLOCK_ERASE_128KB               = 0xD8,
    QUAD_LOAD_PROG_DATA_RANDOM      = 0x34,
    PROGRAM_EXECUTE                 = 0x10,
    PAGE_DATA_READ                  = 0x13,
    FAST_READ_QUAD_OUTPUT           = 0x6B,
} FlashOpCode;

typedef enum {
    REG_PROTECTION_ADR  = 0xA0,
    REG_CONFIG_ADR      = 0xB0,
    REG_STATUS_ADR      = 0xC0
} StatusRegAddress;

typedef struct {
    uint8_t  mfr_id;
    uint16_t device_id;
} JedecID;

typedef enum {
    OPTYPE_COMMON  = 0,
    OPTYPE_READ    = 1,
    OPTYPE_WRITE   = 2,
    OPTYPE_WRAP    = 3
} OspiOperationType;


HAL_StatusTypeDef extFlashInit(OSPI_HandleTypeDef *hospi);
HAL_StatusTypeDef extFlashReset(OSPI_HandleTypeDef *hospi);
HAL_StatusTypeDef extFlashGetID(OSPI_HandleTypeDef *hospi, JedecID *id);
HAL_StatusTypeDef extFlashQuadLoadProgramDataRandom(OSPI_HandleTypeDef *hospi, uint8_t* p_data, uint16_t data_size, uint16_t column_adr);
HAL_StatusTypeDef extFlashProgramExecute(OSPI_HandleTypeDef *hospi, uint16_t page_adr);
HAL_StatusTypeDef extFlashBlockErase128kB(OSPI_HandleTypeDef *hospi, uint16_t block_adr);
HAL_StatusTypeDef extFlashPageDataRead(OSPI_HandleTypeDef *hospi, uint16_t page_adr);
HAL_StatusTypeDef extFlashFastReadQuadOutput(OSPI_HandleTypeDef *hospi, uint8_t *p_data, uint32_t data_size);


#ifdef __cplusplus
}
#endif

#endif /* INC_W25N01GVZEIG_FLASH_DRIVER_H_ */

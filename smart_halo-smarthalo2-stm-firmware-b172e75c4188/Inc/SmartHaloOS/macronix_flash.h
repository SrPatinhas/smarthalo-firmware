// ------------------------------------------------------------------------------------------------
/* INTELLIGENT PCB SYSTEMS PROPRIETARY - www.ipcbsystems.com
 COPYRIGHT (c) 2004-2016 BY INTELLIGENT PCB SYSTEMS CORPORATION. ALL RIGHTS RESERVED. NO PART OF
 THIS PROGRAM OR PUBLICATION MAY BE REPRODUCED, TRANSMITTED, TRANSCRIBED, STORED IN A RETRIEVAL
 SYSTEM, OR TRANSLATED INTO ANY LANGUAGE OR COMPUTER LANGUAGE IN ANY FORM OR BY ANY MEANS,
 ELECTRONIC, MECHANICAL, MAGNETIC, OPTICAL, CHEMICAL, MANUAL, OR OTHERWISE, WITHOUT THE PRIOR
 WRITTEN PERMISSION OF INTELLIGENT PCB SYSTEMS INC.
 */
// ------------------------------------------------------------------------------------------------
/*!@file    macronix_flash.h
 
 */
// ------------------------------------------------------------------------------------------------

#ifndef __MACRONIX_FLASH_H__
#define __MACRONIX_FLASH_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define MACRONIX_FLASH_SUCCESS						(0)
#define MACRONIX_FLASH_ERROR							(-1)
#define MACRONIX_FLASH_BAD_PARAMETER			(-2)
#define MACRONIX_FLASH_NON_INITIALIZED		(-3)
#define MACRONIX_FLASH_TRANSACTION_ERROR	(-4)

/* M25P SPI Flash supported commands */
#define MACRONIX_FLASH_WRITE            0x02  /* Write to Memory instruction */
#define MACRONIX_FLASH_WRSR             0x01  /* Write Status Register instruction */
#define MACRONIX_FLASH_WREN             0x06  /* Write enable instruction */
#define MACRONIX_FLASH_READ             0x03  /* Read from Memory instruction */
#define MACRONIX_FLASH_RDSR             0x05  /* Read Status Register instruction  */
#define MACRONIX_FLASH_RDID             0x9F  /* Read identification */
#define MACRONIX_FLASH_SE               0x20  /* Sector Erase instruction */
#define MACRONIX_FLASH_BE               0xD8  /* Bulk Erase instruction */
#define MACRONIX_FLASH_CE               0xC7  /* Chip Erase instruction */

#define MACRONIX_FLASH_DP								0xB9	/* Enter deep power mode */
#define MACRONIX_FLASH_RDP							0xAB	/* Exit deep power mode */

#define MACRONIX_FLASH_WIP_FLAG         0x01  /* Write In Progress (WIP) flag */

#define MACRONIX_FLASH_PAGESIZE        	0x100
#define MACRONIX_FLASH_SECTORSIZE       0x1000
//#define MACRONIX_FLASH_NBRE_SECTOR      0x400
#define MACRONIX_FLASH_NBRE_SECTOR      0x800

//#define MACRONIX_FLASH_ID              	0xC22016	// Macronix MX25L3206E
//#define MACRONIX_FLASH_ID              	0xC22017	// Macronix MX25L6406E

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================
typedef struct {
	uint16_t addr;

	int8_t (*write)(uint16_t, uint8_t *, uint16_t); // Dev address, data, data size. Return 0 on success
	int8_t (*read)(uint16_t, uint8_t *, uint16_t); // Dev address, data, data size. Return 0 on success
	int8_t (*slave_select)(int8_t); // Slave select function. 1 to enable and 0 to disable. Return 0 on success
} macronix_flash_t;

// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
int8_t MACRONIX_FLASH_init(macronix_flash_t *m_flash);
int8_t MACRONIX_FLASH_EraseSector(macronix_flash_t *m_flash, uint32_t SectorAddr);
int8_t MACRONIX_FLASH_EraseBlock(macronix_flash_t *m_flash, uint32_t BlockAddr);
int8_t MACRONIX_FLASH_EraseBulk(macronix_flash_t *m_flash);
int8_t MACRONIX_FLASH_WriteBuffer(macronix_flash_t *m_flash, uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
int8_t MACRONIX_FLASH_ReadBuffer(macronix_flash_t *m_flash, uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
int8_t MACRONIX_FLASH_Sleep(macronix_flash_t *m_flash);
int8_t MACRONIX_FLASH_Wake(macronix_flash_t *m_flash);

#endif  /* __MACRONIX_FLASH_H__ */

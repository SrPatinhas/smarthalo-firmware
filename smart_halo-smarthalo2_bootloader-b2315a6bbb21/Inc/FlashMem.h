/**
 * @file    FlashMem.h
 * @brief   Module to interact with the internal flash memory
 * @author  Dany Thiffeault
 * @date    2018-12-06 (creation)
 * @details Internal flash memory access manager.
 */

#ifndef __FLASHMEM__H
#define __FLASHMEM__H

/**
 * Include
 */
#include "main.h"
#include "memory_map.h"

/**
 * Public definition
 */
#define FLASHMEM_UT			0		///< Enables or not the unit tests of this module.
#define FLASHMEM_PAGE_SIZE	2048	///< Flash memory page size, in bytes.

/**
 * Public type
 */


/**
 * Public functions
 */
bool FlashMemRead(uint32_t u32Addr, uint32_t u32Length, uint8_t* pu8Data);
bool FlashMemReadFullPage(uint32_t u32AddrInPage, uint8_t* pu8Data, uint32_t u32DataBufferSize);
bool FlashMemWrite(uint32_t u32StartAddr, uint32_t u32Length, const uint8_t* pu8Data, bool* pbWriteSuccess);
bool FlashMemWriteFullPage(uint32_t u32AddrInPage, const uint8_t* pu8Data, bool* pbWriteSuccess);
bool FlashMemErasePage(uint32_t u32AddrInPage, uint32_t u32NbOfPages);
bool FlashMemIsEmpty(uint32_t u32StartAddr, uint32_t u32Length, bool* pbIsEmpty);

#if (FLASHMEM_UT == 1)
bool FlashMem_UT(void);
#endif

#endif

/**
 * @file    FlashMem.c
 * @brief   Module to interact with the internal flash memory
 * @author  Jean-Francois Lemire
 * @date    2019-06-17 (creation)
 * @details Internal flash memory access manager.
 */

/**
 * Includes
 */
#include "FlashMem.h"


/**
 * Private Definitions
 */


// #define FLASHMEM_START_ADDR		((uint32_t)&__region_USER_CODE_start__)	///< Defines the start address of the supported Flash memory.
// #define FLASHMEM_END_ADDR		((uint32_t)&__region_USER_CODE_end__)		///< Defines the end address of the supported Flash memory.
#define FLASHMEM_PAGE_START(x)	(x - (x % FLASHMEM_PAGE_SIZE))		///< Macro to find the page start address of any given address.

/**
* Private Typedef
*/


/**
 * Private Variables
 */


/**
 * Private Functions
 */


/**
 * @brief       FlashMemWrite
 * @details     Write into the internal Flash memory.
 * @public
 * @param[in]   u32StartAddr	starting write address. Must be aligned on 32bits.
 * @param[in]   u32Length		number of 32bits words to write.
 * @param[in]   pu32Data		32bits words to write.
 * @param[out]  pbWriteSuccess	indicates if the write operation was a success.
 * @return      true if success, false otherwise.
 */
bool FlashMemWrite(uint32_t u32StartAddr, uint32_t u32Length, const uint8_t* pu8Data, bool* pbWriteSuccess)
{
    uint32_t			u32Idx			= 0;
    uint32_t			u32WriteAddr	= u32StartAddr;
    HAL_StatusTypeDef 	halStatus 		= HAL_ERROR;
    uint64_t            u64Data  = 0;
    // Init the write success parameter.
    if (pbWriteSuccess == NULL) return false;
    *pbWriteSuccess = false;

    // Validate memory overflow.
    if (pu8Data == NULL) return false;
    if (u32StartAddr < MCB_MAGIC) return false;
    if (u32StartAddr + u32Length > (UC_END_ADDR + 1)) return false;

    // Validate address. Writing must always be aligned to 32bits.
    if ((u32StartAddr % 4) != 0) return true;

    // Write the flash.
    HAL_FLASH_Unlock();

    u32Length /= 8;

    for (u32Idx = 0; u32Idx < u32Length; u32Idx ++) {
        memcpy(&u64Data, pu8Data, 8);
        halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32WriteAddr, u64Data);
        if (halStatus != HAL_OK) {
            HAL_FLASH_Lock();
            return true;
        }

        u32WriteAddr += 8;
        pu8Data += 8;
    }

    HAL_FLASH_Lock();

    *pbWriteSuccess = true;
    return true;
}

/**
 * @brief       FlashMemWriteFullPage
 * @details     Write a full page into the internal Flash memory.
 * @public
 * @param[in]   u32AddrInPage	address in page. Page's start address will be found.
 * @param[in]   pu32Data		32bits words to write.
 * @param[out]  pbWriteSuccess	indicates if the write operation was a success.
 * @return      true if success, false otherwise.
 */
bool FlashMemWriteFullPage(uint32_t u32AddrInPage, const uint8_t* pu8Data, bool* pbWriteSuccess)
{
    uint32_t u32StartAddr 	= 0;

    // Init the write success parameter.
    if (pbWriteSuccess == NULL) return false;
    *pbWriteSuccess = false;

    // Validate memory overflow.
    if (pu8Data == NULL) return false;
    if (u32AddrInPage < MCB_MAGIC) return false;
    if (u32AddrInPage > UC_END_ADDR) return false;

    // Find the start address of the page to erase.
    u32StartAddr = FLASHMEM_PAGE_START(u32AddrInPage);

    return FlashMemWrite(u32StartAddr, FLASHMEM_PAGE_SIZE, pu8Data, pbWriteSuccess);
}

/**
 * @brief       FlashMemErasePage
 * @details     Erase one or many pages of the internal Flash memory.
 * @public
 * @param[in]   u32StartAddr	address in page. Page's start address will be found.
 * @param[in]   u32NbOfPages	number of pages to erase.
 * @return      true if success, false otherwise.
 */
bool FlashMemErasePage(uint32_t u32AddrInPage, uint32_t u32NbOfPages)
{
    uint32_t u32StartAddr 	= 0;
    uint32_t u32PageError 	= 0;

    HAL_StatusTypeDef halStatus = HAL_ERROR;
    static FLASH_EraseInitTypeDef EraseInitStruct;

    // Validate memory overflow.
    if (u32AddrInPage < MCB_MAGIC)	return false;
    if (u32AddrInPage > UC_END_ADDR) 	return false;

    // Find the start address of the page to erase.
    u32StartAddr = FLASHMEM_PAGE_START(u32AddrInPage);

    // Validate the number of pages to erase.
    if ((u32StartAddr + (u32NbOfPages * FLASHMEM_PAGE_SIZE)) > (UC_END_ADDR + 1)) 	return false;

    // Erase the flash
    EraseInitStruct.TypeErase 	= FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page        = (u32StartAddr - 0x08000000) / FLASHMEM_PAGE_SIZE;
    EraseInitStruct.NbPages 	= u32NbOfPages;

    HAL_FLASH_Unlock();
    halStatus = HAL_FLASHEx_Erase(&EraseInitStruct, &u32PageError);
    HAL_FLASH_Lock();

    if (halStatus != HAL_OK) {
        return false;
    }

    return true;
}

/**
 * @brief       FlashMemRead
 * @details     Read part of the internal Flash memory.
 * @public
 * @param[in]   u32Addr		address to start reading.
 * @param[in]   u32Length	number of bytes (8bits) to read.
 * @param[out]  pu8Data		container for the read data.
 * @return      true if success, false otherwise.
 */
bool FlashMemRead(uint32_t u32Addr, uint32_t u32Length, uint8_t* pu8Data)
{
    if (pu8Data == NULL) return false;
    if (u32Addr < MCB_MAGIC) return false;
    if ((u32Addr + u32Length) > (UC_END_ADDR + 1)) return false;

    memcpy(pu8Data, (void*)u32Addr, u32Length);

    return true;
}

/**
 * @brief       FlashMemReadFullPage
 * @details     Read a full page of the internal Flash memory.
 * @public
 * @param[in]   u32AddrInPage		address in page. Page's start address will be found.
 * @param[out]  pu8Data				container for the read data.
 * @param[in]   u32DataBufferSize	size of the buffer used to store data. Must be at least big enough for one page.
 * @return      true if success, false otherwise.
 */
bool FlashMemReadFullPage(uint32_t u32AddrInPage, uint8_t* pu8Data, uint32_t u32DataBufferSize)
{
    uint32_t u32StartAddr = 0;

    if (pu8Data == NULL) return false;
    if (u32DataBufferSize < FLASHMEM_PAGE_SIZE) return false;
    if (u32AddrInPage < MCB_MAGIC) return false;
    if (u32AddrInPage >= UC_END_ADDR) return false;

    // Locate the start address of the page.
    u32StartAddr = FLASHMEM_PAGE_START(u32AddrInPage);

    // Read it.
    return FlashMemRead(u32StartAddr, FLASHMEM_PAGE_SIZE, pu8Data);
}

/**
 * @brief       FlashMemIsEmpty
 * @details     Verify if memory is empty and writable (0xFF) or not.
 * @public
 * @param[in]   u32StartAddr	address to start validation.
 * @param[in]   u32Length		number of bytes (8bits) to validate.
 * @param[out]  pbIsEmpty		indicates if range is empty (0xFF) or not.
 * @return      true if success, false otherwise.
 */
bool FlashMemIsEmpty(uint32_t u32StartAddr, uint32_t u32Length, bool* pbIsEmpty)
{
    uint8_t* 	pu8Data 		= NULL;
    uint32_t 	u32NbChecked 	= 0;

    if (pbIsEmpty == NULL) return false;

    // Address is out of range. Not writable.
    if ((u32StartAddr < MCB_MAGIC) || (u32StartAddr + u32Length >= UC_END_ADDR)) {
        *pbIsEmpty = false;
        return true;
    }

    // Cycle in memory and verify.
    for (pu8Data = (uint8_t*)u32StartAddr; u32NbChecked < u32Length; u32NbChecked++, pu8Data++) {
        if (*pu8Data != 0xFF) {
            *pbIsEmpty = false;
            return true;
        }
    }

    *pbIsEmpty = true;
    return true;
}

#if (FLASHMEM_UT == 1)
#warning "FlashMem Unit Tests activated"

uint8_t u8FlashMemUTBuffer[FLASHMEM_PAGE_SIZE];

bool FlashMem_UT_WriteReadEraseFullPage(void);
bool FlashMem_UT_WriteRead(void);
bool FlashMem_UT_WriteNonEmpty(void);
bool FlashMem_UT_OutOfBound(void);

/**
 * @brief       FlashMem_UT()
 * @details     Execute all the unit tests of this module.
 * @public
 * @return      true if success, false otherwise.
 */
bool FlashMem_UT(void)
{
    // Cases to test:
    // 1- Write/Read/Erase full page
    // 2- Write/Read part of memory
    // 3- Write non-empty memory
    // 4- Read/Write/Erase out of bound (address, length)

    if (FlashMem_UT_WriteReadEraseFullPage() == false) while (1);
    if (FlashMem_UT_WriteRead() == false) while (1);
    if (FlashMem_UT_WriteNonEmpty() == false) while (1);
    if (FlashMem_UT_OutOfBound() == false) while (1);

    return true;
}

/**
 * @brief       FlashMem_UT_WriteReadEraseFullPage()
 * @details     Execute the unit test for: Write, read, erase full pages and empty verifications.
 * @public
 * @return      true if success, false otherwise.
 */
bool FlashMem_UT_WriteReadEraseFullPage(void)
{
    uint32_t 	u32Idx 			= 0;
    bool 		bIsEmpty 		= false;
    bool		bWriteSuccess	= false;

    // Ensure the memory is empty, perform a preliminary erase.
    if (FlashMemErasePage(((uint32_t)&__region_EE_start__) + 10, 1) == false) return false;

    // Ensure memory is empty.
    if (FlashMemIsEmpty(((uint32_t)&__region_EE_start__) + 1, FLASHMEM_PAGE_SIZE, &bIsEmpty) == false) return false;
    if (bIsEmpty == false) return false;

    // Write a full page
    for (u32Idx = 0; u32Idx < FLASHMEM_PAGE_SIZE; ++u32Idx) {
        u8FlashMemUTBuffer[u32Idx] = u32Idx % 256;
    }

    if (FlashMemWriteFullPage(((uint32_t)&__region_EE_start__), (uint32_t*) u8FlashMemUTBuffer, &bWriteSuccess) == false) return false;
    if (bWriteSuccess == false) return false;

    // Ensure memory is not empty.
    bIsEmpty = false;
    if (FlashMemIsEmpty(((uint32_t)&__region_EE_start__) + 98, FLASHMEM_PAGE_SIZE, &bIsEmpty) == false) return false;
    if (bIsEmpty == true) return false;

    // Reload page and compare.
    memset(u8FlashMemUTBuffer, 0, sizeof(u8FlashMemUTBuffer));
    if (FlashMemReadFullPage(((uint32_t)&__region_EE_start__) + 687, u8FlashMemUTBuffer, sizeof(u8FlashMemUTBuffer)) == false) return false;

    for (u32Idx = 0; u32Idx < FLASHMEM_PAGE_SIZE; ++u32Idx) {
        if (u8FlashMemUTBuffer[u32Idx] != u32Idx % 256) return false;
    }

    // Erase again the page.
    if (FlashMemErasePage(((uint32_t)&__region_EE_start__), 1) == false) return false;

    // Ensure memory is empty.
    if (FlashMemIsEmpty(((uint32_t)&__region_EE_start__), FLASHMEM_PAGE_SIZE, &bIsEmpty) == false) return false;
    if (bIsEmpty == false) return false;

    return true;
}

/**
 * @brief       FlashMem_UT_WriteRead()
 * @details     Execute the unit test for: Write, read of partial pages and empty verifications.
 * @public
 * @return      true if success, false otherwise.
 */
bool FlashMem_UT_WriteRead(void)
{
    uint32_t 	u32TestValue 	= 0xDEADBEEF;
    bool		bWriteSuccess	= false;

    // Erase a page.
    if (FlashMemErasePage(((uint32_t)&__region_FS_start__), 1) == false) return false;

    // Write
    if (FlashMemWrite(((uint32_t)&__region_FS_start__), 4, &u32TestValue, &bWriteSuccess) == false) return false;
    if (bWriteSuccess == false) return false;

    // Read back.
    u32TestValue = 0;
    if (FlashMemRead(((uint32_t)&__region_FS_start__), 4, (uint8_t*)&u32TestValue) == false) return false;

    if (u32TestValue != 0xDEADBEEF) return false;

    // Clear memory after test.
    if (FlashMemErasePage(((uint32_t)&__region_FS_start__), 1) == false) return false;

    return true;
}

/**
 * @brief       FlashMem_UT_WriteNonEmpty()
 * @details     Execute the unit test for: Write at non-empty addresses.
 * @public
 * @return      true if success, false otherwise.
 */
bool FlashMem_UT_WriteNonEmpty(void)
{
    uint32_t 	u32TestValue 	= 0xDEADBEEF;
    bool		bWriteSuccess	= false;

    // Erase again the page.
    if (FlashMemErasePage(((uint32_t)&__region_HK_start__), 1) == false) return false;

    // Write once.
    if (FlashMemWrite(((uint32_t)&__region_HK_start__) + 300, 4, &u32TestValue, &bWriteSuccess) == false) return false;
    if (bWriteSuccess == false) return false;

    // Write twice.
    if (FlashMemWrite(((uint32_t)&__region_HK_start__) + 300, 4, &u32TestValue, &bWriteSuccess) == false) return false;
    if (bWriteSuccess == true) return false;

    // Clear memory after test.
    if (FlashMemErasePage(((uint32_t)&__region_HK_start__), 1) == false) return false;

    return true;
}

/**
 * @brief       FlashMem_UT_OutOfBound()
 * @details     Execute the unit test for: validating all input parameters.
 * @public
 * @return      true if success, false otherwise.
 */
bool FlashMem_UT_OutOfBound(void)
{
    bool bIsEmpty 		= false;
    bool bWriteSuccess	= false;

    // Test low address.
    if (FlashMemRead(FLASHMEM_START_ADDR - 1, 10, u8FlashMemUTBuffer) == true) return false;
    if (FlashMemReadFullPage(FLASHMEM_START_ADDR - 1, u8FlashMemUTBuffer, sizeof(u8FlashMemUTBuffer)) == true) return false;
    if (FlashMemWrite(FLASHMEM_START_ADDR - 1, 10, (const uint32_t*) u8FlashMemUTBuffer, &bWriteSuccess) == true) return false;
    if (bWriteSuccess == true) return false;
    if (FlashMemWriteFullPage(FLASHMEM_START_ADDR - 1, (const uint32_t*) u8FlashMemUTBuffer, &bWriteSuccess) == true) return false;
    if (bWriteSuccess == true) return false;
    if (FlashMemErasePage(FLASHMEM_START_ADDR - 1, 1) == true) return false;

    bIsEmpty = true;
    if (FlashMemIsEmpty(FLASHMEM_START_ADDR - 1, 10, &bIsEmpty) == false) return false;
    if (bIsEmpty == true) return false;

    // Test high address.
    if (FlashMemRead(FLASHMEM_END_ADDR + 1, 10, u8FlashMemUTBuffer) == true) return false;
    if (FlashMemReadFullPage(FLASHMEM_END_ADDR + 1, u8FlashMemUTBuffer, sizeof(u8FlashMemUTBuffer)) == true) return false;
    if (FlashMemWrite(FLASHMEM_END_ADDR + 1, 10, (const uint32_t*) u8FlashMemUTBuffer, &bWriteSuccess) == true) return false;
    if (bWriteSuccess == true) return false;
    if (FlashMemWriteFullPage(FLASHMEM_END_ADDR + 1, (const uint32_t*) u8FlashMemUTBuffer, &bWriteSuccess) == true) return false;
    if (bWriteSuccess == true) return false;
    if (FlashMemErasePage(FLASHMEM_END_ADDR + 1, 1) == true) return false;

    // Test unaligned write address.
    if (FlashMemWrite(((uint32_t)&__region_FS_start__) + 11, 10, (const uint32_t*) u8FlashMemUTBuffer, &bWriteSuccess) == false) return false;
    if (bWriteSuccess == true) return false;

    bIsEmpty = true;
    if (FlashMemIsEmpty(FLASHMEM_END_ADDR + 1, 10, &bIsEmpty) == false) return false;
    if (bIsEmpty == true) return false;

    // Test overflow address.
    if (FlashMemRead(FLASHMEM_END_ADDR - 9, 10, u8FlashMemUTBuffer) == true) return false;
    if (FlashMemWrite(FLASHMEM_END_ADDR - 9, 10, (const uint32_t*) u8FlashMemUTBuffer, &bWriteSuccess) == true) return false;
    if (bWriteSuccess == true) return false;

    bIsEmpty = true;
    if (FlashMemIsEmpty(FLASHMEM_END_ADDR - 9, 10, &bIsEmpty) == false) return false;
    if (bIsEmpty == true) return false;

    // Test buffer size.
    if (FlashMemReadFullPage(FLASHMEM_START_ADDR, u8FlashMemUTBuffer, sizeof(u8FlashMemUTBuffer) - 1) == true) return false;

    // Test NULL pointers.
    if (FlashMemRead(FLASHMEM_START_ADDR, 10, NULL) == true) return false;
    if (FlashMemReadFullPage(FLASHMEM_START_ADDR, NULL, sizeof(u8FlashMemUTBuffer)) == true) return false;
    if (FlashMemWrite(FLASHMEM_START_ADDR, 10, NULL, &bWriteSuccess) == true) return false;
    if (bWriteSuccess == true) return false;
    if (FlashMemWrite(FLASHMEM_START_ADDR, 10, (const uint32_t*) u8FlashMemUTBuffer, NULL) == true) return false;
    if (FlashMemWriteFullPage(FLASHMEM_START_ADDR, NULL, &bWriteSuccess) == true) return false;
    if (bWriteSuccess == true) return false;
    if (FlashMemWriteFullPage(FLASHMEM_START_ADDR, (const uint32_t*) u8FlashMemUTBuffer, NULL) == true) return false;
    if (FlashMemIsEmpty(FLASHMEM_START_ADDR - 1, 10, NULL) == true) return false;

    return true;
}

#endif

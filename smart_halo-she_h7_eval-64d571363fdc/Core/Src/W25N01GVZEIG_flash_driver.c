/**
 * @file W25N01GVZEIG_flash_driver.c
 * @author Felix Cormier
 * @date May 13 2021
 * @brief Driver for the Winbond W25N01GVZEIG NAND flash chip.
 *
 * Flash Chip Datasheet:
 * https://www.winbond.com/resource-files/w25n01gv%20revl%20050918%20unsecured.pdf
 *
 * To use the chip, first run the extFlashInit() function.
 * Writing is done in 2 parts. Write a page to the chip's buffer using
 * extFlashQuadLoadProgramDataRandom(), then let the chip write it into memory
 * by using extFlashProgramExecute().
 * Reading is also done in two parts. Read from memory to the chip's buffer by
 * using extFlashPageDataRead(), then read the data over QPSI using
 * extFlashFastReadQuadOutput().
 *
 * The chip also supports ECC error correction and bad block management. See
 * datasheet for details.
 */

#ifdef __cplusplus
extern "C"{
#endif

#include "W25N01GVZEIG_flash_driver.h"
#include <assert.h>
#include <stdio.h>

/* Static function declarations */
static void PrepRegularCmdTypeDef(OSPI_RegularCmdTypeDef *cmd, uint8_t instruction, uint8_t n_dummies);
static HAL_StatusTypeDef extFlashWriteStatusReg(OSPI_HandleTypeDef *hospi, StatusRegAddress adr, uint32_t val);
static HAL_StatusTypeDef extFlashWriteEnable(OSPI_HandleTypeDef *hospi);
//static HAL_StatusTypeDef extFlashReadStatusReg(OSPI_HandleTypeDef *hospi, StatusRegAddress adr, uint8_t *p_val);
//static HAL_StatusTypeDef extFlashWriteDisable(OSPI_HandleTypeDef *hospi);
//static HAL_StatusTypeDef extFlashManageBadBlock();
//static HAL_StatusTypeDef extFlashReadBbmLut();

/**
 * @brief Initialize flash chip by writing to status registers.
 * @param hospi an octoSPI handler.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashInit(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Enable Writing */
  status |= extFlashWriteEnable(hospi);

  /* Write to Status Register 1: Protection */
  status |= extFlashWriteStatusReg(hospi, REG_PROTECTION_ADR, 0x00);
  
  /* Write to Status Register 2: Configuration */
  status |= extFlashWriteStatusReg(hospi, REG_CONFIG_ADR, 0x00);

  return status;
}

/**
 * @brief Reset flash chip.
 * @param hospi an octoSPI handler.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashReset(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;
  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, DEVICE_RESET, 0);

  uint8_t data = 0;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  status |= HAL_OSPI_Transmit(hospi, &data, OSPI_TIMEOUT);

  return status;
}

/**
 * @brief Get JEDEC ID from flash chip.
 * @param hospi an octoSPI handler.
 * @param id a pointer to an empty JedecID where the ID will be written.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashGetID(OSPI_HandleTypeDef *hospi, JedecID *id)
{
  HAL_StatusTypeDef status = HAL_OK;

  // Make an array and set it up as the ospi data buffer
  uint8_t jedec_id[3];

  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, JEDEC_ID, 8);

  cmd.NbData = 3;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  status |= HAL_OSPI_Receive(hospi, &jedec_id[0], OSPI_TIMEOUT);

  id->mfr_id = jedec_id[0];
  id->device_id = ((uint16_t) jedec_id[1]) << 8 | jedec_id[2];

  return status;
}

/**
 * @brief Program data into the memory buffer.
 *
 * Loads data 1 to 2,112 bytes at previously erased memory locations. Does not
 * clear the unused bytes in the data buffer. Requires a "Program Execute" to
 * then load from buffer to memory.
 * @param hospi an octoSPI handler.
 * @param p_data a pointer to the data to be written.
 * @param data_size the amount of bytes to be written.
 * @param column_adr the address of the column to start writing.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashQuadLoadProgramDataRandom(OSPI_HandleTypeDef *hospi, uint8_t* p_data, uint16_t data_size, uint16_t column_adr)
{
  HAL_StatusTypeDef status = HAL_OK;
  assert( p_data != NULL );
  assert( column_adr <= (N_COLUMNS - 1) );
  assert( data_size <= (N_COLUMNS - column_adr) );

  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, QUAD_LOAD_PROG_DATA_RANDOM, 0);

  cmd.Address = column_adr;
  cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  cmd.DataMode = HAL_OSPI_DATA_4_LINES;
  cmd.NbData = data_size;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  status |= HAL_OSPI_Transmit(hospi, p_data, OSPI_TIMEOUT);

  return status;
}

/**
 * @brief Program the data from the chip's buffer to the memory.
 *
 * Must be done after any "Load Program" instruction.
 * @param hospi an octoSPI handler.
 * @param page_adr the 16-bit address of the page to write to.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashProgramExecute(OSPI_HandleTypeDef *hospi, uint16_t page_adr)
{
  HAL_StatusTypeDef status = HAL_OK;
  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, PROGRAM_EXECUTE, 8);

  // Send 16-bit column address as data, since it comes after the dummy cycles
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.NbData = 2;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  status |= HAL_OSPI_Transmit(hospi, (uint8_t *) &page_adr, OSPI_TIMEOUT);

  return status;
}

/**
 * @brief Erase a whole block of memory
 *
 * Returns all 128kB in that block to their default 0xFF value.
 * @param hospi an octoSPI handler.
 * @param block_adr the 10-bit block address.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashBlockErase128kB(OSPI_HandleTypeDef *hospi, uint16_t block_adr)
{
  HAL_StatusTypeDef status = HAL_OK;
  assert( block_adr <= (N_BLOCKS - 1) );
  /* Erase a whole block of memory to 0xFF */
  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, BLOCK_ERASE_128KB, 8);

  // Send 16-bit column address as data, since it comes after the dummy cycles
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.NbData = 2;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  status |= HAL_OSPI_Transmit(hospi, (uint8_t *) &block_adr, OSPI_TIMEOUT);

  return status;
}

/**
 * @brief Read a whole page into the chip's buffer
 *
 * @param hospi an octoSPI handler.
 * @param page_adr the 16-bit page address.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashPageDataRead(OSPI_HandleTypeDef *hospi, uint16_t page_adr)
{
  HAL_StatusTypeDef status = HAL_OK;
  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, PAGE_DATA_READ, 8);

  // Send 16-bit column address as data, since it comes after the dummy cycles
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.NbData = 2;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  status |= HAL_OSPI_Transmit(hospi, (uint8_t *) &page_adr, OSPI_TIMEOUT);

  return status;
}

/**
 * @brief Reads from the buffer using all 4 QPSI lines
 *
 * @param hospi an octoSPI handler.
 * @param p_data a pointer to an array in which to write the data.
 * @param data_size the amount of bytes to be read.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef extFlashFastReadQuadOutput(OSPI_HandleTypeDef *hospi, uint8_t *p_data, uint32_t data_size)
{
  HAL_StatusTypeDef status = HAL_OK;
  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, FAST_READ_QUAD_OUTPUT, 32);

  // Send 16-bit column address as data, since it comes after the dummy cycles
  cmd.DataMode = HAL_OSPI_DATA_4_LINES;
  cmd.NbData = data_size;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  status |= HAL_OSPI_Transmit(hospi, p_data, OSPI_TIMEOUT);

  return status;
}


//**************************************************************************//
//                           Static Functions
//**************************************************************************//

/**
 * @brief Prepares an octoSPI command
 *
 * Prepares a command with a single 8-bit instruction and some dummy clocks if
 * desired. The rest of the message like address, alternate bytes and data
 * can be added on later.
 * @param cmd a pointer to an OSPI_RegularCmdTypeDef command structure.
 * @param instruction the 8-bit instruction.
 * @param n_dummies the amount of dummy clocks to insert before the data phase.
 * @return void
 */
static void PrepRegularCmdTypeDef(OSPI_RegularCmdTypeDef *cmd, uint8_t instruction, uint8_t n_dummies)
{
  // Makes a simple command with just the instruction and dummy clock cycles.

  cmd->OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd->FlashId = HAL_OSPI_FLASH_ID_1;
  cmd->Instruction = instruction;
  cmd->InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
  cmd->InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
  cmd->InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  cmd->Address = 0x00;
  cmd->AddressMode = HAL_OSPI_ADDRESS_NONE;
  cmd->AddressSize = HAL_OSPI_ADDRESS_16_BITS;
  cmd->AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
  cmd->AlternateBytes = 0x00;
  cmd->AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd->AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_8_BITS;
  cmd->AlternateBytesDtrMode = HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE;
  cmd->DataMode = HAL_OSPI_DATA_NONE;
  cmd->NbData = 0;
  cmd->DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
  cmd->DummyCycles = n_dummies;
  cmd->DQSMode = HAL_OSPI_DQS_DISABLE;
  cmd->SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
}

/**
 * @brief Reads a specific status register
 *
 * @param hospi an octoSPI handler.
 * @param adr the address of the status register.
 * @param p_val a pointer to a uint8_t where the register data should be written.
 * @return the status via a HAL_StatusTypeDef.
 */
/*static HAL_StatusTypeDef extFlashReadStatusReg(OSPI_HandleTypeDef *hospi, StatusRegAddress adr, uint8_t *p_val)
{
  // Make an array and set it up as the ospi data buffer
  hospi->pBuffPtr = p_val;
  hospi->XferSize = 1;
  hospi->State = HAL_OSPI_STATE_READ_CMD_CFG;

  OSPI_RegularCmdTypeDef cmd;

  PrepRegularCmdTypeDef(&cmd, READ_STATUS_REG, 0);
  
  cmd.Address = adr;
  cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  cmd.AddressSize = HAL_OSPI_ADDRESS_8_BITS;
  cmd.NbData = 1;
  cmd.DataMode = HAL_OSPI_DATA_1_LINE;

  return HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
}*/

/**
 * @brief Writes to a specific status register
 *
 * @param hospi an octoSPI handler.
 * @param adr the address of the status register.
 * @param val the value to be written to the register.
 * @return the status via a HAL_StatusTypeDef.
 */
static HAL_StatusTypeDef extFlashWriteStatusReg(OSPI_HandleTypeDef *hospi, StatusRegAddress adr, uint32_t val)
{
  /* Write to one of the 3 status registers*/
  HAL_StatusTypeDef status = HAL_OK;
  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, WRITE_STATUS_REG, 0);

  cmd.Address = adr;
  cmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  cmd.AddressSize = HAL_OSPI_ADDRESS_8_BITS;
  // Disable block protect, disable status reg protect, disable write protect
  cmd.AlternateBytes = val;
  cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_1_LINE;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  return status;
}

/**
 * @brief Enables writing to the memory chip
 *
 * @param hospi an octoSPI handler.
 * @return the status via a HAL_StatusTypeDef.
 */
static HAL_StatusTypeDef extFlashWriteEnable(OSPI_HandleTypeDef *hospi)
{
  /* Send Write Enable instruction */
  HAL_StatusTypeDef status = HAL_OK;

  uint8_t data = 0xAA;

  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, WRITE_ENABLE, 0);
  printf("Made command...");

  cmd.DataMode = HAL_OSPI_DATA_1_LINE;
  cmd.NbData = 1;

  status |= HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
  printf("Prepped command... Status: %i", status);
  status |= HAL_OSPI_Transmit(hospi, &data, OSPI_TIMEOUT);
  printf("Transmitted command... Status: %i", status);
  return status;
}

/**
 * @brief Disables writing to the memory chip
 *
 * @param hospi an octoSPI handler.
 * @return the status via a HAL_StatusTypeDef.
 */
/*static HAL_StatusTypeDef extFlashWriteDisable(OSPI_HandleTypeDef *hospi)
{
  OSPI_RegularCmdTypeDef cmd;
  PrepRegularCmdTypeDef(&cmd, WRITE_DISABLE, 0);

  return  HAL_OSPI_Command(hospi, &cmd, OSPI_TIMEOUT);
}*/

/**
 * @brief Manage bad memory blocks
 *
 * Re-maps a block of memory from a physical address to a different "logical"
 * address so that all good blocks can be read continuously.
 */
/*static HAL_StatusTypeDef extFlashManageBadBlock()
{
  HAL_StatusTypeDef status = HAL_OK;
  // TODO
  return status;
}*/

/**
 * @brief Read the Bad Block Management (BBM) Look-Up Table (LUT)
 *
 * Reads the Bad Block Management Look-Up Table to determine which blocks
 * are re-mapped.
 */
/*static HAL_StatusTypeDef extFlashReadBbmLut()
{
  HAL_StatusTypeDef status = HAL_OK;
  // TODO
  return status;
}*/

#ifdef __cplusplus
}
#endif

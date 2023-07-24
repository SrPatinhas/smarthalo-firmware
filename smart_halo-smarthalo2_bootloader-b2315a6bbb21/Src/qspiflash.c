#include "qspiflash.h"
#include "quadspi.h"
#include "log.h"

//spiflash_cmd_tbl_t my_spiflash_cmds = {
//		.write_disable = 0x04,
//		.write_enable = 0x06,
//		.page_program = 0x02,
//		.read_data = 0x03,
//		.read_data_fast = 0x0b,
//		.write_sr = 0x01,
//		.read_sr = 0x05,
//		.block_erase_4 = 0x20,
//		.block_erase_8 = 0x00,
//		.block_erase_16 = 0x00,
//		.block_erase_32 = 0x52,
//		.block_erase_64 = 0xd8,
//		.chip_erase = 0xc7,
//		.device_id = 0x90,
//		.jedec_id = 0x9f,
//		.sr_busy_bit = 0x01,
//		.deep_sleep = 0xB9,
//		.wake = 0xAB};

QSPI_CommandTypeDef sCommand;
QSPI_AutoPollingTypeDef sConfig;
volatile uint8_t CmdCplt, RxCplt, TxCplt, StatusMatch, TimeOut;

/**
 * @brief  Command completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    CmdCplt++;
}

/**
 * @brief  Rx Transfer completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    RxCplt++;
}

/**
 * @brief  Tx Transfer completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    TxCplt++;
}

/**
 * @brief  Status Match callbacks
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi)
{
    StatusMatch++;
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Flash_Error_Handler(int res)
{
//	   HAL_OK       = 0x00,
//	  HAL_ERROR    = 0x01,
//	  HAL_BUSY     = 0x02,
//	  HAL_TIMEOUT  = 0x03
    LOG("Flash_Error_Handler: res: 0x%X\n", res);

    /* User may add here some code to deal with this error */
    // while (1) {
    // }
}

/**
 * @brief  This function send a Write Enable and wait it is effective.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
    int res;

    /* Enable write operations ------------------------------------------ */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = WRITE_ENABLE_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if ((res = HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sConfig.Match = 0x02;
    sConfig.Mask = 0x02;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval = 0x10;
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    sCommand.Instruction = READ_STATUS_REG_CMD;
    sCommand.DataMode = QSPI_DATA_1_LINE;

    if ((res = HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
}

/**
 * @brief  This function read the SR of the memory and wait the EOP.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static void QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi)
{
    int res;
    /* Configure automatic polling mode to wait for memory ready ------ */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = READ_STATUS_REG_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    sConfig.Match = 0x00;
    sConfig.Mask = 0x01;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval = 0x10;
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    if ((res = HAL_QSPI_AutoPolling_IT(hqspi, &sCommand, &sConfig)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
}

/**
 * @brief  This function configure the dummy cycles on memory side.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static void QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
    uint8_t reg;
    int res;

    /* Read Volatile Configuration register --------------------------- */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = READ_VOL_CFG_REG_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.NbData = 1;

    if ((res = HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    if ((res = HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    /* Enable write operations ---------------------------------------- */
    QSPI_WriteEnable(hqspi);

    /* Write Volatile Configuration register (with new dummy cycles) -- */
    sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
    MODIFY_REG(reg, 0xF0, (DUMMY_CLOCK_CYCLES_READ_QUAD << POSITION_VAL(0xF0)));

    if ((res = HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    if ((res = HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
}

/**
 * @brief  This function reset the QSPI memory.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static void QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi)
{
    int res;

    /* Initialize the reset enable command */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = RESET_ENABLE_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    if ((res = HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    /* Send the reset memory command */
    sCommand.Instruction = RESET_MEMORY_CMD;
    if ((res = HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    /* Configure automatic polling mode to wait the memory is ready */
    StatusMatch = 0;

    /* Configure automatic polling mode to wait for end of program ----- */
    QSPI_AutoPollingMemReady(hqspi);
    while (!StatusMatch)
        ;
    return;
}

void SPIFLASH_init()
{

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    QSPI_ResetMemory(&hqspi);

    /* Configure Volatile Configuration register (with new dummy cycles) */
    QSPI_DummyCyclesCfg(&hqspi);
}

/*spiffs_read and spiffs_write will never be called with parameters that break the logical page boundary*/
int SPIFLASH_write(uint32_t addr, uint32_t len, uint8_t *buf)
{
    int res;
    /* Enable write operations ----------------------------------------- */
    QSPI_WriteEnable(&hqspi);

    /* Writing Sequence ------------------------------------------------ */
    sCommand.Instruction = QUAD_IN_FAST_PROG_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.DataMode = QSPI_DATA_4_LINES;
    sCommand.NbData = len;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Address = addr;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
//	TxCplt = 0;
//	if ((res = HAL_QSPI_Transmit_DMA(&hqspi, buf)) != HAL_OK) {
    if ((res = HAL_QSPI_Transmit(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
//	while (!TxCplt)
//		;
    StatusMatch = 0;

    /* Configure automatic polling mode to wait for end of program ----- */
    QSPI_AutoPollingMemReady(&hqspi);
    while (!StatusMatch)
        ;
//	console_send(console_LOG, "Write 0x%X %X %X %X, len: %d\n", buf[0], buf[1], buf[2], buf[3], len);

    return HAL_OK;
}
/*spiffs_read and spiffs_write will never be called with parameters that break the logical page boundary*/
int SPIFLASH_read(uint32_t addr, uint32_t len, uint8_t *buf)
{
    int res;

    /* Reading Sequence ------------------------------------------------ */
    sCommand.Instruction = QUAD_OUT_FAST_READ_CMD;
    sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ_QUAD;
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.DataMode = QSPI_DATA_4_LINES;
    sCommand.NbData = len;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ_QUAD;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Address = addr;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
    RxCplt = 0;
//	if ((res = HAL_QSPI_Receive_DMA(&hqspi, buf)) != HAL_OK) {
    if ((res = HAL_QSPI_Receive(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
//	while (!RxCplt)
//		;
//	console_send(console_LOG, "Read 0x%X %X %X %X, len: %d\n", buf[0], buf[1], buf[2], buf[3], len);

    return HAL_OK;
}

int SPIFLASH_read_jedec_id(uint32_t *jedec_id)
{
    int res;
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = READ_ID_CMD2;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.NbData = 4;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
    RxCplt = 0;
    if ((res = HAL_QSPI_Receive(&hqspi, (uint8_t *) jedec_id, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
//	if ((res = HAL_QSPI_Receive_DMA(&hqspi, (uint8_t *) jedec_id)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
//	while (!RxCplt)
//		;
    return HAL_OK;
}

int SPIFLASH_erase_SUBSECTOR(uint32_t addr)
{
    int res;
    /* Enable write operations ------------------------------------------- */
    QSPI_WriteEnable(&hqspi);

    /* Erasing Sequence -------------------------------------------------- */
    sCommand.Instruction = SUBSECTOR_ERASE_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = addr;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
    StatusMatch = 0;

    /* Configure automatic polling mode to wait for end of erase ------- */
    QSPI_AutoPollingMemReady(&hqspi);
    while (!StatusMatch)
        ;
    return HAL_OK;
}

int SPIFLASH_erase_SECTOR(uint32_t addr)
{
    int res;
    /* Enable write operations ------------------------------------------- */
    QSPI_WriteEnable(&hqspi);

    /* Erasing Sequence -------------------------------------------------- */
    sCommand.Instruction = SECTOR_ERASE_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.Address = addr;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
    StatusMatch = 0;

    /* Configure automatic polling mode to wait for end of erase ------- */
    QSPI_AutoPollingMemReady(&hqspi);
    while (!StatusMatch)
        ;
    return HAL_OK;
}
int SPIFLASH_chip_erase()
{
    int res;
    /* Enable write operations ------------------------------------------- */
    QSPI_WriteEnable(&hqspi);

    /* Erasing Sequence -------------------------------------------------- */
    sCommand.Instruction = BULK_ERASE_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;

    CmdCplt = 0;
    if ((res = HAL_QSPI_Command_IT(&hqspi, &sCommand)) != HAL_OK) {
        Flash_Error_Handler(res);
    }
    while (!CmdCplt)
        ;
    StatusMatch = 0;

    /* Configure automatic polling mode to wait for end of erase ------- */
    QSPI_AutoPollingMemReady(&hqspi);
    while (!StatusMatch)
        ;
    return HAL_OK;
}

int SPIFLASH_deep_sleep()
{
    int res;

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = QSPI_DEEP_SLEEP;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    return HAL_OK;
}

int SPIFLASH_wake()
{
    int res;

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = QSPI_WAKEUP;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(res);
    }

    return HAL_OK;
}


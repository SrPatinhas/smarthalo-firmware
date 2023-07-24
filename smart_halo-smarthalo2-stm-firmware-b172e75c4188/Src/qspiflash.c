#include <SystemUtilitiesTask.h>
#include "qspiflash.h"
#include "quadspi.h"
#include "wwdg.h"
#include "reboot.h"

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
volatile uint8_t CmdCplt, TxCplt, StatusMatch, TimeOut;

static SemaphoreHandle_t qspiMutex = NULL;
static StaticSemaphore_t qspiMutexBuffer;

#define RETRY_MAX   4

/**
 * @brief  Command completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi) {
    CmdCplt++;
}

/**
 * @brief  Tx Transfer completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi) {
    TxCplt++;
}

/**
 * @brief  Status Match callbacks
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi) {
    StatusMatch++;
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static HAL_StatusTypeDef Flash_Error_Handler(const char *func, int line,
                                             int res)
{
    //  HAL_OK       = 0x00,
    //  HAL_ERROR    = 0x01,
    //  HAL_BUSY     = 0x02,
    //  HAL_TIMEOUT  = 0x03
    log_Shell(
        "Flash oups from task: %s\n"
        "%s:%d, code: 0x%02x",
        pcTaskGetName(xTaskGetCurrentTaskHandle()), func, line, res);
    log_Shell("hqspi->State: 0x%02x, hqspi->ErrorCode: 0x%lx", hqspi.State,
              hqspi.ErrorCode);

    switch (res) {
    case HAL_OK:
        log_Shell("HAL_OK? what are we doing here?");
        break;
    case HAL_ERROR:
        log_Shell("HAL_ERROR -- cannot recover");
        goto hang;
    case HAL_BUSY:
        log_Shell("HAL_BUSY -- wait and try again");
        break;
    case HAL_TIMEOUT:
        log_Shell("HAL_TIMEOUT -- we waited a long time");
        goto hang;
    default:
        log_Shell("res (%d) was something we have never seen before", res);
        goto hang;
    }

    return res;

hang:
    /* User may add here some code to deal with this error */
#ifdef MEGABUG_HUNT
    while (1);
#else
    SOFT_CRASH(eQSPIFLASH);
    __builtin_unreachable();
#endif
}

/**
 * @brief  This function send a Write Enable and wait it is effective.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static HAL_StatusTypeDef QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
    int res;

    /* Enable write operations ------------------------------------------ */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = WRITE_ENABLE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if ((res = HAL_QSPI_Command(hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    /* Configure automatic polling mode to wait for write enabling ---- */
    sConfig.Match           = 0x02;
    sConfig.Mask            = 0x02;
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    sCommand.Instruction = READ_STATUS_REG_CMD;
    sCommand.DataMode    = QSPI_DATA_1_LINE;

    if ((res = HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig,
                                    HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) !=
        HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    return HAL_OK;
}

/**
 * @brief  This function read the SR of the memory and wait the EOP.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static HAL_StatusTypeDef QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi)
{
    int res;
    /* Configure automatic polling mode to wait for memory ready ------ */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_STATUS_REG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    sConfig.Match           = 0x00;
    sConfig.Mask            = 0x01;
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    if ((res = HAL_QSPI_AutoPolling_IT(hqspi, &sCommand, &sConfig)) != HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    return HAL_OK;
}

/**
 * @brief  This function configure the dummy cycles on memory side.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static HAL_StatusTypeDef QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
    uint8_t reg;
    int     res;

    /* Read Volatile Configuration register --------------------------- */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.NbData            = 1;

    if ((res = HAL_QSPI_Command(hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    if ((res = HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) !=
        HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    /* Enable write operations ---------------------------------------- */
    if ((res = QSPI_WriteEnable(hqspi)) != HAL_OK) {
        log_Shell("%s: write enabled failed with 0x%02x", __func__, res);
        return res;
    }

    /* Write Volatile Configuration register (with new dummy cycles) -- */
    sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
    MODIFY_REG(reg, 0xF0, (DUMMY_CLOCK_CYCLES_READ_QUAD << POSITION_VAL(0xF0)));

    if ((res = HAL_QSPI_Command(hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    if ((res = HAL_QSPI_Transmit(hqspi, &reg,
                                 HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    return HAL_OK;
}

/**
 * @brief  This function reset the QSPI memory.
 * @param  hqspi: QSPI handle
 * @retval None
 */
static HAL_StatusTypeDef QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi)
{
    int res;

    /* Initialize the reset enable command */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = RESET_ENABLE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    if ((res = HAL_QSPI_Command(hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    /* Send the reset memory command */
    sCommand.Instruction = RESET_MEMORY_CMD;
    if ((res = HAL_QSPI_Command(hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        return Flash_Error_Handler(__func__, __LINE__, res);
    }

    /* Configure automatic polling mode to wait the memory is ready */
    StatusMatch = 0;

    /* Configure automatic polling mode to wait for end of program ----- */
    if ((res = QSPI_AutoPollingMemReady(hqspi)) != HAL_OK) {
        log_Shell("%s:%d QSPI_AutoPollingMemReady returns %d", __func__,
                  __LINE__, res);
        return res;
    }
    while (!StatusMatch)
        HAL_WWDG_Refresh(&hwwdg);
    return HAL_OK;
}

static void SPIFLASH_LOCK(const char *caller)
{
    if (qspiMutex == NULL) {
        log_Shell("%s: mutex not initialized", __func__);
        return;
    }

    if (xSemaphoreTake(qspiMutex, 1000) == pdFALSE) {
        log_Shell("%s: mutex timed out, caller: %s", __func__, caller);
    }
    HAL_WWDG_Refresh(&hwwdg);
}

static void SPIFLASH_UNLOCK()
{
    if (qspiMutex == NULL) {
        log_Shell("%s: mutex not initialized", __func__);
        return;
    }
    xSemaphoreGive(qspiMutex);
    HAL_WWDG_Refresh(&hwwdg);
}

void SPIFLASH_init()
{
    if (qspiMutex == NULL) {
        qspiMutex = xSemaphoreCreateMutexStatic(&qspiMutexBuffer);
        configASSERT(qspiMutex);
        xSemaphoreGive(qspiMutex);
    }
    // log_Shell("%s: mutex is ready", __func__);

    SPIFLASH_LOCK(__func__);

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    QSPI_ResetMemory(&hqspi);

    // Configure Volatile Configuration register (with new dummy cycles)
    QSPI_DummyCyclesCfg(&hqspi);

    SPIFLASH_UNLOCK();
}

/*! @brief  write function used mainly by SPIFFS

            Addr must be on page boundaries

    @param[in]  addr    offset into the flash device
    @param[in]  len     length of write
    @param[in]  buf     pointer to data to be writen
    @return     HAL_OK on success
*/
int SPIFLASH_write(uint32_t addr, uint32_t len, uint8_t *buf)
{
    int res = 0xff;

    // log_Shell("%s: addr: %lx, len: %lu, buf: %p", __func__, addr, len, buf);

    SPIFLASH_LOCK(__func__);

    // Enable write operations
    for (int i = 0; res != HAL_OK && i < RETRY_MAX; i++) {
        res = QSPI_WriteEnable(&hqspi);
        if (res == HAL_ERROR) break;
    }
    if (res != HAL_OK) {
        log_Shell("%s: QSPI_WriteEnable returned 0x%02x", __func__, res);
        goto done;
    }

    // Write sequence
    sCommand.Instruction       = QUAD_IN_FAST_PROG_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = len;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Address           = addr;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
    if ((res = HAL_QSPI_Transmit(&hqspi, buf,
                                 HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
    StatusMatch = 0;

    // Configure automatic polling mode to wait for end of operation
    if ((res = QSPI_AutoPollingMemReady(&hqspi)) != HAL_OK) {
        goto done;
    }
    while (!StatusMatch)
        HAL_WWDG_Refresh(&hwwdg);

done:
    SPIFLASH_UNLOCK();
    return res;
}

/*! @brief  read function used mainly by SPIFFS

            Addr must be on page boundaries

    @param[in]  addr    offset into the flash device
    @param[in]  len     length of read
    @param[out] buf     pointer to buffer to read into
    @return     HAL_OK on success
*/
int SPIFLASH_read(uint32_t addr, uint32_t len, uint8_t *buf)
{
    int res;

    // log_Shell("%s: addr: %lx, len: %lu, buf: %p", __func__, addr, len, buf);

    SPIFLASH_LOCK(__func__);

#if 0
    // Inject an error to see what happens above
    static int count;
    count++;
    if (count > 1000 && (count % 16) == 0) {
        res = HAL_BUSY;
        log_Shell("%s: Injecting error\n", __func__);
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
#endif

    // Read sequence
    sCommand.Instruction       = QUAD_OUT_FAST_READ_CMD;
    sCommand.DummyCycles       = DUMMY_CLOCK_CYCLES_READ_QUAD;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = len;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DummyCycles       = DUMMY_CLOCK_CYCLES_READ_QUAD;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Address           = addr;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
    if ((res = HAL_QSPI_Receive(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) !=
        HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }

done:
    SPIFLASH_UNLOCK();
    HAL_WWDG_Refresh(&hwwdg);
    return res;
}

int SPIFLASH_read_jedec_id(uint32_t *jedec_id)
{
    int res;

    SPIFLASH_LOCK(__func__);

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
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
    if ((res = HAL_QSPI_Receive(&hqspi, (uint8_t *) jedec_id, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }

done:
    SPIFLASH_UNLOCK();    
    return res;
}

int SPIFLASH_erase_SUBSECTOR(uint32_t addr)
{
    int res = 0xff;

    // log_Shell("%s: addr: %lx", __func__, addr);

    SPIFLASH_LOCK(__func__);

    // Enable write operations
    for (int i = 0; res != HAL_OK && i < RETRY_MAX; i++) {
        res = QSPI_WriteEnable(&hqspi);
        if (res == HAL_ERROR) break;
    }
    if (res != HAL_OK) {
        log_Shell("%s: QSPI_WriteEnable returned 0x%02x", __func__, res);
        goto done;
    }

    // Erasing Sequence
    sCommand.Instruction       = SUBSECTOR_ERASE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = addr;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
    StatusMatch = 0;

    // Configure automatic polling mode to wait for end of erase
    if ((res = QSPI_AutoPollingMemReady(&hqspi)) != HAL_OK) {
        log_Shell("%s:%d QSPI_AutoPollingMemReady returns %d", __func__,
                  __LINE__, res);
        goto done;
    }
    while (!StatusMatch)
        HAL_WWDG_Refresh(&hwwdg);

done:
    SPIFLASH_UNLOCK();
    return res;
}

int SPIFLASH_erase_SECTOR(uint32_t addr)
{
    int res = 0xff;

    // log_Shell("%s: addr: %lx", __func__, addr);

    SPIFLASH_LOCK(__func__);

    // Enable write operations
    for (int i = 0; res != HAL_OK && i < RETRY_MAX; i++) {
        res = QSPI_WriteEnable(&hqspi);
        if (res == HAL_ERROR) break;
    }
    if (res != HAL_OK) {
        log_Shell("%s: QSPI_WriteEnable returned 0x%02x", __func__, res);
        goto done;
    }

    // Erasing Sequence
    sCommand.Instruction      = SECTOR_ERASE_CMD;
    sCommand.AddressMode      = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize      = QSPI_ADDRESS_24_BITS;
    sCommand.Address          = addr;
    sCommand.DataMode         = QSPI_DATA_NONE;
    sCommand.DummyCycles      = 0;
    sCommand.DdrMode          = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;

    if ((res = HAL_QSPI_Command(&hqspi, &sCommand,
                                HAL_QPSI_TIMEOUT_DEFAULT_VALUE)) != HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
    StatusMatch = 0;

    // Configure automatic polling mode to wait for end of erase
    if ((res = QSPI_AutoPollingMemReady(&hqspi)) != HAL_OK) {
        log_Shell("%s:%d QSPI_AutoPollingMemReady returns %d", __func__,
                  __LINE__, res);
        goto done;
    }
    while (!StatusMatch)
        HAL_WWDG_Refresh(&hwwdg);

done:
    SPIFLASH_UNLOCK();
    return res;
}

int SPIFLASH_chip_erase()
{
    int res = 0xff;

    SPIFLASH_LOCK(__func__);

    // Enable write operations
    for (int i = 0; res != HAL_OK && i < RETRY_MAX; i++) {
        res = QSPI_WriteEnable(&hqspi);
        if (res == HAL_ERROR) break;
    }
    if (res != HAL_OK) {
        log_Shell("%s: QSPI_WriteEnable returned 0x%02x", __func__, res);
        goto done;
    }

    // Erasing Sequence
    sCommand.Instruction = BULK_ERASE_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.DataMode    = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;

    CmdCplt = 0;
    if ((res = HAL_QSPI_Command_IT(&hqspi, &sCommand)) != HAL_OK) {
        Flash_Error_Handler(__func__, __LINE__, res);
        goto done;
    }
    while (!CmdCplt)
        ;
    StatusMatch = 0;

    // Configure automatic polling mode to wait for end of erase
    if ((res = QSPI_AutoPollingMemReady(&hqspi)) != HAL_OK) {
        log_Shell("%s:%d QSPI_AutoPollingMemReady returns %d", __func__,
                  __LINE__, res);
        goto done;
    }
    while (!StatusMatch)
        HAL_WWDG_Refresh(&hwwdg);

done:
    SPIFLASH_UNLOCK();
    return res;
}

int SPIFLASH_deep_sleep() {
    //int res;
    // TODO: SPIFLASH_deep_sleep
    return HAL_OK;
}

int SPIFLASH_wake() {
    //int res;
    // TODO: SPIFLASH_wake

    return HAL_OK;
}


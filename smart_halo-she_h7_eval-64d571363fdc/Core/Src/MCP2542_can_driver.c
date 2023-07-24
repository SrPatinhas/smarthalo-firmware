/**
 * @file MCP2542_can_driver.c
 * @author Felix Cormier
 * @date May 14 2021
 * @brief Driver for the Microchip MCP2542 FD CAN Transceiver.
 *
 * Transceiver datasheet:
 * https://ww1.microchip.com/downloads/en/DeviceDoc/MCP2542FD-MCP2542WFD-4WFD-Data-Sheet-DS20005514C.pdf
 *
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "MCP2542_can_driver.h"

/* Static function declarations */
static void canDriverBuildTxHeader(FDCAN_TxHeaderTypeDef* tx_header);

/**
 * @brief Initializes the transceiver.
 * @param hfdcan an FD CAN peripheral handler.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef canDriverInit(FDCAN_HandleTypeDef *hfdcan, GPIO_TypeDef *stby_port, uint16_t stby_pin)
{
  // If any other config is desired, do it here. Once the CAN is started,
  // the configuration registers can't be written to.

  // Set the STBY pin low
  HAL_GPIO_WritePin(stby_port, stby_pin, GPIO_PIN_SET);
  return HAL_FDCAN_Start(hfdcan);
}

/**
 * @brief Sends a single test byte (0x55) on the CAN bus.
 * @param hfdcan an FD CAN peripheral handler.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef canDriverSendTestByte(FDCAN_HandleTypeDef *hfdcan)
{
  FDCAN_TxHeaderTypeDef tx_header;
  uint8_t data = 0x55;
  canDriverBuildTxHeader(&tx_header);
  return canDriverSendMsg(hfdcan, &tx_header, &data);
}

/**
 * @brief Sends a message on the CAN bus.
 * @param pTxHeader a pointer to a the desired header for the message.
 * @param pTxData the data to be send. Length is specified in the header.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef canDriverSendMsg(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData)
{
  return HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, pTxHeader, pTxData);
}

/**
 * @brief Receives a message from the CAN bus.
 * @param pRxHeader a pointer to an empty header struct. Will be filled with the received header.
 * @param pRxData a pointer to where the received data should be transfered. Length is in header.
 * @return the status via a HAL_StatusTypeDef.
 */
HAL_StatusTypeDef canDriverReceiveMsg(FDCAN_HandleTypeDef *hfdcan, FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData)
{
  return HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, pRxHeader, pRxData);
}

//**************************************************//
//               Static Functions
//**************************************************//

static void canDriverBuildTxHeader(FDCAN_TxHeaderTypeDef* tx_header)
{
  tx_header->Identifier = 0x7AA;
  tx_header->IdType = FDCAN_STANDARD_ID;
  tx_header->TxFrameType = FDCAN_DATA_FRAME;
  tx_header->DataLength = FDCAN_DLC_BYTES_1;
  tx_header->ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  tx_header->BitRateSwitch = FDCAN_BRS_OFF;
  tx_header->FDFormat = FDCAN_FD_CAN;
  tx_header->TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  tx_header->MessageMarker = 0x01;
}

#ifdef __cplusplus
 }
#endif

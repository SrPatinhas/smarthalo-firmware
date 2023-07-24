/**
 * @file MCP2542_can_driver.h
 * @author Felix Cormier
 * @date May 14 2021
 * @brief Driver for the Microchip MCP2542 FD CAN Transceiver.
 */

#ifndef INC_MCP2542_CAN_DRIVER_H_
#define INC_MCP2542_CAN_DRIVER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"

HAL_StatusTypeDef canDriverInit(FDCAN_HandleTypeDef *hfdcan, GPIO_TypeDef *stby_port, uint16_t stby_pin);
HAL_StatusTypeDef canDriverSendTestByte(FDCAN_HandleTypeDef *hfdcan);
HAL_StatusTypeDef canDriverSendMsg(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData);
HAL_StatusTypeDef canDriverReceiveMsg(FDCAN_HandleTypeDef *hfdcan, FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData);

#ifdef __cplusplus
 }
#endif

#endif /* INC_MCP2542_CAN_DRIVER_H_ */

/**
 * @file charge_ios.c
 * @author Felix Cormier
 * @date May 17 2021
 * @brief Driver for the IOs of the MCP73871 charger IC.
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "charge_ios.h"
#include "main.h"

/**
 * @brief Get the charge status.
 * @return the status as a ChargeStatus typedef.
 */
ChargeStatus chargeIOsGetStatus()
{
  GPIO_PinState stat1 = HAL_GPIO_ReadPin(CHARGE_STAT1_GPIO_Port, CHARGE_STAT1_Pin);
  GPIO_PinState stat2 = HAL_GPIO_ReadPin(CHARGE_STAT2_GPIO_Port, CHARGE_STAT2_Pin);

  if (stat1 == 0 && stat2 == 0 ) { return FAULT; }
  else if (stat1 == 0 && stat2 == 1 ) { return CHARGING; }
  else if (stat1 == 1 && stat2 == 0 ) { return CHARGED; }
  else { return SHUTDOWN; }
}

/**
 * @brief Enable battery charging
 */
void chargeIOsEnableCharging()
{
  HAL_GPIO_WritePin(CHARGE_CE_GPIO_Port, CHARGE_CE_Pin, GPIO_PIN_SET);
}

/**
 * @brief Disable battery charging
 */
void chargeIOsDisableCharging()
{
  HAL_GPIO_WritePin(CHARGE_CE_GPIO_Port, CHARGE_CE_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Enable the charge timer
 */
void chargeIOsEnableTimer()
{
  HAL_GPIO_WritePin(CHARGE_nTE_GPIO_Port, CHARGE_nTE_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Disable the charge timer
 */
void chargeIOsDisableTimer()
{
  HAL_GPIO_WritePin(CHARGE_nTE_GPIO_Port, CHARGE_nTE_Pin, GPIO_PIN_SET);
}

#ifdef __cplusplus
 }
#endif

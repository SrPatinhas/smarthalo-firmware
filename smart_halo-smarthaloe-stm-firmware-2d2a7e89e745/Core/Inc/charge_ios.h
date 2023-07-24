/**
 * @file charge_ios.h
 * @author Felix Cormier
 * @date May 17 2021
 * @brief Driver for the IOs of the MCP73871 charger IC.
 */

#ifndef INC_CHARGE_IOS_H_
#define INC_CHARGE_IOS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"

typedef enum {
  SHUTDOWN,
  CHARGING,
  CHARGED,
  FAULT
} ChargeStatus;

ChargeStatus chargeIOsGetStatus(void);
void chargeIOsEnableCharging(void);
void chargeIOsDisableCharging(void);
void chargeIOsEnableTimer(void);
void chargeIOsDisableTimer(void);

#ifdef __cplusplus
 }
#endif

#endif /* INC_CHARGE_IOS_H_ */
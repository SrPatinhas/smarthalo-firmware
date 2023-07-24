/**
 * @file power_ios.c
 * @author Felix Cormier
 * @date May 17 2021
 * @brief Driver for the IOs of the power supplies.
 */

#ifndef INC_POWER_IOS_H_
#define INC_POWER_IOS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"

#define ADC_TIMEOUT           0xFFFFFFFF
#define ADC_MAX_VALUE         0xFFFF

typedef struct {
  GPIO_TypeDef *enable_port;
  uint16_t enable_pin;
} LdoTypeDef;

typedef enum {
  LDO_NRF,
  LDO_LCD,
  LDO_BUZ
} LdoEnum;

typedef enum {
  LDO_DISABLE,
  LDO_ENABLE
} LdoState;

typedef enum {
  POWER_BAD,
  POWER_GOOD
} PowerState;

void powerIOsToggleLDO(LdoEnum ldo_id, LdoState state);
PowerState powerIOsIsPowerGood(void);
HAL_StatusTypeDef powerIOsGetInputVoltage(ADC_HandleTypeDef *hadc, double *voltage);

#ifdef __cplusplus
 }
#endif

#endif /* INC_POWER_IOS_H_ */

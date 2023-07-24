/**
 * @file power_ios.c
 * @author Felix Cormier
 * @date May 17 2021
 * @brief Driver for the IOs of the power supplies.
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "power_ios.h"
#include "main.h"

static void vinBuildConfig(ADC_ChannelConfTypeDef * Pconfig);

/**
 * @brief Enables or disables an LDO.
 * @param ldo_id the LDO to be controlled.
 * @param state the state to set the LDO.
 */
void powerIOsToggleLDO(LdoEnum ldo_id, LdoState state)
{
  switch(ldo_id) {
    case LDO_NRF:
      HAL_GPIO_WritePin(LDO_NRF_EN_GPIO_Port, LDO_NRF_EN_Pin, (GPIO_PinState) state);
      break;
    case LDO_LCD:
      HAL_GPIO_WritePin(LDO_LCD_EN_GPIO_Port, LDO_LCD_EN_Pin, (GPIO_PinState) state);
      break;
    case LDO_BUZ:
      HAL_GPIO_WritePin(LDO_BZR_EN_GPIO_Port, LDO_BZR_EN_Pin, (GPIO_PinState) state);
      break;
  }
}

/**
 * @brief Enables or disables an LDO.
 * @return the state of PGOOD
 */
PowerState powerIOsIsPowerGood()
{
  return (HAL_GPIO_ReadPin(PGOOD_VIN_GPIO_Port, PGOOD_VIN_Pin) ? POWER_GOOD : POWER_BAD);
}

HAL_StatusTypeDef powerIOsGetInputVoltage(ADC_HandleTypeDef *hadc, double *voltage)
{
  // ADC gets initialized by CubeMX before entering this function.
  ADC_ChannelConfTypeDef config;

  HAL_StatusTypeDef status = 0;
  vinBuildConfig(&config);
  status |= HAL_ADC_ConfigChannel(hadc, &config);
  status |= HAL_ADCEx_Calibration_Start(hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  status |= HAL_ADC_Start(hadc);
  status |= HAL_ADC_PollForConversion(hadc, ADC_TIMEOUT);
  uint32_t val32 = HAL_ADC_GetValue(hadc);
  status |= HAL_ADC_Stop(hadc);

  *voltage = (double) val32 / ADC_MAX_VALUE * 79.5 ;

  return status;
}


//**********************************************//
//              Static Functions                //
//**********************************************//

static void vinBuildConfig(ADC_ChannelConfTypeDef * Pconfig)
{
  Pconfig->Channel = VIN_ADC_CH;
  Pconfig->Rank = ADC_REGULAR_RANK_1;
  Pconfig->SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
  Pconfig->SingleDiff = ADC_SINGLE_ENDED;
  Pconfig->OffsetNumber = ADC_OFFSET_NONE;
  Pconfig->Offset = 0;
  Pconfig->OffsetRightShift = DISABLE;
  Pconfig->OffsetSignedSaturation = DISABLE;
}

#ifdef __cplusplus
 }
#endif



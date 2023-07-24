/**
 * @file battery.c
 * @author Felix Cormier
 * @date May 18 2021
 * @brief Driver for reading the battery voltage via ADC.
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "battery.h"
#include "main.h"

static void batteryBuildConfig(ADC_ChannelConfTypeDef * Pconfig);

HAL_StatusTypeDef batteryGetVoltage(ADC_HandleTypeDef *hadc, double *voltage)
{
  // ADC gets initialized by CubeMX before entering this function.
  ADC_ChannelConfTypeDef config;

  HAL_StatusTypeDef status = 0;
  batteryBuildConfig(&config);
  status |= HAL_ADC_ConfigChannel(hadc, &config);
  status |= HAL_ADCEx_Calibration_Start(hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  status |= HAL_ADC_Start(hadc);
  status |= HAL_ADC_PollForConversion(hadc, ADC_TIMEOUT);
  uint32_t val32 = HAL_ADC_GetValue(hadc);
  status |= HAL_ADC_Stop(hadc);

  *voltage = (double) val32 / ADC_MAX_VALUE * 5.6 ;

  return status;
}

//**********************************************//
//              Static Functions                //
//**********************************************//

static void batteryBuildConfig(ADC_ChannelConfTypeDef * Pconfig)
{
  Pconfig->Channel = BATTERY_ADC_CH;
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

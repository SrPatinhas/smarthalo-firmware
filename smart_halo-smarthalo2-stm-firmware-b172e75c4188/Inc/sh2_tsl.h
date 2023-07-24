/**
 * @file    sh2_tsl.h
 * @brief   SmartHalo additions to generated tsl library
 * @details The TSL library comes automatically from CubeMX. These
 *          are our additions.
 */

#ifndef __SH2_TSL_H__
#define __SH2_TSL_H__

#include "tsl_user.h"

extern TSL_LinRotParam_T MyLinRots_Param[TSLPRM_TOTAL_ALL_LINROTS];

void tsl_user_ConfigureThresholds(uint8_t * payload);
void tsl_user_ConfigureMaxThreshold(uint8_t max);

#endif // __SH2_TSL_H__

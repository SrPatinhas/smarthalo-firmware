/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

 /** @cond To make doxygen skip this file */

/** @file
 *
 * @defgroup ble_sdk_app_ggms_bondmngr_cfg CGM Bond Manager Configuration
 * @{
 * @ingroup ble_sdk_app_cgms
 * @brief Definition of bond manager configurable parameters
 */

#ifndef BLE_BONDMNGR_CFG_H__
#define BLE_BONDMNGR_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Number of CCCDs used in the CGM application. */
#define BLE_BONDMNGR_CCCD_COUNT            5

/**@brief Maximum number of bonded centrals. */
#define BLE_BONDMNGR_MAX_BONDED_CENTRALS   7


#ifdef __cplusplus
}
#endif

#endif // BLE_BONDMNGR_CFG_H__

/** @} */
/** @endcond */

/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
#ifndef _CONN_BLE_USER_MEM_H
#define _CONN_BLE_USER_MEM_H

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_conn_s130_codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup conn_ble_user_mem Functions for managing memory for user memory request on connectivity device.
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    Connectivity auxiliary functions for providing static memory required by the SoftDevice.
 */

#include "ble.h"
#include "ser_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Connection - user memory mapping structure.
 *
 * @note  This structure is used to map keysets to connection instances, and will be stored in a static table.
 */
//lint -esym(452,ser_ble_user_mem_t)
typedef struct
{
  uint16_t             conn_handle;        /**< Connection handle.*/
  uint8_t              conn_active;        /**< Indication that user memory for this connection is used by the SoftDevice. 0: memory used; 1: memory not used. */
  ble_user_mem_block_t mem_block;          /**< User memory block structure, see @ref ble_user_mem_block_t. */
  uint8_t              mem_table[64];      /**< Memory table. */
} sercon_ble_user_mem_t;

/**@brief Allocates instance in m_user_mem_table[] for storage.
 *
 * @param[out]    p_index             Pointer to the index of allocated instance.
 *
 * @retval NRF_SUCCESS                Success.
 * @retval NRF_ERROR_NO_MEM           No free instance available.
 */
uint32_t conn_ble_user_mem_context_create(uint32_t *p_index);

/**@brief Releases the instance identified by a connection handle.
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @retval NRF_SUCCESS                Context released.
 * @retval NRF_ERROR_NOT_FOUND        Instance with the conn_handle not found.
 */
uint32_t conn_ble_user_mem_context_destroy(uint16_t conn_handle);

/**@brief Finds index of the instance identified by a connection handle in m_user_mem_table[].
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @param[out]    p_index             Pointer to the index of entry in the context table corresponding to the given conn_handle.
 *
 * @retval NRF_SUCCESS                Context table entry found.
 * @retval NRF_ERROR_NOT_FOUND        Instance with the conn_handle not found.
 */
uint32_t conn_ble_user_mem_context_find(uint16_t conn_handle, uint32_t *p_index);
/** @} */


#ifdef __cplusplus
}
#endif

#endif //_CONN_BLE_USER_MEM_H

/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#ifndef BLE_GATT_STRUCT_SERIALIZATION_H
#define BLE_GATT_STRUCT_SERIALIZATION_H

#include "ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ble_gatt_char_props_t_enc(void const * const p_void_struct,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index);

uint32_t ble_gatt_char_props_t_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_void_struct);

uint32_t ble_gatt_char_ext_props_t_enc(void const * const p_void_struct,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index);

uint32_t ble_gatt_char_ext_props_t_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_void_struct);

uint32_t ble_gatt_enable_params_t_enc(void const * const p_void_struct,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index);

uint32_t ble_gatt_enable_params_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_struct);

#ifdef __cplusplus
}
#endif

#endif /*BLE_GATT_STRUCT_SERIALIZATION_H*/

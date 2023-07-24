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

/**
 * @addtogroup ser_conn Connectivity application code
 * @ingroup ble_sdk_lib_serialization
 */

/** @file
 *
 * @defgroup ser_reset_cmd_decoder Reset Command Decoder in the connectivity chip
 * @{
 * @ingroup ser_conn
 *
 * @brief   Decoder for serialized reset command from the Application Chip.
 *
 * @details This file contains declaration of common function used for reset command decoding and
 *          sending responses back to the Application Chip after a command is processed.
 */

#ifndef SER_CONN_RESET_CMD_DECODER_H__
#define SER_CONN_RESET_CMD_DECODER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@brief A function for processing the encoded reset commands from the Application Chip.
 *
 * @details     The function decodes encoded system reset command and performs software reset.
 *              This command does not send back the Command Response packet to the Application Chip.
 */
void ser_conn_reset_command_process(void);


#ifdef __cplusplus
}
#endif

#endif /* SER_CONN_RESET_CMD_DECODER_H__ */

/** @} */


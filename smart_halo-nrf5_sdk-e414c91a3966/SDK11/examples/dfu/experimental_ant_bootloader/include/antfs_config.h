#ifndef ANTFS_CONFIG_H__
#define ANTFS_CONFIG_H__
/* Copyright (c)  2015 Nordic Semiconductor. All Rights Reserved.
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
 
#define ANTFS_DEVICE_TYPE    16u        /**< ANT device type for channel configuration. */


// ant-fs Compile switches.
//#define ANTFS_AUTH_TYPE_PAIRING       /**< Use pairing and key exchange authentication. */
//#define ANTFS_AUTH_TYPE_PASSKEY       /**< Use passkey authentication. */
#define ANTFS_AUTH_TYPE_PASSTHROUGH     /**< Allow host to bypass authentication. */
#define ANTFS_INCLUDE_UPLOAD            /**< Support upload operation. */

#endif //ANTFS_CONFIG_H__

/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright � Dynastream Innovations Inc. 2015
All rights reserved.
*/


#ifndef ANT_IO_TX_H__
#define ANT_IO_TX_H__

#include <stdint.h>
#include "ant_stack_handler_types.h"
#include "bsp.h"


/**@brief Function for configuring and opening master channel.
 *
 */
void ant_io_tx_setup(void);


/**@brief Function for processing channel event messages for the IO Master.
 *
 * @param[in] p_ant_evt A pointer to the received ANT event to handle.
 */
void ant_io_tx_event_handler(ant_evt_t * p_ant_evt);


#endif  // ANT_IO_RX_H__


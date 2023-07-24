/*
 * SH_app_fifo_twi.h
 *
 *  Created on: 2016-05-18
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_QUEUE_TWI_H_
#define SH_FIRMWARE_CODE_SH_QUEUE_TWI_H_

#include <stdint.h>
#include <stdlib.h>
#include "SH_typedefs.h"


uint32_t queue_length(SH_twi_command_buffer_t ** p_queue);

void enqueue(SH_twi_command_buffer_t **p_queue, SH_twi_command_t data);

SH_twi_command_t dequeue(SH_twi_command_buffer_t **p_queue);

SH_twi_command_t check_queue(SH_twi_command_buffer_t **p_queue, uint32_t position);


#endif /* SH_FIRMWARE_CODE_SH_QUEUE_TWI_H_ */

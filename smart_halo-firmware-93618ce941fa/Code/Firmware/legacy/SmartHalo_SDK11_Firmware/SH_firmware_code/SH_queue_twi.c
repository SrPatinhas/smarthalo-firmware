/*
 * SH_app_fifo_twi.c
 *
 *  Created on: 2016-05-18
 *      Author: SmartHalo
 */

#include "SH_queue_twi.h"
#include "stdafx.h"
#include "sdk_common.h"
#include "nordic_common.h"
#include "SH_TWI.h"
#include "SH_typedefs.h"

uint32_t queue_length(SH_twi_command_buffer_t ** p_queue)
{
	SH_twi_command_buffer_t *p_tmp = *p_queue;
	uint32_t queue_lenght = 0;

	if (*p_queue == NULL)
	{
		return queue_lenght;
	}

	queue_lenght = 1;
    while (p_tmp->next != NULL)
    {
 	   p_tmp = p_tmp->next;
 	   queue_lenght ++;
    }

	return queue_lenght;

}

void enqueue(SH_twi_command_buffer_t **p_queue, SH_twi_command_t data)
{

	SH_twi_command_buffer_t *new_twi_command = malloc(sizeof *new_twi_command);

	if (new_twi_command != NULL)
	{
	   new_twi_command->next = NULL;
	   new_twi_command->twi_command = data;
	   if (*p_queue == NULL)
	   {
		   *p_queue = new_twi_command;
	   }
	   else
	   {
		   SH_twi_command_buffer_t *p_tmp = *p_queue;
	       while (p_tmp->next != NULL)
	       {
	    	   p_tmp = p_tmp->next;
	       }
	       p_tmp->next = new_twi_command;
	   }
	}
}

SH_twi_command_t dequeue(SH_twi_command_buffer_t **p_queue)
{
	SH_twi_command_t data;
    /* Check if the queue is not empty */
    if (*p_queue != NULL)
    {
        /* Création d'un élément temporaire pointant vers le deuxième élément de la file. */
    	SH_twi_command_buffer_t *p_tmp = (*p_queue)->next;
        /* Valeur à retourner */
        data = (*p_queue)->twi_command;
        /* Effacement du premier élément. */
        free(*p_queue);
        *p_queue = NULL;
        /* On fait pointer la file vers le deuxième élément. */
        *p_queue = p_tmp;
    }
    else {

    }

    return data;
}

SH_twi_command_t check_queue(SH_twi_command_buffer_t **p_queue, uint32_t position){

	SH_twi_command_t data;
	uint32_t i=1;
	if (*p_queue != NULL){

		SH_twi_command_buffer_t *p_tmp = *p_queue;

		while (i< position)
		{
		   p_tmp = p_tmp->next;
		   i++;
		}
		data = p_tmp->twi_command;
	}

	return data;

}

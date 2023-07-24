/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Tone generation based on nRF HW timer
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#include "Tone.h"

#include <stdint.h>

#include "nrf_drv_timer.h"
#include "app_error.h"
#include "nrf_delay.h"

static uint8_t TONEPIN = 0;
static int toggle_count = 0;
static bool isPlaying = false;

const nrf_drv_timer_t TIMER_TONE = NRF_DRV_TIMER_INSTANCE(4);

static void Tone_timer_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:

        	if (toggle_count != 0)
        	{
				// toggle the pin
        		nrf_gpio_pin_toggle(TONEPIN);

				if (toggle_count > 0)
				  toggle_count--;
			}
			else
			{
				Tone_stop();
				printf("DONE\r\n");
			}
            break;

        default:
            //Do nothing.
            break;
    }
}

void Tone_setup(uint8_t tonePin)
{
	uint32_t err_code;

	TONEPIN = tonePin;
	nrf_gpio_cfg_output(TONEPIN);
	nrf_gpio_pin_write(TONEPIN, 0);
	isPlaying = false;

	err_code = nrf_drv_timer_init(&TIMER_TONE, NULL, Tone_timer_event_handler);
	APP_ERROR_CHECK(err_code);
}

void Tone_play(uint16_t frequency, uint32_t duration)
{
	uint32_t time_us = (uint32_t)((float)500000/frequency); //Time(in miliseconds) between consecutive compare events.
	uint32_t time_ticks;

	time_ticks = nrf_drv_timer_us_to_ticks(&TIMER_TONE, time_us);

	nrf_drv_timer_extended_compare(
	         &TIMER_TONE, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

	 // Calculate the toggle count
	if (duration > 0)
	{
	  toggle_count = 2 * frequency * duration / 1000;
	}
	else
	{
	  toggle_count = -1;
	}

	isPlaying = true;

	nrf_drv_timer_enable(&TIMER_TONE);
}

void Tone_stop()
{
	nrf_drv_timer_disable(&TIMER_TONE);
	nrf_gpio_pin_write(TONEPIN,0);
	isPlaying = false;
}

bool Tone_isPlaying()
{
	return isPlaying;
}

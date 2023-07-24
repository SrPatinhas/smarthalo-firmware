/*
 * SH_timer_for_state_machine.c

 *
 *  Created on: 2016-02-25
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "SH_timer_for_state_machine.h"
#include "nrf_drv_timer.h"

#define APP_TIMER_STATE_MACHINE_PRESCALER 16
#define TIMEOUT_VALUE_FOR_STATE_MACHINE_TIMER 20

const nrf_drv_timer_t TIMER_LED = NRF_DRV_TIMER_INSTANCE(2);
static uint16_t timer_count = 0;
/*
 * PUSH_BUTTON_TIMER_HANDLER
 * This function handles what happens when the timer gets to the TIMEOUT_VALUE
 * Says that the users has entered a long tap
 *
 */
void end_timer_handler(nrf_timer_event_t event_type, void* p_context);

void initialize_and_start_timer_for_state_machine(){

    uint32_t time_ms = 1; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;


    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    nrf_drv_timer_init(&TIMER_LED, NULL, end_timer_handler);


    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);

    nrf_drv_timer_extended_compare(
         &TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    nrf_drv_timer_enable(&TIMER_LED);

}

void end_timer_handler(nrf_timer_event_t event_type, void* p_context){

    switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
        	if (timer_count <1000) {
        		timer_count ++;
        	}
        	else {
        		timer_count = 0;
        	}
          break;

        default:
            //Do nothing.
         break;
    }

}

uint16_t get_timer_count(){
	return timer_count;
}

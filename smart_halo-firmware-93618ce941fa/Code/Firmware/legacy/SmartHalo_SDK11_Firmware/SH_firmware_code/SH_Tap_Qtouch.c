/*
 * SH_Tap_Qtouch.c
 *
 *  Created on: 2016-04-15
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "SH_Tap_Qtouch.h"
#include "SH_typedefs.h"
#include "SEGGER_RTT.h"
#include "app_util_platform.h"
 /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY			APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)

/*
 * TOUCH_EVENT_HANDLER
 * When an action is detected on the TAPSENSOR PIN
 * This function handles what happens when we push on the touch sensor
 * and when we release it
 */
static void touch_event_handler(nrf_drv_gpiote_pin_t pin_no, nrf_gpiote_polarity_t button_action);

/*
 * TAP_TIMER_HANDLER
 * This function handles what happens when the
 * tap timer reaches timeout
 */
static void tap_timer_handler(void *p_context);

static void heartbeat_discriminant_timer_handler(void *p_context);


/*
 * DOUBLE_TAP_TIMER_HANDLER
 * This function handles what happens when the
 * double tap timer reaches timeout
 */
void double_tap_timer_handler(void *p_context);

/*
 *TAP_BUTTON_INIT
 * Initialize the GPIO drivers for the QTOUCH
 */
static void tap_button_init(void);

/*
 * Flags for when the interrupts of the Qtouch pin is called
 */
static bool tap_ready_flag = false;

static uint8_t global_pin_no; //should always be qtouch pin

static uint8_t global_button_action;	//button released or button pushed


static bool begun = false;							// To initialize the TapMode


void* p_context; //empty pointer for timer handler

//Flag that is set to high when the button is pressed and the timer
//for tap reaches time out
static bool the_tap_was_a_long_tap = false;

//Flag to specify when there's a new tap
static bool there_was_a_new_tap = false;

//Flag to specify that there was only one tap
static bool there_was_one_tap = false;

//Flag to specify that there was a double tap
static bool there_was_a_double_tap =false;

//Flag to specify that user wants to change display (only one tap and short tap)
static bool switch_display = false;

//To count the number of tap while the double tap timer isn't over
static uint8_t number_of_tap =0;

//Flag that is set to high when the qtouch button is pushed
static bool button_timer_started = false;

//Flag to specify when the user want to put on/off night light (only one tap and long tap)
static bool night_light_flag = false;

static bool real_tap_not_heartbeat = false;

bool timer_started=false;


//Creation of 3 timers for the Q touch module
APP_TIMER_DEF(tap_timer_id);
APP_TIMER_DEF(heartbeat_discriminant_timer_id);
APP_TIMER_DEF(double_tap_timer_id);


//DOUBLE TAP TIMER HANDLER
void double_tap_timer_handler(void *p_context){

	app_timer_stop(double_tap_timer_id);
	there_was_a_new_tap = true;

	if (number_of_tap >= 2){
		there_was_a_double_tap = true;
	}
	else if (number_of_tap == 1){
		there_was_one_tap = true;
	}
	else {
		there_was_a_double_tap = false;
	}

	number_of_tap = 0;
}




static void heartbeat_discriminant_timer_handler(void *p_context)
{
	if(nrf_drv_gpiote_in_is_set(TAPSENSOR_PIN))
	{
		real_tap_not_heartbeat = true;
		global_button_action = APP_BUTTON_PUSH;
		//SEGGER_RTT_WriteString(0, "\nTap detected, button pressed\n");
	}
	//else
	//{
		//SEGGER_RTT_WriteString(0, ".");
	//}
	timer_started=false;
}

//TAP TIMER HANDLER
void tap_timer_handler(void *p_context){

	app_timer_stop(tap_timer_id);
	the_tap_was_a_long_tap = true;
	if (number_of_tap == 1){
		night_light_flag = true;
		switch_display = false;
	}
}


/**Function for handling events on the touch sensor.
 *
 * @param[in]   pin_no   The pin number where an event as occured
 */
static void touch_event_handler(nrf_drv_gpiote_pin_t pin_no, nrf_gpiote_polarity_t button_action)
{
	global_pin_no = pin_no;
	global_button_action = button_action;
	//SEGGER_RTT_WriteString(0, ":");
	if(!timer_started && !real_tap_not_heartbeat)
	{
		app_timer_start(heartbeat_discriminant_timer_id, 2000, p_context);// about 65ms
		timer_started = true;
	}
}

//TAPCODE INIT
void SH_Tap_Qtouch_init(){

	if (!begun){

		// Creates the timers
		app_timer_create(&tap_timer_id, APP_TIMER_MODE_SINGLE_SHOT, tap_timer_handler);
		app_timer_create(&double_tap_timer_id, APP_TIMER_MODE_SINGLE_SHOT, double_tap_timer_handler);
		app_timer_create(&heartbeat_discriminant_timer_id, APP_TIMER_MODE_SINGLE_SHOT, heartbeat_discriminant_timer_handler);

		//Output and input
		tap_button_init();
		begun = true;
	}
}

/**Function for initializing the button handler module
 * for the touch sensor
 */
static void tap_button_init(void)
{
	nrf_drv_ppi_init();
	nrf_drv_gpiote_init();

	nrf_drv_gpiote_in_config_t config_touch_interrupt = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
#ifdef BOARD_PCA10040
	config_touch_interrupt.pull = NRF_GPIO_PIN_PULLDOWN;
#elif defined(SMARTHALO_EE)
	config_touch_interrupt.pull = NRF_GPIO_PIN_NOPULL;
#else
#error "no target"
#endif
	nrf_drv_gpiote_in_init(TAPSENSOR_PIN, &config_touch_interrupt, touch_event_handler);
	nrf_drv_gpiote_in_event_enable(TAPSENSOR_PIN, true);

}

//CHECK TAP READY FLAG
bool SH_check_tap_ready_flag(){

	return tap_ready_flag;
}

//CHECK PIN NO TAP
uint8_t SH_check_pin_no_tap(){
	return global_pin_no;
}

//CHECK BUTTON ACTION TAP
uint8_t SH_check_button_action_tap(){
	return global_button_action;
}

bool tap_is_not_heartbeat()
{
	return real_tap_not_heartbeat;
}

void reset_heartbeat_flag()
{
	real_tap_not_heartbeat = false;
}


//PUT DOWN TAP READY FLAG
void SH_put_down_tap_ready_flag(){
	tap_ready_flag = false;
}

//GET NIGHT LIGHT FLAG
bool get_night_light_flag(){

	if (there_was_a_new_tap && there_was_one_tap && night_light_flag){
		there_was_one_tap = false;
		there_was_a_new_tap = false;
		night_light_flag =false;
		return true;
	}
	else{
		return false;
	}

}

//GET SWITCH DISPLAY FLAG
bool get_switch_display_flag(void){

	if (there_was_a_new_tap && there_was_one_tap && switch_display){
		there_was_one_tap = false;
		there_was_a_new_tap = false;
		switch_display = false;
		return true;
	}

	else{
		return false;
	}
}

//GET GOAL COMPLETITON FLAG
bool get_goal_completition_flag(void){

	if(there_was_a_new_tap && there_was_a_double_tap){
		there_was_a_new_tap = false;
		there_was_a_double_tap = false;
		return true;
	}
	else{
		return false;
	}
}

//CHECK_TAP, used to check tap when not in tapcode mode
void SH_check_tap(){

	if(SH_check_tap_ready_flag())
	{
		SH_put_down_tap_ready_flag();

		if(real_tap_not_heartbeat)
		{
			real_tap_not_heartbeat = false;
			if (global_button_action == APP_BUTTON_PUSH)
			{
				switch (global_pin_no){

					case TAPSENSOR_PIN :
						number_of_tap++;
						app_timer_start(tap_timer_id, APP_TIMER_TICKS(TIMEOUT_VALUE_FOR_TAP_TIMER, APP_TIMER_PRESCALER), NULL);
						app_timer_start(double_tap_timer_id,APP_TIMER_TICKS(TIMEOUT_VALUE_FOR_DOUBLE_TAP_TIMER, APP_TIMER_PRESCALER), NULL);
						button_timer_started = true;
					break;

					default :
					break;
				}
			}

			if ((global_button_action == APP_BUTTON_RELEASE) && button_timer_started)
			{

				switch (global_pin_no){

					case TAPSENSOR_PIN :

						button_timer_started = false;

						if (!the_tap_was_a_long_tap){
							app_timer_stop(tap_timer_id);
							switch_display = true;
							night_light_flag=false;

						}
						else{
							the_tap_was_a_long_tap = false;
						}


					break;

					default:

					break;
				}
			}
		}

	}
}

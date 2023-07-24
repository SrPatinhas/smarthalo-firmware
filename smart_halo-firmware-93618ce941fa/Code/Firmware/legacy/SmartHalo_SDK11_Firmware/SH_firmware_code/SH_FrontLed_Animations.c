/*
 * SH_FrontLed_Animations.c
 *
 *  Created on: 2016-03-26
 *      Author: SmartHalo
 */
#include "stdafx.h"
#include "SH_FrontLed_Animations.h"
#include "SH_PWM_FrontLight.h"
#include "SH_typedefs.h"

#define TIME_BLINKING	500
#define TIME_ON_BLINK	250
#define BRIGHTNESS_MAX	255
#define FADE_TO_ON      1
#define FADE_TO_OFF     0

APP_TIMER_DEF(front_light_blinking_timer_id);
APP_TIMER_DEF(front_light_fade_timer_id);

static uint8_t brightness = 255;
static bool timer_initialize = false;
static bool front_light_blinking = false;
static bool front_light_is_on = false;

static bool fade_direction;


/*
 * FRONT LIGHT BLINKING TIMER HANDLER
 * This function handles what happens when the
 * blinking timer reaches timeout
 */
static void front_light_blinking_timer_handler(void *p_context);



static void front_light_fading_timer_handler(void *p_context);

/*
 * MAKE THE FRONT LIGHT BLINK
 * This function is called to make the front light blink again
 * Only if the front_light_blinking is true
 */
static void make_the_front_light_blink();

/*
 * MAKE FRONT LIGHT CLOSE
 * This function closes the front light led
 */
void make_front_light_close();


/*
 * MAKE FRONT LIGHT CLOSE
 * This function opens the front light led
 */
void make_front_light_open();

/*
 * FRONT LIGHT TIMER INIT
 */
void front_light_timer_init(){
	if (!timer_initialize){
		app_timer_create(&front_light_blinking_timer_id, APP_TIMER_MODE_SINGLE_SHOT, front_light_blinking_timer_handler);
		app_timer_create(&front_light_fade_timer_id, APP_TIMER_MODE_SINGLE_SHOT, front_light_fading_timer_handler);
		timer_initialize = true;
	}
}

/*
 * SET NEW BRIGHTNESS OF THE FRONT LED
 */
void SH_frontled_set_new_brightness(uint8_t new_brightness){

	brightness = new_brightness;
	SH_PWM_Front_LED_setBrightness(brightness);
	//PWM_show_front_LED();
}

/*
 * GET CURRENT BRIGHTNESS OF THE FRONT LED
 */
uint8_t SH_frontled_get_current_brightness(void){
	return brightness;
}

/*
 *SET THE FRONT LIGHT ON
 */
bool set_FrontLight_On(){

	//make_front_light_open();
	fade_direction = FADE_TO_ON;
	app_timer_start(front_light_fade_timer_id, APP_TIMER_TICKS(16, APP_TIMER_PRESCALER), NULL);
	if (front_light_blinking){
		front_light_blinking = false;
	}
	front_light_is_on = true;
	return true;
}

void make_front_light_open(){

	PWM_set_front_LED_color(BRIGHTNESS_MAX);
	SH_PWM_Front_LED_setBrightness(brightness);
	PWM_show_front_LED();
}

/*
 *SET THE FRONT LIGHT OFF
 */
bool set_FrontLight_Off(){

	//make_front_light_close();
	fade_direction = FADE_TO_OFF;
	app_timer_start(front_light_fade_timer_id, APP_TIMER_TICKS(16, APP_TIMER_PRESCALER), NULL);
	//if (front_light_blinking){
	//	front_light_blinking = false;
	//}
	front_light_is_on = false;
	return true;
}

bool stop_FrontLight_blinking()
{
		front_light_blinking = false;
		return true;

}

/*
 * MAKE THE FRONT LIGHT CLOSE
 */
 void make_front_light_close(){
	clear_pixel_front_LED();
	PWM_show_front_LED();
}

/*
 * SET FRONT LIGHT TO BLINK
 */
bool set_FrontLight_toBlink(){

	front_light_blinking = true;
	make_the_front_light_blink();
	app_timer_start(front_light_blinking_timer_id, APP_TIMER_TICKS(TIME_ON_BLINK, APP_TIMER_PRESCALER), NULL);
	return true;
}

/*
 * MAKE THE FRONT LIGHT BLINK
 */
static void make_the_front_light_blink(){

	if (front_light_blinking){
		if (front_light_is_on){
			make_front_light_close();
			front_light_is_on = false;
		}
		else{
			make_front_light_open();
			front_light_is_on = true;
		}

		//app_timer_start(front_light_blinking_timer_id, APP_TIMER_TICKS(TIME_ON_BLINK, APP_TIMER_PRESCALER), NULL);
	}
}


//FRONT LIGHT BLINKING TIMER HANDLER
static void front_light_blinking_timer_handler(void *p_context){

	//app_timer_stop(front_light_blinking_timer_id);
	make_the_front_light_blink();
	if (front_light_blinking) app_timer_start(front_light_blinking_timer_id, APP_TIMER_TICKS(TIME_ON_BLINK, APP_TIMER_PRESCALER), NULL);
}

static void front_light_fading_timer_handler(void *p_context){

	static uint16_t brightness_fading_counter_on = 0;
	static int16_t brightness_fading_counter_off = 255;

	if(fade_direction == FADE_TO_ON)
	{
		brightness_fading_counter_off = 255;
		if(brightness_fading_counter_on<50)brightness_fading_counter_on += 1;
		else brightness_fading_counter_on += 2;
		if (brightness_fading_counter_on < brightness)
		{
			PWM_set_front_LED_color(brightness_fading_counter_on);
			PWM_show_front_LED();
			app_timer_start(front_light_fade_timer_id, APP_TIMER_TICKS(16, APP_TIMER_PRESCALER), NULL);
		}
		else
		{
			brightness_fading_counter_on = 0;
			brightness_fading_counter_off = brightness;
			app_timer_stop(front_light_fade_timer_id);//only here as a redundant precaution
			PWM_set_front_LED_color(brightness);
			PWM_show_front_LED();
		}
	}
	else
	{
		brightness_fading_counter_on = 0;
		brightness_fading_counter_off -= 2;
		if (brightness_fading_counter_off > 0)
		{
			PWM_set_front_LED_color(brightness_fading_counter_off);
			PWM_show_front_LED();
			app_timer_start(front_light_fade_timer_id, APP_TIMER_TICKS(16, APP_TIMER_PRESCALER), NULL);
		}
		else
		{
			brightness_fading_counter_on = 0;
			brightness_fading_counter_off = brightness;
			app_timer_stop(front_light_fade_timer_id);//only here as a redundant precaution
			PWM_set_front_LED_color(0);
			PWM_show_front_LED();
		}
	}
}

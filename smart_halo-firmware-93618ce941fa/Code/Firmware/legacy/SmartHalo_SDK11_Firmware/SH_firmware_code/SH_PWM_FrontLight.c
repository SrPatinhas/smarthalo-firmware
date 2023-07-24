/*
 * SH_PWM_FrontLight.c
 *
 *  Created on: 2016-03-23
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "SH_PWM_FrontLight.h"
#include "boards.h"
#include "SH_FrontLed_Animations.h"
#include "SH_pinMap.h"

#define ON 	1
#define OFF 0
#define PIN_FRONT_LED						(1 << FRONTLED_PIN)
#define RESCALING_BRIGHTNESS_MAX			255 //(8 bits)
#define SCALE_BRIGHTNESS_MAX				65535 //(32 bits)
#define SHIFT_FOR_PIXEL_COLOR				8

static low_power_pwm_t front_led;			// Create low-power pwm instance for front LED

static uint8_t front_led_color;				//To set the color of the front led

static volatile bool ready_flag = true;            // A flag indicating PWM status.
static bool begun = false;
static uint8_t brightness = 50;

low_power_pwm_config_t low_power_pwm_front_led_config;

//static uint8_t pwm_equivalent (uint8_t pixel_color);
static void pwm_ready_callback(void * p_context);

static void pwm_ready_callback(void * p_context)   // PWM callback function
{
	//low_power_pwm_t * pwm_instance = (low_power_pwm_t*)p_context;
	ready_flag = true;
}

void pwm_front_led_init(void)
{
	if (!begun){

		//Config of the pwm instance

		APP_TIMER_DEF(lpp_timer_3);
		low_power_pwm_front_led_config.active_high = true;
		low_power_pwm_front_led_config.period = UINT8_MAX;
		low_power_pwm_front_led_config.bit_mask = PIN_FRONT_LED;
		low_power_pwm_front_led_config.p_timer_id = &lpp_timer_3;

		//Init and duty set of the front led
		low_power_pwm_init((&front_led), &low_power_pwm_front_led_config, pwm_ready_callback);
		low_power_pwm_duty_set(&front_led, OFF);
		low_power_pwm_start((&front_led), low_power_pwm_front_led_config.bit_mask);

		front_light_timer_init();

		begun = true;
	}
}


void turn_off_front_led_pwm()
{
	low_power_pwm_stop(&front_led);
}

void restart_front_led_pwm()
{
	low_power_pwm_start((&front_led), low_power_pwm_front_led_config.bit_mask);
}

void PWM_set_front_LED_color(uint8_t front_led_pixel_color){

	front_led_color = front_led_pixel_color;
}

void PWM_show_front_LED(){

	//while(!ready_flag);
	//ready_flag = false;
	low_power_pwm_duty_set(&front_led, front_led_color);

}

void SH_PWM_Front_LED_setBrightness(uint8_t brightness_level){

	// Stored brightness value is different than what's passed.
	// This simplifies the actual scaling math later, allowing a fast
	// 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
	// adding 1 here may (intentionally) roll over...so 0 = max brightness
	// (color values are interpreted literally; no scaling), 1 = min
	// brightness (off), 255 = just below max brightness.
	uint8_t newBrightness = brightness_level + 1;
	if(newBrightness != brightness)
	{
		// Compare against prior value
		// Brightness has changed -- re-scale existing data in RAM
		//uint8_t  color;
		//uint8_t	*ptr_of_pixels = pixels;
		uint8_t oldBrightness = brightness - 1; // De-wrap old brightness value
		uint16_t scale;

		if(oldBrightness == 0) scale = 0; // Avoid /0

		else if(brightness_level == RESCALING_BRIGHTNESS_MAX) scale = SCALE_BRIGHTNESS_MAX / oldBrightness;

		else scale = (((uint16_t)newBrightness << SHIFT_FOR_PIXEL_COLOR) - 1) / oldBrightness;

		front_led_color = (front_led_color * scale) >> SHIFT_FOR_PIXEL_COLOR;

		brightness = newBrightness;
	}
}

void clear_pixel_front_LED (){

	PWM_set_front_LED_color(0);

}

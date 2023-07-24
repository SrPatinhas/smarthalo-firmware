/*
 * SH_PWM_Center_LED.c
 *
 *  Created on: 2016-02-24
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "SH_PWM_Center_LED.h"
#include "SH_pinMap.h"

#define PIN_CENTER_LED_R	(1 << CENTRAL_RED_PIN)
#define PIN_CENTER_LED_G	(1 << CENTRAL_GREEN_PIN)
#define PIN_CENTER_LED_B	(1 << CENTRAL_BLUE_PIN)
#define RED_PIXEL_CHANNEL	0
#define BLUE_PIXEL_CHANNEL	0
#define GREEN_PIXEL_CHANNEL	0
#define NUMBBER_0F_PIXELS_CENTER_LED 3
#define RED_ARRAY_CENTER_LED	0
#define GREEN_ARRAY_CENTER_LED	1
#define BLUE_ARRAY_CENTER_LED	2
#define RESCALING_BRIGHTNESS_MAX			255 //(8 bits)
#define SCALE_BRIGHTNESS_MAX				65535 //(32 bits)
#define SHIFT_FOR_PIXEL_COLOR				8
#define ON 	1
#define OFF 0

static low_power_pwm_t red_pixel_central_led;       // Create low-power pwm instance for central LED
static low_power_pwm_t green_pixel_central_led;
static low_power_pwm_t blue_pixel_central_led;
low_power_pwm_config_t low_power_pwm_red_pixel_central_led_config;
low_power_pwm_config_t low_power_pwm_green_pixel_central_led_config;
low_power_pwm_config_t low_power_pwm_blue_pixel_central_led_config;
static volatile bool ready_flag = true;            // A flag indicating PWM status.
static uint8_t center_led[NUMBBER_0F_PIXELS_CENTER_LED]; //To set the color of the pixel of the center led
static bool begun = false;
static uint8_t brightness = 0;

static void pwm_ready_callback(void * p_context);

static void pwm_ready_callback(void * p_context)   // PWM callback function
{
	//low_power_pwm_t * pwm_instance = (low_power_pwm_t*)p_context;
	ready_flag = true;
}


uint32_t pwm_center_led_init(void)
{
	uint32_t err_code = 0;
	if (!begun){

		//Red pixel config

		APP_TIMER_DEF(lpp_timer_0);
		low_power_pwm_red_pixel_central_led_config.active_high = true;
		low_power_pwm_red_pixel_central_led_config.period = UINT8_MAX;
		low_power_pwm_red_pixel_central_led_config.bit_mask = PIN_CENTER_LED_R;
		low_power_pwm_red_pixel_central_led_config.p_timer_id = &lpp_timer_0;

		//Green pixel config

		APP_TIMER_DEF(lpp_timer_1);
		low_power_pwm_green_pixel_central_led_config.active_high = true;
		low_power_pwm_green_pixel_central_led_config.period = UINT8_MAX;
		low_power_pwm_green_pixel_central_led_config.bit_mask = PIN_CENTER_LED_G;
		low_power_pwm_green_pixel_central_led_config.p_timer_id = &lpp_timer_1;

		//Blue pixel config
		low_power_pwm_config_t low_power_pwm_blue_pixel_central_led_config;
		APP_TIMER_DEF(lpp_timer_2);
		low_power_pwm_blue_pixel_central_led_config.active_high = true;
		low_power_pwm_blue_pixel_central_led_config.period = UINT8_MAX;
		low_power_pwm_blue_pixel_central_led_config.bit_mask = PIN_CENTER_LED_B;
		low_power_pwm_blue_pixel_central_led_config.p_timer_id = &lpp_timer_2;

		//Initialization,duty set of the red pixel pwm
		err_code = low_power_pwm_init((&red_pixel_central_led), &low_power_pwm_red_pixel_central_led_config, pwm_ready_callback);
		err_code += low_power_pwm_duty_set(&red_pixel_central_led, OFF);
		err_code += low_power_pwm_start((&red_pixel_central_led), low_power_pwm_red_pixel_central_led_config.bit_mask);

		//Initialization,duty set of the red pixel pwm
		err_code += low_power_pwm_init((&green_pixel_central_led), &low_power_pwm_green_pixel_central_led_config, pwm_ready_callback);
		err_code += low_power_pwm_duty_set(&green_pixel_central_led, OFF);
		err_code += low_power_pwm_start((&green_pixel_central_led), low_power_pwm_green_pixel_central_led_config.bit_mask);

		//Initialization,duty set of the red pixel pwm
		err_code += low_power_pwm_init((&blue_pixel_central_led), &low_power_pwm_blue_pixel_central_led_config, pwm_ready_callback);
		err_code += low_power_pwm_duty_set(&blue_pixel_central_led, OFF);
		err_code += low_power_pwm_start((&blue_pixel_central_led), low_power_pwm_blue_pixel_central_led_config.bit_mask);

		begun = true;
	}
	return err_code;
}

void PWM_set_center_LED_color(uint8_t red_pixel_color, uint8_t green_pixel_color, uint8_t blue_pixel_color){

	center_led[RED_ARRAY_CENTER_LED]=red_pixel_color;
	center_led[GREEN_ARRAY_CENTER_LED]= green_pixel_color;
	center_led[BLUE_ARRAY_CENTER_LED]= blue_pixel_color;
}

void PWM_show_center_LED(){

	//Red pixel
	while(!ready_flag);
	ready_flag = false;
	low_power_pwm_duty_set(&red_pixel_central_led, (uint8_t)center_led[RED_ARRAY_CENTER_LED]);

	//Green pixel
	while(!ready_flag);
	ready_flag = false;
	low_power_pwm_duty_set(&green_pixel_central_led, (uint8_t)center_led[GREEN_ARRAY_CENTER_LED]);

	//Blue pixel
	while(!ready_flag);
	ready_flag = false;
	low_power_pwm_duty_set(&blue_pixel_central_led, (uint8_t)center_led[BLUE_ARRAY_CENTER_LED]);

}

void turn_off_center_led_pwm()
{
	low_power_pwm_stop(&red_pixel_central_led);
	low_power_pwm_stop(&green_pixel_central_led);
	low_power_pwm_stop(&blue_pixel_central_led);
}

void restart_center_led_pwm()
{
	low_power_pwm_start((&red_pixel_central_led), low_power_pwm_red_pixel_central_led_config.bit_mask);
	low_power_pwm_start((&green_pixel_central_led), low_power_pwm_green_pixel_central_led_config.bit_mask);
	low_power_pwm_start((&blue_pixel_central_led), low_power_pwm_blue_pixel_central_led_config.bit_mask);
}

void SH_PWM_Center_LED_setBrightness(uint8_t brightness_level){

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

		for(int i=0; i<NUMBBER_0F_PIXELS_CENTER_LED; i++)
		{
			center_led[i] = (center_led[i] * scale) >> SHIFT_FOR_PIXEL_COLOR;
		}

		brightness = newBrightness;
	}
}

//Sets the pixel of center led at 0
void clear_pixel_center_LED (){

	PWM_set_center_LED_color(0,0,0);

}

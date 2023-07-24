/*
 * SH_CenterLed_Animations.c
 *
 *  Created on: 2016-02-29
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include <stdint.h>
#include "SH_CenterLed_Animations.h"
#include "SH_PWM_Center_LED.h"
#include "SH_HaloPixelnRF.h"
#include "SH_Temperature_monitoring.h"
#include "SH_Animations_colors.h"
#include "SH_Primary_state_machine.h"

#define DELAY_BETWEEN_REFRESH_OF_ANIMATIONS		TIMER_FOR_ANIMATION_TIME_BETWEEN_CONSECUTIVE_COMPARE_EVENTS
#define BRIGHTNESS_MULTIPLICATOR  				(unsigned int)((BRIGHTNESS_MAX+1)/NUMBER_OF_BRIGHTNESS_CHANGES) //8*32 = 256, 256 = Max Brightness
#define NUMBER_OF_BRIGHTNESS_CHANGES			(unsigned int)(64+(-0.77*(DELAY_BETWEEN_REFRESH_OF_ANIMATIONS-1)))
#define NUMBER_OF_RINGS_CALL_NOTIFICATION		12
#define NUMBER_OF_RINGS_SMS_NOTIFICATION		2
#define NUMBER_OF_FLASH_GOAL_COMPLETION			3
#define DELAY_BETWEEN_RINGS_CALL_NOTIFICATION	1	//(2 MS)
#define	DELAY_SILENCE_CALL_NOTIFICATION			300 //( S)
#define DElAY_FOR_BRIGHTNESS_UPDATE				5 //(10 MS)
#define MIN_BRIGHTNESS_LOW_BATTERY_NOTIFCATION	0.2*NUMBER_OF_BRIGHTNESS_CHANGES
#define MAX_BRIGHTNESS_LOW_BATTERY_NOTIFCATION	0.8*NUMBER_OF_BRIGHTNESS_CHANGES
#define NO_RESET 								0
#define MAX_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE (NUMBER_OF_BRIGHTNESS_CHANGES*0.50)
#define MIN_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE (NUMBER_OF_BRIGHTNESS_CHANGES*0.2)

#define BATTERY_ON_CHARGE_FIRST_QUARTER		25
#define BATTERY_ON_CHARGE_SECOND_QUARTER	50
#define BATTERY_ON_CHARGE_THIRD_QUARTER		75
#define	BATTERY_ON_CHARGE_LAST_QUARTER		100

static const uint8_t no_parameter_animations = 0;

static uint8_t brightness = 50;
static float red_pixel = 0;
static float green_pixel = 0;
static float blue_pixel = 0;

//static void center_LED_On(float r, float g, float b, uint8_t level_of_brightness);

void SH_centerled_set_new_brightness(uint8_t new_brightness){

	#ifndef TEST_HALO_BOX
	brightness = (uint8_t)(new_brightness * led_brightness_correction_for_temperature());
	#else
	brightness = new_brightness;
	#endif
	SH_PWM_Center_LED_setBrightness(brightness);
	//PWM_show_center_LED();
}

uint8_t SH_centerled_get_current_brightness(void){
	return brightness;
}

//Plays the exception notification animation on the center LED
bool Animations_exceptionNotification_center_led (bool reset, uint8_t parameter_1, uint8_t parameter_2){

	SH_PWM_Center_LED_setBrightness(brightness);
	static uint8_t level_of_brightness_fade_in = 0;
	static uint8_t level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES;

	static uint16_t counter_pairingfaded_animation = 0;

	//Animation is red
	red_pixel = (float)RED_PIXEL_YELLOW/BRIGHTNESS_MAX;
	green_pixel = (float)GREEN_PIXEL_YELLOW/BRIGHTNESS_MAX;
	blue_pixel = (float)BLUE_PIXEL_YELLOW/BRIGHTNESS_MAX;

	if (!reset){
		//Modify the brightness of the LED after a delay (simulated with the static variable)
		if ((counter_pairingfaded_animation%DElAY_FOR_BRIGHTNESS_UPDATE) == 0){

			//Fade in
			if (level_of_brightness_fade_in<NUMBER_OF_BRIGHTNESS_CHANGES){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_in);
				level_of_brightness_fade_in++ ;
			}

			//Fade out
			else if (level_of_brightness_fade_out>0){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
				level_of_brightness_fade_out--;
			}

			//Turns off the leds when the animation is over and reinitialize the static variables
			else{
				turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
				counter_pairingfaded_animation = 0;
				level_of_brightness_fade_in = 0;
				level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES;
				return true;
			}
		}

		counter_pairingfaded_animation++;
		return false;
	}

	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		counter_pairingfaded_animation = 0;
		level_of_brightness_fade_in = 0;
		level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES;
		return true;
	}
}

//Plays the low battery notification animation on the center LED
bool Animations_lowBattery_center_led (bool reset, uint8_t parameter_1, uint8_t parameter_2){

	SH_PWM_Center_LED_setBrightness(brightness);

	static uint8_t level_of_brightness_fade_in = MIN_BRIGHTNESS_LOW_BATTERY_NOTIFCATION;
	static uint8_t level_of_brightness_fade_out = MAX_BRIGHTNESS_LOW_BATTERY_NOTIFCATION;

	static uint16_t counter_lowbattery_animation = 0;

	//Animation is in red
	red_pixel = (float)RED_PIXEL_ALARM/BRIGHTNESS_MAX;
	green_pixel = (float)GREEN_PIXEL_ALARM/BRIGHTNESS_MAX;
	blue_pixel = (float)BLUE_PIXEL_ALARM/BRIGHTNESS_MAX;

	if (!reset){
		//Modify the brightness of the LED after a delay (simulated with the static variable)
		if (counter_lowbattery_animation%25 == 0){

			//Fade in
			if (level_of_brightness_fade_in<MAX_BRIGHTNESS_LOW_BATTERY_NOTIFCATION){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_in);
				level_of_brightness_fade_in++ ;
			}

			//Fade out
			else if (level_of_brightness_fade_out>=MIN_BRIGHTNESS_LOW_BATTERY_NOTIFCATION) {
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
				level_of_brightness_fade_out--;

			}

			//Turns off the leds when the animation is over and reinitialize the static variables
			else{
				turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
				counter_lowbattery_animation = 0;
				level_of_brightness_fade_in = MIN_BRIGHTNESS_LOW_BATTERY_NOTIFCATION;
				level_of_brightness_fade_out = MAX_BRIGHTNESS_LOW_BATTERY_NOTIFCATION;
				return true;

			}
		}

		counter_lowbattery_animation++;
		return false;
	}
	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		counter_lowbattery_animation = 0;
		level_of_brightness_fade_in = MIN_BRIGHTNESS_LOW_BATTERY_NOTIFCATION;
		level_of_brightness_fade_out = MAX_BRIGHTNESS_LOW_BATTERY_NOTIFCATION;
		return true;
	}
}

bool Animations_goal_completion_center_led (bool reset, uint8_t parameter_1, uint8_t parameter_2){

	SH_PWM_Center_LED_setBrightness(brightness);

	static uint8_t level_of_brightness_fade_in = 0;
	static uint16_t counter_goal_completion = 0;
	static uint8_t level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
	static uint8_t number_of_flashes = 0;

	//Animation is pink
	red_pixel = (float)RED_PIXEL_GOAL_COMPLETION/BRIGHTNESS_MAX;
	green_pixel = (float)GREEN_PIXEL_GOAL_COMPLETION/BRIGHTNESS_MAX;
	blue_pixel = (float)BLUE_PIXEL_GOAL_COMPLETION/BRIGHTNESS_MAX;
	if (!reset){
		if (counter_goal_completion%10 == 0){
			if (level_of_brightness_fade_in < NUMBER_OF_BRIGHTNESS_CHANGES - 1){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_in);
				level_of_brightness_fade_in++;
			}
			else if (level_of_brightness_fade_out > 0){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
				level_of_brightness_fade_out--;
			}

			else {
				turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
				level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
				level_of_brightness_fade_in = 0;
				number_of_flashes++;
				if (number_of_flashes == NUMBER_OF_FLASH_GOAL_COMPLETION){
					counter_goal_completion = 0;
					number_of_flashes = 0;
					return true;
				}
			}
		}

		counter_goal_completion ++;
		return false;

	}

	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		level_of_brightness_fade_in = 0;
		counter_goal_completion = 0;
		return true;
	}

}

//Plays the shortTap animation on the center LED
bool Animations_shortTap_center_led (bool reset, uint8_t parameter_1, uint8_t parameter_2){

	SH_PWM_Center_LED_setBrightness(brightness);

	static uint8_t level_of_brightness = 0;
	static uint16_t counter_shorttap_animation = 0;

	//Animation is white
	red_pixel = (float)RED_PIXEL_WHITE/BRIGHTNESS_MAX;
	green_pixel = (float)GREEN_PIXEL_WHITE/BRIGHTNESS_MAX;
	blue_pixel = (float)BLUE_PIXEL_WHITE/BRIGHTNESS_MAX;

	if (!reset){
		if (level_of_brightness<NUMBER_OF_BRIGHTNESS_CHANGES){

			//Increases the brightness of the center led after a delay (simulated with the static variable)
			if (counter_shorttap_animation%1 == 0){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness);
				level_of_brightness+=5;
			}
		}

		//Turns off the leds when the animation is over and reinitialize static variable
		else {
			if (counter_shorttap_animation %1 == 0){
				turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
				counter_shorttap_animation = 0;
				level_of_brightness = 0;
				return true;
			}
		}

		counter_shorttap_animation++;
		return false;
	}
	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		counter_shorttap_animation = 0;
		level_of_brightness = 0;
		return true;
	}
}

//Plays the call Notification animation on the center LED
bool Animations_callNotification (bool reset, uint8_t parameter_1, uint8_t parameter_2){

	SH_PWM_Center_LED_setBrightness(brightness);
	static uint16_t counter_callnotification_animation = 0;
	static uint16_t counter_callnotification_animation_blink = 0;
	static uint8_t number_of_flash = 0;
	static int8_t level_of_brightness = 0;
	static int8_t level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;

	//The notification is blue
	red_pixel = (float)RED_PIXEL_NOTIFICATION/BRIGHTNESS_MAX;
	green_pixel = (float)GREEN_PIXEL_NOTIFICATION/BRIGHTNESS_MAX;
	blue_pixel = (float)BLUE_PIXEL_NOTIFICATION/BRIGHTNESS_MAX;

	if (!reset){
		//Plays the notification for a number of rings
		if (number_of_flash < NUMBER_OF_RINGS_CALL_NOTIFICATION){

			//Modify the brightness of the LED after a delay (simulated with the static variable)
			if(counter_callnotification_animation_blink % 1 == 0){

				if (level_of_brightness < NUMBER_OF_BRIGHTNESS_CHANGES){
					center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness);
					level_of_brightness+=15;
				}

				else if (level_of_brightness_fade_out > 0){
					center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
					level_of_brightness_fade_out-=15;
				}

				else {
					turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
					number_of_flash++;
					level_of_brightness = 0;
					level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
				}
			}

			counter_callnotification_animation_blink++;
		}

		//The LED are turned off for a certain number of call of the function
		else {
				counter_callnotification_animation++;

			//Turns off the leds when the animation is over and reinitialize the static variables
			if (counter_callnotification_animation >= DELAY_SILENCE_CALL_NOTIFICATION){
				counter_callnotification_animation = 0;
				number_of_flash = 0;
				level_of_brightness = 0;
				counter_callnotification_animation_blink = 0;
				return true;
			}
		}

		return false;
	}

	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		counter_callnotification_animation = 0;
		number_of_flash = 0;
		level_of_brightness = 0;
		counter_callnotification_animation_blink = 0;
		level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		return true;
	}
}

//Plays the sms Notification animation on the center LED
bool Animations_smsNotification (bool reset, uint8_t parameter_1, uint8_t parameter_2){

	SH_PWM_Center_LED_setBrightness(brightness);

	static uint16_t counter_smsnotification_animation = 0;
	static uint16_t counter_smsnotification_animation_blink = 0;
	static uint8_t number_of_flash = 0;
	static int8_t level_of_brightness = 0;
	static int8_t level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;

	//The notification is blue
	red_pixel = (float)RED_PIXEL_NOTIFICATION/BRIGHTNESS_MAX;
	green_pixel = (float)GREEN_PIXEL_NOTIFICATION/BRIGHTNESS_MAX;
	blue_pixel = (float)BLUE_PIXEL_NOTIFICATION/BRIGHTNESS_MAX;

	if (!reset){
		//Plays the notification for a number of rings
		if (number_of_flash < NUMBER_OF_RINGS_SMS_NOTIFICATION){

			//Modify the brightness of the LED after a delay (simulated with the static variable)


				if(level_of_brightness < NUMBER_OF_BRIGHTNESS_CHANGES){
					if (counter_smsnotification_animation_blink % 2 == 0){
						center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness);
						level_of_brightness +=10;
					}
				}

				else if (level_of_brightness_fade_out > 0){
					if (number_of_flash == NUMBER_OF_RINGS_SMS_NOTIFICATION - 1){
						if (counter_smsnotification_animation_blink % 4 == 0){
							center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
							level_of_brightness_fade_out-=10;
						}
					}
					else {
						if (counter_smsnotification_animation_blink % 2 == 0){
							center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
							level_of_brightness_fade_out-=10;
						}
					}
				}

				else{
					turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
					number_of_flash++;
					level_of_brightness = 0;
					level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
				}

			counter_smsnotification_animation_blink++;
		}

		//The LED are turned off after a certain number of call of the function
		else {
				counter_smsnotification_animation++;
			//Turns off the leds when the animation is over and reinitialize the static variables
			if (counter_smsnotification_animation >= 100){
				counter_smsnotification_animation = 0;
				number_of_flash = 0;
				level_of_brightness = 0;
				counter_smsnotification_animation = 0;
				return true;
			}
		}

		return false;
	}

	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		counter_smsnotification_animation = 0;
		number_of_flash = 0;
		level_of_brightness = 0;
		counter_smsnotification_animation = 0;
		level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		return true;
	}
}

//Plays the sms Notification animation on the center LED
bool Animations_battery_charge_center_led (bool reset, uint8_t soc_battery, uint8_t parameter_2){

	SH_PWM_Center_LED_setBrightness(brightness);

	static uint16_t counter_battery_charge_animation = 0;
	//static uint8_t number_of_flash = 0;
	static uint8_t level_of_brightness = MIN_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE;
	static uint8_t level_of_brightness_fade_out = MAX_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE;

	if (!reset){
		if (soc_battery < BATTERY_ON_CHARGE_FIRST_QUARTER){
			red_pixel = (float)RED_PIXEL_RED/BRIGHTNESS_MAX;
			green_pixel = (float)GREEN_PIXEL_RED/BRIGHTNESS_MAX;
			blue_pixel = (float)BLUE_PIXEL_RED/BRIGHTNESS_MAX;
		}
		else if (soc_battery < BATTERY_ON_CHARGE_SECOND_QUARTER){
			red_pixel = (float)RED_PIXEL_YELLOW/BRIGHTNESS_MAX;
			green_pixel = (float)GREEN_PIXEL_YELLOW/BRIGHTNESS_MAX;
			blue_pixel = (float)BLUE_PIXEL_YELLOW/BRIGHTNESS_MAX;
		}
		else if (soc_battery < BATTERY_ON_CHARGE_THIRD_QUARTER){
			red_pixel = (float)RED_PIXEL_MID_GREEN/BRIGHTNESS_MAX;
			green_pixel = (float)GREEN_PIXEL_MID_GREEN/BRIGHTNESS_MAX;
			blue_pixel = (float)BLUE_PIXEL_MID_GREEN/BRIGHTNESS_MAX;
		}
		else{
			red_pixel = (float)RED_PIXEL_GREEN/BRIGHTNESS_MAX;
			green_pixel = (float)GREEN_PIXEL_GREEN/BRIGHTNESS_MAX;
			blue_pixel = (float)BLUE_PIXEL_GREEN/BRIGHTNESS_MAX;
		}

		if(counter_battery_charge_animation % 30 == 0){
			if (level_of_brightness < MAX_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness);
				level_of_brightness ++;
			}
			else if (level_of_brightness_fade_out >= MIN_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
				level_of_brightness_fade_out --;
			}
			else{
				//turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
				counter_battery_charge_animation = 0;
				level_of_brightness = MIN_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE;
				level_of_brightness_fade_out = MAX_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE;
				return true;
			}
		}
		counter_battery_charge_animation++;
		return false;
	}

	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		counter_battery_charge_animation = 0;
		level_of_brightness = MIN_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE;
		level_of_brightness_fade_out = MAX_BRIGHTNESS_CHANGES_BATERY_ON_CHARGE;
		return true;
	}


}

//Lights the LED the in the rgb color with the desired brightness
 void center_LED_On(float r, float g, float b, uint8_t level_of_brightness){

	PWM_set_center_LED_color(r*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,g*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,b*BRIGHTNESS_MULTIPLICATOR*level_of_brightness);
	PWM_show_center_LED();
}

//Turn off center led
bool turn_Off_center_LED(bool reset, uint8_t parameter_1, uint8_t parameter_2){
	clear_pixel_center_LED();
	PWM_show_center_LED();
	return true;
}


bool Animations_paired_indicator_center_led (bool reset, uint8_t parameter_1, uint8_t parameter_2)
{

	SH_PWM_Center_LED_setBrightness(brightness);

	static float level_of_brightness = 0;
	static uint16_t counter_pairedIndicator_animation = 0;
	static float level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES/2 - 1;

	//Animation is green
	red_pixel = 0;
	green_pixel = (float)GREEN_PIXEL_WHITE/BRIGHTNESS_MAX;
	blue_pixel = 0;

	if (!reset){
		if (level_of_brightness<NUMBER_OF_BRIGHTNESS_CHANGES/2){

			//Increases the brightness of the center led after a delay (simulated with the static variable)
			if (counter_pairedIndicator_animation%(DElAY_FOR_BRIGHTNESS_UPDATE/4) == 0){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness);
				level_of_brightness+=0.3f;
			}

		}
		else if(level_of_brightness_fade_out > 2)// we do not want the led to go back down to 0 brightness for UX reasons
		{
			if (counter_pairedIndicator_animation%(DElAY_FOR_BRIGHTNESS_UPDATE/4) == 0){
				center_LED_On(red_pixel, green_pixel, blue_pixel, level_of_brightness_fade_out);
				level_of_brightness_fade_out-=0.3f;
			}
		}

		//Turns off the leds when the animation is over and reinitialize static variable
		else {
			if (counter_pairedIndicator_animation %10 == 0){
				//turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
				counter_pairedIndicator_animation = 0;
				level_of_brightness = 3; // we do not want the led to go back down to 0, and increment by one to make it smoother
				level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES/2 - 1;;
				return true;
			}
		}

		counter_pairedIndicator_animation++;
		return false;
	}
	else{
		turn_Off_center_LED(NO_RESET, no_parameter_animations, no_parameter_animations);
		counter_pairedIndicator_animation = 0;
		level_of_brightness = 0;
		return true;
	}
}






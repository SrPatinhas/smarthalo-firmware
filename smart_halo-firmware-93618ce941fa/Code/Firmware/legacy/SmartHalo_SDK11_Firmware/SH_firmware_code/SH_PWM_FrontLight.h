/*
 * SH_PWM_FrontLight.h
 *
 *  Created on: 2016-03-23
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_PWM_FRONTLIGHT_H_
#define SH_FIRMWARE_CODE_SH_PWM_FRONTLIGHT_H_

//PWM_FRONT_LED_INIT
//Initialization of the pwm instance for the front LED
//To call before using any front led animation
void pwm_front_led_init(void);

//PWM_SET_FRONT_LED_COLOR
//To set the color of the front LED
//Call the show function to see the color
void PWM_set_front_LED_color(uint8_t front_led_pixel_color);

//CLEAR_PIXEL_FRONT_LED
//Sets 0 for the color of the pixel of the center led
void clear_pixel_front_LED ();

//PWM_SHOW_FRONT_LED
//Show the color that is set for the pixel of the front LED
void PWM_show_front_LED();

//SH_PWM_FRONT_LED_SETBRIGHTNESS
void SH_PWM_Front_LED_setBrightness(uint8_t brightness_level);

void turn_off_front_led_pwm();

void restart_front_led_pwm();


#endif /* SH_FIRMWARE_CODE_SH_PWM_FRONTLIGHT_H_ */

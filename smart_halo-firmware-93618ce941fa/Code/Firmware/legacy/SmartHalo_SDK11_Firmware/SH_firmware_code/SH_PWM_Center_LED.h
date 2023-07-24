/*
 * SH_PWM_Center_LED.h
 *
 *  Created on: 2016-02-24
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_PWM_CENTER_LED_H_
#define SH_FIRMWARE_CODE_SH_PWM_CENTER_LED_H_
#include <stdint.h>

//PWM_SET_CENTER_LED_COLOR
//To set the colors of the pixel of the center LED
//Call the show function to see the colors
void PWM_set_center_LED_color(uint8_t red_pixel_color, uint8_t green_pixel_color, uint8_t blue_pixel_color);


//PWM_CENTER_LED_INIT
//Initialization of the pwm instances for the center LED
//To call before using any center led animation
uint32_t pwm_center_led_init();

//PWM_SHOW_CENTER_LED
//Show the colors that are set for the pixel of the center LED
void PWM_show_center_LED();

//SH_PWM_CENTER_LED_SETBRIGHTNESS
void SH_PWM_Center_LED_setBrightness(uint8_t brightness_level);

//CLEAR_PIXEL_CENTER_LED
//Sets 0 for the color of the pixel of the center led
void clear_pixel_center_LED();



void turn_off_center_led_pwm();

void restart_center_led_pwm();

#endif /* SH_FIRMWARE_CODE_SH_PWM_CENTER_LED_H_ */

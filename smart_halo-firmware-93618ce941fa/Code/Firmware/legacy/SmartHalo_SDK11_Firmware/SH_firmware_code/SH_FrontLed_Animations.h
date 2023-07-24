/*
 * SH_FrontLed_Animations.h
 *
 *  Created on: 2016-03-26
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_FRONTLED_ANIMATIONS_H_
#define SH_FIRMWARE_CODE_SH_FRONTLED_ANIMATIONS_H_



//GET_CURRENT_BRIGHTNESS
//Returns the current brightness (0 to 255)
uint8_t SH_frontled_get_current_brightness(void);

//SET_NEW_BRIGHTNESS
//Modifies the value of the brightness and shows the results on the LEDs
void SH_frontled_set_new_brightness(uint8_t new_brightness);

//FRONT LIGHT TIMER INIT
//Initialization of the timer
//
void front_light_timer_init();

//Front light will be turned on for a delay and then turned off for the same delay
//until you call the function set_FrontLight_Off
bool set_FrontLight_toBlink();

//Front light will be turned on
bool set_FrontLight_On();

//Front light will be turned off
bool set_FrontLight_Off();

bool stop_FrontLight_blinking();

void make_front_light_close();

void make_front_light_open();

#endif /* SH_FIRMWARE_CODE_SH_FRONTLED_ANIMATIONS_H_ */

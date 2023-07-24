/*
 * SH_CenterLed_Animations.h
 *
 *  Created on: 2016-02-29
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_CENTERLED_ANIMATIONS_H_
#define SH_FIRMWARE_CODE_SH_CENTERLED_ANIMATIONS_H_

#include <stdbool.h>

//GET_CURRENT_BRIGHTNESS
//Returns the current brightness (0 to 255)
uint8_t SH_centerled_get_current_brightness(void);

//SET_NEW_BRIGHTNESS
//Modifies the value of the brightness and shows the results on the LEDs
//
void SH_centerled_set_new_brightness(uint8_t new_brightness);

//TURN_OFF_CENTER_LED
//Turns off the center LED
//only has to be called once
bool turn_Off_center_LED(bool reset, uint8_t, uint8_t);

//ANIMATIONS_CALLNOTIFICATION
//Increases the brightness of the center led in blue
//blink for NUMBER_OF_RINGS_CALL_NOTIFICATION
//the leds are turned off at the end of the animation
bool Animations_callNotification (bool reset, uint8_t, uint8_t);

//ANIMATIONS_SMSNOTIFICATION
//Increases slowly the brightness of the center led in blue
//blink for NUMBER_OF_RINGS_SMS_NOTIFICATION
//the leds are turned off at the end of the animation
bool Animations_smsNotification (bool reset, uint8_t, uint8_t);

//ANIMATIONS_SHORTTAP_center_led
//Increases the brightness of the center led in white
//the leds are turned off at the end of the animation
bool Animations_shortTap_center_led (bool reset, uint8_t, uint8_t);

//ANIMATIONS_LOWBATTERY_center_led
//Increases slowly the brightness of the center led in yellow
//from 20% brightness to 80% in 2s
//Decreases slowly the brightness of the center led in yellow
//from 80% to 20% in 2s
//the leds are turned off at the end of the animation
bool Animations_lowBattery_center_led(bool reset, uint8_t, uint8_t);

//ANIMATIONS_PAIRINGFADED_center_led
//Increases the brightness of the center led in red
//the leds are turned off at the end of the animation
bool Animations_exceptionNotification_center_led (bool reset, uint8_t, uint8_t);

//ANIMATIONS_GOAL_COMPLETION_center_led
bool Animations_goal_completion_center_led (bool reset, uint8_t parameter_1, uint8_t parameter_2);

//ANIMATIONS_BATTERY_CHARGE_center_led
bool Animations_battery_charge_center_led (bool reset, uint8_t soc_battery, uint8_t parameter_2);

void center_LED_On(float r, float g, float b, uint8_t level_of_brightness);

bool Animations_paired_indicator_center_led (bool reset, uint8_t parameter_1, uint8_t parameter_2);


#endif /* SH_FIRMWARE_CODE_SH_CENTERLED_ANIMATIONS_H_ */

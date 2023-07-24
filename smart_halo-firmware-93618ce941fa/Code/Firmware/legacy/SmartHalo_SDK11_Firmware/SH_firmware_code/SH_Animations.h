/*
 * SH_Animations.h
 *
 *  Created on: 2016-06-07
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_ANIMATIONS_H_
#define SH_FIRMWARE_CODE_SH_ANIMATIONS_H_


#include <stdint.h>
#include <stdbool.h>

#define NUMBER_OF_STEPS_SLIGHT_TURN		7
#define NUMBER_OF_STEPS_HARD_TURN		7
#define NUMBER_OF_STEPS_TURN			10

#define PERCENTAGE_MINIMAL_OF_BRIGHTNESS_AS_THE_CROW_FLIES 	.2
#define PERCENTAGE_MAXIMAL_OF_BRIGHTNESS_AS_THE_CROW_FLIES	.8

/* Function prototypes */

//GET_CURRENT_BRIGHTNESS
//Returns the current brightness (0 to 255)
uint8_t SH_halo_get_current_brightness(void);

//SET_NEW_BRIGHTNESS
//Modifies the value of the brightness and shows the results on the LEDs
//
void SH_halo_set_new_brightness(uint8_t new_brightness);

//TURNOFFALLLEDS
bool turnOffAllLEDs(bool reset, uint8_t parameter_1, uint8_t parameter_2);

/*SUCCESS
 * Sets the SmartHalo colors on the LEDs
 * returns True when the animation is done
 */
bool Animations_success(bool reset, uint8_t, uint8_t);

//STRAIGHT
//void Animations_straight();
//returns True when the animation is done
bool Animations_straight(bool reset, uint8_t, uint8_t);

//SLIGHTRIGHT
//7 steps of animation
//Call the seventh step x times to make the led blink x time
//You have to call the step one of the animation before calling
//any of the 2,3,4,5 step
//You have to call step six before calling step seven for the first time
//Step seven turns off the leds at the end of the animation
////returns True when the step of the animation is done
bool Animations_slightRight_stepOne(bool reset, uint8_t, uint8_t);

bool Animations_slightRight_stepTwo(bool reset, uint8_t, uint8_t);

bool Animations_slightRight_stepThree(bool reset, uint8_t, uint8_t);

bool Animations_slightRight_stepFour(bool reset, uint8_t, uint8_t);

bool Animations_slightRight_stepFive(bool reset, uint8_t, uint8_t);

bool Animations_slightRight_stepSix(bool reset, uint8_t, uint8_t);

bool Animations_slightRight_stepSeven(bool reset, uint8_t, uint8_t);

bool Animations_slightRight_turnfailed(bool reset, uint8_t, uint8_t);

//SLIGHTLEFT
//7 steps of animation
//Call the seventh step x times to make the led blink x time
//You have to call the step one of the animation before calling
//any of the 2,3,4,5 step
//You have to call step six before calling step seven for the first time
//Step seven turns off the leds at the end of the animation
////returns True when the step of the animation is done
bool Animations_slightLeft_stepOne(bool reset, uint8_t, uint8_t);

bool Animations_slightLeft_stepTwo(bool reset, uint8_t, uint8_t);

bool Animations_slightLeft_stepThree(bool reset, uint8_t, uint8_t);

bool Animations_slightLeft_stepFour(bool reset, uint8_t, uint8_t);

bool Animations_slightLeft_stepFive(bool reset, uint8_t, uint8_t);

bool Animations_slightLeft_stepSix(bool reset, uint8_t, uint8_t);

bool Animations_slightLeft_stepSeven(bool reset, uint8_t, uint8_t);

bool Animations_slightLeft_turnfailed(bool reset, uint8_t, uint8_t);

//RIGHT
//10 steps of animation
//Call the ten step x times to make the led blink x time
//You have to call the step one of the animation before calling
//any of the 2,3,4,5,6,7,8 step
//You have to call step 9 before calling step 10 for the first time
//Step 10 turns off the leds at the end of the animation
////returns True when the step of the animation is done
bool Animations_right_stepOne(bool reset, uint8_t, uint8_t);

bool Animations_right_stepTwo(bool reset, uint8_t, uint8_t);

bool Animations_right_stepThree(bool reset, uint8_t, uint8_t);

bool Animations_right_stepFour(bool reset, uint8_t, uint8_t);

bool Animations_right_stepFive(bool reset, uint8_t, uint8_t);

bool Animations_right_stepSix(bool reset, uint8_t, uint8_t);

bool Animations_right_stepSeven(bool reset, uint8_t, uint8_t);

bool Animations_right_stepEight(bool reset, uint8_t, uint8_t);

bool Animations_right_stepNine(bool reset, uint8_t, uint8_t);

bool Animations_right_stepTen(bool reset, uint8_t, uint8_t);

bool Animations_right_turnfailed(bool reset, uint8_t, uint8_t);

//LEFT
//10 steps of animation
//Call the ten step x times to make the led blink x time
//You have to call the step one of the animation before calling
//any of the 2,3,4,5,6,7,8 step
//You have to call step 9 before calling step 10 for the first time
//Step 10 turns off the leds at the end of the animation
////returns True when the step of the animation is done
bool Animations_left_stepOne(bool reset, uint8_t, uint8_t);

bool Animations_left_stepTwo(bool reset, uint8_t, uint8_t);

bool Animations_left_stepThree(bool reset, uint8_t, uint8_t);

bool Animations_left_stepFour(bool reset, uint8_t, uint8_t);

bool Animations_left_stepFive(bool reset, uint8_t, uint8_t);

bool Animations_left_stepSix(bool reset, uint8_t, uint8_t);

bool Animations_left_stepSeven(bool reset, uint8_t, uint8_t);

bool Animations_left_stepEight(bool reset, uint8_t, uint8_t);

bool Animations_left_stepNine(bool reset, uint8_t, uint8_t);

bool Animations_left_stepTen(bool reset, uint8_t, uint8_t);

bool Animations_left_turnfailed(bool reset, uint8_t, uint8_t);


//HARDRIGHT
//7 steps of animation
//Call the seventh step x times to make the led blink x time
//You have to call the step one of the animation before calling
//any of the 2,3,4,5 step
//You have to call step six before calling step seven for the first time
//Step seven turns off the leds at the end of the animation
////returns True when the step of the animation is done
bool Animations_hardRight_stepOne(bool reset, uint8_t, uint8_t);

bool Animations_hardRight_stepTwo(bool reset, uint8_t, uint8_t);

bool Animations_hardRight_stepThree(bool reset, uint8_t, uint8_t);

bool Animations_hardRight_stepFour(bool reset, uint8_t, uint8_t);

bool Animations_hardRight_stepFive(bool reset, uint8_t, uint8_t);

bool Animations_hardRight_stepSix(bool reset, uint8_t, uint8_t);

bool Animations_hardRight_stepSeven(bool reset, uint8_t, uint8_t);

bool Animations_hardRight_turnfailed(bool reset, uint8_t, uint8_t);


//HARDLEFT
//7 steps of animation
//Call the seventh step x times to make the led blink x time
//You have to call the step one of the animation before calling
//any of the 2,3,4,5 step
//You have to call step six before calling step seven for the first time
//Step seven turns off the leds at the end of the animation
////returns True when the step of the animation is done
bool Animations_hardLeft_stepOne(bool reset, uint8_t, uint8_t);

bool Animations_hardLeft_stepTwo(bool reset, uint8_t, uint8_t);

bool Animations_hardLeft_stepThree(bool reset, uint8_t, uint8_t);

bool Animations_hardLeft_stepFour(bool reset, uint8_t, uint8_t);

bool Animations_hardLeft_stepFive(bool reset, uint8_t, uint8_t);

bool Animations_hardLeft_stepSix(bool reset, uint8_t, uint8_t);

bool Animations_hardLeft_stepSeven(bool reset, uint8_t, uint8_t);

bool Animations_hardLeft_turnfailed(bool reset, uint8_t, uint8_t);


//UTURN
//UTURN has a first call function lights the LEDS gradually
//Call this function x times to make the leds blink x time
//returns True when the animation is done
bool Animations_uTurn_first_call(bool reset, uint8_t, uint8_t);
bool Animations_uTurn(bool reset, uint8_t, uint8_t);

//DEACTIVATE ALARM
bool Animations_deactivate_alarm(bool reset, uint8_t, uint8_t);

//DESTINATION
//returns True when the animation is done
bool Animations_destination(bool reset, uint8_t, uint8_t);

//DISCONNECTED
bool Animations_disconnected(bool reset, uint8_t null_1, uint8_t null_2);

//PAIRING
//Call this animation after calling start_pairing!
//returns True when the animation is done
bool Animations_pairing(bool reset, uint8_t, uint8_t);

//PAIRINGFADE
//
//returns True when the animation is done
bool Animations_pairingFade(bool reset, uint8_t, uint8_t);

//ALARM
//2 steps of animation
// First : lights the circle of leds one by one in red (the leds are still on at the end of
//the animation)
// Second :	Makes the LEDs blink (the leds are off at the end of the animation, blinking starts
//by turning off the leds)
//Call the second step x times to make the led blink x time
bool Animations_alarm_red_circle (bool reset, uint8_t, uint8_t);
bool Animations_alarm_blinking (bool reset, uint8_t, uint8_t);

//WRONGTAPCODE
//plays the alarm animation, but slower..
bool Animations_wrongTapCode(bool reset, uint8_t, uint8_t);

//LONGTAP
bool Animations_longTap(bool reset, uint8_t, uint8_t);

//SMARTHALO ON CHARGE
//Indicates to the user the percentage of charge of the battery
bool Animations_smarthalo_on_charge(bool reset, uint8_t, uint8_t);

//FIRST CONNECTION
//Play the first connection animation
//Call back the wait for onboarding animation to make the lights flash
bool Animations_first_connection(bool reset, uint8_t, uint8_t);

bool Animations_first_connection_wait_for_onboarding(bool reset, uint8_t, uint8_t);

//SPEEDOMETER

bool SH_set_new_speed_max_and_min(uint8_t new_speed_min, uint8_t new_speed_max);

uint8_t SH_get_speed_max_speedometer();

uint8_t SH_get_speed_min_speedometer();

bool Animations_speedometer (bool reset, uint8_t speed, uint8_t null_2);

//CADENCE
bool Animations_cadence (bool reset, uint8_t speed_goal, uint8_t actual_speed);

//AS THE CROW FLIES
bool Animations_as_the_crow_flies(bool reset, uint8_t percentage_of_led_to_light, uint8_t percentage_of_distance_to_destination);

//REROUTING
bool Animations_rerouting(bool reset, uint8_t null_1, uint8_t null_2);

//GOALCOMPLETION
//@ param completion : Percentage of goal completion
bool Animations_goalCompletion(bool reset, uint8_t, uint8_t);


#endif /* SH_FIRMWARE_CODE_SH_ANIMATIONS_H_ */

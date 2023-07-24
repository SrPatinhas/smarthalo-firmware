/*
 * SH_Animations.c
 *
 *  Created on: 2016-06-07
 *      Author: SmartHalo
 */


#include "SH_Animations.h"
#include "stdafx.h"
#include "SH_HaloPixelnRF.h"
#include "nrf_soc.h"
#include "SH_Temperature_monitoring.h"
#include "SH_Animations_colors.h"
#include "SH_Primary_state_machine.h"

#ifdef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
#define LESS_LEDS_FOR_ANIMATIONS			true
#else
#define LESS_LEDS_FOR_ANIMATIONS			false
#endif
#define MAX_NUMBER_OF_BRIGHTNESS_CHANGES			64
#define DIVISION_FOR_NUMBER_OF_BRIGHTNESS_CHANGES 	3//(unsigned int) ((0.125 * DELAY_BETWEEN_REFRESH_OF_ANIMATIONS) + 2.75)
#define BRIGHTNESS_MULTIPLICATOR  				(unsigned int) ((BRIGHTNESS_MAX+1)/NUMBER_OF_BRIGHTNESS_CHANGES) //8*32 = 256, 256 = Max Brightness
#define NUMBER_OF_BRIGHTNESS_CHANGES			(unsigned int)((MAX_NUMBER_OF_BRIGHTNESS_CHANGES+(-0.77*(DELAY_BETWEEN_REFRESH_OF_ANIMATIONS-1)))/DIVISION_FOR_NUMBER_OF_BRIGHTNESS_CHANGES)

#define MOD(a,b) ((((a)%(b))+(b))%(b))

#ifdef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
#define NOON			23
#define NOON_PLUS_ONE	1
#define EIGHTH         	2
#define EIGHTH_PLUS_ONE	4
#define QUARTER			5
#define QUARTER_PLUS_ONE   7
#define HALF           	11
#define HALF_PLUS_ONE	13
#define LQUARTER		17
#define LQUARTER_PLUS_ONE 19
#define NUMBER_OF_LEDS_HALF_HALO	11
#define NUMBER_OF_LEDS_QUARTER_HALO	5
#else
#define NOON						24
#define NOON_PLUS_ONE				0
#define EIGHTH         				3
#define EIGHTH_PLUS_ONE				3
#define QUARTER						6
#define QUARTER_PLUS_ONE  			 6
#define HALF           				12
#define HALF_PLUS_ONE				12
#define LQUARTER					18
#define LQUARTER_PLUS_ONE 18
#define NUMBER_OF_LEDS_HALF_HALO	13
#define NUMBER_OF_LEDS_QUARTER_HALO	7
#endif


#define NUMBER_OF_ITERATIONS_PER_BRIGHTNESS_CHANGES	5

#define NUMBER_OF_ITERATIONS_HALO_BEFORE_LIGHTING_THE_OTHER_SET_OF_LED	(unsigned int) (NUMBER_OF_BRIGHTNESS_CHANGES/NUMBER_OF_ITERATIONS_PER_BRIGHTNESS_CHANGES) // a modifier avec number_of_brightness_changes
#define NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(number_of_leds_to_light) ((number_of_leds_to_light/2 * NUMBER_OF_ITERATIONS_HALO_BEFORE_LIGHTING_THE_OTHER_SET_OF_LED)+NUMBER_OF_BRIGHTNESS_CHANGES)	// a modifier avec number_of_brightness_changes
#define NUMBER_MAX_OF_SETS_TO_LIGHT_HALF_HALO		6
#define NUMBER_MAX_OF_SETS_TO_LIGHT_QUARTER_HALO	3

#define CALCULATE_NUMBER_OF_ITERATIONS(number_of_leds_to_light) ((number_of_leds_to_light*NUMBER_OF_ITERATIONS_HALO_BEFORE_LIGHTING_THE_OTHER_SET_OF_LED)+NUMBER_OF_BRIGHTNESS_CHANGES)
#define CALCULATE_NUMBER_OF_SETS_TO_LIGHT(led_max,led_min) ((MOD((int)(led_max-led_min), NUMBER_OF_LEDS)/2) + 1)

#define TIME_THAT_THE_CIRCLE_HAS_TO_FILL_UP_FOR_ALARM 	TIMER_FOR_ALARM_RED_CIRCLE
#define DELAY_BETWEEN_REFRESH_OF_ANIMATIONS				TIMER_FOR_ANIMATION_TIME_BETWEEN_CONSECUTIVE_COMPARE_EVENTS
#define CALCULATE_REFRESH_RATE_OF_ANIMATIONS(speed) 	(unsigned int) ((speed * (NUMBER_OF_BRIGHTNESS_CHANGES/(MAX_NUMBER_OF_BRIGHTNESS_CHANGES/DIVISION_FOR_NUMBER_OF_BRIGHTNESS_CHANGES))) + 1)
#define CALCULATE_TIME_OF_PAUSE(time) 	(unsigned int) ((time / DELAY_BETWEEN_REFRESH_OF_ANIMATIONS))
#define NUMBER_OF_BRIGHTESS_CHANGES_PAIRING	6

#define PAUSE_FOR_SUCCESS		CALCULATE_TIME_OF_PAUSE(250)
#define PAUSE_WRONG_TAP_CODE	CALCULATE_TIME_OF_PAUSE(500)
#define PAUSE_ALARM_RED_CIRCLE	CALCULATE_TIME_OF_PAUSE((unsigned int) TIME_THAT_THE_CIRCLE_HAS_TO_FILL_UP_FOR_ALARM/NUMBER_OF_LEDS)
#define PAUSE_1_ALARM_BLINK		CALCULATE_TIME_OF_PAUSE(200)
#define PAUSE_2_ALARM_BLINK		CALCULATE_TIME_OF_PAUSE(400)
#define PAUSE_BEFORE_MOVING_LIGHTS_FOR_PAIRING	(unsigned int) ((-(7/41) * DELAY_BETWEEN_REFRESH_OF_ANIMATIONS) + 6 )
#define WHITE_PAUSE_PAIRING		CALCULATE_TIME_OF_PAUSE(50)

#define ARRAY_LEFT_LED_AS_THE_CROW_FLIES 0
#define ARRAY_CENTER_LED_AS_THE_CROW_FLIES 1
#define ARRAY_RIGHT_LED_AS_THE_CROW_FLIES 2
#define ARRAY_FOURTH_LED_AS_THE_CROW_FLIES 3
#define NUMBER_OF_LEDS_AS_THE_CROW_FLIES 4
#define BRIGHTNESS_CHANGES_FOR_AS_THE_CROW_FLIES(number_of_leds_to_light) (0.07/NUMBER_OF_LEDS * number_of_leds_to_light)
#define BRIGHTNESS_CHANGES_FOR_AS_THE_CROW_FLIES_FIRST_CALL 0.0375

#define LED_MIN_SPEEDOMETER 3
#define LED_MAX_SPEEDOMETER 21
#define NUMBER_OF_LEDS_SPEEDOMETER 19

#define NUMBER_OF_FLASH_FOR_DESTINATION 5
#define NUMBER_OF_FLASH_WRONG_TAP_CODE	5
#define NUMBER_OF_FLASH_FOR_DISCONNECTED 3
#define NUMBER_OF_FLASH_SMARTHALO_ON_CHARGE	4
#define NUMBER_OF_FLASH_GOAL_COMPLETION	1

#define BATTERY_ON_CHARGE_FIRST_QUARTER		25
#define BATTERY_ON_CHARGE_SECOND_QUARTER	50
#define BATTERY_ON_CHARGE_THIRD_QUARTER		75
#define	BATTERY_ON_CHARGE_LAST_QUARTER		100

#define LED_GOAL_CADENCE 12

const float PAIRING_MASK_RGBCOLOR [NUMBER_OF_BRIGHTESS_CHANGES_PAIRING] = {1, 0.75, 0.50, 0.25, 0.10, 0};


typedef enum
{	SLOWEST = CALCULATE_REFRESH_RATE_OF_ANIMATIONS(20),
	SLOW = CALCULATE_REFRESH_RATE_OF_ANIMATIONS(10),
	MED_SLOW = CALCULATE_REFRESH_RATE_OF_ANIMATIONS(6),
	MED = CALCULATE_REFRESH_RATE_OF_ANIMATIONS(4),
	MED_FAST = CALCULATE_REFRESH_RATE_OF_ANIMATIONS(2),
	FAST = CALCULATE_REFRESH_RATE_OF_ANIMATIONS(1)
}speed_of_animations;


static uint8_t red_pixel = FALSE;
static uint8_t green_pixel = FALSE;
static uint8_t blue_pixel = FALSE;

static uint8_t brightness = 255;

static uint8_t speed_min_speedometer = 0;
static uint8_t speed_max_speedometer = 50;


/*Private function prototypes */

/*ALL LIGHTS ON
 * Lights all the LED between led_max and led_min by increasing the brightness
 * Lights the LED in the RGB colors defined by r, g, and b.
 * brightness_level is the brightness level the LEDs have to be set to
*/
static void allLightsOn(uint led_max, uint led_min, float r, float g, float b,int level_of_brightness);

/*ALL LIGHTS ON HALO
 * Lights all the LED between led_max and led_min by increasing the brightness
 * Lights the LED in the RGB colors defined by r, g, and b.
 * brightness_level is the brightness level the LEDs have to be set to
 *
*/
static void allLightsOn_Halo(uint led_max, uint led_min, uint r,uint g, uint b,int level_of_brightness, uint8_t number_of_iterations);

/*ALL LIGHTS ON QUARTER HALO
 * Lights all the LED between led_max and led_min by increasing brightness
 * each led doesn't have the same brightness
 * Lights the LED in the RGB colors defined by r, g, and b.
 * brightness_level is the brightness level the LEDs have to be set to
*/
//static void allLightsOn_Halo(uint led_max, uint led_min, uint r,uint g, uint b,int level_of_brightness, uint8_t number_of_iterations);

/*ALL LIGHTS ON NUMBER OF LEDS HALO
 * Lights the number of LED wanted between led_max and led_min by increasing brightness
 * each led doesn't have the same brightness
 * Lights the LED in the RGB colors defined by r, g, and b.
 * brightness_level is the brightness level the LEDs have to be set to
*/
static void allLightsOn_number_of_LEDs_Halo(uint8_t total_number_of_led_to_light, float r,float g, float b,int number_of_iterations);


/*ALL LIGHTS OFF
 * Turns off all the LED between led_max and led_min by decreasing the brightness
 * The LEDs have the RGB colors defined by r, g, and b.
 * brightness_level is the brightness level the LEDs have to be set to
*/
static void allLightsOff(uint8_t led_max, uint8_t led_min, float r, float g, float b, int brightness_level);

/*LIGHTS ONE BY ONE
 * Lights two (or three) led between led_max and led_min
 * Depending on the step of animation
 * Lights the LED in the RGB colors defined by r, g, and b.
 * brightness_level is the brightness level the LEDs have to be set to
*/
static void lightsOneByOneGreen(uint led_max, uint led_min,uint step_of_animation, int brightness_level);

static bool step_of_animations_turn_by_turn(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light,uint8_t set_of_leds_to_light);

static void lightsGreen(uint led_max, uint led_min,uint step_of_animation, int brightness_level);

static bool animation_flash_the_leds_on_off(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light,float red_pixel_color, float green_pixel_color, float blue_pixel_color);

static bool animation_flash_the_leds_off_on(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light,float red_pixel_color, float green_pixel_color, float blue_pixel_color);

static bool animation_dim_the_leds_off(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light, float red_pixel_color, float green_pixel_color, float blue_pixel_color);

static bool animation_dim_the_leds_on(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light, float red_pixel_color, float green_pixel_color, float blue_pixel_color);

/*SET WHICH LED TO LIGHT AS THE CROW FLIES
 * Depending on the percentage of led to light (heading for as the crow flies)
 * Moves the leds that are already open to point in the right direction
 * Returns true when the leds position and brightness are set
 */
static bool set_which_led_to_light_as_the_crow_flies (uint8_t number_of_the_leds_to_light[], float previous_percentage_of_led_to_light, uint8_t goal_percentage_of_led_to_light, float brightness_of_the_leds []);

/*SET NEW COLORS FOR DESTINATION AS THE CROW FLIES
 * Depending on the previous percentage to destination and the current percentage to destination
 * Changes the color of the LEDs until it represent the right percentage to destination
 */
static bool set_new_colors_for_destination_as_the_crow_flies (uint8_t pixel_as_the_crow_flies [],  uint8_t goal_percentage_to_destination, uint8_t * previous_percentage_to_destination);

static bool Animations_as_the_crow_flies_first_call(uint8_t number_of_the_leds_to_light [], float brightness_of_the_leds [], uint8_t pixel_as_the_crow_flies []);

static bool Animations_speedometer_first_call();

static bool as_the_crow_flies_is_going_clockwise (uint8_t goal_center_led, uint8_t current_center_led);

static bool turn_as_the_crow_flies_clockwise(uint8_t number_of_the_leds_to_light[], float brightness_changes, float brightness_of_the_leds []);

static bool turn_as_the_crow_flies_counter_clockwise(uint8_t number_of_the_leds_to_light[], float brightness_changes, float brightness_of_the_leds []);

static bool set_as_the_crow_flies_heading_leds(uint8_t number_of_the_leds_to_light[], float brightness_changes, float brightness_of_the_leds [], float exact_number_of_led_to_light);

static void set_Speedometer_Colors(float pixel_for_the_led [], uint8_t led_to_light);

static bool decrease_cadence (uint8_t led_to_light, uint8_t current_led);

static bool increase_cadence (uint8_t led_to_light, uint8_t current_led);

static bool Animations_cadence_first_call ();

/* Sets the colors of the Smarthalo
 * but it does not show the colors on the LEDs
 */
static void set_SmartHalo_Colors();

//Set SH new brightness
void SH_halo_set_new_brightness(uint8_t new_brightness){
	#ifndef TEST_HALO_BOX
	brightness = (uint8_t)(new_brightness * led_brightness_correction_for_temperature());
	#else
	brightness = new_brightness;
	#endif
	HaloPixel_setBrightness(brightness);
}

//Get brightness of the smarthalo
uint8_t SH_halo_get_current_brightness(void){
	return brightness;
}

//turn off all the lights
bool turnOffAllLEDs(bool reset, uint8_t parameter_1, uint8_t parameter_2){

	HaloPixel_clear();
	HaloPixel_show();
	return true;
}

//Lights all the LEDS between led_max and led_min in the same color, increasing brightness
static void allLightsOn(uint led_max, uint led_min, float r, float g, float b,int level_of_brightness){

	if(level_of_brightness >=NUMBER_OF_BRIGHTNESS_CHANGES || ((level_of_brightness + 1) * BRIGHTNESS_MULTIPLICATOR >= BRIGHTNESS_MAX)){
		level_of_brightness = BRIGHTNESS_MAX;
	}

	else{
		level_of_brightness = level_of_brightness * BRIGHTNESS_MULTIPLICATOR;
	}

    if (led_max>=led_min){

    	for (int led=led_min; led<=led_max;led++){
    		HaloPixel_setPixelColor(led%NUMBER_OF_LEDS,r*level_of_brightness,g*level_of_brightness,b*level_of_brightness);
    	}
    	HaloPixel_show();
    }

    //If we have to light the slight left side of the Halo
    else{
    	for (uint i=0; i<=(led_min - led_max) ;i++){
    		int led = MOD((led_min+i),NUMBER_OF_LEDS);
    		HaloPixel_setPixelColor(led,r*level_of_brightness,g*level_of_brightness,b*level_of_brightness);
    	}
    	HaloPixel_show();
    }

}

static void allLightsOn_Halo(uint led_max, uint led_min, uint r,uint g, uint b,int level_of_brightness, uint8_t number_of_iterations){

	uint8_t number_of_sets_of_led_to_light = number_of_iterations/NUMBER_OF_ITERATIONS_HALO_BEFORE_LIGHTING_THE_OTHER_SET_OF_LED;

	uint8_t max_number_of_sets_to_light = CALCULATE_NUMBER_OF_SETS_TO_LIGHT(led_max, led_min);

	if (number_of_sets_of_led_to_light>=max_number_of_sets_to_light){
		number_of_sets_of_led_to_light = max_number_of_sets_to_light - 1;
	}

	uint8_t led_1;
	int8_t led_2;

	int temp_level_of_brightness = level_of_brightness;

	for (uint8_t i=0; i<=number_of_sets_of_led_to_light;i++){

		if (led_max>led_min){
			led_1 = (led_min + i)% NUMBER_OF_LEDS;
			led_2 = MOD((led_max-i), NUMBER_OF_LEDS);
		}

		else{
			led_1 = MOD((led_max - i), NUMBER_OF_LEDS);
			led_2 = MOD(((led_min+i)), NUMBER_OF_LEDS);
		}

		HaloPixel_setBrightness(brightness);
		temp_level_of_brightness = (level_of_brightness - (NUMBER_OF_ITERATIONS_HALO_BEFORE_LIGHTING_THE_OTHER_SET_OF_LED * i));

		if (temp_level_of_brightness >= NUMBER_OF_BRIGHTNESS_CHANGES || ((temp_level_of_brightness + 1) * BRIGHTNESS_MULTIPLICATOR >= BRIGHTNESS_MAX)){
			temp_level_of_brightness = BRIGHTNESS_MAX;
		}
		else{
			temp_level_of_brightness = temp_level_of_brightness * BRIGHTNESS_MULTIPLICATOR;
		}

		HaloPixel_setPixelColor(led_1%NUMBER_OF_LEDS, r*temp_level_of_brightness,g*temp_level_of_brightness,b*temp_level_of_brightness);
		HaloPixel_setPixelColor(led_2%NUMBER_OF_LEDS, r*temp_level_of_brightness,g*temp_level_of_brightness,b*temp_level_of_brightness);
    }

	HaloPixel_show();

}

static void allLightsOn_number_of_LEDs_Halo(uint8_t total_number_of_led_to_light, float r,float g, float b,int number_of_iterations){

	HaloPixel_setBrightness(brightness);
	uint8_t number_of_led_to_light_with_iterations = number_of_iterations/NUMBER_OF_ITERATIONS_HALO_BEFORE_LIGHTING_THE_OTHER_SET_OF_LED;
	int temp_level_of_brightness = number_of_iterations;

	if (number_of_led_to_light_with_iterations>total_number_of_led_to_light) {
		number_of_led_to_light_with_iterations = total_number_of_led_to_light;
	}

	for(uint8_t led = 0; led<=number_of_led_to_light_with_iterations; led++){
		if (led<total_number_of_led_to_light){
			temp_level_of_brightness = number_of_iterations;
			temp_level_of_brightness = temp_level_of_brightness - (led*NUMBER_OF_ITERATIONS_HALO_BEFORE_LIGHTING_THE_OTHER_SET_OF_LED);
			if (temp_level_of_brightness >= NUMBER_OF_BRIGHTNESS_CHANGES || ((temp_level_of_brightness + 1) * BRIGHTNESS_MULTIPLICATOR >= BRIGHTNESS_MAX)){
				temp_level_of_brightness = BRIGHTNESS_MAX;
			}
			else if (temp_level_of_brightness < 0){
				temp_level_of_brightness = 0;
			}
			else{
				temp_level_of_brightness = temp_level_of_brightness * BRIGHTNESS_MULTIPLICATOR;
			}
			HaloPixel_setPixelColor(led, r*temp_level_of_brightness, g*temp_level_of_brightness, b*temp_level_of_brightness);
		}
	}
	HaloPixel_show();
}

//Turns off all the LEDS between led_max and led_min in the same color, decreasing brightness
static void allLightsOff(uint8_t led_max, uint8_t led_min, float r, float g, float b, int brightness_level){

	for (int led=led_min; led<=led_max;led++){

		HaloPixel_setPixelColor(led%NUMBER_OF_LEDS, r*BRIGHTNESS_MULTIPLICATOR*brightness_level, g*BRIGHTNESS_MULTIPLICATOR*brightness_level, b*BRIGHTNESS_MULTIPLICATOR*brightness_level);

	}
	HaloPixel_show();
}

//Lights the right set of LEDs for the animation
static void lightsOneByOneGreen(uint led_max, uint led_min,uint step_of_animation, int brightness_level){

	uint8_t led_1 = led_min + step_of_animation;
    int led_2 = (led_max-step_of_animation) % NUMBER_OF_LEDS;

	if((brightness_level+1) * BRIGHTNESS_MULTIPLICATOR <= BRIGHTNESS_MAX){
		//Lights the LEDS two by two (one on each side of the "halo")
		HaloPixel_setColor(led_1, HaloPixel_RGB_Color(BRIGHTNESS_MAX - (brightness_level*BRIGHTNESS_MULTIPLICATOR),BRIGHTNESS_MAX,BRIGHTNESS_MAX - (brightness_level*BRIGHTNESS_MULTIPLICATOR)));
		HaloPixel_setColor(led_2, HaloPixel_RGB_Color(BRIGHTNESS_MAX - (brightness_level*BRIGHTNESS_MULTIPLICATOR),BRIGHTNESS_MAX,BRIGHTNESS_MAX - (brightness_level*BRIGHTNESS_MULTIPLICATOR)));
	}
	else{
		HaloPixel_setColor(led_1, HaloPixel_RGB_Color(0,BRIGHTNESS_MAX,0));
		HaloPixel_setColor(led_2, HaloPixel_RGB_Color(0,BRIGHTNESS_MAX,0));

	}

	HaloPixel_show();
}

//Lights the right set of LEDs for the animation
static void lightsGreen(uint led_max, uint led_min,uint step_of_animation, int brightness_level){

	for(uint8_t i = 0;i<=step_of_animation; i++){

        uint led_1 = led_min + i;
        int led_2 = (led_max-i) % NUMBER_OF_LEDS;

       //Lights the LEDS two by two (one on each side of the "halo")
    	HaloPixel_setColor(led_1, HaloPixel_RGB_Color(0,BRIGHTNESS_MAX,0));
    	HaloPixel_setColor(led_2, HaloPixel_RGB_Color(0,BRIGHTNESS_MAX,0));

   }

    	HaloPixel_show();
}

//STEP_OF_ANIMATION_TURN_BY_TURN
static bool step_of_animations_turn_by_turn(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light,uint8_t set_of_leds_to_light){

	static uint16_t counter_animation_step = 0;
	static uint8_t level_of_brightness = 0;

	if (!reset){
		//Lights the right sets of LEDs in green for step two and increases brightness
		if (counter_animation_step % FAST == 0){
			lightsOneByOneGreen(led_max_to_light, led_min_to_light, set_of_leds_to_light, level_of_brightness);
			level_of_brightness++;
		}

		//Returns true when the animation is done
		if ((level_of_brightness*BRIGHTNESS_MULTIPLICATOR) > BRIGHTNESS_MAX){
			level_of_brightness=0;
			counter_animation_step=0;
			return true;
		}

		counter_animation_step++;
		return false;
	}

	else{
		level_of_brightness=0;
		counter_animation_step=0;
		return true;
	}

}

//ANIMATION_FLASH ON_OFF
static bool animation_flash_the_leds_on_off(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light,float red_pixel_color, float green_pixel_color, float blue_pixel_color){

	static uint16_t counter_flash = 0;
	static bool fades_in_is_done = false;
	static bool fades_out_is_done = false;

	if (!reset){

		HaloPixel_setBrightness(brightness);

		//Dim the LEDs in green the fades out
		if (counter_flash%FAST == 0){

			if(!fades_in_is_done){
				fades_in_is_done = animation_dim_the_leds_on(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
			}

			else {
				fades_out_is_done = animation_dim_the_leds_off(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
			}
		}
		//Returns true when the animation is done
		if (fades_out_is_done){
			fades_out_is_done = false;
			fades_in_is_done = false;
			turnOffAllLEDs(0,0,0);
			counter_flash=0;
			return true;
		}

		counter_flash++;
		return false;
	}

	else{
		fades_in_is_done = animation_dim_the_leds_on(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
		fades_out_is_done = animation_dim_the_leds_off(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
		turnOffAllLEDs(0,0,0);
		fades_out_is_done = false;
		fades_in_is_done = false;
		counter_flash=0;
		return true;
	}

}

//ANIMATION_FLASH OFF_ON
static bool animation_flash_the_leds_off_on(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light,float red_pixel_color, float green_pixel_color, float blue_pixel_color){

	static uint16_t counter_flash = 0;
	static bool fades_in_is_done = false;
	static bool fades_out_is_done = false;

	if (!reset){

		HaloPixel_setBrightness(brightness);

		//Dim the LEDs in green the fades out
		if (counter_flash%FAST == 0){

			if(!fades_out_is_done){
				fades_out_is_done = animation_dim_the_leds_off(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
			}

			else {
				fades_in_is_done = animation_dim_the_leds_on(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
			}

		}
		//Returns true when the animation is done
		if (fades_in_is_done){
			fades_out_is_done = false;
			fades_in_is_done = false;
			counter_flash=0;
			return true;
		}

		counter_flash++;
		return false;
	}

	else{
		fades_in_is_done = animation_dim_the_leds_on(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
		fades_out_is_done = animation_dim_the_leds_off(reset, led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color);
		turnOffAllLEDs(0,0,0);
		fades_out_is_done = false;
		fades_in_is_done = false;
		counter_flash=0;
		return true;
	}

}

//LEDS DIMS OFFace

static bool animation_dim_the_leds_off(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light, float red_pixel_color, float green_pixel_color, float blue_pixel_color){

	static uint8_t level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES-1;

	if (!reset){
		if (level_of_brightness_fade_out > 0){
			allLightsOff(led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color, level_of_brightness_fade_out);
			level_of_brightness_fade_out--;
			return false;
		}

		else{
			level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES-1;
			return true;
		}
	}

	else{
		level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES-1;
		return true;
	}

}

//LEDS DIMS ON
static bool animation_dim_the_leds_on(bool reset, uint8_t led_max_to_light, uint8_t led_min_to_light, float red_pixel_color, float green_pixel_color, float blue_pixel_color){

	static uint8_t level_of_brightness_fade_in = 0;

	if (!reset){
		if (level_of_brightness_fade_in < NUMBER_OF_BRIGHTNESS_CHANGES){
			allLightsOn(led_max_to_light, led_min_to_light, red_pixel_color, green_pixel_color, blue_pixel_color, level_of_brightness_fade_in);
			level_of_brightness_fade_in++;
			return false;
		}

		else{
			level_of_brightness_fade_in = 0;
			return true;
		}
	}

	else{
		level_of_brightness_fade_in = 0;
		return true;
	}

}

//STRAIGHT
bool Animations_straight(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;
	static uint8_t level_of_brightness = 0;
	static uint16_t counter_straight_animation = 0;
	static uint8_t number_of_iterations_straight_animation = 0;

	if (!reset) {
		HaloPixel_setBrightness(brightness);

		//Increase brightness until brightness gets to max brightness
		if (counter_straight_animation%MED == 0){
			if (number_of_iterations_straight_animation <= NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_HALF_HALO)){
				allLightsOn_Halo(QUARTER, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel, level_of_brightness,number_of_iterations_straight_animation);
				number_of_iterations_straight_animation++;
				level_of_brightness++;
			}

			//Lights fades out
			else if (animation_dim_the_leds_off(reset, QUARTER, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
				//Return true when the animation is done
				level_of_brightness = 0;
				counter_straight_animation=0;
				number_of_iterations_straight_animation = 0;
				turnOffAllLEDs(0,0,0);
				return true;
			}
		}
		counter_straight_animation++;
		return false;
	}

	else{
		animation_dim_the_leds_off(reset, QUARTER, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		level_of_brightness = 0;
		counter_straight_animation=0;
		number_of_iterations_straight_animation = 0;
		turnOffAllLEDs(0,0,0);
		return true;
	}

}

/*
 * SLIGHTRIGHT
 */

//STEPONE
bool Animations_slightRight_stepOne(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	static uint16_t counter_slightright_stepone = 0;
	static uint8_t level_of_brightness = 0;
	static uint8_t number_of_iterations_slightRight_animation = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		//Lights the LEDs to indicate direction in white by increasing brightness
		if (counter_slightright_stepone%MED_FAST == 0){
			allLightsOn_Halo(QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel,level_of_brightness,number_of_iterations_slightRight_animation);
			level_of_brightness++;
			number_of_iterations_slightRight_animation++;
		}

		//Returns true when the animation is done
		if (number_of_iterations_slightRight_animation > NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_QUARTER_HALO)){
			level_of_brightness=0;
			counter_slightright_stepone=0;
			number_of_iterations_slightRight_animation = 0;
			return true;
		}

		counter_slightright_stepone++;
		return false;
	}

	else{
		level_of_brightness=0;
		counter_slightright_stepone=0;
		number_of_iterations_slightRight_animation = 0;
		turnOffAllLEDs(0,0,0);
		return true;
	}
}

//STEPTWO
bool Animations_slightRight_stepTwo(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 0;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(QUARTER-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES - 1);
	}

		is_over = step_of_animations_turn_by_turn(reset, QUARTER, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;


}

//STEPTHREE
bool Animations_slightRight_stepThree(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 1;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(QUARTER-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(QUARTER, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES-1);

	}
	is_over = step_of_animations_turn_by_turn(reset, QUARTER, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;

}

//STEPFOUR
bool Animations_slightRight_stepFour(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 2;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (!LESS_LEDS_FOR_ANIMATIONS){
		allLightsOn(QUARTER-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES - 1);
		}
		lightsGreen(QUARTER, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES-1);
	}

	is_over = step_of_animations_turn_by_turn(reset, QUARTER, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}

#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
//STEPFIVE
bool Animations_slightRight_stepFive(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 3;
	static bool is_over;

	red_pixel = 0;
	green_pixel = 1;
	blue_pixel = 0;

	if(!reset){
		HaloPixel_setBrightness(brightness);
		lightsGreen(QUARTER, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, QUARTER, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;

}
//STEPSIX
bool Animations_slightRight_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		if (animation_dim_the_leds_off(reset, QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else{
		animation_dim_the_leds_off(reset, QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}
//STEPSEVEN
bool Animations_slightRight_stepSeven(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}
#else
//STEPFIVE
bool Animations_slightRight_stepFive(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		if (animation_dim_the_leds_off(reset, QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else{
		animation_dim_the_leds_off(reset, QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}

//STEPSIX
bool Animations_slightRight_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}
#endif


//TURN FAILED
bool Animations_slightRight_turnfailed(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = RED_PIXEL_RED/BRIGHTNESS_MAX;
	green_pixel = GREEN_PIXEL_RED/BRIGHTNESS_MAX;
	blue_pixel = BLUE_PIXEL_RED/BRIGHTNESS_MAX;

	if (animation_flash_the_leds_on_off(reset, QUARTER, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

/*
 * RIGHT
 */

//STEPONE
bool Animations_right_stepOne(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	static uint16_t counter_right_stepone = 0;
	static uint8_t level_of_brightness = 0;
	static uint8_t number_of_iterations_right_animation = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		//Lights the LEDs to indicate direction in white by increasing brightness
		if (counter_right_stepone%MED_FAST == 0){
			allLightsOn_Halo(HALF, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel, level_of_brightness,number_of_iterations_right_animation);
			number_of_iterations_right_animation++;
			level_of_brightness++;
		}
		//Returns true when the animation is done
		if (number_of_iterations_right_animation > NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_HALF_HALO)){
			level_of_brightness=0;
			counter_right_stepone=0;
			number_of_iterations_right_animation=0;
			return true;
		}

		counter_right_stepone++;
		return false;
	}

	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness=0;
		counter_right_stepone=0;
		number_of_iterations_right_animation=0;
		return true;
	}

}

//STEPTWO
bool Animations_right_stepTwo(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	int set_of_leds_to_light = 0;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		allLightsOn(HALF-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;

}

//STEPTHREE
bool Animations_right_stepThree(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 1;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(HALF-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);

		lightsGreen(HALF, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;

}

//STEPFOUR
bool Animations_right_stepFour(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 2;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(HALF-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(HALF, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}

//STEPFIVE
bool Animations_right_stepFive(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 3;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(HALF-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(HALF, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}

//STEPSIX
bool Animations_right_stepSix(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 4;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		allLightsOn(HALF-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(HALF, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}

//STEPSEVEN
bool Animations_right_stepSeven(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 5;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (!LESS_LEDS_FOR_ANIMATIONS)
		{
			allLightsOn(HALF-(set_of_leds_to_light+1),NOON_PLUS_ONE + set_of_leds_to_light+1, 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		}
		lightsGreen(HALF, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);

	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}


#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
//STEPEIGHT
bool Animations_right_stepEight(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 6;
	static bool is_over;

	red_pixel = 0;
	green_pixel = 1;
	blue_pixel = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		lightsGreen(HALF, NOON_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, NOON_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}
//STEPNINE
bool Animations_right_stepNine(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, HALF, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}


		return false;
	}

	else{
		animation_dim_the_leds_off(reset, HALF, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}
//STEPTEN
bool Animations_right_stepTen(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, HALF, NOON_PLUS_ONE , red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}


}
#else
//STEPEIGHT
bool Animations_right_stepEight(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, HALF, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}


		return false;
	}

	else{
		animation_dim_the_leds_off(reset, HALF, NOON_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}
//STEPNINE
bool Animations_right_stepNine(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, HALF, NOON_PLUS_ONE , red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}


}
#endif





//TURN FAILED
bool Animations_right_turnfailed(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = RED_PIXEL_RED/BRIGHTNESS_MAX;
	green_pixel = GREEN_PIXEL_RED/BRIGHTNESS_MAX;
	blue_pixel = BLUE_PIXEL_RED/BRIGHTNESS_MAX;

	if (animation_flash_the_leds_on_off(reset, HALF, NOON_PLUS_ONE , red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

/*
 * HARDRIGHT
 */

//STEPONE
bool Animations_hardRight_stepOne(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	static uint16_t counter_hardRight_stepone = 0;
	static uint8_t level_of_brightness = 0;
	static uint8_t number_of_iterations_hardRight_animation = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		//Lights the LEDs to indicate direction in white by increasing brightness
		if (counter_hardRight_stepone%MED_FAST == 0){
			allLightsOn_Halo(HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel,level_of_brightness, number_of_iterations_hardRight_animation);
			level_of_brightness++;
			number_of_iterations_hardRight_animation++;
		}

		//Returns true when the animation is done
		if (number_of_iterations_hardRight_animation > NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_QUARTER_HALO)){
			level_of_brightness=0;
			counter_hardRight_stepone=0;
			number_of_iterations_hardRight_animation = 0;
			return true;
		}

		counter_hardRight_stepone++;
		return false;
	}

	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness=0;
		counter_hardRight_stepone=0;
		number_of_iterations_hardRight_animation = 0;
		return true;
	}

}

//STEPTWO
bool Animations_hardRight_stepTwo(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 0;
	static bool is_over;

	if(!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(HALF-(set_of_leds_to_light+1),QUARTER_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
	}
	is_over = step_of_animations_turn_by_turn(reset, HALF, QUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;

}

//STEPTHREE
bool Animations_hardRight_stepThree(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 1;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(HALF-(set_of_leds_to_light+1),QUARTER_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(HALF, QUARTER_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, QUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;

}

//STEPFOUR
bool Animations_hardRight_stepFour(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 2;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		if(!LESS_LEDS_FOR_ANIMATIONS)
		{
			allLightsOn(HALF-(set_of_leds_to_light+1),QUARTER_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		}
		lightsGreen(HALF, QUARTER_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, QUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
//STEPFIVE
bool Animations_hardRight_stepFive(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 3;
	static bool is_over;

	red_pixel = 0;
	green_pixel = 1;
	blue_pixel = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		lightsGreen(HALF, QUARTER_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, HALF, QUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}
//STEPSIX
bool Animations_hardRight_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else {
		animation_dim_the_leds_off(reset, HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}
}
//STEPSEVEN
bool Animations_hardRight_stepSeven(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}
#else
//STEPFIVE
bool Animations_hardRight_stepFive(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else {
		animation_dim_the_leds_off(reset, HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}
}

//STEPSIX
bool Animations_hardRight_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}
#endif

//TURN FAILED
bool Animations_hardRight_turnfailed(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = RED_PIXEL_RED/BRIGHTNESS_MAX;
	green_pixel = GREEN_PIXEL_RED/BRIGHTNESS_MAX;
	blue_pixel = BLUE_PIXEL_RED/BRIGHTNESS_MAX;

	if (animation_flash_the_leds_on_off(reset, HALF, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

/*HARDLEFT
 *
 */

//STEPONE
bool Animations_hardLeft_stepOne(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	static uint16_t counter_hardLeft_stepone = 0;
	static uint8_t level_of_brightness = 0;
	static uint8_t number_of_iterations_hardLeft_animation = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		//Lights the LEDs to indicate direction in white by increasing brightness
		if (counter_hardLeft_stepone%MED_FAST == 0){
			allLightsOn_Halo(LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel,level_of_brightness, number_of_iterations_hardLeft_animation);
			level_of_brightness++;
			number_of_iterations_hardLeft_animation++;
		}
		//Returns true when the animation is done
		if (number_of_iterations_hardLeft_animation > NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_QUARTER_HALO)){
			level_of_brightness=0;
			counter_hardLeft_stepone=0;
			number_of_iterations_hardLeft_animation = 0;
			return true;
		}

		counter_hardLeft_stepone++;
		return false;
	}

	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness=0;
		counter_hardLeft_stepone=0;
		number_of_iterations_hardLeft_animation = 0;
		return true;
	}

}

//STEPTWO
bool Animations_hardLeft_stepTwo(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 0;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(LQUARTER-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, LQUARTER, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;

}

//STEPTHREE
bool Animations_hardLeft_stepThree(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 1;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(LQUARTER-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(LQUARTER, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, LQUARTER, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

//STEPFOUR
bool Animations_hardLeft_stepFour(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 2;
	static bool is_over;

	if(!reset){
		HaloPixel_setBrightness(brightness);
		if (!LESS_LEDS_FOR_ANIMATIONS)
		{
			allLightsOn(LQUARTER-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		}
		lightsGreen(LQUARTER, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, LQUARTER, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
//STEPFIVE
bool Animations_hardLeft_stepFive(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 3;
	static bool is_over;

	red_pixel = 0;
	green_pixel = 1;
	blue_pixel = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		lightsGreen(LQUARTER, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, LQUARTER, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}
//STEPSIX
bool Animations_hardLeft_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else{
		animation_dim_the_leds_off(reset, LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}
//STEPSEVEN
bool Animations_hardLeft_stepSeven(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}
#else
//STEPFIVE
bool Animations_hardLeft_stepFive(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else{
		animation_dim_the_leds_off(reset, LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}
//STEPSIX
bool Animations_hardLeft_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}
#endif


//TURNFAILED
bool Animations_hardLeft_turnfailed(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = RED_PIXEL_RED/BRIGHTNESS_MAX;
	green_pixel = GREEN_PIXEL_RED/BRIGHTNESS_MAX;
	blue_pixel = BLUE_PIXEL_RED/BRIGHTNESS_MAX;

	if (animation_flash_the_leds_on_off(reset, LQUARTER, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

/*
 * LEFT
 */

//STEPONE
bool Animations_left_stepOne(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	static uint16_t counter_left_stepone = 0;
	static uint8_t level_of_brightness = 0;
	static uint8_t number_of_iterations_left_animation = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		//Lights the LEDs to indicate direction in white by increasing brightness
		if (counter_left_stepone%MED_FAST == 0){
			allLightsOn_Halo(NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel,level_of_brightness, number_of_iterations_left_animation);
			level_of_brightness++;
			number_of_iterations_left_animation++;
		}
		//Returns true when the animation is done
		if (number_of_iterations_left_animation > NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_HALF_HALO)){
			level_of_brightness=0;
			counter_left_stepone=0;
			number_of_iterations_left_animation=0;
			return true;
		}

		counter_left_stepone++;
		return false;
	}

	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness=0;
		counter_left_stepone=0;
		number_of_iterations_left_animation=0;
		return true;
	}

}

//STEPTWO
bool Animations_left_stepTwo(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 0;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(NOON-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;

}

//STEPTHREE
bool Animations_left_stepThree(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 1;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(NOON-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);

		lightsGreen(NOON, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;

}

//STEPFOUR
bool Animations_left_stepFour(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 2;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(NOON-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(NOON, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

//STEPFIVE
bool Animations_left_stepFive(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 3;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(NOON-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(NOON, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

//STEPSIX
bool Animations_left_stepSix(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 4;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		allLightsOn(NOON-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(NOON, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}
	is_over = step_of_animations_turn_by_turn(reset, NOON, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

//STEPSEVEN
bool Animations_left_stepSeven(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 5;
	static bool is_over;
	if (!reset){
		HaloPixel_setBrightness(brightness);
		if(!LESS_LEDS_FOR_ANIMATIONS)
		{
			allLightsOn(NUMBER_OF_LEDS-(set_of_leds_to_light+1),HALF_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		}
		lightsGreen(NOON, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
//STEP EIGHT
bool Animations_left_stepEight(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 6;
	static bool is_over;

	red_pixel = 0;
	green_pixel = 1;
	blue_pixel = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		lightsGreen(NOON, HALF_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, HALF_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}

//STEP NINE
bool Animations_left_stepNine(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}
		return false;
	}

	else{
		animation_dim_the_leds_off(reset, NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}

//STEPTEN
bool Animations_left_stepTen(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}
}

#else
//STEP EIGHT
bool Animations_left_stepEight(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (animation_dim_the_leds_off(reset, NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}
		return false;
	}

	else{
		animation_dim_the_leds_off(reset, NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}

}

//STEPNINE
bool Animations_left_stepNine(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}
}
#endif



//TURNFAILED
bool Animations_left_turnfailed(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = RED_PIXEL_RED/BRIGHTNESS_MAX;
	green_pixel = GREEN_PIXEL_RED/BRIGHTNESS_MAX;
	blue_pixel = BLUE_PIXEL_RED/BRIGHTNESS_MAX;

	if (animation_flash_the_leds_on_off(reset, NOON, HALF_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

/*
 * SLIGHTLET
 */

//STEPONE
bool Animations_slightLeft_stepOne(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	static uint16_t counter_slightLeft_stepone = 0;
	static uint8_t level_of_brightness = 0;
	static uint8_t number_of_iterations_slightLeft_animation = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		//Lights the LEDs to indicate direction in white by increasing brightness
		if (counter_slightLeft_stepone%MED_FAST == 0){
			allLightsOn_Halo(NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel,level_of_brightness, number_of_iterations_slightLeft_animation);
			level_of_brightness++;
			number_of_iterations_slightLeft_animation++;
		}
		//Returns true when the animation is done
		if (number_of_iterations_slightLeft_animation > NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_QUARTER_HALO)){
			level_of_brightness=0;
			counter_slightLeft_stepone=0;
			number_of_iterations_slightLeft_animation = 0;
			return true;
		}

		counter_slightLeft_stepone++;
		return false;
	}
	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness=0;
		counter_slightLeft_stepone=0;
		number_of_iterations_slightLeft_animation = 0;
		return true;
	}

}

//STEPTWO
bool Animations_slightLeft_stepTwo(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 0;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		allLightsOn(NOON-(set_of_leds_to_light+1),LQUARTER_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, LQUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;

}

//STEPTHREE
bool Animations_slightLeft_stepThree(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 1;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		allLightsOn(NOON-(set_of_leds_to_light+1),LQUARTER_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		lightsGreen(NOON, LQUARTER_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, LQUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;

}

//STEPFOUR
bool Animations_slightLeft_stepFour(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 2;
	static bool is_over;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		if (!LESS_LEDS_FOR_ANIMATIONS)
		{
			allLightsOn(NOON-(set_of_leds_to_light+1),LQUARTER_PLUS_ONE+(set_of_leds_to_light+1), 1, 1, 1,NUMBER_OF_BRIGHTNESS_CHANGES);
		}
		lightsGreen(NOON, LQUARTER_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}

	is_over = step_of_animations_turn_by_turn(reset, NOON, LQUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}
	return false;
}

#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
//STEPFIVE
bool Animations_slightLeft_stepFive(bool reset, uint8_t null, uint8_t null_2){

	int set_of_leds_to_light = 3;
	static bool is_over;
	red_pixel = 0;
	green_pixel = 1;
	blue_pixel = 0;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		lightsGreen(NOON, LQUARTER_PLUS_ONE, set_of_leds_to_light-1, NUMBER_OF_BRIGHTNESS_CHANGES);
	}
	is_over = step_of_animations_turn_by_turn(reset, NOON, LQUARTER_PLUS_ONE, set_of_leds_to_light);

	if (is_over){
		is_over = false;
		return true;
	}

	return false;
}

//STEPSIX
bool Animations_slightLeft_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){

		HaloPixel_setBrightness(brightness);

		if (animation_dim_the_leds_off(reset, NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else{
		animation_dim_the_leds_off(reset, NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}
}

//STEPSEVEN
bool Animations_slightLeft_stepSeven(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

#else
//STEPFIVE
bool Animations_slightLeft_stepFive(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (!reset){

		HaloPixel_setBrightness(brightness);

		if (animation_dim_the_leds_off(reset, NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
			return true;
		}

		return false;
	}

	else{
		animation_dim_the_leds_off(reset, NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		return true;
	}
}

//STEPSIX
bool Animations_slightLeft_stepSix(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	if (animation_flash_the_leds_on_off(reset, NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

#endif


//TURN FAILED
bool Animations_slightLeft_turnfailed(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = RED_PIXEL_RED/BRIGHTNESS_MAX;
	green_pixel = GREEN_PIXEL_RED/BRIGHTNESS_MAX;
	blue_pixel = BLUE_PIXEL_RED/BRIGHTNESS_MAX;

	if (animation_flash_the_leds_on_off(reset, NOON, LQUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
		turnOffAllLEDs(0,0,0);
		return true;
	}
	else{
		return false;
	}

}

/*
 * UTURN
 */
bool Animations_uTurn_first_call(bool reset, uint8_t null, uint8_t null_2){

	red_pixel = TRUE;
	green_pixel = FALSE;
	blue_pixel = FALSE;

	static uint8_t level_of_brightness_fade_in = 0;
	static uint16_t counter_uturn_animation = 0;
	static uint8_t number_of_iterations_uturn_animation=0;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		if (counter_uturn_animation%FAST == 0){

			//Fading in and out to blink
			if (number_of_iterations_uturn_animation <= NUMBER_OF_ITERATIONS_ANIMATION_LIGHT_LEDS_TO_BY_TWO(NUMBER_OF_LEDS_HALF_HALO)){
				//Fades in
				allLightsOn_Halo(LQUARTER, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel,level_of_brightness_fade_in, number_of_iterations_uturn_animation);
				level_of_brightness_fade_in++;
				number_of_iterations_uturn_animation++;
			}

			//Fades out
			else if (animation_dim_the_leds_off(reset, LQUARTER, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel)){
				//turns off the leds and re initialize the static variables and return true when animation is done
				turnOffAllLEDs(0,0,0);
				level_of_brightness_fade_in = 0;
				counter_uturn_animation = 0;
				number_of_iterations_uturn_animation = 0;
				return true;
			}
		}


		counter_uturn_animation++;
		return false;
	}

	else{
		turnOffAllLEDs(0,0,0);
		animation_dim_the_leds_off(reset, LQUARTER, QUARTER_PLUS_ONE, red_pixel, green_pixel, blue_pixel);
		level_of_brightness_fade_in = 0;
		counter_uturn_animation = 0;
		number_of_iterations_uturn_animation = 0;
		return true;
	}

}
bool Animations_uTurn(bool reset, uint8_t null, uint8_t null_2){

	float red = (float) 	RED_PIXEL_RED / BRIGHTNESS_MAX;
	float green = (float) 	GREEN_PIXEL_RED / BRIGHTNESS_MAX;
	float blue = (float) 	BLUE_PIXEL_RED / BRIGHTNESS_MAX;

	if (!reset){

		//Fading in and out to blink
		if(animation_flash_the_leds_on_off(reset, LQUARTER, QUARTER_PLUS_ONE, red, green, blue)){
			return true;
		}
		else{
			return false;
		}

	}

	else{
		animation_flash_the_leds_on_off(reset, LQUARTER, QUARTER_PLUS_ONE, red, green, blue);
		turnOffAllLEDs(0,0,0);
		return true;
	}
}

/*
 * ANIMATIONS DEACTIVATE ALARM
 */
bool Animations_deactivate_alarm(bool reset, uint8_t null, uint8_t null_2){

	HaloPixel_setBrightness(brightness);

	float red_pixel_deactivate_alarm = (float) RED_PIXEL_GREEN / BRIGHTNESS_MAX;
	float green_pixel_deactivate_alarm = (float) GREEN_PIXEL_GREEN / BRIGHTNESS_MAX;
	float blue_pixel_deactivate_alarm = (float) BLUE_PIXEL_GREEN / BRIGHTNESS_MAX;

	if (!reset){
		HaloPixel_setBrightness(brightness);

		//Fading in and out to blink
		if(animation_flash_the_leds_on_off(reset, NUMBER_OF_LEDS-1,0,red_pixel_deactivate_alarm, green_pixel_deactivate_alarm, blue_pixel_deactivate_alarm)){
			turnOffAllLEDs(0,0,0);
			return true;
		}

		return false;
	}

	else{
		animation_flash_the_leds_on_off(reset, NUMBER_OF_LEDS-1, 0, red_pixel_deactivate_alarm, green_pixel_deactivate_alarm, blue_pixel_deactivate_alarm);
		turnOffAllLEDs(0,0,0);
		return true;
	}
}

/*
 * DESTINATION
 */
bool Animations_destination(bool reset, uint8_t null, uint8_t null_2){

	HaloPixel_setBrightness(brightness);

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	static uint8_t number_of_flashes = 0;

	if (!reset){

		if(animation_flash_the_leds_on_off(reset, NUMBER_OF_LEDS-1, 0, red_pixel, green_pixel, blue_pixel)){
			turnOffAllLEDs(0,0,0);
			number_of_flashes++;
		}
		if (number_of_flashes == NUMBER_OF_FLASH_FOR_DESTINATION){
			number_of_flashes = 0;
			return true;
		}


		return false;
	}

	else{
		animation_flash_the_leds_on_off(reset, NUMBER_OF_LEDS-1, 0, red_pixel, green_pixel, blue_pixel);
		turnOffAllLEDs(0,0,0);
		return true;
	}
}


/*
 * PAIRINGFADE
 */
bool Animations_pairingFade(bool reset, uint8_t null, uint8_t null_2) {

	HaloPixel_setBrightness(brightness);

	float red_pixel_pairingfade=(float)RED_PIXEL_PAIRING_FADE/BRIGHTNESS_MAX;
	float green_pixel_pairingfade=(float) GREEN_PIXEL_PAIRING_FADE/BRIGHTNESS_MAX;
	float blue_pixel_pairingfade=(float) BLUE_PIXEL_PAIRING_FADE/BRIGHTNESS_MAX;
	static uint16_t counter_pairingfade_animation = 0;

	if (!reset){
		if (counter_pairingfade_animation%SLOWEST == 0){

			//fades out
			if (animation_dim_the_leds_off(reset, NUMBER_OF_LEDS-1, 0, red_pixel_pairingfade, green_pixel_pairingfade, blue_pixel_pairingfade)){
				turnOffAllLEDs(0,0,0);
				counter_pairingfade_animation = 0;
				return true;
			}
		}
		counter_pairingfade_animation++;
		return false;
	}

	else{
		animation_dim_the_leds_off(reset, NUMBER_OF_LEDS-1, 0, red_pixel_pairingfade, green_pixel_pairingfade, blue_pixel_pairingfade);
		turnOffAllLEDs(0,0,0);
		counter_pairingfade_animation = 0;
		return true;
	}
}

//PAIRING
bool Animations_pairing(bool reset, uint8_t null, uint8_t null_2){

	HaloPixel_setBrightness(brightness);

	static uint8_t leds_to_be_light=0;
	static int8_t led=0;
	static int8_t led_temp=0;
	static uint8_t counter_pairing_animation = 0;
	float level_of_brightness=0;
	static bool lights_are_lighten = false;
	static bool light_full_circle = false;
	static bool ready_to_decrease_brightness = false;
	static bool light_first_led_circle_done = false;
	static uint8_t white_pause = 0;

	if (!reset){
	//Start the circle with the 5 LEDs
	if (counter_pairing_animation%SLOW ==0){

		if (!lights_are_lighten){

			for (uint8_t i=0; i <= leds_to_be_light; i++){
				level_of_brightness = (PAIRING_MASK_RGBCOLOR[i]);
				led_temp = MOD((led-i), NUMBER_OF_LEDS);
				HaloPixel_setPixelColor(led_temp, level_of_brightness*BRIGHTNESS_MAX,level_of_brightness*BRIGHTNESS_MAX,level_of_brightness*BRIGHTNESS_MAX);
				}

			leds_to_be_light++;
			if (leds_to_be_light == NUMBER_OF_BRIGHTESS_CHANGES_PAIRING){
				lights_are_lighten = true;
			}
			else {
				led=(led+1)%NUMBER_OF_LEDS;
			}

		}

		//Continue the circle with the 5 lEDs
		else if (lights_are_lighten && !light_full_circle){

			for (uint8_t i=0; i<NUMBER_OF_BRIGHTESS_CHANGES_PAIRING; i++){
				led_temp = MOD((led-i), NUMBER_OF_LEDS);
				level_of_brightness = (PAIRING_MASK_RGBCOLOR[i]);
				HaloPixel_setPixelColor_with_gamma_correction(led_temp, level_of_brightness * BRIGHTNESS_MAX, level_of_brightness * BRIGHTNESS_MAX, level_of_brightness * BRIGHTNESS_MAX);

			}
			led=(led+1)%NUMBER_OF_LEDS;
			if (led == NUMBER_OF_LEDS - 1){
				light_first_led_circle_done = true;
			}
			if (led == 4 && light_first_led_circle_done){
				light_full_circle = true;
				light_first_led_circle_done = false;
			}
		}

		//Lights all LED in white one by one
		else if (light_full_circle && !ready_to_decrease_brightness){
			HaloPixel_setPixelColor(led, BRIGHTNESS_MAX, BRIGHTNESS_MAX, BRIGHTNESS_MAX);
			if (counter_pairing_animation%(1)==0){
				led=(led+1);
				if (led == NUMBER_OF_LEDS){
					light_first_led_circle_done = true;
					led = led % NUMBER_OF_LEDS;
				}
				if (led == NUMBER_OF_BRIGHTESS_CHANGES_PAIRING -1 && light_first_led_circle_done){
					ready_to_decrease_brightness = true;
					light_first_led_circle_done = false;
				}

			}

		}

		//All LEDS fades out
		else if (ready_to_decrease_brightness){
			if (white_pause < WHITE_PAUSE_PAIRING){
				white_pause ++;
			}
			else {
				if (animation_dim_the_leds_off(reset, NUMBER_OF_LEDS-1, 0 , 1, 1, 1)){
					turnOffAllLEDs(0,0,0);
					counter_pairing_animation = 0;
					light_full_circle = false;
					lights_are_lighten = false;
					ready_to_decrease_brightness = false;
					leds_to_be_light = 0;
					white_pause = 0;
					led = 0;
					return true;
				}
			}

		}
		HaloPixel_show();

	}

	counter_pairing_animation++;
	return false;

	}

	else{
		turnOffAllLEDs(0,0,0);
		led = 0;
		animation_dim_the_leds_on(reset, led, led, 1, 1, 1);
		animation_dim_the_leds_off(reset, NUMBER_OF_LEDS-1, 0 , 1, 1, 1);
		counter_pairing_animation = 0;
		light_full_circle = false;
		lights_are_lighten = false;
		ready_to_decrease_brightness = false;
		leds_to_be_light = 0;
		white_pause = 0;
		return true;
	}
}


/*
 * LONGTAP
 */
bool Animations_longTap (bool reset, uint8_t null, uint8_t null_2){

	HaloPixel_setBrightness(brightness);

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	static uint16_t counter_longtap = 0;

	if (!reset){
		if (counter_longtap%MED_FAST == 0){

			//Lights all the leds
			if (animation_dim_the_leds_on(reset, NUMBER_OF_LEDS-1, 0 , red_pixel, green_pixel, blue_pixel)){
				counter_longtap=0;
				return true;
			}
		}
		counter_longtap ++;
		return false;
	}

	else{
		animation_dim_the_leds_on(reset, NUMBER_OF_LEDS-1, 0 , red_pixel, green_pixel, blue_pixel);
		turnOffAllLEDs(0,0,0);
		counter_longtap=0;
		return true;
	}

}

/*
 * SUCCESS
 */
bool Animations_success(bool reset, uint8_t null, uint8_t null_2){

	static uint16_t counter_success_animation = 0;
	static uint8_t brightness_level_fade_in = 0;
	static uint8_t brightness_level_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
	static uint8_t color_pause = 0;

	HaloPixel_setBrightness(brightness);

	if (!reset){
		// fade in
		if (brightness_level_fade_in < NUMBER_OF_BRIGHTNESS_CHANGES){
			if (counter_success_animation%MED_SLOW == 0 ){
				HaloPixel_setBrightness(BRIGHTNESS_MAX);
				set_SmartHalo_Colors();
				HaloPixel_setBrightness(brightness_level_fade_in * BRIGHTNESS_MULTIPLICATOR);
				HaloPixel_show();
				brightness_level_fade_in++;
			}
		  }

		 // fade out
		 if (brightness_level_fade_in >= NUMBER_OF_BRIGHTNESS_CHANGES){
			 if (color_pause < PAUSE_FOR_SUCCESS){
				 color_pause ++;
			 }
			 else if (counter_success_animation%MED_SLOW == 0 ){
				HaloPixel_setBrightness(BRIGHTNESS_MAX);
				set_SmartHalo_Colors();
				HaloPixel_setBrightness(brightness_level_fade_out * BRIGHTNESS_MULTIPLICATOR);
				HaloPixel_show();
				brightness_level_fade_out -- ;
			}

			//Return true when the animation is done
			if (brightness_level_fade_out <= 0){
				counter_success_animation=0;
				brightness_level_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
				brightness_level_fade_in = 0;
				turnOffAllLEDs(0,0,0);
				color_pause = 0;
				return true;
			}
		 }
		 counter_success_animation++;
		 return false;
	}

	else{
		counter_success_animation=0;
		brightness_level_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		brightness_level_fade_in = 0;
		turnOffAllLEDs(0,0,0);
		color_pause = 0;
		return true;
	}
}

/*
 * WRONGTAPCODE
 */
bool Animations_wrongTapCode(bool reset, uint8_t null, uint8_t null_2) {

	HaloPixel_setBrightness(brightness);

	red_pixel = RED_PIXEL_ALARM;
	green_pixel = GREEN_PIXEL_ALARM;
	blue_pixel = BLUE_PIXEL_ALARM;
	static uint16_t counter_wrongtapcode_animation = 0;
	static uint8_t led = 0;
	static bool leds_are_on = false;
	static uint8_t number_of_blink = 0;

	if (!reset){

		//Lights the led one by one in red
		if  (led<NUMBER_OF_LEDS){
			if (counter_wrongtapcode_animation%PAUSE_WRONG_TAP_CODE == 0){
				HaloPixel_setColor(led, HaloPixel_RGB_Color(red_pixel, green_pixel, blue_pixel));
				HaloPixel_showPixel(led);
				led++;
				leds_are_on = true;
			}
		}

		//Blink the leds
		else if (number_of_blink<=NUMBER_OF_FLASH_WRONG_TAP_CODE && led==NUMBER_OF_LEDS){

			//Turn off the LEDs
			if(counter_wrongtapcode_animation%PAUSE_WRONG_TAP_CODE == 0){
				if (leds_are_on){
					turnOffAllLEDs(0,0,0);
					leds_are_on = false;
					number_of_blink++;
				}

				//Lights the LEDs
				else {
					for (uint8_t i=0;i<NUMBER_OF_LEDS;i++){
						HaloPixel_setColor(i, HaloPixel_RGB_Color(red_pixel, green_pixel, blue_pixel));
					}
					HaloPixel_show();
					leds_are_on = true;
				}
			}
		}

		//Reinitialize the leds and returns true when the animation is done
		else if (number_of_blink>NUMBER_OF_FLASH_WRONG_TAP_CODE){
			counter_wrongtapcode_animation = 0;
			led = 0;
			turnOffAllLEDs(0,0,0);
			number_of_blink=0;
			return true;
		}

		counter_wrongtapcode_animation++;
		return false;

		}

	else{
		counter_wrongtapcode_animation = 0;
		led = 0;
		turnOffAllLEDs(0,0,0);
		number_of_blink=0;
		return true;
	}
}

/*
 * ALARMREDCIRCLE
 */
bool Animations_alarm_red_circle (bool reset, uint8_t null, uint8_t null_2) {

	HaloPixel_setBrightness(brightness);

	red_pixel = RED_PIXEL_RED;
	green_pixel = GREEN_PIXEL_RED;
	blue_pixel = BLUE_PIXEL_RED;
	static uint16_t counter_alarm_circle_animation = 0;
	static uint8_t led = 0;

	if (!reset){
		if (counter_alarm_circle_animation % PAUSE_ALARM_RED_CIRCLE ==0){

			//Lights the led one by one
		  if (led < NUMBER_OF_LEDS){
			  for (uint8_t i=0; i<led ; i++){
				  HaloPixel_setColor(led, HaloPixel_RGB_Color(BRIGHTNESS_MAX,0,0));
			  }
			  HaloPixel_setColor(led, HaloPixel_RGB_Color(BRIGHTNESS_MAX,0,0));
			  HaloPixel_showPixel(led);
			  led ++;
		  }

		  //Reinitialize the static variable and returns true when the animation is done
		  else {
			  counter_alarm_circle_animation=0;
			  led = 0;
			  return true;
		  }
		}

		counter_alarm_circle_animation++;
		return false;
	}

	else{
		counter_alarm_circle_animation=0;
		led = 0;
		return true;
	}

}

/*
 * ALARM_BLINKING
 */
bool Animations_alarm_blinking (bool reset, uint8_t null, uint8_t null_2) {

	HaloPixel_setBrightness(brightness);

	static uint16_t counter_alarm_blink_animation = 0;

	if (!reset){
		if (counter_alarm_blink_animation==0){
			turnOffAllLEDs(0,0,0);
		}

		//Blinking

		//Lights the leds in red
		else if (counter_alarm_blink_animation == PAUSE_1_ALARM_BLINK){
			for (uint8_t led=0; led<NUMBER_OF_LEDS; led++){
				HaloPixel_setColor(led, HaloPixel_RGB_Color(RED_PIXEL_ALARM, GREEN_PIXEL_ALARM, BLUE_PIXEL_ALARM));
			}
			HaloPixel_show();
		}

		//Closes the leds and returns true when the animation is done
		else if (counter_alarm_blink_animation == PAUSE_2_ALARM_BLINK){
			turnOffAllLEDs(0,0,0);
			counter_alarm_blink_animation=0;
			return true;
		}

		counter_alarm_blink_animation++;
		return false;
	}

	else{
		turnOffAllLEDs(0,0,0);
		counter_alarm_blink_animation=0;
		return true;
	}

}

/*
 * GOAL_COMPLETION
 */
bool Animations_goalCompletion(bool reset, uint8_t completion, uint8_t null_2){

	float red_pixel_goal = (float) RED_PIXEL_GOAL_COMPLETION/BRIGHTNESS_MAX;
	float green_pixel_goal = (float) GREEN_PIXEL_GOAL_COMPLETION/BRIGHTNESS_MAX;
	float blue_pixel_goal = (float) BLUE_PIXEL_GOAL_COMPLETION/BRIGHTNESS_MAX;

	float percentage_of_completion = (float)completion/100.0;
	uint8_t number_of_leds_to_light = NUMBER_OF_LEDS * percentage_of_completion;

	static uint16_t counter_goalcompletion_animation=0;
	static uint8_t counter_goal_completion_iterations=0;
	static uint8_t counter_goal_completion_iterations_fade_out=0;
	static uint8_t number_of_flash =0;

	HaloPixel_setBrightness(brightness);

	if (!reset){
		if (counter_goalcompletion_animation%MED ==0){

			//Leds lights one by one
			if (counter_goal_completion_iterations<CALCULATE_NUMBER_OF_ITERATIONS(number_of_leds_to_light)){
				allLightsOn_number_of_LEDs_Halo(number_of_leds_to_light, red_pixel_goal, green_pixel_goal, blue_pixel_goal, counter_goal_completion_iterations);
				counter_goal_completion_iterations++;
			}

			//Flash the last led to show completion
			else if (number_of_flash < NUMBER_OF_FLASH_GOAL_COMPLETION){

				if (animation_flash_the_leds_off_on(reset, number_of_leds_to_light-1, number_of_leds_to_light-1, red_pixel_goal, green_pixel_goal, blue_pixel_goal)){
					number_of_flash++;
				}
				counter_goal_completion_iterations_fade_out=CALCULATE_NUMBER_OF_ITERATIONS(number_of_leds_to_light);
			}

			//Turns off the leds one by one
			else{
				if (counter_goal_completion_iterations_fade_out>0){
					allLightsOn_number_of_LEDs_Halo(number_of_leds_to_light, red_pixel_goal, green_pixel_goal, blue_pixel_goal, counter_goal_completion_iterations_fade_out);
					counter_goal_completion_iterations_fade_out--;
				}

				//Turns off all the leds, reinitialize the static variables and return true when the animation is done
				else {
					turnOffAllLEDs(0,0,0);
					counter_goalcompletion_animation=0;
					counter_goal_completion_iterations=0;
					counter_goal_completion_iterations_fade_out=CALCULATE_NUMBER_OF_ITERATIONS(number_of_leds_to_light);
					number_of_flash=0;
					HaloPixel_clear();
					return true;
				}
			}
		}

		counter_goalcompletion_animation++;
		return false;
	}

	else{
		animation_flash_the_leds_off_on(reset, number_of_leds_to_light-1, number_of_leds_to_light-1, red_pixel_goal, green_pixel_goal, blue_pixel_goal);
		turnOffAllLEDs(0,0,0);
		counter_goalcompletion_animation=0;
		counter_goal_completion_iterations=0;
		counter_goal_completion_iterations_fade_out=CALCULATE_NUMBER_OF_ITERATIONS(number_of_leds_to_light);
		number_of_flash=0;
		HaloPixel_clear();
		return true;
	}

}

/*
 * CADENCE
 */
static bool Animations_cadence_first_call(){

	static uint8_t brightness_level = 0;
	static bool increase_done = false;
	static bool decrease_done = false;

	HaloPixel_setPixelColor(LED_GOAL_CADENCE, brightness_level * BRIGHTNESS_MULTIPLICATOR, brightness_level * BRIGHTNESS_MULTIPLICATOR, brightness_level * BRIGHTNESS_MULTIPLICATOR);
	brightness_level ++;

	if (brightness_level >= NUMBER_OF_BRIGHTNESS_CHANGES && !increase_done){
		increase_done = increase_cadence (LED_MAX_SPEEDOMETER, LED_MIN_SPEEDOMETER - 1);
	}

	else if (increase_done && !decrease_done){
		decrease_done = decrease_cadence (LED_MIN_SPEEDOMETER, LED_MAX_SPEEDOMETER);
	}

	else if (decrease_done){
		brightness_level = 0;
		increase_done = false;
		decrease_done = false;
		return false;
	}

	return true;

}

static bool increase_cadence (uint8_t led_to_light, uint8_t current_led){

	static uint8_t i = 1;
	static uint8_t led ;
	static uint8_t brightness_level = 0;
	static uint8_t counter_cadence = 0;

	led = current_led + i;

	if (counter_cadence % MED_FAST == 0){
		if (led <= led_to_light){
			if (led < LED_GOAL_CADENCE){
				HaloPixel_setPixelColor(led, brightness_level * BRIGHTNESS_MULTIPLICATOR,0,0);
			}
			if (led > LED_GOAL_CADENCE){
				HaloPixel_setPixelColor(led, 0, brightness_level * BRIGHTNESS_MULTIPLICATOR, 0);
			}
				HaloPixel_setPixelColor(led-1, 0, 0, 0);
				HaloPixel_setPixelColor(LED_GOAL_CADENCE, BRIGHTNESS_MAX, BRIGHTNESS_MAX, BRIGHTNESS_MAX);
			HaloPixel_show();

			if (brightness_level < NUMBER_OF_BRIGHTNESS_CHANGES - 1){
				brightness_level ++;

			}
			else{
				i++;
				brightness_level = 0;
			}
		}
		else{
			i = 1;
			brightness_level = 0;
			return true;
		}
	}
	return false;
	counter_cadence ++;
}

static bool decrease_cadence (uint8_t led_to_light, uint8_t current_led){
	static uint8_t i = 1;
	static uint8_t led ;
	static uint8_t brightness_level = 0;
	static uint8_t counter_cadence = 0;

	led = current_led - i;

	if (counter_cadence % MED_FAST == 0){
		if (led >= led_to_light){

			if (led < LED_GOAL_CADENCE){
				HaloPixel_setPixelColor(led, brightness_level * BRIGHTNESS_MULTIPLICATOR,0,0);
			}
			if (led > LED_GOAL_CADENCE){
				HaloPixel_setPixelColor(led, 0, brightness_level * BRIGHTNESS_MULTIPLICATOR, 0);
			}

			HaloPixel_setPixelColor(led+1, 0, 0, 0);
			HaloPixel_setPixelColor(LED_GOAL_CADENCE, BRIGHTNESS_MAX, BRIGHTNESS_MAX, BRIGHTNESS_MAX);
			HaloPixel_show();

			if (brightness_level < NUMBER_OF_BRIGHTNESS_CHANGES - 1){
				brightness_level ++;
			}

			else{
				i++;
				brightness_level = 0;
			}
		}
		else{
			i = 1;
			brightness_level = 0;
			return true;
		}
	}

	return false;
	counter_cadence ++;
}

bool Animations_cadence (bool reset, uint8_t speed_goal, uint8_t actual_speed){

	static uint8_t previous_speed = 0;
	static bool cadence_first_call = true;
	static bool cadence_set = false;
	uint8_t led_to_light;
	uint8_t current_led;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		led_to_light = (uint8_t)((float)actual_speed/(2.00*speed_goal) * NUMBER_OF_LEDS_SPEEDOMETER) + LED_MIN_SPEEDOMETER - 1;
		current_led = (uint8_t)((float)previous_speed/(2.00*speed_goal) * NUMBER_OF_LEDS_SPEEDOMETER)+ LED_MIN_SPEEDOMETER - 1;

 		if (cadence_first_call){
			cadence_first_call = Animations_cadence_first_call();
		}
		else if (led_to_light < current_led){
			cadence_set = decrease_cadence(led_to_light, current_led);
		}
		else if (led_to_light > current_led){
			cadence_set = increase_cadence(led_to_light, current_led);
		}
		if (cadence_set){
			previous_speed = actual_speed;
			cadence_set = false;
			return true;
		}

		return false;
	}

	else{
		cadence_first_call = true;
		cadence_set = false;
		previous_speed = 0;
		return true;
	}
}

/*
 * SPEEDOMETER
 */

uint8_t SH_get_speed_min_speedometer(){
	return speed_min_speedometer;
}

uint8_t SH_get_speed_max_speedometer(){
	return speed_max_speedometer;
}

bool SH_set_new_speed_max_and_min(uint8_t new_speed_min, uint8_t new_speed_max){

	if (new_speed_min < new_speed_max){
		speed_min_speedometer = new_speed_min;
		speed_max_speedometer = new_speed_max;
		return true;
	}
	else{
		return false;
	}

}

static bool Animations_decrease_speed(uint8_t previous_led_max, uint8_t led_max){

	static uint8_t i;
	static uint8_t level_of_brightness = BRIGHTNESS_MAX - 1;
	static uint16_t couter_speedometer_first_call = 0;
	float pixel_for_led [PIXELS_PER_LED];
	uint8_t temp_brightness_level=0;

	for (i=previous_led_max; i> led_max; i--){
		set_Speedometer_Colors(pixel_for_led, i);
		if (level_of_brightness - 5 * i< 0){
			temp_brightness_level = 0;
		}
		else{
			temp_brightness_level = level_of_brightness - 5 * i;
		}
		if (temp_brightness_level >= NUMBER_OF_BRIGHTNESS_CHANGES){
			temp_brightness_level = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		}
		if (temp_brightness_level < 0){
			temp_brightness_level = 0;
		}
		if (couter_speedometer_first_call%MED_FAST == 0){
			HaloPixel_setPixelColor(i, pixel_for_led[ARRAY_RED_PIXEL] * temp_brightness_level * BRIGHTNESS_MULTIPLICATOR, pixel_for_led[ARRAY_GREEN_PIXEL]* temp_brightness_level * BRIGHTNESS_MULTIPLICATOR,pixel_for_led[ARRAY_BLUE_PIXEL]* temp_brightness_level * BRIGHTNESS_MULTIPLICATOR);
		}
	}
	HaloPixel_show();
	if (temp_brightness_level > 0){
		couter_speedometer_first_call++;
		level_of_brightness --;
		return false;
	}
	else{
		HaloPixel_setPixelColor(led_max + 1, 0,0,0);
		HaloPixel_show();
		couter_speedometer_first_call = 0;
		level_of_brightness = BRIGHTNESS_MAX - 1;
		return true;
	}
}

static bool Animations_increase_speed(uint8_t previous_led_max, uint8_t led_max){

	static uint8_t i;
	static uint8_t level_of_brightness = 0;
	static uint16_t couter_speedometer_first_call = 0;
	float pixel_for_led [PIXELS_PER_LED];
	uint8_t temp_brightness_level=0;

	for (i=previous_led_max + 1; i<= led_max; i++){
		set_Speedometer_Colors(pixel_for_led, i);
		if (level_of_brightness - 5 * i< 0){
			temp_brightness_level = 0;
		}
		else{
			temp_brightness_level = level_of_brightness - 5 * i;
		}
		if (temp_brightness_level >= NUMBER_OF_BRIGHTNESS_CHANGES){
			temp_brightness_level = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		}
		if (temp_brightness_level < 0){
			temp_brightness_level = 0;
		}
		if (couter_speedometer_first_call%MED_FAST == 0){
			HaloPixel_setPixelColor(i, pixel_for_led[ARRAY_RED_PIXEL] * temp_brightness_level * BRIGHTNESS_MULTIPLICATOR, pixel_for_led[ARRAY_GREEN_PIXEL]* temp_brightness_level * BRIGHTNESS_MULTIPLICATOR,pixel_for_led[ARRAY_BLUE_PIXEL]* temp_brightness_level * BRIGHTNESS_MULTIPLICATOR);
		}
	}
	HaloPixel_show();
	if (temp_brightness_level < NUMBER_OF_BRIGHTNESS_CHANGES - 1){
		couter_speedometer_first_call++;
		level_of_brightness ++;
		return false;
	}
	else{
		couter_speedometer_first_call = 0;
		level_of_brightness = 0;
		return true;
	}
}

static bool Animations_speedometer_first_call(){

	static bool open_lights = false;
	static bool close_lights = false;

	if (!open_lights){
		open_lights = Animations_increase_speed(LED_MIN_SPEEDOMETER - 1, LED_MAX_SPEEDOMETER);

	}
	else if (!close_lights){
		close_lights = Animations_decrease_speed(LED_MAX_SPEEDOMETER, LED_MIN_SPEEDOMETER - 1);
	}
	else{
		close_lights = false;
		open_lights = false;
		return false;
	}

	return true;

}

bool Animations_speedometer (bool reset, uint8_t speed, uint8_t null_2){

	uint8_t led_min;
	uint8_t led_max;
	static uint8_t previous_speed;
	static bool speedometer_first_call = true;
	static bool speed_set = false;

	if (!reset){
		HaloPixel_setBrightness(brightness);
		led_min = (uint8_t)((float)(NUMBER_OF_LEDS_SPEEDOMETER * previous_speed) / (speed_max_speedometer - speed_min_speedometer)) + LED_MIN_SPEEDOMETER;
		led_max = (uint8_t)((float)(NUMBER_OF_LEDS_SPEEDOMETER * speed) / (speed_max_speedometer - speed_min_speedometer))+ LED_MIN_SPEEDOMETER;

		if (speedometer_first_call){
			speedometer_first_call = Animations_speedometer_first_call();
		}
		else if (led_min < led_max){
			speed_set = Animations_increase_speed (led_min - 1, led_max);
		}
		else if (led_max < led_min){
			speed_set = Animations_decrease_speed (led_min, led_max);
		}

		if(speed_set){
			previous_speed = speed;
			speed_set = false;
			return true;
		}
		else{
			return speed_set;
		}
	}

	else{
		speedometer_first_call = true;
		previous_speed = 0;
		return true;
	}
}

/*
 * AS THE CROW FLIES
 */
bool Animations_as_the_crow_flies(bool reset, uint8_t percentage_of_led_to_light, uint8_t percentage_of_distance_covered_to_destination){

	static uint8_t pixel_as_the_crow_flies[PIXELS_PER_LED] = {0,0,0};

	static float previous_percentage_of_led_to_light = 0;

	static uint8_t previous_percentage_of_distance_to_destination = 0;

	static uint8_t as_the_crow_flies_counter = 0;

	static uint8_t number_of_the_leds_to_light[NUMBER_OF_LEDS_AS_THE_CROW_FLIES] = {0,0,0};

	static float brightness_of_the_leds[NUMBER_OF_LEDS_AS_THE_CROW_FLIES] = {0,0,0,0};

	static bool heading_set = false;

	static bool as_the_crow_flies_first_call = true;


	HaloPixel_setBrightness(brightness);

	if((percentage_of_led_to_light > 100) || (percentage_of_distance_covered_to_destination > 100))
	{
		return true;
	}

	if (!reset){

		if (as_the_crow_flies_counter%FAST == 0){

			HaloPixel_setBrightness(brightness);

			//Plays the animation of the first call if we display the as the crow flie for the first time
			if (as_the_crow_flies_first_call){
				as_the_crow_flies_first_call = Animations_as_the_crow_flies_first_call(number_of_the_leds_to_light, brightness_of_the_leds, pixel_as_the_crow_flies);
			}

			//If the heading is set
			else if (heading_set){

				//returns true when the pixel color are set
				if (previous_percentage_of_distance_to_destination == percentage_of_distance_covered_to_destination){
					as_the_crow_flies_counter = 0;
					heading_set = false;
					previous_percentage_of_led_to_light = percentage_of_led_to_light;
					return true;
				}

				else {
					set_new_colors_for_destination_as_the_crow_flies(pixel_as_the_crow_flies, percentage_of_distance_covered_to_destination, &previous_percentage_of_distance_to_destination);
				}
			}

			//If the heading isn't set
			else if (!heading_set){

					//If the color of the LEDs indicates the right distance to destination we only change the heading
					if (previous_percentage_of_distance_to_destination == percentage_of_distance_covered_to_destination){
						heading_set = set_which_led_to_light_as_the_crow_flies(number_of_the_leds_to_light, previous_percentage_of_led_to_light, percentage_of_led_to_light,brightness_of_the_leds);
					}

					//Heading and color of the LEDs are modified
					else {
						heading_set = set_which_led_to_light_as_the_crow_flies(number_of_the_leds_to_light, previous_percentage_of_led_to_light, percentage_of_led_to_light, brightness_of_the_leds);
						set_new_colors_for_destination_as_the_crow_flies(pixel_as_the_crow_flies, percentage_of_distance_covered_to_destination, &previous_percentage_of_distance_to_destination);
					}
			}

			//Lights the led in the right brightness and color
			HaloPixel_clear();
			HaloPixel_setPixelColor(number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_RED_PIXEL] * brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_GREEN_PIXEL] * brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] ,pixel_as_the_crow_flies[ARRAY_BLUE_PIXEL] * brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES]);
			HaloPixel_setPixelColor(number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_RED_PIXEL] * brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_GREEN_PIXEL] * brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_BLUE_PIXEL] * brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES]);
			HaloPixel_setPixelColor(number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_RED_PIXEL] * brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_GREEN_PIXEL] * brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_BLUE_PIXEL] * brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES]);
			if (brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] > 0){
				HaloPixel_setPixelColor(number_of_the_leds_to_light[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_RED_PIXEL] * brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_GREEN_PIXEL] * brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES], pixel_as_the_crow_flies[ARRAY_BLUE_PIXEL] * brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES]);
			}


		}
		HaloPixel_show();
		as_the_crow_flies_counter ++;
		return false;
	}

	//if we want to reset the animation
	else{
		turnOffAllLEDs(0,0,0);
		previous_percentage_of_led_to_light = 0;
		previous_percentage_of_distance_to_destination = 0;
		as_the_crow_flies_counter = 0;
		for (uint8_t i=0; i<NUMBER_OF_LEDS_AS_THE_CROW_FLIES; i++){
			brightness_of_the_leds[i] = 0;
		}
		for (uint8_t i=0; i<NUMBER_OF_LEDS_AS_THE_CROW_FLIES; i++){
			number_of_the_leds_to_light[i] = 0;
		}
		as_the_crow_flies_first_call = true;
		heading_set = false;
		return true;
	}

}

bool as_the_crow_flies_is_going_clockwise (uint8_t goal_center_led, uint8_t current_center_led){

	if (goal_center_led > current_center_led){
		if (goal_center_led - current_center_led<= NUMBER_OF_LEDS/2){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if (current_center_led - goal_center_led <= NUMBER_OF_LEDS/2){
			return false;
		}
		else{
			return true;
		}
	}

	return true;
}

static bool Animations_as_the_crow_flies_first_call(uint8_t number_of_the_leds_to_light [], float brightness_of_the_leds [], uint8_t pixel_as_the_crow_flies []){

	static bool heading_set = false;

	static uint8_t percentage_of_led_to_light = 99;

	static uint8_t previous_percentage_of_distance_to_do = 100;

	static uint8_t percentage_of_distance_to_do = 0;

	static uint8_t counter_as_the_crow_flies = 0;

	static uint8_t goal_center_led;

	static float goal_percentage_in_float;

	float brightness_changes;

	float exact_number_of_led_to_light;

	goal_percentage_in_float = (float) percentage_of_led_to_light / 100;

	exact_number_of_led_to_light = (float) (NUMBER_OF_LEDS * goal_percentage_in_float);

	brightness_changes = BRIGHTNESS_CHANGES_FOR_AS_THE_CROW_FLIES_FIRST_CALL;

	goal_center_led = ((uint8_t)(NUMBER_OF_LEDS * goal_percentage_in_float)) % NUMBER_OF_LEDS;

	if(!heading_set){

		//Light the leds for as the crow flies
		if (brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES]< 0.5 - brightness_changes && brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES]< 1 - brightness_changes && brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES]< 0.5 - brightness_changes && brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES]<= 0){

			number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = MOD((number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] - 1), NUMBER_OF_LEDS);
			number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = (number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] + 1)% NUMBER_OF_LEDS;


			if (brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] + 2 * brightness_changes < 1){
				brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] + 2* brightness_changes;
				brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] + brightness_changes;
				brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] + brightness_changes;
			}

			else{
				brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = 1 - brightness_changes;
				brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = 0.5 - brightness_changes;
				brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = 0.5 - brightness_changes;
			}
			heading_set = false;
		}

		//Turns the heading clockwise to go around the halo
		else if (number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] != goal_center_led){
			turn_as_the_crow_flies_clockwise(number_of_the_leds_to_light, brightness_changes, brightness_of_the_leds);
		}

		//Adjust the heading
		else{
			heading_set = set_as_the_crow_flies_heading_leds(number_of_the_leds_to_light, brightness_changes, brightness_of_the_leds, exact_number_of_led_to_light);
		}
	}

	//Changes the color of the LEDs for the first call animation (from green to orange)
	if (previous_percentage_of_distance_to_do!=percentage_of_distance_to_do){
			if (counter_as_the_crow_flies % MED == 0){
				set_new_colors_for_destination_as_the_crow_flies(pixel_as_the_crow_flies, percentage_of_distance_to_do, &previous_percentage_of_distance_to_do);
			}
	}

	//If the as the crow flies have been completely around the Halo we end the animation
	if(heading_set && (previous_percentage_of_distance_to_do==percentage_of_distance_to_do)){
		previous_percentage_of_distance_to_do = 100;
		percentage_of_distance_to_do = 0;
		percentage_of_led_to_light = 100;
		heading_set = false;
		return false;
	}

	counter_as_the_crow_flies ++;
	return true;
}

static bool set_which_led_to_light_as_the_crow_flies (uint8_t number_of_the_leds_to_light[], float previous_percentage_of_led_to_light, uint8_t goal_percentage_of_led_to_light, float brightness_of_the_leds []){

	float percentage_of_led;

	float exact_number_of_led_to_light;

	uint8_t difference_between_previous_and_goal_percentage_of_led_to_light;

	uint8_t goal_center_led;

	bool brightness_set = false;

	float brightness_changes;


	difference_between_previous_and_goal_percentage_of_led_to_light = MOD((int8_t)(previous_percentage_of_led_to_light - (uint8_t)goal_percentage_of_led_to_light), 100);

	if (difference_between_previous_and_goal_percentage_of_led_to_light == 0){
		return true;
	}
	//Depending on the distance between the as the crow flies leds opened and the goal as the crow flies led
	//The LEDS will move slower or faster
	brightness_changes = BRIGHTNESS_CHANGES_FOR_AS_THE_CROW_FLIES(difference_between_previous_and_goal_percentage_of_led_to_light);


	percentage_of_led = ((float)goal_percentage_of_led_to_light/100.00);


	exact_number_of_led_to_light = (float) (NUMBER_OF_LEDS * percentage_of_led);


	goal_center_led = ((uint8_t)exact_number_of_led_to_light) % NUMBER_OF_LEDS;

	//If the LED that are ON aren't the right one for the percentage of led to light
	if (goal_center_led != number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES]){

		// rotation_direction
		if(as_the_crow_flies_is_going_clockwise(goal_center_led, number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES]))
		//If the LEDs have to move clockwise
		{
			turn_as_the_crow_flies_clockwise(number_of_the_leds_to_light, brightness_changes, brightness_of_the_leds);
		}

		//If the leds are turning counter clockwise
		else{
			turn_as_the_crow_flies_counter_clockwise(number_of_the_leds_to_light, brightness_changes, brightness_of_the_leds);
		}
	}


	//If the LEDs that are on are the right ones for the heading
	else{
		brightness_set = set_as_the_crow_flies_heading_leds(number_of_the_leds_to_light, brightness_changes, brightness_of_the_leds, exact_number_of_led_to_light);

	}


	if (brightness_set){
		return true;
	}

	return false;
}

static bool set_as_the_crow_flies_heading_leds(uint8_t number_of_the_leds_to_light[], float brightness_changes, float brightness_of_the_leds [], float exact_number_of_led_to_light){

	bool brightness_set;

	number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = MOD((number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] - 1), NUMBER_OF_LEDS);
	number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = MOD((number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] + 1), NUMBER_OF_LEDS);

	float indicator_for_brithness_of_leds_next_to_center_led = (1.00)* (exact_number_of_led_to_light - number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES]);

	if (indicator_for_brithness_of_leds_next_to_center_led <= 0){
		if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] < 0.5){
			brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] + brightness_changes;
			brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES]= brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES]  - brightness_changes;
			brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES]= brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES]  + brightness_changes;
			if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] >= 0.5){
				brightness_set = true;
			}
		}
		else if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] > 0.5) {
			brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES]  - brightness_changes;
			brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES]  + brightness_changes;
			if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] <= 0.5){
				brightness_set = true;
			}
		}
		else {
			brightness_set = true;
		}
	}
	else if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] > indicator_for_brithness_of_leds_next_to_center_led){
		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] - brightness_changes;
		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES]  + brightness_changes;
		if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] <= indicator_for_brithness_of_leds_next_to_center_led){
			brightness_set = true;
		}
	}
	else if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] < indicator_for_brithness_of_leds_next_to_center_led){
		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = (brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] - brightness_changes);
		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES]  + brightness_changes;
		if (brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] >= indicator_for_brithness_of_leds_next_to_center_led){
				brightness_set = true;
		}
	}
	return brightness_set;
}

static bool turn_as_the_crow_flies_clockwise(uint8_t number_of_the_leds_to_light[], float brightness_changes, float brightness_of_the_leds []){


	//If the left LED is closing
	if ((brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] - brightness_changes) < brightness_changes){

		number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES];
		number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES];
		number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = number_of_the_leds_to_light[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES];
		number_of_the_leds_to_light[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = MOD((number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] + 1),NUMBER_OF_LEDS);

		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES];
		brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = 1;
		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES];
		brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = 0;
	}

	//If the left led is almost closed
	else if ((brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] - brightness_changes) < (brightness_changes * 2)){

		number_of_the_leds_to_light[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = MOD((number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] + 1), NUMBER_OF_LEDS);

		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] - brightness_changes;
		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] + brightness_changes;
		brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] - brightness_changes;
		brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] + brightness_changes;
	}

	//Closing the left led
	else{
		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] - brightness_changes;
		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] + brightness_changes;
		brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = 1;
	}

	return true;

}

static bool turn_as_the_crow_flies_counter_clockwise(uint8_t number_of_the_leds_to_light[], float brightness_changes, float brightness_of_the_leds []){

	//If the right LED is closing
	if ((brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] - brightness_changes) < brightness_changes){

		number_of_the_leds_to_light[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES];
		number_of_the_leds_to_light[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES];
		number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = number_of_the_leds_to_light[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES];
		number_of_the_leds_to_light[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = MOD((number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] - 1), NUMBER_OF_LEDS);

		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES];
		brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = 1;
		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES];
		brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = 0;
	}

	//If the right led is almost closed
	else if ((brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] - brightness_changes) < (brightness_changes * 2)){

		number_of_the_leds_to_light[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = MOD((number_of_the_leds_to_light[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] - 1), NUMBER_OF_LEDS);

		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] + brightness_changes;
		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] - brightness_changes;
		brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] - brightness_changes;
		brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_FOURTH_LED_AS_THE_CROW_FLIES] + brightness_changes;
	}

	//Closing the right led
	else{
		brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_RIGHT_LED_AS_THE_CROW_FLIES] - brightness_changes;
		brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] = brightness_of_the_leds[ARRAY_LEFT_LED_AS_THE_CROW_FLIES] + brightness_changes;
		brightness_of_the_leds[ARRAY_CENTER_LED_AS_THE_CROW_FLIES] = 1;
	}

	return true;
}


static bool set_new_colors_for_destination_as_the_crow_flies (uint8_t pixel_as_the_crow_flies [],  uint8_t goal_percentage_to_destination, uint8_t * previous_percentage_to_destination){

	uint8_t percentage_to_destination=0;

	if (goal_percentage_to_destination > *previous_percentage_to_destination){
		percentage_to_destination = *previous_percentage_to_destination + 1;
	}
	if (goal_percentage_to_destination < *previous_percentage_to_destination){
		percentage_to_destination = *previous_percentage_to_destination - 1;
	}

	//Defines the color of the led depending on the distance of the destination turns from orange to green
	if (percentage_to_destination <= 50){
		pixel_as_the_crow_flies[ARRAY_RED_PIXEL] = BRIGHTNESS_MAX;
		pixel_as_the_crow_flies [ARRAY_GREEN_PIXEL] = BRIGHTNESS_GREEN_MIN_AS_THE_CROW_FLIES + ((BRIGHTNESS_MAX - BRIGHTNESS_GREEN_MIN_AS_THE_CROW_FLIES)*((float)(percentage_to_destination*2)/100));
		pixel_as_the_crow_flies [ARRAY_BLUE_PIXEL] = 0;
	}
	else if (percentage_to_destination > 50){
		pixel_as_the_crow_flies[ARRAY_RED_PIXEL] = BRIGHTNESS_RED_MAX_AS_THE_CROW_FLIES * (((float)(100 - percentage_to_destination)/100)*2);
		pixel_as_the_crow_flies [ARRAY_GREEN_PIXEL] = BRIGHTNESS_MAX;
		pixel_as_the_crow_flies [ARRAY_BLUE_PIXEL] = 0;
	}

	pixel_as_the_crow_flies[ARRAY_RED_PIXEL] = correction_gamma_led_rgb[pixel_as_the_crow_flies[ARRAY_RED_PIXEL]];
	pixel_as_the_crow_flies[ARRAY_GREEN_PIXEL] = correction_gamma_led_rgb[pixel_as_the_crow_flies[ARRAY_GREEN_PIXEL]];
	pixel_as_the_crow_flies[ARRAY_BLUE_PIXEL] = correction_gamma_led_rgb[pixel_as_the_crow_flies[ARRAY_BLUE_PIXEL]];

	*previous_percentage_to_destination = percentage_to_destination;

	if (percentage_to_destination != goal_percentage_to_destination){
		return false;
	}

	else{
		return true;
	}
}

/*
 * FIRST CONNECTION
 */
bool Animations_first_connection(bool reset, uint8_t null, uint8_t null_2){

	static uint16_t first_connetion_counter_fade_in=0;

	static uint16_t first_connection_counter_fade_out=NUMBER_OF_BRIGHTNESS_CHANGES - 1;

	static uint16_t first_conection_counter = 0;

	HaloPixel_setBrightness(brightness);

	red_pixel = 1;
	green_pixel = 1;
	blue_pixel = 1;

	if (!reset){
		if(first_conection_counter%MED_FAST == 0){

			//LEDs lights one by one
			if (first_connetion_counter_fade_in < CALCULATE_NUMBER_OF_ITERATIONS(NUMBER_OF_LEDS)){
				allLightsOn_number_of_LEDs_Halo(NUMBER_OF_LEDS, red_pixel, green_pixel, blue_pixel, first_connetion_counter_fade_in);
				first_connetion_counter_fade_in++;
			}

			//Lights fade out
			else{
				allLightsOff(NUMBER_OF_LEDS, 0, red_pixel, green_pixel, blue_pixel, first_connection_counter_fade_out);
				first_connection_counter_fade_out--;
			}

			//Re initialize static variables for next call of the animation and returns true when the animation is done
			if (first_connection_counter_fade_out==0){
				turnOffAllLEDs(0,0,0);
				first_connection_counter_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
				first_connetion_counter_fade_in = 0;
				first_conection_counter = 0;
				return true;
			}
		}

		first_conection_counter++;
		return false;
	}

	else{
		turnOffAllLEDs(0,0,0);
		first_connection_counter_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		first_connetion_counter_fade_in = 0;
		first_conection_counter = 0;
		return true;
	}
}

/*
 * FIRST CONNECTION WAIT FOR ONBOARDING
 */
bool Animations_first_connection_wait_for_onboarding(bool reset, uint8_t null, uint8_t null_2){

	static uint16_t first_connetion_counter_fade_in_wait_for_onboarding = 0;

	static uint16_t first_connection_counter_fade_out_wait_for_onboarding = NUMBER_OF_BRIGHTNESS_CHANGES - 1;

	static uint16_t first_conection_counter_wait_for_onboarding = 0;


	HaloPixel_setBrightness(brightness);

	if(first_conection_counter_wait_for_onboarding%SLOWEST == 0){

		//LEDS fade in
		if (first_connetion_counter_fade_in_wait_for_onboarding < NUMBER_OF_BRIGHTNESS_CHANGES){
			allLightsOff(NUMBER_OF_LEDS, 0, RED_PIXEL_CONNECTION, GREEN_PIXEL_CONNECTION, BLUE_PIXEL_CONNECTION, first_connetion_counter_fade_in_wait_for_onboarding);
			first_connetion_counter_fade_in_wait_for_onboarding++;
		}

		//LEDS fade out
		else{
			allLightsOff(NUMBER_OF_LEDS, 0, RED_PIXEL_CONNECTION, GREEN_PIXEL_CONNECTION, BLUE_PIXEL_CONNECTION, first_connection_counter_fade_out_wait_for_onboarding);
			first_connection_counter_fade_out_wait_for_onboarding--;
		}

		//Re initialize static variables for next call of the animation and returns true when the animation is done
		if (first_connection_counter_fade_out_wait_for_onboarding==0){
			turnOffAllLEDs(0,0,0);
			first_connection_counter_fade_out_wait_for_onboarding = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
			first_connetion_counter_fade_in_wait_for_onboarding = 0;
			first_conection_counter_wait_for_onboarding = 0;
			return true;
		}
	}

	first_conection_counter_wait_for_onboarding++;
	return false;
}

bool Animations_smarthalo_on_charge(bool reset, uint8_t battery_charge, uint8_t null_2){

	uint8_t percentage_of_battery_charge;

	float red;
	float green;
	float blue;

	static uint16_t couter_smarthalo_on_charge_animations = 0;

	static uint16_t level_of_brightness = 0;

	static uint8_t level_of_brightness_fade_in = 0;

	static uint8_t level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1 ;

	static uint8_t number_of_flash = 0;

	static uint8_t level_of_brightness_turn_off_the_leds = NUMBER_OF_BRIGHTNESS_CHANGES - 1;

	uint8_t number_of_leds_to_light = battery_charge * NUMBER_OF_LEDS / 100;


	HaloPixel_setBrightness(brightness);

	percentage_of_battery_charge = battery_charge;


	if (!reset){
		if (couter_smarthalo_on_charge_animations%MED_FAST ==0){

			//Depending on the percentage of the charge of the battery we light more or less
			//leds and in differents colors
			if (percentage_of_battery_charge <= BATTERY_ON_CHARGE_FIRST_QUARTER){
				red = (float) RED_PIXEL_RED / BRIGHTNESS_MAX;
				green = (float) GREEN_PIXEL_RED / BRIGHTNESS_MAX;
				blue = (float) BLUE_PIXEL_RED / BRIGHTNESS_MAX;

			}
			else if (percentage_of_battery_charge <= BATTERY_ON_CHARGE_SECOND_QUARTER){
				red = (float)RED_PIXEL_YELLOW / BRIGHTNESS_MAX;
				green = (float)GREEN_PIXEL_YELLOW/ BRIGHTNESS_MAX;
				blue = (float)BLUE_PIXEL_YELLOW / BRIGHTNESS_MAX;
			}
			else if (percentage_of_battery_charge <= BATTERY_ON_CHARGE_THIRD_QUARTER){
				red = (float)RED_PIXEL_MID_GREEN / BRIGHTNESS_MAX;
				green = (float)GREEN_PIXEL_MID_GREEN / BRIGHTNESS_MAX;
				blue = (float)BLUE_PIXEL_MID_GREEN / BRIGHTNESS_MAX;
			}
			else {
				red = (float)RED_PIXEL_GREEN / BRIGHTNESS_MAX;
				green = (float)GREEN_PIXEL_GREEN / BRIGHTNESS_MAX;
				blue = (float)BLUE_PIXEL_GREEN / BRIGHTNESS_MAX;
			}

			//Lights the leds one by one
			if (level_of_brightness <= CALCULATE_NUMBER_OF_ITERATIONS(number_of_leds_to_light)){
				allLightsOn_number_of_LEDs_Halo(number_of_leds_to_light, red, green, blue, level_of_brightness);
				level_of_brightness++;
			}

			//Flash the quarter that indicates the level of the battery
			else if (number_of_flash < NUMBER_OF_FLASH_SMARTHALO_ON_CHARGE){
				if (couter_smarthalo_on_charge_animations%MED ==0){

					//Fades out
					if (level_of_brightness_fade_out > 0){
						allLightsOff(number_of_leds_to_light - 1, number_of_leds_to_light - 1, red, green, blue, level_of_brightness_fade_out);
						level_of_brightness_fade_out--;
					}

					//Fades in
					else if (level_of_brightness_fade_in < NUMBER_OF_BRIGHTNESS_CHANGES){
						allLightsOff(number_of_leds_to_light - 1, number_of_leds_to_light - 1, red, green, blue, level_of_brightness_fade_in);
						level_of_brightness_fade_in++;
					}

					//Re initialize the variables to flash again the lights
					else {
						number_of_flash++;
						level_of_brightness_fade_in = 0;
						level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
					}
				}
			}

			//Fades out all the lights
			else if (level_of_brightness_turn_off_the_leds >0){
				allLightsOff(number_of_leds_to_light-1, 0, red, green, blue, level_of_brightness_turn_off_the_leds);
				level_of_brightness_turn_off_the_leds--;
			}

			//Turn off all leds, reinitialize the static variables and returns true when the animation is done
			else{
				turnOffAllLEDs(0,0,0);
				level_of_brightness_fade_in = 0;
				level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
				level_of_brightness_turn_off_the_leds = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
				level_of_brightness = 0;
				couter_smarthalo_on_charge_animations = 0;
				number_of_flash = 0;
				return true;
			}

		}

		couter_smarthalo_on_charge_animations++;
		return false;
	}
	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness_fade_in = 0;
		level_of_brightness_fade_out = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		level_of_brightness_turn_off_the_leds = NUMBER_OF_BRIGHTNESS_CHANGES - 1;
		level_of_brightness = 0;
		couter_smarthalo_on_charge_animations = 0;
		number_of_flash = 0;
		return true;
	}

}

static void set_Speedometer_Colors(float pixel_for_the_led [], uint8_t led_to_light){

	if (led_to_light == 3){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 57;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 181;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 74;
	}
	if (led_to_light == 4){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 78;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 186;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 68;
	}
	if (led_to_light == 5){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 100;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 191;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 62;
	}
	if (led_to_light == 6){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 121;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 196;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 56;
	}
	if (led_to_light == 7){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 142;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 201;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 50;
	}
	if (led_to_light == 8){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 164;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 207;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 45;
	}
	if (led_to_light == 9){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 185;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 212;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 39;
	}
	if (led_to_light == 10){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 206;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 217;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 33;
	}
	if (led_to_light == 11){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 228;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 222;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 27;
	}
	if (led_to_light == 12){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 249;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 227;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 21;
	}
	if (led_to_light == 13){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 250;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 205;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 23;
	}
	if (led_to_light == 14){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 250;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 183;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 25;
	}
	if (led_to_light == 15){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 251;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 161;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 26;
	}
	if (led_to_light == 16){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 252;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 139;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 28;
	}
	if (led_to_light == 17){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 252;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 117;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 30;
	}
	if (led_to_light == 18){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 253;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 95;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 32;
	}
	if (led_to_light == 19){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 254;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 73;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 33;
	}
	if (led_to_light == 20){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 254;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 51;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 35;
	}
	if (led_to_light == 21){
		pixel_for_the_led[ARRAY_RED_PIXEL] = 255;
		pixel_for_the_led[ARRAY_GREEN_PIXEL] = 29;
		pixel_for_the_led[ARRAY_BLUE_PIXEL] = 37;
	}
	pixel_for_the_led[ARRAY_RED_PIXEL] = (float)correction_gamma_led_rgb[(uint8_t)pixel_for_the_led[ARRAY_RED_PIXEL]]/BRIGHTNESS_MAX;
	pixel_for_the_led[ARRAY_GREEN_PIXEL] = (float)correction_gamma_led_rgb[(uint8_t)pixel_for_the_led[ARRAY_GREEN_PIXEL]]/BRIGHTNESS_MAX;
	pixel_for_the_led[ARRAY_BLUE_PIXEL] = (float)correction_gamma_led_rgb[(uint8_t)pixel_for_the_led[ARRAY_BLUE_PIXEL]]/BRIGHTNESS_MAX;
}

/*
 * ANIMATIONS REROUTING
 */
bool Animations_rerouting(bool reset, uint8_t null_1, uint8_t null_2){

	HaloPixel_setBrightness(brightness);

	float red_pixel_pairingfade=(float)RED_PIXEL_REROUTING/BRIGHTNESS_MAX;
	float green_pixel_pairingfade=(float) GREEN_PIXEL_REROUTING/BRIGHTNESS_MAX;
	float blue_pixel_pairingfade=(float) BLUE_PIXEL_REROUTING/BRIGHTNESS_MAX;
	static uint8_t level_of_brightness = NUMBER_OF_BRIGHTNESS_CHANGES;
	static uint16_t counter_rerouting_animation = 0;

	if (!reset){
		if (counter_rerouting_animation%SLOW == 0){

			//fades out
			allLightsOff(NUMBER_OF_LEDS-1, 0, red_pixel_pairingfade, green_pixel_pairingfade, blue_pixel_pairingfade, level_of_brightness);
			level_of_brightness--;

			//turns off the leds, re initialize the static variable and return true when animation is done
			if (level_of_brightness == 0){
				turnOffAllLEDs(0,0,0);
				level_of_brightness = NUMBER_OF_BRIGHTNESS_CHANGES;
				counter_rerouting_animation = 0;
				return true;
			}
		}
		counter_rerouting_animation++;
		return false;
		}

	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness = NUMBER_OF_BRIGHTNESS_CHANGES;
		counter_rerouting_animation = 0;
		return true;
	}
}

/*
 * ANIMATIONS DISCONNECTED
 */
bool Animations_disconnected(bool reset, uint8_t null_1, uint8_t null_2){

	HaloPixel_setBrightness(brightness);

	float red_pixel_pairingfade=(float)RED_PIXEL_DISCONNECTED/BRIGHTNESS_MAX;
	float green_pixel_pairingfade=(float) GREEN_PIXEL_DISCONNECTED/BRIGHTNESS_MAX;
	float blue_pixel_pairingfade=(float) BLUE_PIXEL_DISCONNECTED/BRIGHTNESS_MAX;
	static uint8_t level_of_brightness = NUMBER_OF_BRIGHTNESS_CHANGES;
	static uint8_t number_of_flashes = 0;
	static uint16_t counter_disconnected_animation = 0;

	if (!reset){
		if (counter_disconnected_animation%SLOW == 0){

			//fades out
			allLightsOff(NUMBER_OF_LEDS-1, 0, red_pixel_pairingfade, green_pixel_pairingfade, blue_pixel_pairingfade, level_of_brightness);
			level_of_brightness--;

			//turns off the leds, re initialize the static variable and return true when animation is done
			if (level_of_brightness == 0){
				turnOffAllLEDs(0,0,0);
				level_of_brightness = NUMBER_OF_BRIGHTNESS_CHANGES;
				number_of_flashes ++;
				if (number_of_flashes == NUMBER_OF_FLASH_FOR_DISCONNECTED){
					number_of_flashes = 0;
					counter_disconnected_animation = 0;
					return true;
				}
			}
		}
		counter_disconnected_animation++;
		return false;
		}

	else{
		turnOffAllLEDs(0,0,0);
		level_of_brightness = NUMBER_OF_BRIGHTNESS_CHANGES;
		counter_disconnected_animation = 0;
		return true;
	}
}


//SETSMARTHALOCOLORS
static void set_SmartHalo_Colors() {


/*
  HaloPixel_setColor(0, HaloPixel_RGB_Color(123, 77, 155));
  //(87, 97, 171)
  HaloPixel_setColor(1, HaloPixel_RGB_Color(87, 97, 171));
  //(50, 118, 187)
  HaloPixel_setColor(2, HaloPixel_RGB_Color(50, 118, 187));
  //(14, 138, 203) ****BLEU
  HaloPixel_setColor(3, HaloPixel_RGB_Color(14, 138, 203));
  //(18, 145, 188)
  HaloPixel_setColor(4, HaloPixel_RGB_Color(18, 145, 188));
  //(22, 152, 174)
  HaloPixel_setColor(5, HaloPixel_RGB_Color(22, 152, 174));
  //(26, 159, 159)
  HaloPixel_setColor(6, HaloPixel_RGB_Color(26, 159, 159));
  //(29, 166, 144)
  HaloPixel_setColor(7, HaloPixel_RGB_Color(29, 166, 144));
  //(33, 173, 130)
  HaloPixel_setColor(8, HaloPixel_RGB_Color(33, 173, 130));
  //(37, 180, 115) ****VERT
  HaloPixel_setColor(9, HaloPixel_RGB_Color(37, 180, 115));
  //(73, 187, 100)
  HaloPixel_setColor(10, HaloPixel_RGB_Color(73, 187, 100));
  //(110, 194, 86)
  HaloPixel_setColor(11, HaloPixel_RGB_Color(110, 194, 86));
  //(146, 201, 71)
  HaloPixel_setColor(12, HaloPixel_RGB_Color(146, 201, 71));
  //(182, 208, 56)
  HaloPixel_setColor(13, HaloPixel_RGB_Color(182, 208, 56));
  //(219, 215, 42)
  HaloPixel_setColor(14, HaloPixel_RGB_Color(219, 215, 42));
  //(255, 222, 27) **** JAUNE
  HaloPixel_setColor(15, HaloPixel_RGB_Color(255, 222, 27));
  //(251, 188, 40)
  HaloPixel_setColor(16, HaloPixel_RGB_Color(251, 188, 40));
  //(247, 153, 54)
  HaloPixel_setColor(17, HaloPixel_RGB_Color(247, 153, 54));
  //(243, 119, 67)
  HaloPixel_setColor(18, HaloPixel_RGB_Color(243, 119, 67));
  //(240, 85, 80)
  HaloPixel_setColor(19, HaloPixel_RGB_Color(240, 85, 80));
  //(236, 50, 94)
  HaloPixel_setColor(20, HaloPixel_RGB_Color(236, 50, 94));
  //(232, 16, 107) ***ROSEROUGE
  HaloPixel_setColor(21, HaloPixel_RGB_Color(232, 16, 107));
  //(196, 36, 123)
  HaloPixel_setColor(22, HaloPixel_RGB_Color(196, 36, 123));
  //(159, 57, 139)
  HaloPixel_setColor(23, HaloPixel_RGB_Color(159, 57, 139));
  //(123, 77, 155)

  */
	/*
	HaloPixel_setColor_with_correction_gamma(0, HaloPixel_RGB_Color(123, 77, 155));
	  //(87, 97, 171)
	HaloPixel_setColor_with_correction_gamma(1, HaloPixel_RGB_Color(87, 97, 171));
	  //(50, 118, 187)
	HaloPixel_setColor_with_correction_gamma(2, HaloPixel_RGB_Color(50, 118, 187));
	  //(14, 138, 203) ****BLEU
	HaloPixel_setColor_with_correction_gamma(3, HaloPixel_RGB_Color(14, 138, 203));
	  //(18, 145, 188)
	HaloPixel_setColor_with_correction_gamma(4, HaloPixel_RGB_Color(18, 145, 188));
	  //(22, 152, 174)
	HaloPixel_setColor_with_correction_gamma(5, HaloPixel_RGB_Color(22, 152, 174));
	  //(26, 159, 159)
	HaloPixel_setColor_with_correction_gamma(6, HaloPixel_RGB_Color(26, 159, 159));
	  //(29, 166, 144)
	HaloPixel_setColor_with_correction_gamma(7, HaloPixel_RGB_Color(29, 166, 144));
	  //(33, 173, 130)
	HaloPixel_setColor_with_correction_gamma(8, HaloPixel_RGB_Color(33, 173, 130));
	  //(37, 180, 115) ****VERT
	HaloPixel_setColor_with_correction_gamma(9, HaloPixel_RGB_Color(37, 180, 115));
	  //(73, 187, 100)
	HaloPixel_setColor_with_correction_gamma(10, HaloPixel_RGB_Color(73, 187, 100));
	  //(110, 194, 86)
	HaloPixel_setColor_with_correction_gamma(11, HaloPixel_RGB_Color(110, 194, 86));
	  //(146, 201, 71)
	HaloPixel_setColor_with_correction_gamma(12, HaloPixel_RGB_Color(146, 201, 71));
	  //(182, 208, 56)
	HaloPixel_setColor_with_correction_gamma(13, HaloPixel_RGB_Color(182, 208, 56));
	  //(219, 215, 42)
	HaloPixel_setColor_with_correction_gamma(14, HaloPixel_RGB_Color(219, 215, 42));
	  //(255, 222, 27) **** JAUNE
	HaloPixel_setColor_with_correction_gamma(15, HaloPixel_RGB_Color(255, 222, 27));
	  //(251, 188, 40)
	HaloPixel_setColor_with_correction_gamma(16, HaloPixel_RGB_Color(251, 188, 40));
	  //(247, 153, 54)
	HaloPixel_setColor_with_correction_gamma(17, HaloPixel_RGB_Color(247, 153, 54));
	  //(243, 119, 67)
	HaloPixel_setColor_with_correction_gamma(18, HaloPixel_RGB_Color(243, 119, 67));
	  //(240, 85, 80)
	HaloPixel_setColor_with_correction_gamma(19, HaloPixel_RGB_Color(240, 85, 80));
	  //(236, 50, 94)
	HaloPixel_setColor_with_correction_gamma(20, HaloPixel_RGB_Color(236, 50, 94));
	  //(232, 16, 107) ***ROSEROUGE
	HaloPixel_setColor_with_correction_gamma(21, HaloPixel_RGB_Color(232, 16, 107));
	  //(196, 36, 123)
	HaloPixel_setColor_with_correction_gamma(22, HaloPixel_RGB_Color(196, 36, 123));
	  //(159, 57, 139)
	HaloPixel_setColor_with_correction_gamma(23, HaloPixel_RGB_Color(159, 57, 139));
	  //(123, 77, 155)*/

	/*HaloPixel_setColor(0, HaloPixel_RGB_Color(87, 93, 171));

	HaloPixel_setColor(1, HaloPixel_RGB_Color(50, 112, 187));

	HaloPixel_setColor(2, HaloPixel_RGB_Color(14, 131, 203));

	HaloPixel_setColor(3, HaloPixel_RGB_Color(21, 139, 181));

	HaloPixel_setColor(4, HaloPixel_RGB_Color(28, 148, 160));

	HaloPixel_setColor(5, HaloPixel_RGB_Color(36, 156, 139));

	HaloPixel_setColor(6, HaloPixel_RGB_Color(43, 164, 117));

	HaloPixel_setColor(7, HaloPixel_RGB_Color(50, 173, 96));

	HaloPixel_setColor(8, HaloPixel_RGB_Color(57, 181, 74));

	HaloPixel_setColor(9, HaloPixel_RGB_Color(89, 189, 65));

	HaloPixel_setColor(10, HaloPixel_RGB_Color(121, 196, 56));

	HaloPixel_setColor(11, HaloPixel_RGB_Color(153, 204, 48));

	HaloPixel_setColor(12, HaloPixel_RGB_Color(185, 212, 39));

	HaloPixel_setColor(13, HaloPixel_RGB_Color(217, 219, 30));

	HaloPixel_setColor(14, HaloPixel_RGB_Color(249, 227, 21));

	HaloPixel_setColor(15, HaloPixel_RGB_Color(246, 192, 35));

	HaloPixel_setColor(16, HaloPixel_RGB_Color(243, 157, 50));

	HaloPixel_setColor(17, HaloPixel_RGB_Color(241, 122, 64));

	HaloPixel_setColor(18, HaloPixel_RGB_Color(238, 86, 78));

	HaloPixel_setColor(19, HaloPixel_RGB_Color(235, 51, 93));

	HaloPixel_setColor(20, HaloPixel_RGB_Color(232, 16, 107));

	HaloPixel_setColor(21, HaloPixel_RGB_Color(196, 35, 123));

	HaloPixel_setColor(22, HaloPixel_RGB_Color(159, 54, 139));

	HaloPixel_setColor(23, HaloPixel_RGB_Color(123, 74, 155 )); */


	  HaloPixel_setColor_with_correction_gamma(0, HaloPixel_RGB_Color(87, 93, 171));

  HaloPixel_setColor_with_correction_gamma(1, HaloPixel_RGB_Color(50, 112, 187));

  HaloPixel_setColor_with_correction_gamma(2, HaloPixel_RGB_Color(14, 131, 203));

  HaloPixel_setColor_with_correction_gamma(3, HaloPixel_RGB_Color(21, 139, 181));

  HaloPixel_setColor_with_correction_gamma(4, HaloPixel_RGB_Color(28, 148, 160));

  HaloPixel_setColor_with_correction_gamma(5, HaloPixel_RGB_Color(36, 156, 139));

  HaloPixel_setColor_with_correction_gamma(6, HaloPixel_RGB_Color(43, 164, 117));

  HaloPixel_setColor_with_correction_gamma(7, HaloPixel_RGB_Color(50, 173, 96));

  HaloPixel_setColor_with_correction_gamma(8, HaloPixel_RGB_Color(57, 181, 74));

  HaloPixel_setColor_with_correction_gamma(9, HaloPixel_RGB_Color(89, 189, 65));

  HaloPixel_setColor_with_correction_gamma(10, HaloPixel_RGB_Color(121, 196, 56));

  HaloPixel_setColor_with_correction_gamma(11, HaloPixel_RGB_Color(153, 204, 48));

  HaloPixel_setColor_with_correction_gamma(12, HaloPixel_RGB_Color(185, 212, 39));

  HaloPixel_setColor_with_correction_gamma(13, HaloPixel_RGB_Color(217, 219, 30));

  HaloPixel_setColor_with_correction_gamma(14, HaloPixel_RGB_Color(249, 227, 21));

  HaloPixel_setColor_with_correction_gamma(15, HaloPixel_RGB_Color(246, 192, 35));

  HaloPixel_setColor_with_correction_gamma(16, HaloPixel_RGB_Color(243, 157, 50));

  HaloPixel_setColor_with_correction_gamma(17, HaloPixel_RGB_Color(241, 122, 64));

  HaloPixel_setColor_with_correction_gamma(18, HaloPixel_RGB_Color(238, 86, 78));

  HaloPixel_setColor_with_correction_gamma(19, HaloPixel_RGB_Color(235, 51, 93));

  HaloPixel_setColor_with_correction_gamma(20, HaloPixel_RGB_Color(232, 16, 107));

  HaloPixel_setColor_with_correction_gamma(21, HaloPixel_RGB_Color(196, 35, 123));

  HaloPixel_setColor_with_correction_gamma(22, HaloPixel_RGB_Color(159, 54, 139));

  HaloPixel_setColor_with_correction_gamma(23, HaloPixel_RGB_Color(123, 74, 155 ));
}

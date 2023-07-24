/*
 * old_SH_Animations.c
 *
 *  Created on: 2016-03-02
 *      Author: SmartHalo
 */


/*
void Animations_uTurn(){

	red_pixel = TRUE;
	green_pixel = FALSE;
	blue_pixel = FALSE;

  //Fading in and out to blink
	HaloPixel_setBrightness(brightness);
    allLightsOn(LQUARTER, QUARTER, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
    allLightsOff(LQUARTER, QUARTER, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
}*/

/*void Animations_straight(){

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	HaloPixel_setBrightness(brightness);
	allLightsOn(LQUARTER, NUMBER_OF_LEDS, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, 4);
}*/

/*void Animations_destination(){
	HaloPixel_setBrightness(brightness);

	red_pixel = FALSE;
	green_pixel = TRUE;
	blue_pixel = FALSE;

	for (uint k=0;k<4;k++){
		allLightsOn(NUMBER_OF_LEDS-1,0,red_pixel, green_pixel, blue_pixel,0,8);
		allLightsOff(NUMBER_OF_LEDS-1,0,red_pixel, green_pixel, blue_pixel,0,8);
  }
}*/

/*void Animations_longTap (){

	HaloPixel_setBrightness(brightness);

	red_pixel = TRUE;
	green_pixel = TRUE;
	blue_pixel = TRUE;

	allLightsOn(NUMBER_OF_LEDS-1, 0, red_pixel, green_pixel, blue_pixel, 0, DELAY_BRIGHTNESS);
}*/

/*void Animations_success(){
  // fade in
  for (uint8_t brightness_level=0; brightness_level<=BRIGHTNESS_MAX; brightness_level++) {

	set_SmartHalo_Colors();
    HaloPixel_setBrightness(brightness_level);
    HaloPixel_show();
    nrf_delay_ms(6);
  }
  nrf_delay_ms(100);

  // fade out
  for (uint8_t brightness_level=BRIGHTNESS_MAX; brightness_level>0; brightness_level--) {

	set_SmartHalo_Colors();
    HaloPixel_setBrightness(brightness_level);
    HaloPixel_show();
    nrf_delay_ms(6);
  }
  turnOffAllLEDs();
}*/

/*void Animations_alarm(Step stepOfAnimation) {

  HaloPixel_setBrightness(brightness);

  red_pixel = TRUE;
  green_pixel = FALSE;
  blue_pixel = FALSE;

  switch (stepOfAnimation){

  	  case FIRST:
  		for (uint i=0;i<NUMBER_OF_LEDS;i++){
  		    HaloPixel_setColor(i, HaloPixel_RGB_Color(255,0,0));
  		    HaloPixel_showPixel(i);
  		    nrf_delay_ms(300);
  		  }

  		  turnOffAllLEDs();
  	  break;

  	  case SECOND:

  		 for (uint i=0;i<NUMBER_OF_LEDS;i++){
  		    HaloPixel_setColor(i, HaloPixel_RGB_Color(255,0,0));
  		    HaloPixel_showPixel(i);
  		 }

  		 nrf_delay_ms(100);
  		 turnOffAllLEDs();
  	  break;

  	  default : break;

  }

}*/

/*void Animations_goalCompletion(uint8_t completion) {

	red_pixel = RED_PIXEL_GOAL_COMPLETITION;
	green_pixel =GREEN_PIXEL_GOAL_COMPLETITION;
	blue_pixel = BLUE_PIXEL_GOAL_COMPLETITION;

	float percentage_of_completion = (float) completion/100;
	uint number_of_leds_to_light = NUMBER_OF_LEDS * percentage_of_completion;

	HaloPixel_setBrightness(brightness);

	for (uint i = 0; i<number_of_leds_to_light;i++){

		HaloPixel_setColor(i, HaloPixel_RGB_Color(232, 16, 107));
		HaloPixel_showPixel(i);
		nrf_delay_ms(50);
	}

	nrf_delay_ms(2000);

	for (int j=BRIGHTNESS_MULTIPLICATOR-1;j>=0;j--) {
		for (uint i=0;i<number_of_leds_to_light;i++) {
			HaloPixel_setColor(i, HaloPixel_RGB_Color(red_pixel*j, green_pixel*j, blue_pixel*j));
			HaloPixel_showPixel(i);
			nrf_delay_ms(4);
		}
		nrf_delay_ms(10);
	}

	HaloPixel_clear();
}*/

/*void Animations_wrongTapCode() {

  HaloPixel_setBrightness(brightness);

  red_pixel = TRUE;
  green_pixel = FALSE;
  blue_pixel = FALSE;

  for (uint i=0;i<NUMBER_OF_LEDS;i++){
    HaloPixel_setColor(i, HaloPixel_RGB_Color(255,0,0));
    HaloPixel_showPixel(i);
    nrf_delay_ms(1000);
  }

  turnOffAllLEDs();

  for (uint j=0;j<5;j++){
	  nrf_delay_ms(500);
    for (uint i=0;i<NUMBER_OF_LEDS;i++){
      HaloPixel_setColor(i, HaloPixel_RGB_Color(255,0,0));
      HaloPixel_showPixel(i);
    }
    nrf_delay_ms(500);
    turnOffAllLEDs();
  }

}*/

//
/*static void waitPairing (uint led_min, uint led_max){

   for(int led=led_min; led<led_max; led++) {
	   for (uint i=0; i<NUMBER_OF_BRIGHTESS_CHANGES_PAIRING; i++){
		   uint pixel_color = PAIRING_MASK_RGBCOLOR[i];
		   HaloPixel_setColor(((NUMBER_OF_LEDS-i)+led)%NUMBER_OF_LEDS, HaloPixel_RGB_Color(pixel_color,pixel_color,pixel_color));
	   }
      HaloPixel_show();

      nrf_delay_ms(30);
  }

   HaloPixel_clear();
}*/

/*static void pairing(uint led_min, uint led_max) {

  for(int led=led_min; led<led_max; led++) {

	  for (uint i=0; i<NUMBER_OF_BRIGHTESS_CHANGES_PAIRING; i++)
	  {
		  uint pixel_color = PAIRING_MASK_RGBCOLOR[i];
		  HaloPixel_setColor(led-i, HaloPixel_RGB_Color(pixel_color,pixel_color,pixel_color));
	  }

      HaloPixel_show();

      nrf_delay_ms(30);
  }

  HaloPixel_clear();
}*/
/*bool Animations_waitPairing (){

	static uint16_t counter_brightness = 0;
	static uint16_t counter_waitpairing_animation = 0;
	static int8_t led = 0;


	if (counter_waitpairing_animation%100 ==0){
		while (counter_brightness != NUMBER_OF_BRIGHTESS_CHANGES_PAIRING - 1){
			uint pixel_color = PAIRING_MASK_RGBCOLOR[counter_brightness];
			HaloPixel_setColor(((NUMBER_OF_LEDS-counter_brightness)+led)%NUMBER_OF_LEDS, HaloPixel_RGB_Color(pixel_color,pixel_color,pixel_color));
			counter_brightness++;
		}
		HaloPixel_show();
		counter_brightness = 0;
		led++;
	}

	if (led == NUMBER_OF_LEDS){
		HaloPixel_clear();
		counter_brightness=0;
		counter_waitpairing_animation=0;
		led = 0;
		return true;
	}

	else {
		HaloPixel_clear();
		counter_waitpairing_animation++;
		return false;
	}

}*/

/*
void Animations_slightRight(Step step_of_animation){

	HaloPixel_setBrightness(brightness);

	if (step_of_animation == FIRST){
		red_pixel = TRUE;
		green_pixel = TRUE;
		blue_pixel = TRUE;
	}

	else{
		red_pixel = FALSE;
		green_pixel = TRUE;
		blue_pixel = FALSE;
	}

	int set_of_leds_to_light = (int) step_of_animation - 2;

	switch ((uint) step_of_animation) {  //Which step we are

		case FIRST :  // Everything white
			allLightsOn(QUARTER,0, red_pixel, green_pixel, blue_pixel,DELAY_BRIGHTNESS,WAIT_DELAY_LONG);
		break;

		case SECOND :  //First and last LED green
		  lightsOneByOne(QUARTER, 0, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);

		break;

		case THIRD :  //Second set of LED green
		  lightsOneByOne(QUARTER, 0, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		break;

		case FOURTH :  //Third set of LED green
		  lightsOneByOne(QUARTER, 0, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		break;

		case FIFTH :  //Fading in and out to blink
			allLightsOn(QUARTER, 0, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
			allLightsOff(QUARTER, 0, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
		break;
	  }
}
*/

/*
void Animations_hardRight(Step step_of_animation){

	HaloPixel_setBrightness(brightness);

	if (step_of_animation == FIRST){
		red_pixel = TRUE;
		green_pixel = TRUE;
		blue_pixel = TRUE;
	}

	else{
		red_pixel = FALSE;
		green_pixel = TRUE;
		blue_pixel = FALSE;
	}

	int set_of_leds_to_light = (int) step_of_animation - 2;

	switch ((uint) step_of_animation) {  //Which step we are

		case FIRST :  // Everything white
	      allLightsOn(HALF,QUARTER, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
	    break;

	    case SECOND :  //First and last LED green
	      lightsOneByOne(HALF, QUARTER,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
	    break;

	    case THIRD :  //Second set of LED green
	      lightsOneByOne(HALF, QUARTER,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
	    break;

	    case FOURTH :  //Third set of LED green
	      lightsOneByOne(HALF, QUARTER,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
	    break;

	    case FIFTH :  //Fading in and out to blink
	      //How will we stop blinking ??
	    	allLightsOn(HALF,QUARTER, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
	        allLightsOff(HALF, QUARTER,  red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
		break;
	  }
}*/

/*void Animations_hardLeft(Step step_of_animation){

	HaloPixel_setBrightness(brightness);

	if (step_of_animation == FIRST){
		red_pixel = TRUE;
		green_pixel = TRUE;
		blue_pixel = TRUE;
	}

	else{
		red_pixel = FALSE;
		green_pixel = TRUE;
		blue_pixel = FALSE;
	}

	int set_of_leds_to_light = (int) step_of_animation - 2;

	switch ((uint) step_of_animation) {  //Which step we are

		case FIRST :  // Everything white
		  allLightsOn(LQUARTER,HALF, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS,WAIT_DELAY_LONG);
		break;

		case SECOND :  //First and last LED green
		  lightsOneByOne(LQUARTER,HALF, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		break;

		case THIRD :  //Second set of LED green
		  lightsOneByOne(LQUARTER,HALF, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		break;

		case FOURTH :  //Third set of LED green
		  lightsOneByOne(LQUARTER,HALF, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		break;

		case FIFTH :  //Fading in and out to blink
			allLightsOn(LQUARTER, HALF,  red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
			allLightsOff(LQUARTER, HALF,  red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);

    break;
  }
}
*/

/*void Animations_slightLeft(Step step_of_animation){

	HaloPixel_setBrightness(brightness);

	if (step_of_animation == FIRST){
		red_pixel = TRUE;
		green_pixel = TRUE;
		blue_pixel = TRUE;
	}

	else{
		red_pixel = FALSE;
		green_pixel = TRUE;
		blue_pixel = FALSE;
	}

	int set_of_leds_to_light = (int) step_of_animation - 2;

	switch ((uint) step_of_animation) {  //Which step we are

		case FIRST :  // Everything white
			allLightsOn(NUMBER_OF_LEDS, LQUARTER, red_pixel, green_pixel, blue_pixel,DELAY_BRIGHTNESS,WAIT_DELAY_LONG);
		break;

		case SECOND :  //First and last LED green
		  lightsOneByOne(NUMBER_OF_LEDS, LQUARTER, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);

		break;

		case THIRD :  //Second set of LED green
		  lightsOneByOne(NUMBER_OF_LEDS, LQUARTER, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		break;

		case FOURTH :  //Third set of LED green
		  lightsOneByOne(NUMBER_OF_LEDS, LQUARTER, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		break;

		case FIFTH :  //Fading in and out to blink
		  //How will we stop blinking ??
			allLightsOn(NUMBER_OF_LEDS, LQUARTER, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
			allLightsOff(NUMBER_OF_LEDS, LQUARTER, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_LONG);
		break;
	  }
}*/

/*
static void pairingFade() {

	turnOffAllLEDs();
	HaloPixel_setBrightness(brightness);
	uint8_t red_pixel_=(float) 247/BRIGHTNESS_MULTIPLICATOR;
	uint8_t green_pixel_=(float) 247/BRIGHTNESS_MULTIPLICATOR;
	uint8_t blue_pixel_=(float) 8/BRIGHTNESS_MULTIPLICATOR;

	for (uint i=BRIGHTNESS_MULTIPLICATOR; i>0; i--){
		for (uint led = 0 ; led < NUMBER_OF_LEDS ; led++) {

			HaloPixel_setColor(led, HaloPixel_RGB_Color(i*red_pixel_, i*green_pixel_, i*blue_pixel_));
		}

		nrf_delay_ms(100);
		HaloPixel_show();

	}

	turnOffAllLEDs();
}*/

/*void Animations_right(Step step_of_animation){

	HaloPixel_setBrightness(brightness);

	if (step_of_animation == FIRST){
		red_pixel = TRUE;
		green_pixel = TRUE;
		blue_pixel = TRUE;
	}

	else{
		red_pixel = FALSE;
		green_pixel = TRUE;
		blue_pixel = FALSE;
	}

	int set_of_leds_to_light = (int) step_of_animation - 2;

	switch ((uint) step_of_animation) {  //Which step we are

		case FIRST :  // Everything white
			allLightsOn(HALF, 0, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
		 break;

		 case SECOND :  //First and last LED green
			lightsOneByOne(HALF, 0, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case THIRD :  //Second set of LED green
			lightsOneByOne(HALF, 0,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case FOURTH :  //Third set of LED green
			lightsOneByOne(HALF, 0,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case FIFTH :  //Fourth set of LED green
			lightsOneByOne(HALF, 0,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case SIXTH :  //Fifth set of LED green
			lightsOneByOne(HALF, 0,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case SEVENTH :  //Sixth set of LED green
			lightsOneByOne(HALF, 0,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case EIGHT :  //Fading in and out to blink
			//How will we stop blinking ??
				allLightsOn(HALF,0, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
				allLightsOff(HALF, 0,  red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
		 break;
	  }
}*/


/*
void Animations_left(Step step_of_animation){

	HaloPixel_setBrightness(brightness);

	if (step_of_animation == FIRST){
		red_pixel = TRUE;
		green_pixel = TRUE;
		blue_pixel = TRUE;
	}

	else{
		red_pixel = FALSE;
		green_pixel = TRUE;
		blue_pixel = FALSE;
	}

	int set_of_leds_to_light = (int) step_of_animation - 2;

	switch ((uint) step_of_animation) {  //Which step we are

		case FIRST :  // Everything white
			allLightsOn(NUMBER_OF_LEDS, HALF, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
		 break;

		 case SECOND :  //First and last LED green
			lightsOneByOne(NUMBER_OF_LEDS, HALF, red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case THIRD :  //Second set of LED green
			lightsOneByOne(NUMBER_OF_LEDS, HALF,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case FOURTH :  //Third set of LED green
			lightsOneByOne(NUMBER_OF_LEDS, HALF,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case FIFTH :  //Fourth set of LED green
			lightsOneByOne(NUMBER_OF_LEDS, HALF,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case SIXTH :  //Fifth set of LED green
			lightsOneByOne(NUMBER_OF_LEDS, HALF,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case SEVENTH :  //Sixth set of LED green
			lightsOneByOne(NUMBER_OF_LEDS, HALF,  red_pixel, green_pixel, blue_pixel, set_of_leds_to_light);
		 break;

		 case EIGHT :  //Fading in and out to blink
			//How will we stop blinking ??
				allLightsOn(NUMBER_OF_LEDS, HALF, red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
				allLightsOff(NUMBER_OF_LEDS, HALF,  red_pixel, green_pixel, blue_pixel, DELAY_BRIGHTNESS, WAIT_DELAY_SHORT);
		 break;
	  }
}*/

/*
 * bool Animations_alarm(Step stepOfAnimation) {
 *
	HaloPixel_setBrightness(brightness);
	red_pixel = TRUE;
	green_pixel = FALSE;
	blue_pixel = FALSE;
	static uint16_t counter_alarm_circle_animation = 0;
	static uint16_t counter_alarm_blink_animation = 0;
	static uint8_t led = 0;

	switch (stepOfAnimation){

		case FIRST:
  	  		  if (counter_alarm_circle_animation%300 == 0){
  	  			  if (led < NUMBER_OF_LEDS){
  	  				  HaloPixel_setColor(led, HaloPixel_RGB_Color(255,0,0));
  	  				  HaloPixel_showPixel(led);
  	  				  led ++;
  	  			  }

  	  			  else {
  	  				  turnOffAllLEDs();
  	  				  counter_alarm_circle_animation=0;
  	  				  led = 0;
  	  				  return true;
  	  			  }
  	  		  }

  	  		  counter_alarm_circle_animation++;
  	  		  return false;
  	  		  break;

		case SECOND:
			if (counter_alarm_blink_animation==0){
				for (uint8_t led=0; led<NUMBER_OF_LEDS; led++){
					HaloPixel_setColor(led, HaloPixel_RGB_Color(255,0,0));
				}
				HaloPixel_show();
			}

			else if (counter_alarm_blink_animation==100){
				turnOffAllLEDs();
			}

			else if (counter_alarm_blink_animation == 200){
				counter_alarm_blink_animation=0;
				return true;
			}

			counter_alarm_blink_animation++;
			return false;

			break;

		default : return true;
		break;
	}

}*/
 /*static void allLightsOn(uint led_max, uint led_min, uint r,uint g, uint b,uint delay1, uint delay2){

	  for (uint level_of_brightness=0; level_of_brightness<BRIGHTNESS_MULTIPLICATOR; level_of_brightness++){

	    if (led_max>led_min){

	      for (int led=led_min; led<=led_max;led++){
	    	  HaloPixel_setColor(led%NUMBER_OF_LEDS, HaloPixel_RGB_Color(r*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,g*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,b*BRIGHTNESS_MULTIPLICATOR*level_of_brightness));
	      }
	      HaloPixel_show();
	      nrf_delay_ms(delay1*10);
	    }


	    //If we have to light the slight left side of the Halo
	    else{

	    	for (uint led=led_max; led<=led_min ;led++){
	        	HaloPixel_setColor(led, HaloPixel_RGB_Color(r*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,g*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,b*BRIGHTNESS_MULTIPLICATOR*level_of_brightness));
	        	HaloPixel_setColor(led_min-led, HaloPixel_RGB_Color(r*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,g*BRIGHTNESS_MULTIPLICATOR*level_of_brightness,b*BRIGHTNESS_MULTIPLICATOR*level_of_brightness));
	        }
	    	HaloPixel_show();
	    	nrf_delay_ms(delay1*10);
	    }
	    nrf_delay_ms(delay2);
	  }

	}*/

/*
 * static void allLightsOff(uint led_max, uint led_min, uint r, uint g, uint b, uint delay1, uint delay2){


	for (int brightness_level=BRIGHTNESS_MULTIPLICATOR-1; brightness_level>=0; brightness_level--){

	  for (int led=led_min; led<=led_max;led++){

    	HaloPixel_setColor(led%NUMBER_OF_LEDS, HaloPixel_RGB_Color(r*BRIGHTNESS_MULTIPLICATOR*brightness_level,g*BRIGHTNESS_MULTIPLICATOR*brightness_level,b*BRIGHTNESS_MULTIPLICATOR*brightness_level));

	  }
	  HaloPixel_show();
	  nrf_delay_ms(delay1*10);
	  nrf_delay_ms(delay2);
  }

}*/


/*
 * void lightsOneByOne(uint led_max, uint led_min, uint r, uint g, uint b, uint step_of_animation){

	uint middle_led = led_min + ((led_max-led_min)/2);
	uint led_next_to_the_middle_led = middle_led - 1;
    uint led_1 = led_min + step_of_animation;
    int led_2 = (led_max-step_of_animation) % NUMBER_OF_LEDS;

    for (uint brightness_level=0; brightness_level<8; brightness_level++){

    	//Lights the LEDS two by two (one on each side of the "halo")
    	HaloPixel_setColor(led_1, HaloPixel_RGB_Color(r*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level),g*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level),b*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level)));

    	HaloPixel_setColor(led_2, HaloPixel_RGB_Color(r*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level),g*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level),b*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level)));
    	//For the last three LEDS we light the center LED too
    	if (led_1 == led_next_to_the_middle_led){
    		HaloPixel_setColor(middle_led, HaloPixel_RGB_Color(r*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level),g*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level),b*(BRIGHTNESS_MINIMUM_FOR_ONE_BY_ONE+BRIGHTNESS_MULTIPLICATOR_FOR_ONE_BY_ONE*brightness_level)));
    	}

    	HaloPixel_show();
    	nrf_delay_ms(DELAY_BRIGHTNESS);
    }
}
 */





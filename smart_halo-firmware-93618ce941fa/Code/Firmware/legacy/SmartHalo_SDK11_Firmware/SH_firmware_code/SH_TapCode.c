#include "stdafx.h"
#include "SH_TapCode.h"
#include "SH_Animations.h"
#include "SH_HaloPixelnRF.h"
#include "SH_typedefs.h"
#include "SH_Priority_queue.h"
#include "SH_Primary_state_machine.h"
#include "SH_Task_handler.h"
#include "SH_CenterLed_Animations.h"
#include "SH_Tap_Qtouch.h"
#include "SH_pinMap.h"
#include "SEGGER_RTT.h"
#include "app_util_platform.h"

bool night_light_flag =false;

bool button_timer_started = false;

//garbage context pointer, necessary only to satisfy handler prerequesites, remains void
void * p_context;

//static uint8_t global_pin_no=0;
//static uint8_t global_button_action=0;
/*
 * Flag that specifies if the last tap code entered was right
 */
//bool theTapCodewasright = false;

/*
 * This variable is put to true when the TapCode is validated
 * When looking a the flag for the tap code was right, put it back
 * manually to false
 */
 bool newValidationofTapCode = false;

/*
 * Flag that specifies the number of tap that are recorded yet
 */
uint8_t number_of_taps_recorded = 0;

/*
 * Flag that specifies if the last tap code entered was right
 */
bool SH_theTapCodewasright = false;




bool get_newValidationofTapCode()
{
	return newValidationofTapCode;
}

void set_newValidationofTapCode(bool newVal)
{
	 newValidationofTapCode = newVal;
}


bool check_if_theTapCodewasright()
{
	return SH_theTapCodewasright;
}

void reset_tapcode_flag()
{
	SH_theTapCodewasright = false;
}


/*
 * VALIDATEKNOCK
 * Validates the Taps that are saved in the knockReadings array
 * 0 if the tap is wrong, 1 otherwise
 */
static bool validateKnock();

/*
 * PLAYBACKKNOCK
 * plays the secret code
 */
void playbackKnock();

/*
 * LISTENTOSECRETTAP
 * Records the type of tap.
 * Validates the taps when the users has
 * entered the right number of tap
 */
void listenToSecretTap();

/*
 * QTOUCH_TIMER_HANDLER
 * This function handles what happens when the timer gets to the TIMEOUT_VALUE
 * Means that the users has entered a long tap
 *
 */
void tapcode_timer_handler(void *p_context);

uint32_t secretCode_of_SmartHalo[NUMBER_OF_TAPS] = {0, 0, 1, 0, 0};  	// Initial setup
static uint32_t knockReadings[NUMBER_OF_TAPS];    				// When someone tap this array fills with the type of tap (long or short).
static uint currentTapNumber = 0;

/*
 * This variable is put to true when the TapCode is validated
 * When looking a the flag for the tap code was right, put it back
 * manually to false
 */
bool SH_newValidationofTapCode = false;

/*
 * Flag that specifies the number of tap that are recorded yet
 */
uint8_t SH_number_of_taps_recorded = 0;

static bool begun = false;								// To initialize the TapMode

bool TapCodeMode = false;								// To activate the push button event handler

static bool firstTap = true;
static bool the_tap_was_a_long_tap = false;
static bool the_tap_was_a_long_tap_for_new_tapcode = false;
APP_TIMER_DEF(tap_code_timer_id);
APP_TIMER_DEF(tap_code_timeout_timer_id);

bool new_tap_code_tenry_mode =false;

//TAP CODE TIMER HANDLER
void tapcode_timer_handler(void *p_context){

	app_timer_stop(tap_code_timer_id);
	the_tap_was_a_long_tap = true;
	the_tap_was_a_long_tap_for_new_tapcode = true;
	if(!new_tap_code_tenry_mode) listenToSecretTap();

}

bool timeout_flag = false;

void tapcode_timeout_timer_handler(void *p_context)
{
	timeout_flag = true;
	//TapCodeMode = false;
	firstTap = true;
	flush_all_animations();
	insert_tap_animation_leds(&Animations_alarm_blinking);
	insert_tap_animation_leds(&turnOffAllLEDs);
	insert_tap_animation_leds(&Animations_alarm_blinking);
	insert_tap_animation_leds(&turnOffAllLEDs);
	SEGGER_RTT_WriteString(0, "tapcode Timeout\n");
	//@@@ if tapcode incomplete, insert animation
}


//TAPCODE INIT
void SH_TapCode_init(){

	uint32_t err;
	if (!begun){

		//HaloPixel_begin();

		// Creates the timer
		err = app_timer_create(&tap_code_timer_id, APP_TIMER_MODE_SINGLE_SHOT, tapcode_timer_handler);
		APP_ERROR_CHECK(err);
		err = app_timer_create(&tap_code_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, tapcode_timeout_timer_handler);
		APP_ERROR_CHECK(err);


	  //� supprimer pour d�bogage seulement
		//playbackKnock(); //Play the secret knock from the EEPROM (if any) or from the initial setup
	  //

		begun = true;

		TapCodeMode = false;
	}
}


//MODIFY SECRET TAPCODE
bool modify_secret_TapCode(uint32_t* new_secretCode_of_SmartHalo){

	for (uint8_t i=0;i<NUMBER_OF_TAPS;i++){
		secretCode_of_SmartHalo[i] = new_secretCode_of_SmartHalo[i];
	}

	return true;
}

//GET SECRET TAPCODE
const uint32_t * get_secret_TapCode(void){

	return secretCode_of_SmartHalo;
}

//LISTEN TO SECRET TAP
void listenToSecretTap(){


	if (firstTap){
	  // First reset the listening array.
		for (uint i=0; i < NUMBER_OF_TAPS; i++){
			knockReadings[i] = 0;
		}
		currentTapNumber = 0;
		firstTap = false;
		TapCodeMode=true;
	}

  // Stop listening if there is too many Tap
	if (currentTapNumber<NUMBER_OF_TAPS){

		if (the_tap_was_a_long_tap){
			knockReadings[currentTapNumber] = ISALONGTAP;
			flush_all_animations();
			insert_tap_animation_leds(&Animations_longTap);
			insert_tap_animation_leds(&turnOffAllLEDs);
		}

		else{
			knockReadings[currentTapNumber] = ISASHORTTAP;
			insert_tap_animation_center_led(&Animations_shortTap_center_led);

		}

		currentTapNumber ++;
		SH_number_of_taps_recorded = currentTapNumber;
	}

	if(currentTapNumber == NUMBER_OF_TAPS){
		TapCodeMode = false;
		firstTap = true;
		SH_theTapCodewasright = validateKnock();
		if(SH_theTapCodewasright) app_timer_stop(tap_code_timeout_timer_id);
		SH_newValidationofTapCode = true;
	}
}

// Plays back the pattern of the secret code in blinks
void playbackKnock(){

      for (uint i = 0; i < NUMBER_OF_TAPS ; i++){

    	  //reload watchdog
    	  NRF_WDT->RR[0] = WDT_RR_RR_Reload;

    	  turnOffAllLEDs(0,0,0);
    	  nrf_delay_ms(500);

    	  if (secretCode_of_SmartHalo[i] == ISALONGTAP){
        	//Animations_longTap();
        	nrf_delay_ms(1000);
        	turnOffAllLEDs(0,0,0);
    	  }
    	  else if (secretCode_of_SmartHalo[i] == ISASHORTTAP){
        	//Animations_shortTap();
    	  }
      }
      turnOffAllLEDs(0,0,0);
}

/*VALIDATEKNOCK
 * Checks to see if our knock matches the secret.
 * Returns true if it's a good knock, false if it's not.
                                                              */
static bool validateKnock()
{
  uint currentKnockCount = 0;

  //Number of knocks
  for (uint i=0;i<NUMBER_OF_TAPS;i++){

    if (knockReadings[i] == ISALONGTAP || knockReadings[i] == ISASHORTTAP){
      currentKnockCount++;
    }
  }
  // Easiest check first. If the number of knocks is not the same as in the secret code, the code is wrong
  if (currentKnockCount != NUMBER_OF_TAPS){
    return false;
  }

  // If the knock isn't the same as the secret code, the code is wrong
  for (uint i=0; i < NUMBER_OF_TAPS; i++){
    if (secretCode_of_SmartHalo[i] != knockReadings[i]){
    	app_timer_stop(tap_code_timeout_timer_id);
    	flush_all_animations();
    	insert_tap_animation_leds(&Animations_alarm_blinking);
    	insert_tap_animation_leds(&turnOffAllLEDs);
		insert_tap_animation_leds(&Animations_alarm_blinking);
		insert_tap_animation_leds(&turnOffAllLEDs);
      return false;
    }
  }

  return true;
}


bool SH_check_tap_code()
{
	uint8_t button_action = SH_check_button_action_tap();
	bool new_entry = false;

	if(TapCodeMode)
	{
		if (tap_is_not_heartbeat())
		{
			if(!button_timer_started)
			{// this indicates that the button has been pressed: start timers
				 app_timer_stop(tap_code_timeout_timer_id);
				 app_timer_start(tap_code_timer_id, APP_TIMER_TICKS(TIMEOUT_VALUE_FOR_TAP_CODE_TIMER, APP_TIMER_PRESCALER), NULL);
				 button_timer_started = true;
			}
			else if(button_action == NRF_GPIOTE_POLARITY_TOGGLE)
			{
				app_timer_stop(tap_code_timer_id);

				app_timer_start(tap_code_timeout_timer_id, TEN_SECONDS, p_context);
				new_entry = true;
				if (!the_tap_was_a_long_tap)
				{
					app_timer_stop(tap_code_timer_id);
					reset_heartbeat_flag();
					button_timer_started = false;
					//SEGGER_RTT_WriteString(0, "short tap\n");
					if(!new_tap_code_tenry_mode) listenToSecretTap();
					else
					{
						insert_tap_animation_center_led(&Animations_shortTap_center_led);
					}
				}
				else
				{
					the_tap_was_a_long_tap = false;
					insert_tap_animation_leds(&turnOffAllLEDs);
					reset_heartbeat_flag();
					button_timer_started = false;
					//SEGGER_RTT_WriteString(0, "long tap\n");
					if(new_tap_code_tenry_mode)
					{
						insert_tap_animation_leds(&Animations_longTap);
						insert_tap_animation_leds(&turnOffAllLEDs);
					}
				}
			}
		}
	}
	return new_entry;
}




//only works if WDT set to more than fifteen seconds reload time
uint32_t* new_tapcode_from_user(uint32_t* new_tapcode)
{
	SH_enable_TapCode(true);
	bool tapcode_finished = false;
	new_tap_code_tenry_mode = true;
	uint8_t tapcounter = 0;
	timeout_flag = false;
	flush_all_animations();
	app_timer_start(tap_code_timeout_timer_id, TEN_SECONDS, p_context);

	insert_tap_animation_leds(&Animations_destination);
	insert_tap_animation_leds(&turnOffAllLEDs);
	insert_tap_animation_leds(&Animations_destination);
	insert_tap_animation_leds(&turnOffAllLEDs);


	while((!timeout_flag)&&(!tapcode_finished))
	{
		run_animations_outside_primary_state_machine();
		if(SH_check_tap_code())
		{
			NRF_WDT->RR[0] = WDT_RR_RR_Reload;
			app_timer_stop(tap_code_timeout_timer_id);
			new_tapcode[tapcounter] = the_tap_was_a_long_tap_for_new_tapcode;
			the_tap_was_a_long_tap_for_new_tapcode = false;
			++tapcounter;
			app_timer_start(tap_code_timeout_timer_id, TEN_SECONDS, p_context);
		}

		if(tapcounter >= NUMBER_OF_TAPS)
		{
			tapcode_finished = true;
		}
	}

	if(tapcode_finished)
	{
		insert_tap_animation_leds(&Animations_destination);
		insert_tap_animation_leds(&turnOffAllLEDs);
		insert_tap_animation_leds(&Animations_destination);
		insert_tap_animation_leds(&turnOffAllLEDs);
		timeout_flag = false;
		new_tap_code_tenry_mode = false;
		return new_tapcode;
	}
	else
	{
		timeout_flag = false;
		new_tap_code_tenry_mode = false;
		return secretCode_of_SmartHalo;
	}
}






//ENABLE TAPCODE
void SH_enable_TapCode(bool enable_tapcode){

	if (enable_tapcode){
		TapCodeMode = true;
	}

	else{
		TapCodeMode = false;
	}
}





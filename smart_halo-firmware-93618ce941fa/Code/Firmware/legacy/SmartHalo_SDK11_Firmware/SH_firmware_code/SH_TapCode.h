#ifndef SH_TAPCODE_H_
#define SH_TAPCODE_H_

#include "stdafx.h"
#include <stdbool.h>
#include <stdint.h>

#define NUMBER_OF_TAPS							5



//CONSTANTS FOR THE TIMER
#define NUMBER_OF_UNITS_PER_SECONDS 			32768
#define TIMEOUT_VALUE_FOR_TAP_CODE_TIMER		500


#define ISALONGTAP	1
#define ISASHORTTAP	0

#define ENABLE_TAP_CODE true
#define DISABLE_TAP_CODE false


/*
 * SH_TAPCODE_INIT
 */
void SH_TapCode_init();

/*
 * Call this function to get the present secret Tap Code of the SmartHalo
 */
const uint32_t * get_secret_TapCode(void);


uint32_t* new_tapcode_from_user(uint32_t* new_tapcode);


/*
 * To change the secret Tap Code of the SmartHalo
 * returns true if the modification was made successfully
 */
bool modify_secret_TapCode(uint32_t new_secretCode_of_SmartHalo[NUMBER_OF_TAPS]);

/*
 * ENABLE_TAP_CODE
 *
 * This function enables or disables the event handler of the touch sensor
 */
void SH_enable_TapCode(bool);

// returns the variable newValidationofTapCode
bool get_newValidationofTapCode();

// newValidationofTapCode is assigned the boolean value passed as a parameter
void set_newValidationofTapCode(bool);

// checks to see if that last tap was long or short
bool SH_check_tap_code();

//returns theTapCodewasright flag
bool check_if_theTapCodewasright();

//resets theTapCodewasright flag to false
void reset_tapcode_flag();


#endif /* SH_TAPCODE_H_ */


#include "ble.h"
#include "ble_nus.h"
#include "SH_typedefs.h"


#define SLEEP_STATE 					0
#define PARKED_STATE					1
#define UNLOCKED_STATE					2
#define STANDALONE_RIDE_STATE			3
#define PAIRED_STATE					4
#define RIDING_STATE					5
#define NAP_STATE						6
#define ALARM_STATE						7
#define THERMAL_SHUTOFF_STATE			8


#define TICKS_1MS						33
#define TICKS_2MS						65
#define TICKS_4MS						131
#define TICKS_5MS						164
#define TICKS_10MS						328
#define TICKS_15MS						492
#define TICKS_100MS						3277
#define TICKS_1000MS					32768
#define TEN_SECONDS						10*TICKS_1000MS
#define FIFTEEN_SECONDS					15*TICKS_1000MS
#define ONE_MINUTE						60*TICKS_1000MS

#define TIMER_FOR_ALARM_RED_CIRCLE	5000
#define TICKS_FOR_ALARM_RED_CIRCLE	APP_TIMER_TICKS(TIMER_FOR_ALARM_RED_CIRCLE, APP_TIMER_PRESCALER)
#define TIMER_FOR_ANIMATION_TIME_BETWEEN_CONSECUTIVE_COMPARE_EVENTS 15
#define TICKS_FOR_ANIMATION_BUFFER APP_TIMER_TICKS(TIMER_FOR_ANIMATION_TIME_BETWEEN_CONSECUTIVE_COMPARE_EVENTS, APP_TIMER_PRESCALER)

#define ALARM_ACTIVATED					true
#define ALARM_DISABLED					false

#define RESET_ANIMATION					true
#define CONTINUE_ANIMATION				false

#define BATTERY_MINIMUM_VOLTAGE			3.0f
#define BATTERY_MAXIMUM_VOLTAGE			4.2f

#define MORE_OR_LESS_ZERO				20 //this is to eliminate the measurement error. Value determined empirically



 void primary_state_machine(SHP_message_buffer_t* input_message_list,SHP_command_buffer_t* LOW_priority_output_command_list,
 		SHP_command_buffer_t* HIGH_priority_output_command_list, ble_nus_t * p_nus);

//checks to see if the device is connected or not to a central device
 void check_connection(ble_evt_t * p_ble_evt);


 //void display_substate_machine();

//returns the battery levels as a value from 0 to 100 (percentage of charge left in battery)
 uint8_t get_converted_battery_level();


//insert center led animation into buffer
 void insert_tap_animation_center_led(void* tap_function_pointer);


 //insert halo tap animation into buffer
 void insert_tap_animation_leds(void* tap_function_pointer);


 //turns alarm on or off with ALARM_ACTIVATED or ALARM_DISABLED
 void set_alarm(bool alarm_state);

 //alarm manual or auto
 void set_alarm_mode(bool alarm_mode);



//resets all animations to off and flushes both aniamtion buffers
void flush_all_animations();

//executes static functions to execute animations from main functions
void run_animations_outside_primary_state_machine();

void switch_display_substate(SH_display_substate new_substate);

bool get_gap_is_connected_flag();


void change_baseline_led_brightness(uint8_t new_brightness);



#include "SH_Includes.h"
#include "SH_Batmon.h"

//static uint8_t main_state = PARKED_STATE;
static uint8_t main_state = UNLOCKED_STATE;

static SH_display_substate substate = TURN_BY_TURN_SUBSTATE;


SH_Battery_Levels_t battery_level;

extern nrf_drv_wdt_channel_id m_channel_id;

SH_animations_buffer_t animation_function_buffer = {
		.rear = -1
};

SH_animations_buffer_t animation_function_buffer_center_led = {
		.rear = -1
};

//used to display the paired animation when entering paired state for the first time
bool first_iteration = true;

uint8_t front_led_counter;

//used only to fill void uint8_t parameters in the animations functions pointers
const uint8_t empty_parameter_anim =0;

APP_TIMER_DEF(animation_timer_id);
APP_TIMER_DEF(alarm_timer_id);
APP_TIMER_DEF(battery_timer_id);
APP_TIMER_DEF(movement_alarm_timer_id);
APP_TIMER_DEF(thermal_management_timer_id);
APP_TIMER_DEF(sleep_timer_id);


volatile bool battery_check_flag = true;

volatile bool animation_ready_flag = false;
volatile bool led_animation_ready_flag =false;
volatile bool alarm_ready_flag = false;
volatile bool alarm_timer_started = false;
volatile bool alarm_timeout_flag = false;
volatile bool alarm_movement_timer_flag = false;


//alarm is set from mobile application
bool alarm_is_set = false;
bool alarm_mode_auto = false;

bool it_is_time_to_go_to_sleep =false;


//ready to read uart value flag
volatile bool uart_get_ready_flag = false;
//volatile bool uart_tx_empty_flag = true;

//garbage context pointer, necessary only to satisfy handler prerequesites, remains void
void * p_context;

//true if the device is connected to a bluetooth central device
volatile bool gap_is_connected = false;

static bool connection_timeout = false;

uint8_t baseline_led_brightness = 100;

bool thermal_management_flag = false;
bool animation_running = false;
//extern volatile bool newValidationofTapCode;


//executes the next step in the animations
static void execute_animation();
static void execute_LED_animation();

//gets and processes SHP messages on the UART lines
//static void get_uart_sh_message(SHP_message_buffer_t* SHP_msg_buffer);

//after a tap from the touch interruptor, the night light is toggled
static void check_night_light_tap();

//handlers for the timers for alarm and animations
static void animation_timer_handler(void *p_context);
static void alarm_timer_handler(void *p_context);
static void battery_timer_handler(void *p_context);
static void movement_alarm_timer_handler(void *p_context);
static void thermal_management_timer_handler(void *p_context);
static void sleep_timer_handler(void *p_context);


//operations commons to most states
static void round_robin_operations(SHP_message_buffer_t* input_message_list,
		SHP_command_buffer_t* LOW_priority_output_command_list,
		SHP_command_buffer_t* HIGH_priority_output_command_list, ble_nus_t * p_nus);

#ifndef TEST_HALO_BOX
//checks battery levels if ready
static void check_battery();
#ifndef FACTORY_PROGRAMMED_FIRMWARE
//exits alarm state if no more sustained movement
static void check_alarm_movement();
#endif
#endif

static uint8_t convert_battery_levels_to_percentage();

//checks to see if the advertising needs to be reset
static void check_connection_timeout();

//sends to alarm state if there is movement or trigger events
static void alarm_state_check();

//sends to sleep state if there has been no activity in the last minute
static void sleep_state_check();


static void reset_sleep_timer();


//updates front led flash if need be
static void update_front_led_flash();

static void display_substate_machine(ble_nus_t* p_nus);

static bool check_display_mode_tap();

static void thermal_management(ble_nus_t* p_nus);

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
	NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}

void heat_tests(){

		//insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

	insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_success, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

	insert_animation_function_pointer(&animation_function_buffer, &Animations_alarm_blinking, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_alarm_blinking, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_alarm_blinking, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);


	insert_animation_function_pointer(&animation_function_buffer, &Animations_straight, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

	insert_animation_function_pointer(&animation_function_buffer, &Animations_uTurn_first_call, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_uTurn, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_uTurn, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepOne, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepTwo, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepThree, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepFour, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepFive, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepSix, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepSeven, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepEight, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_left_stepNine, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);


	insert_animation_function_pointer(&animation_function_buffer, &Animations_straight, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepOne, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepTwo, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepThree, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepFour, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepFive, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepSix, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepSeven, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepEight, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_right_stepNine, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

	insert_animation_function_pointer(&animation_function_buffer, &Animations_straight, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardRight_stepOne, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardRight_stepTwo, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardRight_stepThree, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardRight_stepFour, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardRight_stepFive, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardRight_stepSix, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);


	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardLeft_stepOne, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardLeft_stepTwo, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardLeft_stepThree, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardLeft_stepFour, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardLeft_stepFive, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_hardLeft_stepSix, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);



}


void primary_state_machine(SHP_message_buffer_t* input_message_list,SHP_command_buffer_t* LOW_priority_output_command_list,
		SHP_command_buffer_t* HIGH_priority_output_command_list, ble_nus_t * p_nus)
{

	//creation of the timers used for sequencing the animations
	uint8_t err_code = app_timer_create(&animation_timer_id,APP_TIMER_MODE_REPEATED,animation_timer_handler);
	err_code = app_timer_start(animation_timer_id, TICKS_FOR_ANIMATION_BUFFER, p_context); //reset every 2ms
    APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&alarm_timer_id,APP_TIMER_MODE_SINGLE_SHOT,alarm_timer_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&battery_timer_id,APP_TIMER_MODE_REPEATED,battery_timer_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(battery_timer_id, ONE_MINUTE, p_context);
	APP_ERROR_CHECK(err_code);


	err_code = app_timer_create(&sleep_timer_id,APP_TIMER_MODE_SINGLE_SHOT,sleep_timer_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(sleep_timer_id, FIFTEEN_SECONDS, p_context);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&movement_alarm_timer_id,APP_TIMER_MODE_SINGLE_SHOT,movement_alarm_timer_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&thermal_management_timer_id,APP_TIMER_MODE_REPEATED,thermal_management_timer_handler);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(thermal_management_timer_id, 2*TICKS_100MS, p_context);//check every 200ms
	APP_ERROR_CHECK(err_code);

	//makes accelerometer reset interrupt 2 pin for movement detection
	SH_reset_int2();
	//heat_tests();
	//start of application animations
	insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &Animations_pairingFade, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);



	while(1)
	{
		//reset watchdog! Do not remove this line!
        nrf_drv_wdt_channel_feed(m_channel_id);
        //reset watchdog! Do not remove this line!

        power_manage();

		switch (main_state)
		{
			case SLEEP_STATE:
				// can be woken by accelerometer or bluetooth connection or usb connection

				check_connection_timeout();
				power_manage();

//				if(!nrf_gpio_pin_read(USB_POWERGOOD_N_PIN))
//				{
//					reenable_essential_devices();
//					err_code = app_timer_start(thermal_management_timer_id, 2*TICKS_100MS, p_context);//check every 200ms
//					err_code = app_timer_start(sleep_timer_id, ONE_MINUTE, p_context);
//					err_code = app_timer_start(battery_timer_id, ONE_MINUTE, p_context);
//					err_code = app_timer_start(animation_timer_id, TICKS_FOR_ANIMATION_BUFFER, p_context); //reset every 2ms
//					APP_ERROR_CHECK(err_code);
//					if(alarm_is_set) main_state = PARKED_STATE;
//					else  main_state = UNLOCKED_STATE;
//				}

				if(SH_check_for_flag_accelerometer_int2())
				{
					reenable_essential_devices();
					SH_reset_flag_accelerometer_int2();//resets the firmware flag
					SH_reset_int2();//resets the interrupt pin logic level in the accelerometer
					err_code = app_timer_start(thermal_management_timer_id, 2*TICKS_100MS, p_context);//check every 200ms
					err_code = app_timer_start(sleep_timer_id, FIFTEEN_SECONDS, p_context);
					err_code = app_timer_start(battery_timer_id, ONE_MINUTE, p_context);
					err_code = app_timer_start(animation_timer_id, TICKS_FOR_ANIMATION_BUFFER, p_context); //reset every 2ms
					APP_ERROR_CHECK(err_code);
					if(alarm_is_set) main_state = PARKED_STATE;
					else  main_state = UNLOCKED_STATE;
				}

				if(gap_is_connected)
				{
					reenable_essential_devices();
					err_code = app_timer_start(thermal_management_timer_id, 2*TICKS_100MS, p_context);//check every 200ms
					err_code = app_timer_start(sleep_timer_id, FIFTEEN_SECONDS, p_context);
					err_code = app_timer_start(battery_timer_id, ONE_MINUTE, p_context);
					err_code = app_timer_start(animation_timer_id, TICKS_FOR_ANIMATION_BUFFER, p_context); //reset every 2ms
					APP_ERROR_CHECK(err_code);
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					main_state = PAIRED_STATE;
					SH_reset_int2();
					first_iteration = true;

					//restart sleep timer and restart advertising
					break;
				}
				break;


			case PARKED_STATE:

				thermal_management(p_nus);

				round_robin_operations(input_message_list,LOW_priority_output_command_list,
						HIGH_priority_output_command_list,p_nus);

				check_connection_timeout();
				SH_enable_TapCode(ENABLE_TAP_CODE);
				SH_check_tap_code();
				if(alarm_is_set)
				{
					if(check_if_theTapCodewasright())
					{
						reset_tapcode_flag();
						if(!alarm_mode_auto) set_alarm(ALARM_DISABLED);
						main_state = UNLOCKED_STATE;
						flush_animation_buffer(&animation_function_buffer);
						flush_animation_buffer(&animation_function_buffer_center_led);
						insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
						insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
						insert_animation_function_pointer(&animation_function_buffer, &Animations_destination, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
						insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					}
				}

				if(gap_is_connected)
				{
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					main_state = PAIRED_STATE;
					SH_reset_int2();
					first_iteration = true;
					SH_enable_TapCode(DISABLE_TAP_CODE);

				}

				alarm_state_check();
				sleep_state_check();
				break;


			case UNLOCKED_STATE:
				thermal_management(p_nus);

				round_robin_operations(input_message_list,LOW_priority_output_command_list,
						HIGH_priority_output_command_list,p_nus);
				check_connection_timeout();
				SH_enable_TapCode(ENABLE_TAP_CODE);
				SH_check_tap_code();
				if(alarm_is_set)
				{
					reset_tapcode_flag();
					//set_alarm(ALARM_ACTIVATED);
					main_state = PARKED_STATE;
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &Animations_destination, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
				}
				if(check_if_theTapCodewasright())
				{
					reset_tapcode_flag();
					set_alarm(ALARM_ACTIVATED);
					main_state = PARKED_STATE;
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &Animations_destination, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
				}
				if(gap_is_connected)
				{
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					main_state = PAIRED_STATE;
					SH_reset_int2();
					first_iteration = true;
					SH_enable_TapCode(DISABLE_TAP_CODE);
				}



				sleep_state_check();
				break;


			case STANDALONE_RIDE_STATE:
				// activate substates of goal completion mode
				//night light turn on and off based upon taps
				thermal_management(p_nus);

				check_connection_timeout();
				sleep_state_check();
				break;


			case PAIRED_STATE:
				// this is where the device is paired and ready to receive commands
				reset_sleep_timer();
				thermal_management(p_nus);

				SH_enable_TapCode(DISABLE_TAP_CODE);

				display_substate_machine(p_nus); //this is responsible for switching display modes by tapping



				if(first_iteration)//first time the code runs through the round robin of the while loop
				{
					first_iteration = false;
					insert_animation_function_pointer(&animation_function_buffer, &Animations_success, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &Animations_paired_indicator_center_led, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
				}

				if(animation_function_buffer_center_led.rear < 0)//if no more animations in buffer
				{ //green glowing heartbeat led for indicating when the device is paired
					insert_animation_function_pointer(&animation_function_buffer_center_led, &Animations_paired_indicator_center_led, CONTINUE_ANIMATION,empty_parameter_anim,empty_parameter_anim);
				}

				round_robin_operations(input_message_list,LOW_priority_output_command_list,
						HIGH_priority_output_command_list,p_nus);

			
				if(!gap_is_connected)
				{
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					if(alarm_is_set) main_state = PARKED_STATE;
					else main_state = UNLOCKED_STATE;
					SH_reset_int2();
				}

				break;


//			case RIDING_STATE:
//				//this accepts navigation commands and there is motion detected
//				thermal_management(p_nus);
//
//				check_night_light_tap();
//				manage_SH_BLE_messages(input_message_list,LOW_priority_output_command_list, HIGH_priority_output_command_list);
//				task_handler(LOW_priority_output_command_list, HIGH_priority_output_command_list, &animation_function_buffer, &animation_function_buffer_center_led,p_nus);
//				execute_animation();
//				execute_LED_animation();
//
//				sleep_state_check();
//				break;

//
//			case NAP_STATE:
//				main_state = PARKED_STATE;
//				//this state may or may not be useful because of the very low power consumption of the device
//				break;


			case ALARM_STATE:
				//calls alarm animation and tap code ready functions
#ifndef FACTORY_PROGRAMMED_FIRMWARE //makes sure that the alarm will never activate in shipping because the code will not exist
				reset_sleep_timer();
				thermal_management(p_nus);

				execute_animation();
				execute_LED_animation();
				check_connection_timeout();

		#ifndef TEST_HALO_BOX
				check_battery();
		#endif


				if(alarm_is_set)
				{
					bool tapcode_not_good = true;
					SH_enable_TapCode(ENABLE_TAP_CODE);

					thermal_management(p_nus);

					//send notification to phone for alarm activated

					if(!alarm_timer_started)
					{
						//flush animations buffer
						//play_sound(WARNING_SOUND);
						if(animation_function_buffer.rear < 0)
							insert_animation_function_pointer(&animation_function_buffer, &Animations_alarm_red_circle, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
						else //only reset the animation if another animation is not complete
							Animations_alarm_red_circle(RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
						err_code = app_timer_start(alarm_timer_id, TICKS_FOR_ALARM_RED_CIRCLE, p_context); //set for 15 seconds
						APP_ERROR_CHECK(err_code);
						alarm_timer_started = true;
					}

					//in the case the animation gets cancelled by another animation
					if(animation_function_buffer.rear < 0)//if no more animations in buffer
					{
						insert_animation_function_pointer(&animation_function_buffer, &Animations_alarm_red_circle, CONTINUE_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					}


					SH_check_tap_code();


						if(check_if_theTapCodewasright())
						{
							reset_tapcode_flag();
							tapcode_not_good = false; //dont enter while loop
							app_timer_stop(alarm_timer_id);
							main_state = UNLOCKED_STATE;
							SH_reset_int2();
							alarm_ready_flag = false;
							alarm_timer_started = false;
							alarm_timeout_flag = false;
							SH_enable_TapCode(DISABLE_TAP_CODE);
							flush_animation_buffer(&animation_function_buffer);
							flush_animation_buffer(&animation_function_buffer_center_led);
							insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
							insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
							insert_animation_function_pointer(&animation_function_buffer, &Animations_destination, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
							insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
							if(!alarm_mode_auto) set_alarm(ALARM_DISABLED);
						}


					#ifndef TEST_HALO_BOX
					check_alarm_movement();
					#endif

					execute_animation();

					if(alarm_ready_flag)
					{
						alarm_ready_flag = false;
						alarm_timeout_flag = false;
						alarm_timer_started = false; //indicates the timer is no longer active
						set_FrontLight_toBlink();
						app_timer_start(alarm_timer_id, ONE_MINUTE, p_context);
						alarm_timer_started = true;
						flush_animation_buffer(&animation_function_buffer);
						insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
						insert_animation_function_pointer(&animation_function_buffer, &Animations_alarm_blinking, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
						piezo_alarm_start();

						while(tapcode_not_good)
						{
							thermal_management(p_nus);
							reset_sleep_timer();
							//reset watchdog! Do not remove this line!
					        nrf_drv_wdt_channel_feed(m_channel_id);
					        //reset watchdog! Do not remove this line!

							if(animation_function_buffer.rear < 0)//if no more animations in buffer
							{
								insert_animation_function_pointer(&animation_function_buffer, &Animations_alarm_blinking, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
							}
						
							execute_animation();
							execute_LED_animation();

							if(alarm_timeout_flag)
							{
								alarm_timeout_flag = false;
								app_timer_stop(alarm_timer_id);
								main_state = PARKED_STATE;
								SH_reset_int2();
								SH_enable_TapCode(DISABLE_TAP_CODE);
								flush_animation_buffer(&animation_function_buffer);
								flush_animation_buffer(&animation_function_buffer_center_led);
								insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
								insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
								stop_FrontLight_blinking();
								set_FrontLight_Off();
								piezo_alarm_stop();
								break;
							}

							//if bluetooth connection is good, send back to paired state
							if(gap_is_connected)
							{
								main_state = PAIRED_STATE;
								SH_reset_int2();
								first_iteration = true;
								app_timer_stop(alarm_timer_id);
								SH_enable_TapCode(DISABLE_TAP_CODE);
								flush_animation_buffer(&animation_function_buffer);
								flush_animation_buffer(&animation_function_buffer_center_led);
								insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
								insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
								insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
								stop_FrontLight_blinking();
								set_FrontLight_Off();
								break;
							}
							SH_enable_TapCode(ENABLE_TAP_CODE);
							SH_check_tap_code(); //check to see if there was a tap event

							//if(get_newValidationofTapCode()) //if there is a new tap code entry
							//{
								//flush animations buffer here
								//set_newValidationofTapCode(false);
								if(check_if_theTapCodewasright())
								{
									reset_tapcode_flag();
									tapcode_not_good = false; //exit while loop
									app_timer_stop(alarm_timer_id);
									main_state = UNLOCKED_STATE;
									SH_reset_int2();
									alarm_ready_flag = false;
									alarm_timer_started = false;
									alarm_timeout_flag = false;
									SH_enable_TapCode(DISABLE_TAP_CODE);
									flush_animation_buffer(&animation_function_buffer);
									flush_animation_buffer(&animation_function_buffer_center_led);
									insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
									insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
									insert_animation_function_pointer(&animation_function_buffer, &Animations_destination, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
									insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

									stop_FrontLight_blinking();
									set_FrontLight_Off();
									piezo_alarm_stop();
									if(!alarm_mode_auto) set_alarm(ALARM_DISABLED);
								}
							//}
						}
					}
				}
				else
				{
					//flush animations buffer here
					main_state = PARKED_STATE;
					SH_reset_int2();
					app_timer_stop(alarm_timer_id);
					alarm_ready_flag = false;
					alarm_timeout_flag = false;
					alarm_timer_started = false;
					//SH_enable_TapCode(DISABLE_TAP_CODE);
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					piezo_alarm_stop();
				}

				if(gap_is_connected)
				{
					//flush animations buffer here
					main_state = PAIRED_STATE;
					SH_reset_int2();
					first_iteration = true;
					app_timer_stop(alarm_timer_id);
					alarm_ready_flag = false;
					alarm_timer_started = false;
					alarm_timeout_flag = false;
					SH_enable_TapCode(DISABLE_TAP_CODE);
					flush_animation_buffer(&animation_function_buffer);
					flush_animation_buffer(&animation_function_buffer_center_led);
					insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
					piezo_alarm_stop();
				}


#else // FACTORY_PROGRAMMED_FIRMWARE
				main_state = UNLOCKED_STATE;
#endif // FACTORY_PROGRAMMED_FIRMWARE

				break;


			case THERMAL_SHUTOFF_STATE :
				//turn off all electronics and, if connected, periodically
				//send messages to the central device stating that it`s too hot

				SH_enable_TapCode(DISABLE_TAP_CODE);
				flush_animation_buffer(&animation_function_buffer);
				flush_animation_buffer(&animation_function_buffer_center_led);
				insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
				insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

				app_timer_stop(animation_timer_id);
				app_timer_stop(alarm_timer_id);
				app_timer_stop(battery_timer_id);
				app_timer_stop(movement_alarm_timer_id);
				app_timer_stop(thermal_management_timer_id);
				app_timer_stop(sleep_timer_id);

				//turn off all power supplies
				shutoff_all_devices();

				uint8_t thermal_error_message_counter = 0;
				while(1)
				{

					if(gap_is_connected && (thermal_error_message_counter < 3)) //send the error message 3 times
					{
						++thermal_error_message_counter;
						send_confirmation_of_reception(p_nus, THERMAL_ERROR);
					}

					power_manage();

					//if device has cooled down, reset device
					//@@@ this will not work if the TWI module is shutdown
					//WDT will catch the error and reset the device if twi is shutoff
					if(!thermal_cutoff_check())
					{
						sd_nvic_SystemReset();
					}
				}
				break;

			default:
				//error message here
				main_state = UNLOCKED_STATE;
				break;
		}
		//power_manage();
	}
}


void check_connection(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
      	{
        case BLE_GAP_EVT_CONNECTED:
        	gap_is_connected = true;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
        	gap_is_connected = false;
        break;

        case BLE_GAP_EVT_TIMEOUT :
        	connection_timeout = true;
        break;

        default:
        	break;
        }
}

bool get_gap_is_connected_flag()
{
	return gap_is_connected;
}


static void display_substate_machine(ble_nus_t* p_nus)
{
	SH_check_tap();

	 switch (substate)
		{
		case TURN_BY_TURN_SUBSTATE:
			if(check_display_mode_tap())
			{
				substate = AS_THE_CROW_FLIES_SUBSTATE;
				send_confirmation_of_reception(p_nus, CURRENT_HEADING);

			}
			break;


		case AS_THE_CROW_FLIES_SUBSTATE:


			if(animation_function_buffer.rear < 0)//if no more animations in buffer
			{
				int heading = 0;
				SH_magnetometer_data average_data_magnetometer;
				SH_as_the_crow_flies_params_t current_as_the_crow_flies_params;
				SH_Magnetometer_read_data();
				average_data_magnetometer.x_axis = MAGNETOMETER_x_axis_data();
				average_data_magnetometer.y_axis = MAGNETOMETER_y_axis_data();
				average_data_magnetometer.z_axis = MAGNETOMETER_z_axis_data();

				heading = SH_Magnetometer_heading(average_data_magnetometer);
				heading = (uint8_t)((heading/3.6f));
				if(heading != 0) //a 0 reading can be erroneous
				{
					current_as_the_crow_flies_params = get_as_the_crow_flies_current_params();
					insert_animation_function_pointer(&animation_function_buffer, &Animations_as_the_crow_flies, CONTINUE_ANIMATION, \
							heading ,current_as_the_crow_flies_params.distance_to_target);
				}
			}


			if(check_display_mode_tap())
			{
				substate = GOAL_COMPLETION_SUBSTATE;
				send_confirmation_of_reception(p_nus, REQUEST_TRACKING_UPDATE);
			}
			break;


		case GOAL_COMPLETION_SUBSTATE:
			if(check_display_mode_tap())
			{
				substate = TURN_BY_TURN_SUBSTATE;
				send_confirmation_of_reception(p_nus, NAVIGATION_MODE);
			}
			break;

		default:
			substate = TURN_BY_TURN_SUBSTATE;
			send_confirmation_of_reception(p_nus, NAVIGATION_MODE);
			break;
		}
}


void switch_display_substate(SH_display_substate new_substate)
{
	substate = new_substate;
}


static bool check_display_mode_tap()
{

	if(get_switch_display_flag())
	{
		return true;

	}
	else
	{
		return false;
	}
}

static void execute_animation()
{
	//reset watchdog! Do not remove this line!
    nrf_drv_wdt_channel_feed(m_channel_id);


	if(animation_ready_flag)
	{
		//uint32_t timecountstart =0;
		//uint32_t timecountend =0;

		//uint32_t elapsed =0;
		//timecountstart = elapsedtime;

	//	SH_halo_set_new_brightness(baseline_led_brightness);

		animation_ready_flag = false;

		func_ptr_t function_to_execute;
		bool reset = false;
		uint8_t parameter1 = 0;
		uint8_t parameter2 = 0;
		bool ready_to_delete = false;
		if(animation_function_buffer.rear >= 0)
		{
			//execute function held at pointer
			function_to_execute = animation_function_buffer.func_ptr_array[0].animation_function_pointer;
			reset = animation_function_buffer.func_ptr_array[0].reset;
			parameter1 = animation_function_buffer.func_ptr_array[0].parameter_1;
			parameter2 = animation_function_buffer.func_ptr_array[0].parameter_2;
			animation_running = true; // to avoid twi communication collisions
			ready_to_delete = function_to_execute(reset,parameter1,parameter2);
			animation_running = false;
			if (reset == RESET_ANIMATION && ready_to_delete){
				animation_function_buffer.func_ptr_array[0].reset = CONTINUE_ANIMATION;
				ready_to_delete = false;
			}
			//update queue, delete pointer to executed function
			if(ready_to_delete)
			{
				delete_animation_function_pointer(&animation_function_buffer);
			}
			//timecountend =elapsedtime;


		}
		//elapsed = timecountend - timecountstart;
		//elapsed++;
	}
}

static void execute_LED_animation()
{
	//reset watchdog! Do not remove this line!
    nrf_drv_wdt_channel_feed(m_channel_id);

	if(led_animation_ready_flag)
	{
		//SH_halo_set_new_brightness(255);

		led_animation_ready_flag =false;
		func_ptr_t function_to_execute;
		bool reset = 0;
		uint8_t parameter1 = 0;
		uint8_t parameter2 = 0;
		bool ready_to_delete = false;
		if(animation_function_buffer_center_led.rear >= 0)
		{
			//execute function held at pointer

			function_to_execute = animation_function_buffer_center_led.func_ptr_array[0].animation_function_pointer;
			reset = animation_function_buffer_center_led.func_ptr_array[0].reset;
			parameter1 = animation_function_buffer_center_led.func_ptr_array[0].parameter_1;
			parameter2 = animation_function_buffer_center_led.func_ptr_array[0].parameter_2;
			ready_to_delete = function_to_execute(reset,parameter1,parameter2);

			//update queue, delete pointer to executed function
			if(ready_to_delete)
			{
				delete_animation_function_pointer(&animation_function_buffer_center_led);
			}
		}
	}
}


static void battery_timer_handler(void *p_context)
{
	battery_check_flag = true;

}

static void sleep_timer_handler(void *p_context)
{
	it_is_time_to_go_to_sleep = true;
}


static void animation_timer_handler(void *p_context)
{

	animation_ready_flag =true;
	led_animation_ready_flag = true;
	front_led_counter++; //used for the front led flash
	//execute_animation();//@@@ patch. real solution is to make twi more robust

}


static void alarm_timer_handler(void *p_context)
{
	alarm_ready_flag = true; //makes the alarm activate

	alarm_timeout_flag = true; //used only the second time this alarm is called, to sound the alarm for only a given amount of time

}


static void movement_alarm_timer_handler(void *p_context)
{
	//play_sound(WARNING_SOUND);
	alarm_movement_timer_flag = true;
}


static void thermal_management_timer_handler(void *p_context)
{
	thermal_management_flag = true;

}


static void thermal_management(ble_nus_t* p_nus)
{
#ifndef TEST_HALO_BOX
	thermal_management_flag = false;

	SH_halo_set_new_brightness(baseline_led_brightness);
	SH_centerled_set_new_brightness(baseline_led_brightness);
	if(thermal_cutoff_check())
	{
		send_confirmation_of_reception(p_nus, THERMAL_ERROR);
		main_state = THERMAL_SHUTOFF_STATE;
	}
	if(animation_function_buffer.rear < 0)
	{
		HaloPixel_show();
	}
	if(animation_function_buffer_center_led.rear < 0)
	{
		PWM_show_center_LED();
	}
#endif
}

void change_baseline_led_brightness(uint8_t new_brightness)
{
	baseline_led_brightness = new_brightness;
}

void insert_tap_animation_center_led(void* tap_function_pointer)
{

	insert_animation_function_pointer(&animation_function_buffer_center_led, tap_function_pointer, CONTINUE_ANIMATION,empty_parameter_anim,empty_parameter_anim);

}

void insert_tap_animation_leds(void* tap_function_pointer)
{

	insert_animation_function_pointer(&animation_function_buffer, tap_function_pointer, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);

}


void set_alarm(bool alarm_state)
{
		alarm_is_set = alarm_state;
}

void set_alarm_mode(bool alarm_mode)
{
	alarm_mode_auto = alarm_mode;
}



static void check_night_light_tap()
{
	if(get_night_light_flag())
	{
		PWM_show_front_LED();
	}
	else
	{
		clear_pixel_front_LED();
	}

}


static void reset_sleep_timer()
{
	uint8_t err_code = 0;
	it_is_time_to_go_to_sleep = false;

	err_code = app_timer_stop(sleep_timer_id);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(sleep_timer_id, FIFTEEN_SECONDS, p_context);
	APP_ERROR_CHECK(err_code);
}


static void sleep_state_check()
{
	if(it_is_time_to_go_to_sleep)// && nrf_gpio_pin_read(USB_POWERGOOD_N_PIN))
	{
//		flush_animation_buffer(&animation_function_buffer);
//		flush_animation_buffer(&animation_function_buffer_center_led);
//		insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
//		insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
//		execute_animation();
//		execute_LED_animation();
//		SH_reset_int2();
//		shutoff_all_devices();
//		app_timer_stop(animation_timer_id);
//		app_timer_stop(alarm_timer_id);
//		app_timer_stop(battery_timer_id);
//		app_timer_stop(movement_alarm_timer_id);
//		app_timer_stop(thermal_management_timer_id);
//		app_timer_stop(sleep_timer_id);
//		it_is_time_to_go_to_sleep = false;
//		main_state = SLEEP_STATE;
	}

}


static void alarm_state_check()
{
#ifndef TEST_HALO_BOX
	if(alarm_is_set)
	{
		if(SH_check_for_flag_accelerometer_int2())
		{
			SH_reset_flag_accelerometer_int2();//resets the firmware flag
			SH_reset_int2();//resets the interrupt pin logic level in the accelerometer
			main_state = ALARM_STATE;
			app_timer_start(movement_alarm_timer_id, TICKS_1000MS, p_context);

		}
	}
#endif

}

#ifndef TEST_HALO_BOX
#ifndef FACTORY_PROGRAMMED_FIRMWARE
//exits alarm state if no more sustained movement
static void check_alarm_movement()
{
	if(alarm_movement_timer_flag && !animation_running)
	{
		alarm_movement_timer_flag = false;

		if(SH_check_for_flag_accelerometer_int2())
		{
			play_sound(WARNING_SOUND);
			app_timer_start(movement_alarm_timer_id, TICKS_1000MS, p_context);
			SH_reset_flag_accelerometer_int2();//resets the firmware flag
			SH_reset_int2();//resets the interrupt pin logic level in the accelerometer
			execute_animation();
		}
		else
		{
			execute_animation();
			SH_reset_flag_accelerometer_int2();//resets the firmware flag
			SH_reset_int2();//resets the interrupt pin logic level in the accelerometer
			main_state = PARKED_STATE;
			app_timer_stop(alarm_timer_id);
			app_timer_stop(movement_alarm_timer_id);
			alarm_ready_flag = false;
			alarm_timer_started = false;
			alarm_timeout_flag = false;
			//SH_enable_TapCode(DISABLE_TAP_CODE);
			flush_animation_buffer(&animation_function_buffer);
			flush_animation_buffer(&animation_function_buffer_center_led);
			insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
		}
	}
}
#endif
#endif

//operations that are common to all states
static void round_robin_operations(SHP_message_buffer_t* input_message_list,SHP_command_buffer_t* LOW_priority_output_command_list,
		SHP_command_buffer_t* HIGH_priority_output_command_list, ble_nus_t * p_nus)
{
#ifndef TEST_HALO_BOX
    check_battery();
#endif
	check_night_light_tap();
	//get_uart_sh_message(input_message_list);

#ifdef BACKDOOR_OPEN
	manage_SH_BLE_messages(input_message_list,LOW_priority_output_command_list, HIGH_priority_output_command_list);
	task_handler(LOW_priority_output_command_list, HIGH_priority_output_command_list, &animation_function_buffer, &animation_function_buffer_center_led,p_nus);
#else
	if(main_state == PAIRED_STATE)
	{
		manage_SH_BLE_messages(input_message_list,LOW_priority_output_command_list, HIGH_priority_output_command_list);
		task_handler(LOW_priority_output_command_list, HIGH_priority_output_command_list, &animation_function_buffer, &animation_function_buffer_center_led,p_nus);
	}
#endif

	execute_animation();
	execute_LED_animation();
	update_front_led_flash();
}


//checks to see if BLE advertising has stopped after a timeout
static void check_connection_timeout()
{
	if(connection_timeout)
	{
		connection_timeout = false;
		ble_advertising_start(BLE_ADV_MODE_FAST);
	}
}
//
#ifndef TEST_HALO_BOX
static void check_battery()
{

	if(battery_check_flag)
	{
		battery_check_flag = false;
		battery_level.SOC = SH_Batmon_getRSOC();
		//SH_adc_read_battery_level();
	}

//	if(SH_sampling_battery_level_is_done())
//	{
//		battery_level = SH_get_battery_levels();
//		//@@@ send a feedback message to the central device with battery level request
//	}


}
#endif


static uint8_t convert_battery_levels_to_percentage()
{
	// convert battery_level to percentage using a lookup table eventually
	if((battery_level.voltage>BATTERY_MINIMUM_VOLTAGE)&&(battery_level.voltage<BATTERY_MAXIMUM_VOLTAGE))
	{
		return (uint8_t)((battery_level.voltage-BATTERY_MINIMUM_VOLTAGE)/(BATTERY_MAXIMUM_VOLTAGE-BATTERY_MINIMUM_VOLTAGE)*100.0);
	}
	else if(battery_level.voltage>BATTERY_MAXIMUM_VOLTAGE)
	{
		return 100;
	}
	else if(battery_level.voltage<BATTERY_MINIMUM_VOLTAGE)
	{
		return 0;
	}
	else
	{
		return 0;
	}
}


uint8_t get_converted_battery_level()
{
	return convert_battery_levels_to_percentage();
}


static void update_front_led_flash()
{
	static bool toggle = true;
	if(front_led_counter > 125) //125 for 250ms
	{
		front_led_counter = 0;
		if(get_front_led_status())
		{
			if(toggle)
				{PWM_show_front_LED();}
			else
				{clear_pixel_front_LED();}

			toggle = !toggle;
		}
	}
}


void flush_all_animations()
{
	flush_animation_buffer(&animation_function_buffer);
	flush_animation_buffer(&animation_function_buffer_center_led);
	insert_animation_function_pointer(&animation_function_buffer_center_led, &turn_Off_center_LED, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
	insert_animation_function_pointer(&animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter_anim,empty_parameter_anim);
}


void run_animations_outside_primary_state_machine()
{
	execute_animation();
	execute_LED_animation();
}
















//#include "stdafx.h"
//
//#include "ble.h"
//#include "ble_hci.h"
//#include "ble_srv_common.h"
//#include "ble_advdata.h"
//#include "ble_advertising.h"
//#include "ble_bas.h"
//#include "ble_hrs.h"
//#include "ble_dis.h"
//#ifdef BLE_DFU_APP_SUPPORT
//#include "ble_dfu.h"
//#include "dfu_app_handler.h"
//#endif // BLE_DFU_APP_SUPPORT
//#include "ble_conn_params.h"
//#include "boards.h"
//#include "sensorsim.h"
//#include "softdevice_handler.h"
//
//#include "device_manager.h"
//#include "pstorage.h"
//#include "app_trace.h"
//
//#include "bsp_btn_ble.h"
//
//#include "app_util_platform.h"
//
//#include "SH_HaloPixelnRF.h"
//#include "SH_BLE.h"
//
//#include "ble_nus.h"
//#include "app_uart.h"
//
//#include "SHP_Frames_and_commands.h"
//#include "SH_Priority_queue.h"
//#include "SH_typedefs.h"
//#include "SH_Animations.h"
//#include "SH_Task_handler.h"
//#include "SH_CenterLed_Animations.h"
//#include "SH_Primary_state_machine.h"
//#include "SH_PWM_FrontLight.h"
//#include "SH_Piezo_sounds.h"
//#include "dfu_ble_svc.h"
//#include "SH_FrontLed_Animations.h"
//#include "SH_TapCode.h"
//#include "SH_UICR_Addresses.h"
//#include "SH_mouvement_detection.h"
//#include "SH_Magnetic_field_detection.h"
//#include "SH_Accelerometer_Magnetometer.h"
//#include "SH_bootloader_version.h"
//#include "ble.h"
//#include "SH_GAP_GATT_Parameters.h"

#include "SH_Includes.h"

#include "SH_Batmon.h"



bool front_led_blink_status = false;
const uint8_t empty_parameter =0;

SH_as_the_crow_flies_params_t current_as_the_crow_flies_params;

bool the_front_light_is_on =false;

uint8_t previous_animation;

static bool check_previous_animation(uint8_t new_animation);

void task_handler(SHP_command_buffer_t* LOW_priority_tasks_to_be_handled,
		SHP_command_buffer_t* HIGH_priority_tasks_to_be_handled,
		SH_animations_buffer_t* animation_function_buffer,
		SH_animations_buffer_t* LED_animation_function_buffer,
		ble_nus_t * p_nus)
{
		if(HIGH_priority_tasks_to_be_handled->rear >= 0)
		{
			handle_command(HIGH_priority_tasks_to_be_handled, 0, animation_function_buffer, LED_animation_function_buffer, p_nus);
			delete_command(HIGH_priority_tasks_to_be_handled);
		}
		else
		{//this section is presently unused
			handle_command(LOW_priority_tasks_to_be_handled, 0, animation_function_buffer, LED_animation_function_buffer, p_nus);
			delete_command(LOW_priority_tasks_to_be_handled);
		}
}


void handle_command(SHP_command_buffer_t* tasks_to_be_handled,
		uint8_t index_position,SH_animations_buffer_t* animation_function_buffer,
		SH_animations_buffer_t* LED_animation_function_buffer,
		ble_nus_t * p_nus)
{
#ifdef DEBUG_UART_MESSAGES
	uint8_t action0[] = "Cont \n\r";
	uint8_t action1[] = "STRAIGHT \n\r";
	uint8_t action2[] = "UTURN \n\r";
	uint8_t action3[] = "TURN_L \n\r";
	uint8_t action4[] = "TURN_R \n\r";
	uint8_t action5[] = "S_Left \n\r";
	uint8_t action6[] = "S_Right \n\r";
	uint8_t action7[] = "H_Left \n\r";
	uint8_t action8[] = "H_Right \n\r";
	uint8_t action9[] = "Dest. \n\r";
	uint8_t action10[] = "turnDone \n\r";
	uint8_t step1[] = "step1 \n\r";
	uint8_t step2[] = "step2 \n\r";
	uint8_t step3[] = "step3 \n\r";
	uint8_t step4[] = "step4 \n\r";
	uint8_t step5[] = "step5 \n\r";
	uint8_t step6[] = "step6 \n\r";
	uint8_t step7[] = "step7 \n\r";
	uint8_t step8[] = "step8 \n\r";
	uint8_t step9[] = "step9 \n\r";
	uint8_t step10[] = "step10 \n\r";
#endif
	switch (tasks_to_be_handled->queue_array[index_position].command)
	{

	////NAVIGATION////
		//===DIRECTION UPDATE===//
		case DIRECTION_UPDATE:
			switch_display_substate(TURN_BY_TURN_SUBSTATE);
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case CONTINUE:
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action0);
					#endif
					break;

				case STRAIGHT:
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action1);
					#endif
					if(check_previous_animation(STRAIGHT))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					insert_animation_function_pointer(animation_function_buffer, &Animations_straight, RESET_ANIMATION,empty_parameter,empty_parameter);
					insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					send_confirmation_of_reception(p_nus, STRAIGHT);
					break;

				case U_TURN:
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action2);
					#endif
					play_sound(WARNING_SOUND);
					if(check_previous_animation(U_TURN))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					insert_animation_function_pointer(animation_function_buffer, &Animations_uTurn, RESET_ANIMATION,empty_parameter,empty_parameter);
					insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					send_confirmation_of_reception(p_nus, U_TURN);
					break;

				case TURN_LEFT:

					if(check_previous_animation(TURN_LEFT))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action3);
					#endif
					switch (tasks_to_be_handled->queue_array[index_position].data[1])
					{
						case STEP_ONE:
							play_sound(TURN_NOTIFICATION_SOUND);
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepOne, RESET_ANIMATION,empty_parameter,empty_parameter);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step1);
							#endif
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_TWO:
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepTwo, RESET_ANIMATION,empty_parameter,empty_parameter);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step2);
							#endif
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_THREE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step3);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepThree, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_FOUR:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step4);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepFour, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_FIVE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step5);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepFive, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_SIX:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step6);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepSix, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_SEVEN:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step7);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepSeven, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_EIGHT:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step8);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepEight, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_NINE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step9);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepNine, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						case STEP_TEN:
							play_sound(TURN_SUCCESFUL_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step10);
							#endif
							#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
								insert_animation_function_pointer(animation_function_buffer, &Animations_left_stepTen, RESET_ANIMATION,empty_parameter,empty_parameter);
								insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							#endif
							send_confirmation_of_reception(p_nus, TURN_LEFT);
							break;

						default:
							break;

					}
					break;

				case TURN_RIGHT:
					if(check_previous_animation(TURN_RIGHT))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action4);
					#endif
					switch (tasks_to_be_handled->queue_array[index_position].data[1])
					{
						case STEP_ONE:
							play_sound(TURN_NOTIFICATION_SOUND);
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepOne, RESET_ANIMATION,empty_parameter,empty_parameter);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step1);
							#endif
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_TWO:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step2);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepTwo, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_THREE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step3);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepThree, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_FOUR:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step4);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepFour, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_FIVE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step5);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepFive, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_SIX:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step6);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepSix, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_SEVEN:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step7);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepSeven, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_EIGHT:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step8);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepEight, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_NINE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step9);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepNine, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_TEN:
							play_sound(TURN_SUCCESFUL_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step10);
							#endif
							#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
								insert_animation_function_pointer(animation_function_buffer, &Animations_right_stepTen, RESET_ANIMATION,empty_parameter,empty_parameter);
								insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							#endif
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						default:
							break;

					}
					break;

				case SLIGHT_LEFT:
					if(check_previous_animation(SLIGHT_LEFT))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action5);
					#endif

					switch (tasks_to_be_handled->queue_array[index_position].data[1])
					{
						case STEP_ONE:
							play_sound(TURN_NOTIFICATION_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step1);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightLeft_stepOne, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_LEFT);
							break;

						case STEP_TWO:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step2);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightLeft_stepTwo, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_LEFT);
							break;

						case STEP_THREE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step3);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightLeft_stepThree, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_LEFT);
							break;

						case STEP_FOUR:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step4);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightLeft_stepFour, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_LEFT);
							break;

						case STEP_FIVE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step5);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightLeft_stepFive, RESET_ANIMATION,empty_parameter,empty_parameter);
							//insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_LEFT);
							break;

						case STEP_SIX:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step6);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightLeft_stepSix, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_SEVEN:
							play_sound(TURN_SUCCESFUL_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step7);
							#endif
							#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
								insert_animation_function_pointer(animation_function_buffer, &Animations_slightLeft_stepSeven, RESET_ANIMATION,empty_parameter,empty_parameter);
								insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							#endif
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						default:
							break;
					}
					break;

				case SLIGHT_RIGHT:
					if(check_previous_animation(SLIGHT_RIGHT))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action6);
					#endif
					switch (tasks_to_be_handled->queue_array[index_position].data[1])
					{
						case STEP_ONE:
							play_sound(TURN_NOTIFICATION_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step1);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightRight_stepOne, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_RIGHT);
							break;

						case STEP_TWO:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step2);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightRight_stepTwo, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_RIGHT);
							break;

						case STEP_THREE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step3);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightRight_stepThree, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_RIGHT);
							break;

						case STEP_FOUR:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step4);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightRight_stepFour, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_RIGHT);
							break;

						case STEP_FIVE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step5);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightRight_stepFive, RESET_ANIMATION,empty_parameter,empty_parameter);
							//insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, SLIGHT_RIGHT);
							break;

						case STEP_SIX:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step6);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_slightRight_stepSix, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_SEVEN:
							play_sound(TURN_SUCCESFUL_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step7);
							#endif
							#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
								insert_animation_function_pointer(animation_function_buffer, &Animations_slightRight_stepSeven, RESET_ANIMATION,empty_parameter,empty_parameter);
								insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							#endif
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						default:
							break;

					}
					break;

				case HARD_LEFT:
					if(check_previous_animation(HARD_LEFT))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action7);
					#endif
					switch (tasks_to_be_handled->queue_array[index_position].data[1])
					{
						case STEP_ONE:
							play_sound(TURN_NOTIFICATION_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step1);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardLeft_stepOne, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_LEFT);
							break;

						case STEP_TWO:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step2);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardLeft_stepTwo, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_LEFT);
							break;

						case STEP_THREE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step3);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardLeft_stepThree, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_LEFT);
							break;

						case STEP_FOUR:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step4);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardLeft_stepFour, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_LEFT);
							break;

						case STEP_FIVE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step5);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardLeft_stepFive, RESET_ANIMATION,empty_parameter,empty_parameter);
							//insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_LEFT);
							break;

						case STEP_SIX:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step6);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardLeft_stepSix, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_SEVEN:
							play_sound(TURN_SUCCESFUL_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step7);
							#endif
							#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
								insert_animation_function_pointer(animation_function_buffer, &Animations_hardLeft_stepSeven, RESET_ANIMATION,empty_parameter,empty_parameter);
								insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							#endif
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						default:
							break;

					}
					break;

				case HARD_RIGHT:
					if(check_previous_animation(HARD_RIGHT))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action8);
					#endif
					switch (tasks_to_be_handled->queue_array[index_position].data[1])
					{
						case STEP_ONE:
							play_sound(TURN_NOTIFICATION_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step1);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardRight_stepOne, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_RIGHT);
							break;

						case STEP_TWO:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step2);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardRight_stepTwo, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_RIGHT);
							break;

						case STEP_THREE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step3);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardRight_stepThree, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_RIGHT);
							break;

						case STEP_FOUR:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step4);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardRight_stepFour, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_RIGHT);
							break;

						case STEP_FIVE:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step5);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardRight_stepFive, RESET_ANIMATION,empty_parameter,empty_parameter);
							//insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, HARD_RIGHT);
							break;

						case STEP_SIX:
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step6);
							#endif
							insert_animation_function_pointer(animation_function_buffer, &Animations_hardRight_stepSix, RESET_ANIMATION,empty_parameter,empty_parameter);
							insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						case STEP_SEVEN:
							play_sound(TURN_SUCCESFUL_SOUND);
							#ifdef DEBUG_UART_MESSAGES
							printf("%s", step7);
							#endif
							#ifndef SMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO
								insert_animation_function_pointer(animation_function_buffer, &Animations_hardRight_stepSeven, RESET_ANIMATION,empty_parameter,empty_parameter);
								insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
							#endif
							send_confirmation_of_reception(p_nus, TURN_RIGHT);
							break;

						default:
							break;
					}
					break;

				case DESTINATION:
					if(check_previous_animation(DESTINATION))
						insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					#ifdef DEBUG_UART_MESSAGES
					printf("%s", action9);
					#endif
					insert_animation_function_pointer(animation_function_buffer, &Animations_destination, RESET_ANIMATION,empty_parameter,empty_parameter);
					insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					send_confirmation_of_reception(p_nus, DESTINATION);
					break;

				default:
					break;
			}
			break;
		//====================//


		//===TURN_COMPLETED===//
		case TURN_COMPLETED:
			//if(check_previous_animation(TURN_COMPLETED))
			//	insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					//analyse data
			flush_animation_buffer(animation_function_buffer);
			insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
			#ifdef DEBUG_UART_MESSAGES
			printf("%s", action10);
			#endif
			send_confirmation_of_reception(p_nus, TURN_COMPLETED);
			break;
		//====================//


		//===RIDE_STATUS_UPDATE===//
		case RIDE_STATUS_UPDATE:
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case START_RIDE:

					break;

				case STOP_RIDE:

					break;

				case PAUSE_RIDE:

					break;
				default:
					break;
			}
			break;
		//====================//


			//===NAVIGATION_MODE===//
		case NAVIGATION_MODE:
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case TURN_BY_TURN:
					switch_display_substate(TURN_BY_TURN_SUBSTATE);
					break;

				case AS_THE_CROW_FLIES:
					switch_display_substate(AS_THE_CROW_FLIES_SUBSTATE);
					break;

				case GOAL_COMPLETION:
					switch_display_substate(GOAL_COMPLETION_SUBSTATE);
					break;
				default:
					break;
			}
			break;
			//====================//


			//===CURRENT_HEADING===//
		case CURRENT_HEADING:

			break;
			//====================//


			//===SET_DESTINATION_HEADING===//
		case SET_DESTINATION_HEADING:

			switch_display_substate(AS_THE_CROW_FLIES_SUBSTATE);
			int heading = 0;
			SH_magnetometer_data average_data_magnetometer;
			//insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs);
			insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);

			SH_Magnetometer_read_data();
			average_data_magnetometer.x_axis = MAGNETOMETER_x_axis_data();
			average_data_magnetometer.y_axis = MAGNETOMETER_y_axis_data();
			average_data_magnetometer.z_axis = MAGNETOMETER_z_axis_data();

			heading = SH_Magnetometer_heading(average_data_magnetometer);
			heading = (uint8_t)((heading/3.6f));
			current_as_the_crow_flies_params.relative_heading = heading;
			current_as_the_crow_flies_params.distance_to_target = tasks_to_be_handled->queue_array[index_position].data[1];

			insert_animation_function_pointer(animation_function_buffer, &Animations_as_the_crow_flies, RESET_ANIMATION, \
					current_as_the_crow_flies_params.relative_heading ,current_as_the_crow_flies_params.distance_to_target);


			break;
			//====================//
	////NAVIGATION////


	////TRACKING////
		//===GOAL_TRACKING_UPDATE===//
		case GOAL_TRACKING_UPDATE:
			switch_display_substate(GOAL_COMPLETION_SUBSTATE);
			insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
			insert_animation_function_pointer(animation_function_buffer, &Animations_goalCompletion, RESET_ANIMATION,tasks_to_be_handled->queue_array[index_position].data[0],tasks_to_be_handled->queue_array[index_position].data[1]);

			break;
		//====================//


		//===REQUEST_TRACKING_UPDATE===//
		case REQUEST_TRACKING_UPDATE:

			break;
		//====================//
	////TRACKING////



	////NIGHT LIGHT////
	//===NIGHT_LIGHT_SET_MODE===//
		case NIGHT_LIGHT_SET_MODE:
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
			//this is redundant and should only be managed in the application
			//the SH should only receive on and off commands
				case NIGHT_LIGHT_AUTO:

					break;

				case NIGHT_LIGHT_MANUAL:

					break;

				default:
					break;
			}
			break;
	//====================//


	//===NIGHT_LIGHT_TOGGLE_POWER===//
		case NIGHT_LIGHT_TOGGLE_POWER:
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case NIGHT_LIGHT_TURN_OFF:
					the_front_light_is_on =false;

					//if(front_led_blink_status)
						stop_FrontLight_blinking();
					set_FrontLight_Off();
					send_confirmation_of_reception(p_nus, NIGHT_LIGHT_TOGGLE_POWER);
					break;

				case NIGHT_LIGHT_TURN_ON:
					the_front_light_is_on =true;
					if(front_led_blink_status) set_FrontLight_toBlink();
					else set_FrontLight_On();
					send_confirmation_of_reception(p_nus, NIGHT_LIGHT_TOGGLE_POWER);
					break;

				default:
					break;
			}
			break;
	//====================//


	//===NIGHT_LIGHT_SET_BLINKING_MODE===//
		case NIGHT_LIGHT_SET_BLINKING_MODE:
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case BLINKING_OFF:
					front_led_blink_status = false;
					stop_FrontLight_blinking();
					if(the_front_light_is_on) set_FrontLight_On();
					send_confirmation_of_reception(p_nus, NIGHT_LIGHT_SET_BLINKING_MODE);

					break;

				case BLINKING_ON:
					front_led_blink_status = true;
					if(the_front_light_is_on) set_FrontLight_toBlink();
					send_confirmation_of_reception(p_nus, NIGHT_LIGHT_SET_BLINKING_MODE);

					break;

				default:
					break;
			}
			break;
	//====================//


	//===NIGHT_LIGHT_SET_BRIGHTNESS===//
		case NIGHT_LIGHT_SET_BRIGHTNESS:
			//SH_PWM_Front_LED_setBrightness(tasks_to_be_handled->queue_array[index_position].data[1]);
			SH_frontled_set_new_brightness(tasks_to_be_handled->queue_array[index_position].data[0]);
			if(the_front_light_is_on) set_FrontLight_On();
			send_confirmation_of_reception(p_nus, NIGHT_LIGHT_SET_BRIGHTNESS);
			break;
	//====================//

	////NIGHT LIGHT////




	////PERSONAL ASSISTANT////

		//===INCOMING_CALL===//
		case INCOMING_CALL:
			play_sound(PHONECALL_SOUND);
			flush_animation_buffer(LED_animation_function_buffer);
			insert_animation_function_pointer(LED_animation_function_buffer,&Animations_callNotification, CONTINUE_ANIMATION,empty_parameter,empty_parameter);
			insert_animation_function_pointer(LED_animation_function_buffer,&turn_Off_center_LED, CONTINUE_ANIMATION,empty_parameter,empty_parameter);
			send_confirmation_of_reception(p_nus, INCOMING_CALL);
			break;
		//====================//

		//===INCOMING_SMS===//
		case INCOMING_SMS:
			play_sound(SMS_SOUND);
			flush_animation_buffer(LED_animation_function_buffer);
			insert_animation_function_pointer(LED_animation_function_buffer,&Animations_smsNotification , CONTINUE_ANIMATION,empty_parameter,empty_parameter);
			insert_animation_function_pointer(LED_animation_function_buffer,&turn_Off_center_LED, CONTINUE_ANIMATION,empty_parameter,empty_parameter);
			send_confirmation_of_reception(p_nus, INCOMING_SMS);
			break;
		//====================//

		//===INCOMING_CUSTOM===//
		case INCOMING_CUSTOM:
			//insert_animation_function_pointer(LED_animation_function_buffer, )
			break;
		//====================//


	////PERSONAL ASSISTANT////





	////ALARM////
	//===ALARM_SET_MODE===//
		case ALARM_SET_MODE:
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case ALARM_AUTO:
					set_alarm_mode(ALARM_MODE_AUTO);
					//set_alarm(ALARM_ACTIVATED);
					send_confirmation_of_reception(p_nus, ALARM_SET_MODE);
					break;

				case ALARM_MANUAL:
					set_alarm_mode(ALARM_MODE_MANUAL);
					//set_alarm(ALARM_DISABLED);
					send_confirmation_of_reception(p_nus, ALARM_SET_MODE);
					break;

				default:
					break;
			}
			break;

		case ALARM_SET_VOLUME :
			piezo_set_volume(tasks_to_be_handled->queue_array[index_position].data[0]);
			send_confirmation_of_reception(p_nus, ALARM_SET_VOLUME);
			break;

		case ALARM_ACTIVATION:
			//analyse data
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case ALARM_ON:
					//set_alarm_mode(ALARM_AUTO);
					set_alarm(ALARM_ACTIVATED);
					send_confirmation_of_reception(p_nus, ALARM_ACTIVATION);
					break;

				case ALARM_OFF:
					//set_alarm_mode(ALARM_MANUAL);
					set_alarm(ALARM_DISABLED);
					send_confirmation_of_reception(p_nus, ALARM_ACTIVATION);
					break;

				default:
					break;
			}
			break;
	//====================//

	////ALARM////



	/// BATTERY MONITOR ///
	//===ALARM_SET_MODE===//
		case GET_BATTERY_LEVEL: ;
			uint8_t data_to_send = SH_Batmon_getRSOC();//get_converted_battery_level();
			send_BLE_command_and_data(p_nus, GET_BATTERY_LEVEL, 1 , &data_to_send);
		//	send_confirmation_of_reception(p_nus, ALARM_SET_VOLUME);

			break;
	/// BATTERY MONITOR ///





	///DFU////
		//===ENTER_BOOTLOADER===//
		case ENTER_BOOTLOADER_SERIAL_MODE:

			app_uart_flush();

			//corrupts the crc check to
			//clear the irk and encryption key as well as central device
			//address to load serial transport layer when in the bootloader
			//(bootloader checks for the presence of peer data to load
			//either ble or serial transport layer)
			clear_peer_data();


			//animation_reset();

			//the address 0x7F000 is for the APP_VALID setting of the bootloader
			//this means that setting it to 0 will force the bootloader to start when device is reset
			write_to_FLASH(APP_VALID_ADDRESS ,0);

			//general purpose retention register settings to reset into bootloader
			sd_power_gpregret_set(10);

			sd_nvic_SystemReset();


			break;
		//====================//


		/*
		 * OTA DFU passes by BLE service
		*/
	///DFU////



	////FACTORY TESTS////

//		//===START_FACTORY_TESTS===//
//			case START_FACTORY_TESTS:
//
//				// this may have to be called before initialising the twi and pwm modules
//
//
//
//
//				break;
	////FACTORY TESTS////



	////TOUCH CONTROL////
		//===SET_NEW_TAPCODE===//
		case SET_NEW_TAPCODE: ; //the semicolon is inserted to allow the declaration of a variable below a label
			//only works if WDT set to more than fifteen seconds reload time
			uint32_t new_tap_code[NUMBER_OF_TAPS];
			memcpy(new_tap_code,new_tapcode_from_user(new_tap_code),NUMBER_OF_TAPS*sizeof(new_tap_code));
			modify_secret_TapCode(new_tap_code);
			send_confirmation_of_reception(p_nus, SET_NEW_TAPCODE);


			break;
		//====================//
	////TOUCH CONTROL////

	////RESERVED OPERATIONS////
		//===RESET_DEVICE===//
		case RESET_DEVICE:

			sd_nvic_SystemReset();
			break;

		//====================//
	////RESERVED OPERATIONS////

////GENERAL OPERATIONS////

		//===SEND_FIRMWARE_VERSION===//
		case SEND_FIRMWARE_VERSION:
		{
			uint8_t data_to_send[2];
			data_to_send[0] = APPLICATION_REVISION_MAJOR;
			data_to_send[1] = APPLICATION_REVISION_MINOR;
			send_BLE_command_and_data(p_nus, SEND_FIRMWARE_VERSION, 2, data_to_send);
			break;
		}
		//====================//

		//===SEND_BOOTLOADER_VERSION===//
		case SEND_BOOTLOADER_VERSION:
		{
			uint8_t data_to_send[2];
			data_to_send[0] = BOOTLOADER_REV_MAJOR;
			data_to_send[1] = BOOTLOADER_REV_MINOR;
			send_BLE_command_and_data(p_nus, SEND_BOOTLOADER_VERSION, 2, data_to_send);
			break;
		}
		//====================//

		//===SEND_SOFTDEVICE_VERSION===//
		case SEND_SOFTDEVICE_VERSION:
		{
			ble_version_t p_version;
			sd_ble_version_get(&p_version);
			uint8_t data_to_send[2];
			data_to_send[0] = (uint8_t)((p_version.subversion_number >> 8) & 0x00FF);
			data_to_send[1] = (uint8_t)(p_version.subversion_number & 0x00FF);
			send_BLE_command_and_data(p_nus, SEND_BOOTLOADER_VERSION, 2, data_to_send);
			break;
		}
		//====================//

		//===SEND_SERIAL_NUMBER===//
		case SEND_SERIAL_NUMBER:
		{
			switch (tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case(SERIAL_LOCK):
				{
					int temp = read_to_UICR(UICR_LOCK_SERIAL_NUMBER_ADDRESS);
					if(temp == -1)
					{
						//@@@ manage this error: serial number not present
					}
					uint8_t data_to_send[4];
					data_to_send[0] = (uint8_t)((temp >> 24) & 0x000000FF);
					data_to_send[1] = (uint8_t)((temp >> 16) & 0x000000FF);
					data_to_send[2] = (uint8_t)((temp>> 8) & 0x000000FF);
					data_to_send[3] = (uint8_t)(temp & 0x000000FF);
					send_BLE_command_and_data(p_nus, SEND_SERIAL_NUMBER, 4, data_to_send);
					break;
				}
				case(SERIAL_KEY):
				{
					int temp = read_to_UICR(UICR_KEY_SERIAL_NUMBER_ADDRESS);
					if(temp == -1)
					{
						//@@@ manage this error: serial number not present
					}
					uint8_t data_to_send[4];
					data_to_send[0] = (uint8_t)((temp >> 24) & 0x000000FF);
					data_to_send[1] = (uint8_t)((temp >> 16) & 0x000000FF);
					data_to_send[2] = (uint8_t)((temp>> 8) & 0x000000FF);
					data_to_send[3] = (uint8_t)(temp & 0x000000FF);
					send_BLE_command_and_data(p_nus, SEND_SERIAL_NUMBER, 4, data_to_send);
					break;
				}
				case(SERIAL_PCBA):
				{
					int temp = read_to_UICR(UICR_PCBA_SERIAL_NUMBER_ADDRESS);
					if(temp == -1)
					{
						//@@@ manage this error: serial number not present
					}
					uint8_t data_to_send[4];
					data_to_send[0] = (uint8_t)((temp >> 24) & 0x000000FF);
					data_to_send[1] = (uint8_t)((temp >> 16) & 0x000000FF);
					data_to_send[2] = (uint8_t)((temp>> 8) & 0x000000FF);
					data_to_send[3] = (uint8_t)(temp & 0x000000FF);
					send_BLE_command_and_data(p_nus, SEND_SERIAL_NUMBER, 4, data_to_send);
					break;
				}
				case(SERIAL_PRODUCT):
				{
					int temp = read_to_UICR(UICR_PRODUCT_SERIAL_NUMBER_ADDRESS);
					if(temp == -1)
					{
						//@@@ manage this error: serial number not present
					}
					uint8_t data_to_send[4];
					data_to_send[0] = (uint8_t)((temp >> 24) & 0x000000FF);
					data_to_send[1] = (uint8_t)((temp >> 16) & 0x000000FF);
					data_to_send[2] = (uint8_t)((temp>> 8) & 0x000000FF);
					data_to_send[3] = (uint8_t)(temp & 0x000000FF);
					send_BLE_command_and_data(p_nus, SEND_SERIAL_NUMBER, 4, data_to_send);
					break;
				}
				default:
					break;
			}


			break;
		}
		//====================//


		//===SET_NEW_HALO_BRIGHTNESS===//
		case SET_NEW_HALO_BRIGHTNESS:
			change_baseline_led_brightness(tasks_to_be_handled->queue_array[index_position].data[0]);
			//SH_halo_set_new_brightness(tasks_to_be_handled->queue_array[index_position].data[0]);
			send_confirmation_of_reception(p_nus, SET_NEW_HALO_BRIGHTNESS);
			break;
		//====================//

		//===SET_NEW_HALO_BRIGHTNESS===//
		case ANIMATION_DEMO:

			switch(tasks_to_be_handled->queue_array[index_position].data[0])
			{
				case PAIRING_ANIMATION_DEMO:
					insert_animation_function_pointer(animation_function_buffer, &Animations_pairing, RESET_ANIMATION,empty_parameter,empty_parameter);
					insert_animation_function_pointer(animation_function_buffer, &Animations_success, RESET_ANIMATION,empty_parameter,empty_parameter);
					insert_animation_function_pointer(animation_function_buffer, &turnOffAllLEDs, RESET_ANIMATION,empty_parameter,empty_parameter);
					send_confirmation_of_reception(p_nus, ANIMATION_DEMO);
					break;

				default:
					break;
			}
			break;
		//====================//


////GENERAL OPERATIONS////


		default:
			//return error
			break;

	}

}



bool get_front_led_status()
{
	return front_led_blink_status;
}


SH_as_the_crow_flies_params_t get_as_the_crow_flies_current_params()
{
	return current_as_the_crow_flies_params;
}

static bool check_previous_animation(uint8_t new_animation)
{
	if(previous_animation != new_animation)
	{
		previous_animation = new_animation;
		return true;
	}
	else
	{
		previous_animation = new_animation;
		return false;
	}


}







/*
 * SH_BLE.h
 *
 *  Created on: Jan 29, 2016
 *      Author: Sean Beitz
 */

#include "SHP_Frames_and_commands.h"
#include "SH_typedefs.h"

#include "SH_Primary_state_machine.h"
#include <stdbool.h>
#include "ble_nus.h"

#ifndef SH_FIRMWARE_CODE_SH_BLE_H_
#define SH_FIRMWARE_CODE_SH_BLE_H_

//==========================================//
/**
 * @brief initialises the proper conditions for ble messages (crc table)
 *
 * Function make a crc table to lookup values (quicker access)
 *
 *returns pass/fail boolean (true is pass)
 *
 */
bool SH_BLE_init();


/**
 * @brief manages messages sent using in house protocol
 *
 * @param SH_BLE_message is a pointer to an array containing a SHP in house protocol message
 *
 *returns a struct that contains the parsed message, if message is valid
 *
 */
void manage_SH_BLE_messages(SHP_message_buffer_t* input_message_list,SHP_command_buffer_t* LOW_priority_output_command_list, SHP_command_buffer_t* HIGH_priority_output_command_list);

/**
 * @brief sends a message to the central device stating that peripheral is ready to receive
 *
 *
  *returns pass/fail boolean (true is pass)
 *
 */
bool ready_to_receive();


/**
 * @brief sends an error report to the central device or confirmation of reception if no errors
 *
 */
void send_BLE_report();


/**
 * @brief retrieves a SHP in house protocol message
 *
 *
  *returns a pointer to the array containing the in house protocol message
 *
 * to be used in conjunction with manage_SH_BLE_messages()
 */
const uint8_t* get_SH_BLE_message();


uint8_t get_uart_shp(uint8_t* msg, uint8_t index, SHP_message_buffer_t* SHP_msg_buffer);


void send_SH_BLE_message();


//sends the confirmed_command with no data bits back to the central device to confirm reception of command
void send_confirmation_of_reception(ble_nus_t * p_nus, uint8_t confirmed_command);



void send_BLE_command_and_data(ble_nus_t * p_nus, uint8_t command, uint8_t length, uint8_t* data);



#endif /* SH_FIRMWARE_CODE_SH_BLE_H_ */

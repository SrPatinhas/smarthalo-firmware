/*
 * SH_Softdevice_and_communication_init.h
 *
 *  Created on: Aug 3, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SH_SOFTDEVICE_AND_COMMUNICATION_INIT_H_
#define SH_FIRMWARE_CODE_SH_SOFTDEVICE_AND_COMMUNICATION_INIT_H_



uint32_t gap_params_init(void);

void on_conn_params_evt(ble_conn_params_evt_t * p_evt);

uint32_t conn_params_init(void);

uint32_t ble_stack_init(void);

//uint32_t device_manager_init(bool erase_bonds);

uint32_t services_init(void);

uint32_t advertising_init(void);

uint32_t uart_init(void);

SHP_message_buffer_t* get_SHP_msg_buffer_address();

bool get_factory_mode_status();

void set_factory_mode_status(bool status);

ble_nus_t* get_m_nus();

void initialise_bluetooth_stack_and_services(bool erase_bonds);

#endif /* SH_FIRMWARE_CODE_SH_SOFTDEVICE_AND_COMMUNICATION_INIT_H_ */

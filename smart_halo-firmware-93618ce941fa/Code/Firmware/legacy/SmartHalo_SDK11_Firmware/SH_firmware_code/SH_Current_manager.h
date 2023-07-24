/*
 * SH_Current_manager.h
 *
 *  Created on: Jul 27, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SH_CURRENT_MANAGER_H_
#define SH_FIRMWARE_CODE_SH_CURRENT_MANAGER_H_

typedef struct device_current_consumption_state{
	bool leds_drivers_state;
	bool piezo_power_supply_state;
	bool touch_power_supply_state;
	bool accelerometer_state;
	bool magnetometer_state;
}device_current_consumption_state_t;

void shutoff_all_devices();


void reenable_all_devices();


void reenable_essential_devices();


void enable_led_drivers();


void disable_led_drivers();


void enable_piezo_supply();


void disable_piezo_supply();


void enable_touch_supply();//used for the touch button


void disable_touch_supply();


void put_accelerometer_to_sleep();


void awaken_accelerometer();


void awaken_magnetometer();


void put_magnetometer_to_sleep();


device_current_consumption_state_t* get_current_consumption_states();

#endif /* SH_FIRMWARE_CODE_SH_CURRENT_MANAGER_H_ */



#include "SH_Includes.h"


device_current_consumption_state_t current_state;


void shutoff_all_devices()
{

	disable_led_drivers();
	disable_piezo_supply();
	disable_touch_supply();
	//put_accelerometer_to_sleep();
	//put_magnetometer_to_sleep();
	turn_off_center_led_pwm();
    turn_off_front_led_pwm();
	SH_twi_disable();
	//app_uart_close();
}

void reenable_all_devices()
{
	SH_twi_initialisation();
	enable_led_drivers();
	HaloPixel_begin();
	enable_piezo_supply();
	enable_touch_supply();
	awaken_accelerometer();
	awaken_magnetometer();
	pwm_center_led_init(); //responsible for 2 mA of current!
    pwm_front_led_init();
	//uart_init();
}


void reenable_essential_devices()
{
	SH_twi_initialisation();
	enable_led_drivers();
	HaloPixel_begin();
	//awaken_accelerometer();
	//awaken_magnetometer();
	restart_center_led_pwm();
    restart_front_led_pwm();
	//uart_init();
}


void enable_led_drivers()
{
	nrf_gpio_cfg_output(EN_VLED);
	nrf_gpio_pin_write(EN_VLED, 1);
	current_state.leds_drivers_state = true;
}


void disable_led_drivers()
{
	nrf_gpio_cfg_output(EN_VLED);
	nrf_gpio_pin_write(EN_VLED, 0);
	current_state.leds_drivers_state = false;
}


void enable_piezo_supply()
{
	nrf_gpio_cfg_output(EN_VPIEZO);
	nrf_gpio_pin_write(EN_VPIEZO, 1);
	current_state.piezo_power_supply_state = true;
}


void disable_piezo_supply()
{
	nrf_gpio_cfg_output(EN_VPIEZO);
	nrf_gpio_pin_write(EN_VPIEZO, 0);
	current_state.piezo_power_supply_state = false;
}


void enable_touch_supply()//used for the touch button
{
	nrf_gpio_cfg_output(EN_2_8V);
	nrf_gpio_pin_write(EN_2_8V, 1);
	current_state.touch_power_supply_state = true;
}


void disable_touch_supply()
{
	nrf_gpio_cfg_output(EN_2_8V);
	nrf_gpio_pin_write(EN_2_8V, 0);
	current_state.touch_power_supply_state = false;
}


void put_accelerometer_to_sleep()
{
	SH_set_accelerometer_to_sleep_mode_while_enabling_interrupts();
//	ACCELEROMETER_set_power_mode(POWER_MODE_SLEEP_MODE);
	current_state.accelerometer_state = true;
}

void put_magnetometer_to_sleep()
{
	SH_Magnetometer_sleep_mode();
	current_state.magnetometer_state = false;
}

void awaken_accelerometer()
{
	SH_initialization_of_accelerometer();
	current_state.accelerometer_state = false;
}


void awaken_magnetometer()
{
	SH_Magnetometer_initialization();
	current_state.magnetometer_state = true;
}





device_current_consumption_state_t* get_current_consumption_states()
{
	return &current_state;
}


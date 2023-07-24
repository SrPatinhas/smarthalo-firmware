/*
 * SH_Accelerometer_INT2.c
 *
 *  Created on: 2016-04-13
 *      Author: SmartHalo
 */


#include "stdafx.h"
#include "app_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include <stdbool.h>
#include "SH_Accelerometer_INT2.h"

volatile bool is_interrupt_active = false;

/*ACCELEROMETER_INT2_EVENTHANDLER
 * When a toggle occurs on the pin, if the pin is High, the interrupt_is_active
 * is set to true
 * @@@ To modify if we only want from LOW_TO_HIGH to put interrupt_is_active to true
 */
static void accelerometer_int2_eventhandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

//ACCELEROMETER_INT2_INITIALISATION
void SH_Accelerometer_INT2_initialisation(){

	ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);
    if(!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
    }
	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
#ifdef BOARD_PCA10040
		config.pull = NRF_GPIO_PIN_PULLDOWN;
#elif defined(SMARTHALO_EE)
		config.pull = NRF_GPIO_PIN_PULLDOWN;
#else
#error "no target"
#endif
	err_code = nrf_drv_gpiote_in_init(PIN_INT2, &config, accelerometer_int2_eventhandler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(PIN_INT2, true);

}

//ACCELEROMETER_INT2_EVENTHANDLER
static void accelerometer_int2_eventhandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){

	//may also want to use GPIOTE_CONFIG_POLARITY_LoToHi to check only for the rising edge of the interrupt signal

	if (pin==PIN_INT2){
		if (nrf_drv_gpiote_in_is_set(PIN_INT2)){
			is_interrupt_active=true;

		}
		else {
//this may reset the value of the flag too rapidly for the main state machine to register.
			//is_interrupt_active=false;
		}
	}
}

//CHECK_FOR_FLAG_ACCELEROMETER_INT2
bool SH_check_for_flag_accelerometer_int2(){

	return is_interrupt_active;
}

//resets flag to false after the main state machine reads the value
void SH_reset_flag_accelerometer_int2(){

	 is_interrupt_active = false;
}



//ENABLE_DISABLE_INTERRUPT_FROM_ACCELEROMETER_INT2
void SH_enable_disable_interrupt_from_accelerometer_int2(bool enable){

	nrf_drv_gpiote_in_event_enable(PIN_INT2, enable);
}











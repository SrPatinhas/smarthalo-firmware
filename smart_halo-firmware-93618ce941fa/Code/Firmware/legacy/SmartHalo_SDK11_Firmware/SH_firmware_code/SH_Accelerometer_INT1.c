/*
 * SH_Accelerometer_INT1.c
 *
 *  Created on: 2016-03-18
 *      Author: SmartHalo
 */

#include "stdafx.h"
#include "app_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include <stdbool.h>
#include "SH_Accelerometer_INT1.h"

static bool is_interrupt_active = false;

/*ACCELEROMETER_INT1_EVENTHANDLER
 * When a toggle occurs on the pin, if the pin is High, the interrupt_is_active
 * is set to true
 * @@@ To modify if we only want from LOW_TO_HIGH to put interrupt_is_active to true
 */
static void accelerometer_int1_eventhandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

//Accelerometer_INT1_INITIALISATION
void SH_Accelerometer_INT1_initialisation(){

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
	err_code = nrf_drv_gpiote_in_init(PIN_INT1, &config, accelerometer_int1_eventhandler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(PIN_INT1, true);

}

//ACCELEROMETER_INT1_EVENTHANDLER
static void accelerometer_int1_eventhandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){

	if (pin==PIN_INT1){
		if (nrf_drv_gpiote_in_is_set(PIN_INT1)){
			is_interrupt_active=true;
		}
		else {
			is_interrupt_active=false;
		}
	}
}

//CHECK_FOR_FLAG_ACCELEROMETER_INT1
bool SH_check_for_flag_accelerometer_int1(){

	return is_interrupt_active;
}

//ENABLE_DISABLE_INTERRUPT_FROM_ACCELEROMETER_INT1
void SH_enable_disable_interrupt_from_accelerometer_int1(bool enable){

	nrf_drv_gpiote_in_event_enable(PIN_INT1, enable);
}


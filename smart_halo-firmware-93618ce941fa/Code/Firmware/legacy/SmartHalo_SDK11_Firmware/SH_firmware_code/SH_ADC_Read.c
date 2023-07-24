

#include "stdafx.h"
#include "boards.h"
#include "nrf_saadc.h"
#include "nrf_drv_saadc.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "SH_Animations.h"
#include "SH_ADC_Read.h"

#define PIN_ADC_READ_V_BATTERY_LEVEL	NRF_SAADC_INPUT_AIN0
#define PIN_ADC_READ_I_BATTERY_LEVEL	NRF_SAADC_INPUT_AIN3
#define CHANNEL_VBAT_SAADC				0
#define CHANNEL_IBAT_SAADC				1

#ifndef NRF_APP_PRIORITY_HIGH
#define NRF_APP_PRIORITY_HIGH 1
#endif

#define NUMBER_OF_SAMPLE 10

#define GAIN_MONITEUR_VOLTAGE 0.75
#define GAIN_MONITEUR_CURRENT 1.1
#define MAX_VOLTAGE 3.3

#define ADC_RESOLUTION 1024 // 2^number of bit NRF_SAADC_RESOLUTION_10BIT

#define ADC_STEP_V_BATTERY_LEVEL MAX_VOLTAGE/ADC_RESOLUTION/GAIN_MONITEUR_VOLTAGE
#define ADC_STEP_I_BATTERY_LEVEL MAX_VOLTAGE/(ADC_RESOLUTION)/(GAIN_MONITEUR_CURRENT)


volatile uint8_t state = 1;

static nrf_saadc_value_t       m_buffer_pool[2][NUMBER_OF_SAMPLE];
static SH_Battery_Levels_t 			battery_level;
static nrf_saadc_channel_config_t  	channel_v_bat_config;
static nrf_saadc_channel_config_t  	channel_i_bat_config;
static uint8_t 						m_adc_channel_enabled;
static bool							voltage_is_sampled = false;
static bool							current_is_sampled = false;
static bool							begun = false;

/*
 * READ_THE_BATTERY_LEVEL
 *
 * Function to handle events from the SAADC channel
 * if the conversion from the buffer is done it will sets the new battery current or voltage levels
 */
static void read_the_battery_level(nrf_drv_saadc_evt_t const * p_event);

/*
 * CONVERSION_OF_V_BATTERY_LEVEL_ADC_READ
 *
 * USES ADC_STEP_V_BATTERY_LEVEL to convert the value on 1024 returned
 * by the buffer conversion for a value between 0 and 4,3V
 *
 */
static float conversion_of_v_battery_level_ADC_read (int16_t adc_value);

/*
 * CONVERSION_OF_I_BATTERY_LEVEL_ADC_READ
 *
 * USES ADC_STEP_I_BATTERY_LEVEL to convert the value on 1024 returned
 * by the buffer conversion for a value between -1 and 1,5 A
 *
 */
static float conversion_of_i_battery_level_ADC_read (int16_t adc_value);


static void read_the_battery_level(nrf_drv_saadc_evt_t const * p_event)
{
	int16_t value[NUMBER_OF_SAMPLE];
	int16_t avg_value=0;


	//If the ADC conversion is done
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		//Sets the other channel to sample next
		if(m_adc_channel_enabled == CHANNEL_VBAT_SAADC)
		{
			nrf_drv_saadc_channel_uninit(CHANNEL_VBAT_SAADC);
			nrf_drv_saadc_channel_init(CHANNEL_IBAT_SAADC, &channel_i_bat_config);
			m_adc_channel_enabled = CHANNEL_IBAT_SAADC;

		}
		else if(m_adc_channel_enabled == CHANNEL_IBAT_SAADC)
		{
			nrf_drv_saadc_channel_uninit(CHANNEL_IBAT_SAADC);
			nrf_drv_saadc_channel_init(CHANNEL_VBAT_SAADC, &channel_v_bat_config);
			m_adc_channel_enabled = CHANNEL_VBAT_SAADC;
		}


		nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 1);

		//
		if (m_adc_channel_enabled == CHANNEL_VBAT_SAADC)
		{
			for (uint8_t i=0; i<NUMBER_OF_SAMPLE; i++){
				value[i] =  p_event->data.done.p_buffer[i];
				avg_value+=value[i];
			}
			avg_value = avg_value/NUMBER_OF_SAMPLE;

			battery_level.current = conversion_of_i_battery_level_ADC_read(avg_value);
			current_is_sampled = true;
		}
		else if (m_adc_channel_enabled == CHANNEL_IBAT_SAADC)
		{
			for (uint8_t i=0; i<NUMBER_OF_SAMPLE; i++){
				value[i] =  p_event->data.done.p_buffer[i];
				avg_value+=value[i];
			}
			avg_value = avg_value/NUMBER_OF_SAMPLE;

			battery_level.voltage = conversion_of_v_battery_level_ADC_read(avg_value);
			voltage_is_sampled = true;

		}
	}
}

static float conversion_of_v_battery_level_ADC_read (int16_t adc_value){

	return ((float)ADC_STEP_V_BATTERY_LEVEL * adc_value);

}

static float conversion_of_i_battery_level_ADC_read (int16_t adc_value){

	return (((float)ADC_STEP_I_BATTERY_LEVEL * adc_value) - 1);

}

void SH_saadc_initialization(void)
{
	if (!begun){
	    //VBAT
	    //set configuration for saadc channel 0
	    channel_v_bat_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
	    channel_v_bat_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	    channel_v_bat_config.gain       = NRF_SAADC_GAIN1_6;
	    channel_v_bat_config.reference  = NRF_SAADC_REFERENCE_INTERNAL;
	    channel_v_bat_config.acq_time   = NRF_SAADC_ACQTIME_10US;
	    channel_v_bat_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
	    channel_v_bat_config.pin_p      = (nrf_saadc_input_t)(PIN_ADC_READ_V_BATTERY_LEVEL);
	    channel_v_bat_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

	    //IBAT
	    //set configuration for saadc channel 1
	    channel_i_bat_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
	    channel_i_bat_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	    channel_i_bat_config.gain       = NRF_SAADC_GAIN1_6;
	    channel_i_bat_config.reference  = NRF_SAADC_REFERENCE_INTERNAL;
	    channel_i_bat_config.acq_time   = NRF_SAADC_ACQTIME_10US;
	    channel_i_bat_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
	    channel_i_bat_config.pin_p      = (nrf_saadc_input_t)(PIN_ADC_READ_I_BATTERY_LEVEL);
	    channel_i_bat_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

	    nrf_drv_saadc_init(NULL, read_the_battery_level);


    	nrf_drv_saadc_channel_init(CHANNEL_VBAT_SAADC, &channel_v_bat_config);

    	m_adc_channel_enabled = CHANNEL_VBAT_SAADC;

    	//Calibration of the SAADC
    	nrf_saadc_task_trigger(NRF_SAADC_TASK_CALIBRATEOFFSET);

    	while (!(nrf_saadc_event_check(NRF_SAADC_EVENT_CALIBRATEDONE))){
    	}

    	begun = true;

    	nrf_drv_saadc_buffer_convert(m_buffer_pool[0],NUMBER_OF_SAMPLE);
    	nrf_drv_saadc_buffer_convert(m_buffer_pool[1],NUMBER_OF_SAMPLE);
	}
}


void SH_adc_read_battery_level(void)
{


	while (!SH_sampling_battery_level_is_done()){
		nrf_drv_saadc_sample();
	}
}

SH_Battery_Levels_t SH_get_battery_levels(){

	voltage_is_sampled = false;
	current_is_sampled = false;

	return battery_level;
}

bool SH_sampling_battery_level_is_done(){


	return (voltage_is_sampled && current_is_sampled);
}


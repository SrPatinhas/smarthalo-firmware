/*
 * SAADC.c
 *
 *  Created on: Nov 16, 2016
 *      Author: sgelinas
 */

#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_saadc.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"

#include "CommandLineInterface.h"

#define VBZ_VSENSE_CONV		(87.0/12.0)
#define VSENSE_LSB			(2.8/1023.0)

#define SAMPLES_IN_BUFFER 5

static nrf_saadc_value_t       m_buffer_pool[2][SAMPLES_IN_BUFFER];
static uint32_t                m_adc_evt_counter;

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        int i;
        printf("ADC event number: %d\r\n",(int)m_adc_evt_counter);
        for (i = 0; i < SAMPLES_IN_BUFFER; i++)
        {
        	int16_t sample = p_event->data.done.p_buffer[i]-1;
        	double sample_conv = (double)sample*VSENSE_LSB*VBZ_VSENSE_CONV;
            printf("%d, %.2fV\r\n", sample, sample_conv);
        }
        m_adc_evt_counter++;
    }
}

void saadc_setup(void)
{
	ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
            NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN3);

    channel_config.reference = NRF_SAADC_REFERENCE_VDD4;
    channel_config.gain = NRF_SAADC_GAIN1_4;

    nrf_drv_saadc_config_t saadc_config;

    saadc_config.resolution = NRF_SAADC_RESOLUTION_10BIT;
	saadc_config.oversample = NRF_SAADC_OVERSAMPLE_DISABLED;
	saadc_config.interrupt_priority = SAADC_CONFIG_IRQ_PRIORITY;


    err_code = nrf_drv_saadc_init(&saadc_config, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0],SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1],SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}

void saadc_sample()
{
	ret_code_t err_code;

	for(int i=0; i<SAMPLES_IN_BUFFER; i++)
	{
		nrf_delay_ms(10);
		err_code = nrf_drv_saadc_sample();
		APP_ERROR_CHECK(err_code);
	}
}

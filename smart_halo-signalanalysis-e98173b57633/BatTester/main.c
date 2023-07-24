/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "app_uart.h"
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "nrfx_clock.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"
#include "nrf.h"
#include "bsp.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_spi.h"
#include "nrf_uart.h"


#define ADC_REF_VOLTAGE_IN_MILLIVOLTS  					600  //!< Reference voltage (in milli volts) used by ADC while doing conversion.
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS 					270  //!< Typical forward voltage drop of the diode (Part no: SD103ATW-7-F) that is connected in series with the voltage supply. This is the voltage drop when the forward current is 1mA. Source: Data sheet of 'SURFACE MOUNT SCHOTTKY BARRIER DIODE ARRAY' available at www.diodes.com.
#define ADC_RES_10BIT                  					1024 //!< Maximum digital value for 10-bit ADC conversion.
#define ADC_PRE_SCALING_COMPENSATION   					6    //!< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE) *ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

#define SPI_MISO_PIN 									30
#define SPI_SCK_PIN  									26
#define SPI_MOSI_PIN 									29
#define SPI_SS_PIN   									31

#define TOUCH_PIN_TSC_G1_IO4 							22 
#define TOUCH_PIN_TSC_G2_IO1 							23 
#define TOUCH_PIN_TSC_G3_IO4 							24 
#define TOUCH_PIN_TSC_G4_IO3 							25
#define NUMBER_OF_TOUCH_PINS 							4

#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED
#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */
#define SAMPLES_IN_BUFFER 5

#define SPI_INSTANCE  0 /**< SPI instance index. */

static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(0);
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;
static volatile bool         spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */
static uint32_t              measured_voltage=0;

#define TEST_STRING 									"SmartHalo"
static uint8_t       m_tx_buf[] = TEST_STRING;           /**< TX buffer. */
static uint8_t       m_rx_buf[sizeof(TEST_STRING) + 1];    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */

#define TOUCH_SAMPLE_SIZE				    			10u
static bool inputTouchTestResults[NUMBER_OF_TOUCH_PINS][TOUCH_SAMPLE_SIZE];
static uint8_t touchCounters[NUMBER_OF_TOUCH_PINS]={0,0,0,0};
//to compare against expected results:  
static bool inputTouchTestEXPECTEDResults[TOUCH_SAMPLE_SIZE]={0,1,0,1,0,1,0,1,0,1};



//=============================
//   EVENT HANDLERS
//=============================

void touch_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	uint8_t pinIndex = 0;
	switch(pin)
	{
		case TOUCH_PIN_TSC_G1_IO4:
			pinIndex = 0;
			break;
		case TOUCH_PIN_TSC_G2_IO1:
			pinIndex = 1;
			break;
		case TOUCH_PIN_TSC_G3_IO4:
			pinIndex = 2;
			break;
		case TOUCH_PIN_TSC_G4_IO3:
			pinIndex = 3;
			break;
	}

	if((touchCounters[pinIndex] < TOUCH_SAMPLE_SIZE) && (action == NRF_GPIOTE_POLARITY_TOGGLE))
	{
		if(touchCounters[pinIndex])
			inputTouchTestResults[pinIndex][touchCounters[pinIndex]] = !inputTouchTestResults[pinIndex][touchCounters[pinIndex-1]];
		else inputTouchTestResults[pinIndex][touchCounters[pinIndex]] = 0; //prevent segfault by accessing element -1 here^
		touchCounters[pinIndex]++;
	}
}

void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
    if (m_rx_buf[0] != 0)
    {

    }
}

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void timer_handler(nrf_timer_event_t event_type, void * p_context)
{

}


//=============================
//   INITIALISATION CODE
//=============================

void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 100us */
    uint32_t ticks = nrfx_timer_us_to_ticks(&m_timer, 100);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
        {
            measured_voltage = p_event->data.done.p_buffer[i];
            //TODO : replace measured voltage by a large array to record large data sets, or a circular buffer, even better
        }

        m_adc_evt_counter++;
    }
}


void saadc_init(void)
{
    ret_code_t err_code;

    bsp_board_init(BSP_INIT_LEDS);

    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          UART_HWFC,
          false,
#if defined (UART_PRESENT)
          NRF_UART_BAUDRATE_115200
#else
          NRF_UARTE_BAUDRATE_115200
#endif
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);


    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

}

void deviceSetup()
{
	ret_code_t err_code;

    //MANAGEMENT SETUP
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    ret_code_t ret_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(ret_code);

    //ADC SETUP
    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();

    //GPIO SETUP
    err_code = nrf_drv_gpiote_init();
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true); //configure as inputs
    in_config.pull = NRF_GPIO_PIN_PULLUP;
	err_code = nrf_drv_gpiote_in_init(TOUCH_PIN_TSC_G1_IO4, &in_config, touch_event_handler);
	err_code = nrf_drv_gpiote_in_init(TOUCH_PIN_TSC_G2_IO1, &in_config, touch_event_handler); 
	err_code = nrf_drv_gpiote_in_init(TOUCH_PIN_TSC_G3_IO4, &in_config, touch_event_handler);
	err_code = nrf_drv_gpiote_in_init(TOUCH_PIN_TSC_G4_IO3, &in_config, touch_event_handler);
	APP_ERROR_CHECK(err_code);

    //SPI SETUP
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));
   
}

//=============================
//   SIGNAL ANALYSIS
//=============================



bool validatePiezoResults()
{
	//TODO Content
	return true;
}

bool piezoTest()
{
  	 bool itPassed = false;
     printf(" \r\nPIEZO_TEST\r\n\n");
     uint32_t resultant_voltage = 0;
     
	 //TODO: record values over a period of a scond or something to anaylse output (whould range between 0 and 20V, will have to determine frequency)

	//just to make sure that signed values never make it in
    if(measured_voltage > 200000) measured_voltage=0;

    //the multiplied by 2 implies i am using a voltage divider to measure half the battery voltage
    resultant_voltage = ADC_RESULT_IN_MILLI_VOLTS(measured_voltage)*2; 
	printf("%ld\r\n",resultant_voltage);
	itPassed = validatePiezoResults();

	return itPassed;
}

bool validateOledResults()
{
	//TODO Content
	return true;
}


bool oledTest()
{
	bool itPassed = false;

	printf(" \r\nOLED_TEST\r\n\n");

	if(spi_xfer_done)
	{
		memset(m_rx_buf, 0, m_length);
		spi_xfer_done = false;
		nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, m_length);
	}

	itPassed = validateOledResults();

	return itPassed;
}

bool validateTouchResults()
{
	bool passFail = 1; 

	for(int i = 0; i < NUMBER_OF_TOUCH_PINS; ++i)
	{
		if(memcmp(&inputTouchTestResults[NUMBER_OF_TOUCH_PINS][0],inputTouchTestEXPECTEDResults, sizeof(inputTouchTestResults[NUMBER_OF_TOUCH_PINS][0])*TOUCH_SAMPLE_SIZE) != 0)
			passFail =false; //memset return 0  if the memory locations are identical, fail test if anything else is returned
	}

	//printf(" pin : %ld , value 1: \r\n", );

	return passFail;
}

//touch test receives input to check if the touch circuitry has continuity to the stm32
bool touchTest() 
{
	bool itPassed = false;

	printf(" \r\nTOUCH_TEST\r\n\n");

	//enables touch sensing via interrupt
	nrfx_gpiote_in_event_enable(TOUCH_PIN_TSC_G1_IO4,true); //setup interrupt 
	nrfx_gpiote_in_event_enable(TOUCH_PIN_TSC_G2_IO1,true);
	nrfx_gpiote_in_event_enable(TOUCH_PIN_TSC_G3_IO4,true);
	nrfx_gpiote_in_event_enable(TOUCH_PIN_TSC_G4_IO3,true);

	nrf_delay_ms(250);//delay for whatever the amount of time we want to run this tests is. 

	//disables touch sensing via interrupt
	nrfx_gpiote_in_event_disable(TOUCH_PIN_TSC_G1_IO4); //unsetup interrupt 
	nrfx_gpiote_in_event_disable(TOUCH_PIN_TSC_G2_IO1);
	nrfx_gpiote_in_event_disable(TOUCH_PIN_TSC_G3_IO4);
	nrfx_gpiote_in_event_disable(TOUCH_PIN_TSC_G4_IO3);

	itPassed = validateTouchResults();

	//reset memory in touch buffers to 0
	for(int i=0; i<NUMBER_OF_TOUCH_PINS; ++i)
	{
		memset(&inputTouchTestResults[i][0],0,sizeof(inputTouchTestResults[i][0])*TOUCH_SAMPLE_SIZE);
	}
	
	return itPassed;
}

//=============================
//   I'M BATMAN
//=============================

void imBatman() //without flow control this causes a uart buffer overflow, do not use
{
    printf("                   ,.ood888888888888boo.,\r\n");
    printf("              .od888P^""            ""^Y888bo.\r\n");
    printf("          .od8P''   ..oood88888888booo.    ``Y8bo.\r\n");
    printf("       .odP'   .ood8888888888888888888888boo.   `Ybo.\r\n");
    printf("     .d8'   od8'd888888888f`8888't888888888b`8bo   `Yb.\r\n");
    printf("    d8'  od8^   8888888888[  `'  ]8888888888   ^8bo  `8b\r\n");
    printf("  .8P  d88'     8888888888P      Y8888888888     `88b  Y8.\r\n");
    printf(" d8' .d8'       `Y88888888'      `88888888P'       `8b. `8b\r\n");
    printf(".8P .88P            """"            """"            Y88. Y8.\r\n");
    printf("88  888                                              888  88\r\n");
    printf("88  888               nananananananana               888  88\r\n");
    printf("88  888.        ..                        ..        .888  88\r\n");
    printf("`8b `88b,     d8888b.od8bo.      .od8bo.d8888b     ,d88' d8'\r\n");
    printf(" Y8. `Y88.    8888888888888b    d8888888888888    .88P' .8P\r\n");
    printf("  `8b  Y88b.  `88888888888888  88888888888888'  .d88P  d8'\r\n");
    printf("    Y8.  ^Y88bod8888888888888..8888888888888bod88P^  .8P\r\n");
    printf("     `Y8.   ^Y888888888888888LS888888888888888P^   .8P'\r\n");
    printf("       `^Yb.,  `^^Y8888888888888888888888P^^'  ,.dP^'\r\n");
    printf("          `^Y8b..   ``^^^Y88888888P^^^'    ..d8P^'\r\n");
    printf("              `^Y888bo.,            ,.od888P^'\r\n");
    printf("                    `^^Y888888888888P^^'\r\n");
}

//=============================
//   MEAT AND POTATOES
//=============================

int main(void)
{
	deviceSetup();
   
    printf("=========================SH2 FUNTIONAL TESTS SIGNAL ANALYSIS========================\r\n\n");
 
    while (1)
    {
        uint8_t uartInChar=0;
        nrf_pwr_mgmt_run();
        NRF_LOG_FLUSH();

        app_uart_get(&uartInChar); // get a value on the uart line 
		// TODO make parser to get longer messages

         if (uartInChar == 'p' || uartInChar == 'P') //Piezo tester, uses adc to validate buzzer circuitry
         {
            piezoTest();         }

         if (uartInChar == 'o' || uartInChar == 'O') //Oled tester, receives spi communication
         {
         	oledTest();
         }

         if (uartInChar == 't' || uartInChar == 'T') //Touch tester, sends high and low values on the touch lines
         {
         	touchTest();
         }
    }
}


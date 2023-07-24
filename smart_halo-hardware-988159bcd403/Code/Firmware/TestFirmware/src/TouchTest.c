/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Tone generation based on nRF HW timer
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#include <string.h>

#include "TouchTest.h"

#include "nrf_drv_config.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "app_timer.h"

#include "SmartHalo.h"
#include "LEDTest.h"
#include "CommandLineInterface.h"
#include "PowerSupply.h"

#define THBDURATIONMIN   		5
#define THBDURATIONMAX   		60000
#define TOUCHLOCDURATIONMIN   	0
#define TOUCHLOCDURATIONMAX   	60000


static uint8_t MODEPIN = 0;
static uint8_t TOUCHPIN = 0;
static uint8_t location = 0;
static int heartbeatcount = 0;
static int prevstate = 0;
static int timeout = 0;

void TouchTest_updateLocation();
void TouchTest_detectedLocation();
void TouchTest_stopTouchLocationTest();

APP_TIMER_DEF(heartbeattimer_id);
APP_TIMER_DEF(touchloctimer_id);

static void TouchTest_heartbeat_timer_handler(void * p_context)
{
	if(heartbeatcount == 0)
	{
		CommandLineInterface_printLine("Heartbeat count test completed with error: No heartbeat detected");
	}
	else if(heartbeatcount%2)
	{
		CommandLineInterface_printf("Heartbeat count test completed with error: %d is an odd edge count\r\n", heartbeatcount);
	}
	else
	{
		CommandLineInterface_printf("Heartbeat count test completed : %d heartbeats in %d seconds\r\n", heartbeatcount/2, timeout);
	}

	nrf_drv_gpiote_in_uninit(TOUCHPIN);

	// Disable Touch supply
	if(PowerSupply_getState(SUPPLY_S2_8V))
	{
		PowerSupply_disable(SUPPLY_S2_8V);
		nrf_delay_ms(100);
	}
}

static void TouchTest_touchloc_timer_handler(void * p_context)
{
	TouchTest_stopTouchLocationTest();

	CommandLineInterface_printf("Touch location test timed out for location %d\r\n", location);

	location = 0;
}

static void TouchTest_createtimers()
{
	ret_code_t err_code;

	err_code = app_timer_create(&heartbeattimer_id, APP_TIMER_MODE_SINGLE_SHOT, TouchTest_heartbeat_timer_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&touchloctimer_id, APP_TIMER_MODE_SINGLE_SHOT, TouchTest_touchloc_timer_handler);
	APP_ERROR_CHECK(err_code);
}

#ifdef BOARD_PCA10040
// HEARTBEAT OPF 15 us every 4 ms in normal mode
static void TouchTest_heartbeat_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if( pin == TOUCHPIN )
	{
		// Falling edge (beginning of heartbeat)
		if((!nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 1))
		{
			nrf_delay_ms(1); // brute force debouncing: to be changed for real chip (10 ms maybe?)
			prevstate = 0;
			heartbeatcount++;
		}

		// Rising edge (end of heartbeat)
		if((nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 0))
		{
			nrf_delay_ms(1); // brute force debouncing: to be changed for real chip (10 ms maybe?)
			prevstate = 1;
			heartbeatcount++;
		}
	}
}
#else
static void TouchTest_heartbeat_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if( pin == TOUCHPIN )
	{
		// Rising edge (beginning of heartbeat)
		if((nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 0))
		{
			prevstate = 1;
			heartbeatcount++;
		}

		// Falling edge (end of heartbeat)
		if((!nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 1))
		{
			prevstate = 0;
			heartbeatcount++;
		}
	}
}
#endif

#ifdef BOARD_PCA10040
static void TouchTest_touch_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	ret_code_t err_code;

	if(pin == TOUCHPIN)
	{
		// Button is pushed (touch surface is touched)
		if((!nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 1))
		{
			nrf_delay_ms(100); // brute force debouncing
			prevstate = 0;
			TouchTest_detectedLocation();
			app_timer_stop(touchloctimer_id);
		}

		// Button is released (touch surface is released)
		else if((nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 0))
		{
			nrf_delay_ms(100); // brute force debouncing
			prevstate = 1;
			if(location < 4)
			{
				location++;
				TouchTest_updateLocation();
				err_code = app_timer_start(touchloctimer_id, APP_TIMER_TICKS(timeout*1000.0, 0), NULL);
				APP_ERROR_CHECK(err_code);
			}
			else
			{
				app_timer_stop(touchloctimer_id);
				nrf_drv_gpiote_in_uninit(TOUCHPIN);
				location = 0;
				nrf_gpio_pin_write(MODEPIN,0); // Low power mode
				LEDTest_showRangePixelColor(0, 23, 0, 0, 0);
				CommandLineInterface_printLine("Touch location test completed successfully!");
			}
		}
	}
}
#else
static void TouchTest_touch_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	ret_code_t err_code;

	if(pin == TOUCHPIN)
	{
		// Button is pushed (touch surface is touched)
		if((nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 0))
		{
			nrf_delay_ms(100); // brute force debouncing
			prevstate = 1;
			TouchTest_detectedLocation();

			if(timeout > 0)
				app_timer_stop(touchloctimer_id);
		}

		// Button is released (touch surface is released)
		else if((!nrf_drv_gpiote_in_is_set(TOUCHPIN)) && (prevstate == 1))
		{
			nrf_delay_ms(100); // brute force debouncing
			prevstate = 0;
			if(location < 4)
			{
				location++;
				TouchTest_updateLocation();

				if(timeout > 0)
				{
					err_code = app_timer_start(touchloctimer_id, APP_TIMER_TICKS(timeout*1000.0, 0), NULL);
					APP_ERROR_CHECK(err_code);
				}
			}
			else if(timeout == 0)
			{
				location = 1;
				TouchTest_updateLocation();
			}
			else
			{
				location = 0;

				TouchTest_stopTouchLocationTest();

				CommandLineInterface_printLine("Touch location test completed successfully!");
			}
		}
	}
}
#endif

void TouchTest_setup(uint8_t touchPin, uint8_t modePin)
{
	TOUCHPIN = touchPin;
	MODEPIN = modePin;

	nrf_gpio_cfg_output(MODEPIN);
	nrf_gpio_pin_write(MODEPIN,0); // Low power mode

    TouchTest_createtimers();

}

void TouchTest_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         TOUCH");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("thb <time>:\t\tStarts a touch heartbeat test for <time> (s)");
	CommandLineInterface_printLine("starttouchloc <time>:\tStarts a touch location test with timeout <time> (s)");
	CommandLineInterface_printLine("stoptouchloc:\t\tStops a touch location test");
	nrf_delay_ms(50);
}

bool TouchTest_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	int value = 0;
	bool parsed = true;

	if(sscanf(RxBuff, "THB %d", &value)==1)
	{
		if(value < THBDURATIONMIN || value > THBDURATIONMAX)
			CommandLineInterface_printf("Invalid D: must be between %d and %d\r\n", THBDURATIONMIN, THBDURATIONMAX);
		else
		{
			CommandLineInterface_printf("Starting touch heartbeat test for %d seconds\r\n", value);
			TouchTest_startHearbeatTest(value);
		}
	}
	else if(sscanf(RxBuff, "STARTTOUCHLOC %d", &value)==1)
	{
		if(value < TOUCHLOCDURATIONMIN || value > TOUCHLOCDURATIONMAX)
			CommandLineInterface_printf("Invalid D: must be between %d and %d\r\n", TOUCHLOCDURATIONMIN, TOUCHLOCDURATIONMAX);
		else
		{
			CommandLineInterface_printLine("Starting touch location test");
			TouchTest_startTouchLocationTest(value);
		}
	}
	else if(strncmp(RxBuff, "STARTDEMOTOUCH",14)==0)
	{

		CommandLineInterface_printLine("Starting touch demo mode");
		TouchTest_startTouchLocationTest(0);

	}
	else if(strncmp(RxBuff, "STOPDEMOTOUCH",13)==0)
	{

		CommandLineInterface_printLine("Stopping touch demo mode");
		TouchTest_stopTouchLocationTest();

	}
	else if(strncmp(RxBuff, "STOPTOUCHLOC", 12)==0)
	{
		CommandLineInterface_printLine("Stopping touch location test");
		TouchTest_stopTouchLocationTest();
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void TouchTest_startHearbeatTest(int duration_s)
{
	heartbeatcount = 0;
	timeout = duration_s;
	uint32_t err_code;

	// Enable Touch supply
	if(!PowerSupply_getState(SUPPLY_S2_8V))
	{
		PowerSupply_enable(SUPPLY_S2_8V);
		nrf_delay_ms(100);
	}

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
	config.pull = NRF_GPIO_PIN_PULLUP;


	err_code = nrf_drv_gpiote_in_init(TOUCHPIN, &config, TouchTest_heartbeat_event_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(TOUCHPIN,true);

	err_code = app_timer_start(heartbeattimer_id, APP_TIMER_TICKS(timeout*1000.0, 0), NULL);
	APP_ERROR_CHECK(err_code);
}

void TouchTest_startTouchLocationTest(int duration_s)
{
	uint32_t err_code;
	timeout = duration_s;
	nrf_gpio_pin_write(MODEPIN,1); // fast mode

	// Enable Touch supply
	if(!PowerSupply_getState(SUPPLY_S2_8V))
	{
		PowerSupply_enable(SUPPLY_S2_8V);
		nrf_delay_ms(100);
	}

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
	config.pull = NRF_GPIO_PIN_PULLDOWN;
	prevstate = 0;

	err_code = nrf_drv_gpiote_in_init(TOUCHPIN, &config, TouchTest_touch_event_handler);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(TOUCHPIN,true);

	if(timeout > 0)
	{
		err_code = app_timer_start(touchloctimer_id, APP_TIMER_TICKS(timeout*1000.0, 0), NULL);
		APP_ERROR_CHECK(err_code);
	}

	// Wait for a low touch signal (no heartbeat)
	while(nrf_drv_gpiote_in_is_set(TOUCHPIN));

	location = 1;
	TouchTest_updateLocation();
}

void TouchTest_stopTouchLocationTest()
{
	nrf_drv_gpiote_in_uninit(TOUCHPIN);
	nrf_gpio_pin_write(MODEPIN,0); // Low power mode
	LEDTest_showRangePixelColor(0, 23, 0, 0, 0);

	// Disable Touch supply
	if(PowerSupply_getState(SUPPLY_S2_8V))
	{
		PowerSupply_disable(SUPPLY_S2_8V);
		nrf_delay_ms(100);
	}
}

void TouchTest_updateLocation()
{
#ifdef LEDTEST
#ifdef BOARD_PCA10040
	switch(location)
	{
		case 1:
			LEDTest_showRangePixelColor(0, 23, 0, 0, 0);
			LEDTest_showRangePixelColor(9, 13, 0, 0, 128);
			break;
		case 2:
			LEDTest_showRangePixelColor(9, 13, 0, 0, 0);
			LEDTest_showRangePixelColor(15, 19, 0, 0, 128);
			break;
		case 3:
			LEDTest_showRangePixelColor(15, 19, 0, 0, 0);
			LEDTest_showRangePixelColor(21, 23, 0, 0, 128);
			LEDTest_showRangePixelColor(0, 1, 0, 0, 128);
			break;
		case 4:
			LEDTest_showRangePixelColor(21, 23, 0, 0, 0);
			LEDTest_showRangePixelColor(0, 1, 0, 0, 0);
			LEDTest_showRangePixelColor(3, 7, 0, 0, 128);
			break;
		case 0:
		default:
			LEDTest_showRangePixelColor(0, 23, 0, 0, 0);
			break;

	}
#else
	switch(location)
	{
		case 1:
			LEDTest_showRangePixelColor(0, 23, 0, 0, 0);
			LEDTest_showRangePixelColor(10, 14, 0, 0, 128);
			break;
		case 2:
			LEDTest_showRangePixelColor(10, 14, 0, 0, 0);
			LEDTest_showRangePixelColor(16, 20, 0, 0, 128);
			break;
		case 3:
			LEDTest_showRangePixelColor(16, 20, 0, 0, 0);
			LEDTest_showRangePixelColor(22, 23, 0, 0, 128);
			LEDTest_showRangePixelColor(0, 2, 0, 0, 128);
			break;
		case 4:
			LEDTest_showRangePixelColor(22, 23, 0, 0, 0);
			LEDTest_showRangePixelColor(0, 2, 0, 0, 0);
			LEDTest_showRangePixelColor(4, 8, 0, 0, 128);
			break;
		case 0:
		default:
			LEDTest_showRangePixelColor(0, 23, 0, 0, 0);
			break;

	}
#endif
#endif
}

void TouchTest_detectedLocation()
{
#ifdef LEDTEST
#ifdef BOARD_PCA10040
	switch(location)
	{
		case 1:
			LEDTest_showRangePixelColor(9, 13, 0, 128, 0);
			break;
		case 2:
			LEDTest_showRangePixelColor(15, 19, 0, 128, 0);
			break;
		case 3:
			LEDTest_showRangePixelColor(21, 23, 0, 128, 0);
			LEDTest_showRangePixelColor(0, 1, 0, 128, 0);
			break;
		case 4:
			LEDTest_showRangePixelColor(3, 7, 0, 128, 0);
			break;
		case 0:
		default:
			LEDTest_showRangePixelColor(0, 23, 0, 0, 0);
			break;

	}
#else
	switch(location)
	{
		case 1:
			LEDTest_showRangePixelColor(10, 14, 0, 128, 0);
			break;
		case 2:
			LEDTest_showRangePixelColor(16, 20, 0, 128, 0);
			break;
		case 3:
			LEDTest_showRangePixelColor(22, 23, 0, 128, 0);
			LEDTest_showRangePixelColor(0, 2, 0, 128, 0);
			break;
		case 4:
			LEDTest_showRangePixelColor(4, 8, 0, 128, 0);
			break;
		case 0:
		default:
			LEDTest_showRangePixelColor(0, 23, 0, 0, 0);
			break;

	}
#endif
#endif
}

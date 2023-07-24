/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	The TestFirmware is a complete set of command and functionality to test SmartHalo prototypes
 *
 * The TestFirmware is throwable code to perform quick hardware tests on SmartHalo prototypes.
 * It implements a series of hooks to control the prototype circuit via a Serial Application Software
 * in ASCII mode. The complete list of calls available can be listed via the Help command.
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

// Standard

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Nordic SDK

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"

// SmartHalo
#include "SmartHalo.h"
#include "CommandLineInterface.h"
#include "Bluetooth.h"
#include "SoundTest.h"

#ifdef BOARD_PCA10040
	const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;
#endif

static bool usbsleep = false;

void board_setup()
{
	// Set ISET2 H to enable 500 mA charge current
#ifdef USB_CHARGE_ISET2_PIN
	nrf_gpio_cfg_output(USB_CHARGE_ISET2_PIN);
	nrf_gpio_pin_write(USB_CHARGE_ISET2_PIN, 1);
#endif

	// Check USB bridge
	nrf_gpio_cfg_input(USB_SLEEP_N_PIN, NRF_GPIO_PIN_PULLUP);
	usbsleep = !(nrf_gpio_pin_read(USB_SLEEP_N_PIN));

	// Wakeup USB bridge if sleeping
	if( usbsleep )
	{
		nrf_gpio_cfg_output(USB_WAKEUP_N_PIN);
		nrf_gpio_pin_write(USB_WAKEUP_N_PIN, 0);
	}
}

void sdk_init()
{
	uint32_t err_code;

	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{

	uint32_t patate = NRF_POWER->RESETREAS;
	NRF_POWER->RESETREAS = 0x000F000F;


#ifdef BOARD_PCA10040
    // Configure LED-pins as outputs.
    LEDS_CONFIGURE(LEDS_MASK);
#endif

    // Common init
    sdk_init();

    // General Board setup
	board_setup();

    // Configure UART port
    CommandLineInterface_setup();

    CommandLineInterface_printf("Reset reason: 0x%X\r\n",(int)patate);

    while( true )
    {

#ifdef BOARD_PCA10040
		// Toggle LEDs.
		for (int i = 0; i < LEDS_NUMBER; i++)
		{
			LEDS_INVERT(1 << leds_list[i]);
			nrf_delay_ms(100);
		}
#else
		nrf_delay_ms(100); // Slow down processing loop for piezo sound test
#endif

#ifdef SOUNDTEST
		SoundTest_processSound();
#endif

    }
}


/** @} */

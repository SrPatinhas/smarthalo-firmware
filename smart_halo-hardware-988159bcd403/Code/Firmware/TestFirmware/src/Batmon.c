/*
 * Batmon.c
 *
 *  Created on: Jun 3, 2016
 *      Author: sgelinas
 */

#include <stdbool.h>
#include <string.h>

#include "Batmon.h"
#include "CommandLineInterface.h"

#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"

#include "LC709203F.h"
#include "STC3115.h"

static uint8_t ALARMBPIN = 0;
static uint8_t CHGPIN = 0;
static uint8_t PGPIN = 0;
static int alarmprevstate = 1;
static int chgprevstate = 1;
static int pgprevstate = 1;
static unsigned int alarmstatus = 0;
static unsigned int chgstatus = 0;
static unsigned int pgstatus = 0;
static bool STC3115_PRESENT = false;
static bool LC709203F_PRESENT = false;

bool Batmon_checkPresent()
{
	LC709203F_PRESENT = LC709203F_checkPresent();

	if(LC709203F_PRESENT)
	{
		CommandLineInterface_printLine("LC709203F fuel gage detected");
	}
	else
	{
		STC3115_PRESENT = STC3115_checkPresent();

		if(STC3115_PRESENT)
			CommandLineInterface_printLine("STC3115 fuel gage detected");
	}

	if(!LC709203F_PRESENT && !STC3115_PRESENT)
		CommandLineInterface_printLine("No Battery monitor detected");

	return LC709203F_PRESENT || STC3115_PRESENT;
}

void Batmon_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         BATTERY");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("batalarm:\t\tReturns the alarm status (0=off, 1=on)");
	CommandLineInterface_printLine("batalarm=<value>:\tEnables or disables the alarm (0=off, 1=on)");
	CommandLineInterface_printLine("chgstatus:\t\tReturns the charging status (0=off, 1=on)");
	CommandLineInterface_printLine("pgstatus:\t\tReturns the USB connection status (0=disconnected, 1=connected)");
	nrf_delay_ms(50);


	if(LC709203F_PRESENT)
	{
		LC709203F_printHelp();
	}
	else if(STC3115_PRESENT)
	{
		STC3115_printHelp();
	}
}

bool Batmon_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	int value = 0;
	bool parsed = true;

	if(sscanf(RxBuff, "BATALARM=%d", &value)==1)
	{
		if(value < 0 || value > 1)
			CommandLineInterface_printf("Invalid BATALARM: must be between %d and %d\r\n", 0, 1);
		else
		{
			if(value == 0)
				Batmon_disableAlarmB();
			else
				Batmon_enableAlarmB();

			CommandLineInterface_printf("BATALARM=%d\r\n", Batmon_getAlarmStatus());
		}
	}
	else if(strncmp(RxBuff, "BATALARM", 8)==0)
	{
		CommandLineInterface_printf("BATALARM=%d\r\n", Batmon_getAlarmStatus());
	}
	else if(strncmp(RxBuff, "CHGSTATUS", 9)==0)
	{
		CommandLineInterface_printf("CHGSTATUS=%d\r\n", Batmon_getCHGStatus());
	}
	else if(strncmp(RxBuff, "PGSTATUS", 8)==0)
	{
		CommandLineInterface_printf("PGSTATUS=%d\r\n", Batmon_getPGStatus());
	}
	else
	{
		if(LC709203F_PRESENT)
		{
			parsed = LC709203F_parseAndExecuteCommand(RxBuff, cnt);
		}
		else if(STC3115_PRESENT)
		{
			parsed = STC3115_parseAndExecuteCommand(RxBuff, cnt);
		}
		else
		{
			parsed = false;
		}
	}

	return parsed;
}

static void Batmon_gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	CommandLineInterface_printLine("In GPIOTE event handler");

	if(pin == ALARMBPIN)
	{
		// Button is pushed (touch surface is touched)
		if((!nrf_drv_gpiote_in_is_set(ALARMBPIN)) && (alarmprevstate == 1))
		{
			nrf_delay_ms(100); // brute force debouncing
			alarmprevstate = 0;
			alarmstatus = 1;

			CommandLineInterface_printLine("ALARM: Low battery!");
		}

		// Button is released (touch surface is released)
		else if((nrf_drv_gpiote_in_is_set(ALARMBPIN)) && (alarmprevstate == 0))
		{
			nrf_delay_ms(100); // brute force debouncing
			alarmprevstate = 1;
			alarmstatus = 0;

			CommandLineInterface_printLine("ALARM: battery is ok.");
		}
	}
	else if(pin == CHGPIN)
	{
		// Button is pushed (touch surface is touched)
		if((!nrf_drv_gpiote_in_is_set(CHGPIN)) && (chgprevstate == 1))
		{
			nrf_delay_ms(100); // brute force debouncing
			chgprevstate = 0;
			chgstatus = 1;

			CommandLineInterface_printLine("CHG: Battery is charging.");
		}

		// Button is released (touch surface is released)
		else if((nrf_drv_gpiote_in_is_set(CHGPIN)) && (chgprevstate == 0))
		{
			nrf_delay_ms(100); // brute force debouncing
			chgprevstate = 1;
			chgstatus = 0;

			CommandLineInterface_printLine("CHG: Battery is charged.");
		}
	}
	else if(pin == PGPIN)
	{
		// Button is pushed (touch surface is touched)
		if((!nrf_drv_gpiote_in_is_set(PGPIN)) && (pgprevstate == 1))
		{
			nrf_delay_ms(100); // brute force debouncing
			pgprevstate = 0;
			pgstatus = 1;

			CommandLineInterface_printLine("PG: USB is connected.");
		}

		// Button is released (touch surface is released)
		else if((nrf_drv_gpiote_in_is_set(PGPIN)) && (pgprevstate == 0))
		{
			nrf_delay_ms(100); // brute force debouncing
			pgprevstate = 1;
			pgstatus = 0;

			CommandLineInterface_printLine("PG: USB is disconnected");
		}
	}
}

void Batmon_setup(uint8_t alarmbPin, uint8_t chgPin, uint8_t pgPin)
{
	Batmon_checkPresent();

	if(LC709203F_PRESENT)
	{
		LC709203F_setup();
	}
	else if(STC3115_PRESENT)
	{
		STC3115_setup();
	}

	// Set up alarm pin
	ALARMBPIN = alarmbPin;
  	Batmon_enableAlarmB();

  	// Set up alarm pin
  	CHGPIN = chgPin;
	Batmon_enableCHGInterrupt();

  	// Set up alarm pin
	PGPIN = pgPin;
	Batmon_enablePGInterrupt();
}

void Batmon_enableAlarmB()
{
	uint32_t err_code;

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
	config.pull = NRF_GPIO_PIN_PULLUP;


	err_code = nrf_drv_gpiote_in_init(ALARMBPIN, &config, Batmon_gpiote_event_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(ALARMBPIN,true);

	// If we just reset from a USB connection, let's read the ALARM pin for status
	alarmstatus = nrf_drv_gpiote_in_is_set(ALARMBPIN)?0:1;
	alarmprevstate = alarmstatus;
}

void Batmon_disableAlarmB()
{
	nrf_drv_gpiote_in_uninit(ALARMBPIN);

	alarmstatus = 0;
	alarmprevstate = 0;
}

unsigned int Batmon_getAlarmStatus()
{
	return alarmstatus;
}

void Batmon_enableCHGInterrupt()
{
	uint32_t err_code;

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
	config.pull = NRF_GPIO_PIN_PULLUP;


	err_code = nrf_drv_gpiote_in_init(CHGPIN, &config, Batmon_gpiote_event_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(CHGPIN,true);

	// If we just reset from a USB connection, let's read the CHG pin for status
	chgstatus = nrf_drv_gpiote_in_is_set(CHGPIN)?0:1;
	chgprevstate = chgstatus;
}

void Batmon_disableCHGInterrupt()
{
	nrf_drv_gpiote_in_uninit(CHGPIN);

	chgstatus = 0;
}

unsigned int Batmon_getCHGStatus()
{
	return chgstatus;
}

void Batmon_enablePGInterrupt()
{
	uint32_t err_code;

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
	config.pull = NRF_GPIO_PIN_PULLUP;


	err_code = nrf_drv_gpiote_in_init(PGPIN, &config, Batmon_gpiote_event_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(PGPIN,true);

	// If we just reset from a USB connection, let's read the PG pin for status
	pgstatus = nrf_drv_gpiote_in_is_set(PGPIN)?0:1;
	pgprevstate = pgstatus;
}

void Batmon_disablePGInterrupt()
{
	nrf_drv_gpiote_in_uninit(PGPIN);

	pgstatus = 0;
}

unsigned int Batmon_getPGStatus()
{
	return pgstatus;
}

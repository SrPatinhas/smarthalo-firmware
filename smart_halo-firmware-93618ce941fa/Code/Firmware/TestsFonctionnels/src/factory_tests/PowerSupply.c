/*
 * PowerSupply.c
 *
 *  Created on: 2016-06-22
 *      Author: Seb
 */

#include "PowerSupply.h"
#include "CommandLineInterface.h"

#include "pinmap.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"

#include <stdio.h>
#include <string.h>

static bool VLED_ON = false;
static bool VPIEZO_ON = false;
static bool S2_8V_ON = false;

void PowerSupply_disableAll();
void PowerSupply_enableAll();

void PowerSupply_printState(PowerSupplyType supply);
PowerSupplyType PowerSupply_getSupplyFromStr(char * SupplyStr, int length);
char * PowerSupply_getSupplyStr(PowerSupplyType supply);

void PowerSupply_setup()
{
	// Configure Supply enable pins
	nrf_gpio_cfg_output(EN_VLED);
	nrf_gpio_cfg_output(EN_2_8V);
	nrf_gpio_cfg_output(EN_VPIEZO);

	PowerSupply_disableAll();
}

void PowerSupply_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         POWERSUPPLY");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("supply <name>:\t\tReturns the state of supply (VLED, VPIEZO, S2_8V, ALL)");
	CommandLineInterface_printLine("supplyon <name>:\tTurns on the supply");
	CommandLineInterface_printLine("supplyoff <name>:\tTurns off the supply");
	nrf_delay_ms(50);
}

bool PowerSupply_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	bool parsed = true;
	char RxBuff2[COMMAND_MAX_LENGTH];
	int length;

	if((cnt>=15) && (sscanf(RxBuff,"SUPPLYOFF %s%n",RxBuff2,&length)==1))
	{
		PowerSupplyType supply = PowerSupply_getSupplyFromStr(RxBuff2, length-10);

		if(supply==SUPPLY_ALL)
			PowerSupply_disableAll();
		else
			PowerSupply_disable(supply);

		PowerSupply_printState(supply);
	}
	else if((cnt>=14) && (sscanf(RxBuff,"SUPPLYON %s%n",RxBuff2,&length)==1))
	{
		PowerSupplyType supply = PowerSupply_getSupplyFromStr(RxBuff2, length-9);

		if(supply==SUPPLY_ALL)
			PowerSupply_enableAll();
		else
			PowerSupply_enable(supply);

		PowerSupply_printState(supply);
	}

	else if((sscanf(RxBuff,"SUPPLY %s%n",RxBuff2,&length)==1))
	{
		PowerSupplyType supply = PowerSupply_getSupplyFromStr(RxBuff2, length-7);

		PowerSupply_printState(supply);
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void PowerSupply_disableAll()
{
	for( PowerSupplyType supply=INVALID_SUPPLY; supply < SUPPLY_S2_8V; supply++)
		{
			PowerSupply_disable((PowerSupplyType)(supply+1));
		}
}

void PowerSupply_enableAll()
{
	for( PowerSupplyType supply=INVALID_SUPPLY; supply < SUPPLY_S2_8V; supply++)
	{
		PowerSupply_enable((PowerSupplyType)(supply+1));
	}
}

void PowerSupply_enable(PowerSupplyType supply)
{
	switch (supply)
	{
		case SUPPLY_VLED:
			VLED_ON = true;
			nrf_gpio_pin_write(EN_VLED, 1);
			break;
		case SUPPLY_VPIEZO:
			VPIEZO_ON = true;
			nrf_gpio_pin_write(EN_VPIEZO, 1);
			break;
		case SUPPLY_S2_8V:
			S2_8V_ON = true;
			nrf_gpio_pin_write(EN_2_8V, 1);
			break;
		case INVALID_SUPPLY:
		default:
			break;
	}
}

void PowerSupply_disable(PowerSupplyType supply)
{
	switch (supply)
	{
		case SUPPLY_VLED:
			VLED_ON = false;
			nrf_gpio_pin_write(EN_VLED, 0);
			break;
		case SUPPLY_VPIEZO:
			VPIEZO_ON = false;
			nrf_gpio_pin_write(EN_VPIEZO, 0);
			break;
		case SUPPLY_S2_8V:
			S2_8V_ON = false;
			nrf_gpio_pin_write(EN_2_8V, 0);
			break;
		case INVALID_SUPPLY:
		default:
			break;
	}
}

bool PowerSupply_getState(PowerSupplyType supply)
{
	switch (supply)
	{
		case SUPPLY_VLED:
			return VLED_ON;
			break;
		case SUPPLY_VPIEZO:
			return VPIEZO_ON;
			break;
		case SUPPLY_S2_8V:
			return S2_8V_ON;
			break;
		case INVALID_SUPPLY:
		default:
			return false;
			break;
	}
}

void PowerSupply_printState(PowerSupplyType supply)
{
	if(supply != SUPPLY_ALL)
	{
		PowerSupply_getState(supply)?CommandLineInterface_printf("%s ON\r\n",PowerSupply_getSupplyStr(supply)):CommandLineInterface_printf("%s OFF\r\n",PowerSupply_getSupplyStr(supply));
	}
	else
	{
		for( PowerSupplyType lSupply = INVALID_SUPPLY; lSupply < SUPPLY_S2_8V; lSupply++)
		{
			PowerSupply_getState(lSupply+1)?CommandLineInterface_printf("%s ON\r\n",PowerSupply_getSupplyStr(lSupply+1)):CommandLineInterface_printf("%s OFF\r\n",PowerSupply_getSupplyStr(lSupply+1));
		}
	}
}

char * PowerSupply_getSupplyStr(PowerSupplyType supply)
{
	switch(supply)
	{
		case SUPPLY_VLED:
			return "VLED";
			break;
		case SUPPLY_VPIEZO:
			return "VPIEZO";
			break;
		case SUPPLY_S2_8V: // Always active
			return "S2_8V";
			break;
		case SUPPLY_ALL:
			return "ALL";
			break;
		case INVALID_SUPPLY:
		default:
			return "";
			break;
	};
}

PowerSupplyType PowerSupply_getSupplyFromStr(char * SupplyStr, int length)
{
	if(strncmp(PowerSupply_getSupplyStr(SUPPLY_VLED),SupplyStr,length) == 0)
	{
		return SUPPLY_VLED;
	}
	else if(strncmp(PowerSupply_getSupplyStr(SUPPLY_VPIEZO),SupplyStr,length) == 0)
	{
		return SUPPLY_VPIEZO;
	}
	else if(strncmp(PowerSupply_getSupplyStr(SUPPLY_S2_8V),SupplyStr,length) == 0)
	{
		return SUPPLY_S2_8V;
	}
	else if(strncmp(PowerSupply_getSupplyStr(SUPPLY_ALL),SupplyStr,length) == 0)
	{
		return SUPPLY_ALL;
	}
	else
		return INVALIDMODULE;
}

/*
 * Batmon.c
 *
 *  Created on: Jun 3, 2016
 *      Author: sgelinas
 */

#include <stdbool.h>
#include <string.h>

#include "I2C.h"
#include "CommandLineInterface.h"

#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "Batmon.h"

#define TEMPMIN       	-19.95
#define TEMPMAX       	60.05
#define APAMIN			0x0000
#define APAMAX			0x00FF
#define APTMIN			0x0000
#define APTMAX			0xFFFF
#define BATPROFILEMIN	0x0000
#define BATPROFILEMAX	0x0001
#define ALARMRSOCMIN	0x0000
#define ALARMRSOCMAX	0x0064
#define ALARMCELLMIN	0.0
#define ALARMCELLMAX	65.535
#define POWERMODEMIN	0x0001
#define POWERMODEMAX	0x0002
#define STATUSBITMIN	0x0000
#define STATUSBITMAX	0x0001

#define LC709203F_ADDR         ((0x16 >> 1) & (0x7F))

static uint8_t ALARMBPIN = 0;
static int prevstate = 1;
static unsigned int alarmstatus = 0;
static bool LC709203F_PRESENT = false;

uint8_t crc8_ccitt_update (uint8_t inCrc, uint8_t inData)
{
    uint8_t   i;
    uint8_t   data;

    data = inCrc ^ inData;

    for ( i = 0; i < 8; i++ )
    {
        if (( data & 0x80 ) != 0 )
        {
            data <<= 1;
            data ^= 0x07;
        }
        else
        {
            data <<= 1;
        }
    }
    return data;
}

uint8_t computeCRC(uint8_t *msg, int length)
{
	uint8_t crc = 0, i;

	for (i = 0; i < length; i++)
		crc = crc8_ccitt_update(crc, msg[i]);

	return crc;
}

void Batmon_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         BATTERY");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("batrsoc:\t\tReturns the battery relative state of charge (%)");
	CommandLineInterface_printLine("batite:\t\tReturns the indicator to empty (%)");
	CommandLineInterface_printLine("batcellv:\t\tReturns the cell voltage (mV)");
	CommandLineInterface_printLine("batcellt:\t\tReturns the cell temperature (C)");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("batcellt=<value>:\tSets the cell temperature (C)");
	CommandLineInterface_printLine("batapa:\t\tReturns the adjustment pack application");
	CommandLineInterface_printLine("batapa=<value>:\tSets the adjustment pack application");
	CommandLineInterface_printLine("batapt:\t\tReturns the adjustment pack thermistor");
	CommandLineInterface_printLine("batapt=<value>:\tSets the adjustment pack thermistor");
	CommandLineInterface_printLine("batver:\t\tReturns the battery monitor version");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("batalarm:\t\tReturns the alarm status (0=off, 1=on)");
	CommandLineInterface_printLine("batalarm=<value>:\tEnables or disables the alarm (0=off, 1=on)");
	CommandLineInterface_printLine("batlowrscoc:\t\tReturns the alarmb low RSOC threshold (%)");
	CommandLineInterface_printLine("Bbatlowrsoc=<value>:\tSets the alarmb low RSOC threshold (%)");
	CommandLineInterface_printLine("batlowcellv:\t\tReturns the alarmb low cell voltage (mV)");
	CommandLineInterface_printLine("batlowcellv=<value>:\tSets the alarmb low cell voltage (mV)");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("batpower:\t\tReturns the power mode");
	CommandLineInterface_printLine("batpower=<value>:\tSets the power mode");
	CommandLineInterface_printLine("batstatb:\t\tReturns the status bit");
	CommandLineInterface_printLine("batstatb=<value>:\tSets the status bit");
	nrf_delay_ms(50);
}

bool Batmon_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	int value = 0;
	float valuef = 0;
	bool parsed = true;

	if(sscanf(RxBuff, "BATCELLT=%f", &valuef)==1)
	{
		 if(valuef < TEMPMIN || valuef > TEMPMAX)
			 CommandLineInterface_printf("Invalid BATCELLT: must be between %.2fC and %.2fC\r\n", TEMPMIN, TEMPMAX);
		else
		{
			Batmon_setCellTemperature(valuef);
			CommandLineInterface_printf("BATCELLT=%.2fC\r\n", Batmon_getCellTemperature());
		}
	}
	else if(sscanf(RxBuff, "BATAPA=%d", &value)==1)
	{
		if(value < APAMIN || value > APAMAX)
			CommandLineInterface_printf("Invalid BATAPA: must be between %d and %d\r\n", APAMIN, APAMAX);
		else
		{
			Batmon_setAPA(value);
			CommandLineInterface_printf("BATAPA=%d\r\n", Batmon_getAPA());
		}
	}
	else if(sscanf(RxBuff, "BATAPT=%d", &value)==1)
	{
		if(value < APTMIN || value > APTMAX)
			CommandLineInterface_printf("Invalid BATAPT: must be between %d and %d\r\n", APTMIN, APTMAX);
		else
		{
			Batmon_setAPT(value);
			CommandLineInterface_printf("BATAPT=%d\r\n", Batmon_getAPT());
		}
	}
	else if(sscanf(RxBuff, "BATLOWRSOC=%d", &value)==1)
	{
		if(value < ALARMRSOCMIN || value > ALARMRSOCMAX)
			CommandLineInterface_printf("Invalid BATLOWRSOC: must be between %d%% and %d%%\r\n", ALARMRSOCMIN, ALARMRSOCMAX);
		else
		{
			Batmon_setAlarmLowRSOC(value);
			CommandLineInterface_printf("BATLOWRSOC=%d%%\r\n", Batmon_getAlarmLowRSOC());
		}
	}
	else if(sscanf(RxBuff, "BATLOWCELLV=%d", &value)==1)
	{
		if(value < ALARMCELLMIN || value > ALARMCELLMAX)
			CommandLineInterface_printf("Invalid BATLOWCELLV: must be between %.1fV and %.1fV\r\n", ALARMCELLMIN, ALARMCELLMAX);
		else
		{
			Batmon_setAlarmLowCellVoltage(value);
			CommandLineInterface_printf("BATLOWCELLV=%.1fV\r\n", Batmon_getAlarmLowCellVoltage());
		}
	}
	else if(sscanf(RxBuff, "BATPOWER=%d", &value)==1)
	{
		if(value < POWERMODEMIN || value > POWERMODEMAX)
			CommandLineInterface_printf("Invalid BATPOWER: must be between %d and %d\r\n", POWERMODEMIN, POWERMODEMAX);
		else
		{
			Batmon_setPowerMode(value);
			CommandLineInterface_printf("BATPOWER=%d\r\n", Batmon_getPowerMode());
		}
	}
	else if(sscanf(RxBuff, "BATSTATB=%d", &value)==1)
	{
		if(value < STATUSBITMIN || value > STATUSBITMAX)
			CommandLineInterface_printf("Invalid BATSTATB: must be between %d and %d\r\n", STATUSBITMIN, STATUSBITMAX);
		else
		{
			Batmon_setStatusBit(value);
			CommandLineInterface_printf("BATSTATB=%d\r\n", Batmon_getStatusBit());
		}
	}
	else if(sscanf(RxBuff, "BATALARM=%d", &value)==1)
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
	else if(strncmp(RxBuff, "BATRSOC", 7)==0)
	{
		CommandLineInterface_printf("BATRSOC=%d%%\r\n", Batmon_getRSOC());
	}
	else if(strncmp(RxBuff, "BATITE", 6)==0)
	{
		CommandLineInterface_printf("BATITE=%.1f%%\r\n", Batmon_getITE());
	}
	else if(strncmp(RxBuff, "BATCELLV", 8)==0)
	{
		CommandLineInterface_printf("BATCELLV=%.3fV\r\n", Batmon_getCellVoltage());
	}
	else if(strncmp(RxBuff, "BATCELLT", 8)==0)
	{
		CommandLineInterface_printf("BATCELLT=%.2fC\r\n", Batmon_getCellTemperature());
	}
	else if(strncmp(RxBuff, "BATAPA", 6)==0)
	{
		CommandLineInterface_printf("BATAPA=%d\r\n", Batmon_getAPA());
	}
	else if(strncmp(RxBuff, "BATAPT", 6)==0)
	{
		CommandLineInterface_printf("BATAPT=%d\r\n", Batmon_getAPT());
	}
	else if(strncmp(RxBuff, "BATVER", 6)==0)
	{
		CommandLineInterface_printf("BATVER=%d\r\n", Batmon_getICVersion());
	}
	else if(strncmp(RxBuff, "BATLOWRSOC", 10)==0)
	{
		CommandLineInterface_printf("BATLOWRSOC=%d%%\r\n", Batmon_getAlarmLowRSOC());
	}
	else if(strncmp(RxBuff, "BATLOWCELLV", 11)==0)
	{
		CommandLineInterface_printf("BATLOWCELLV=%.3fV\r\n", Batmon_getAlarmLowCellVoltage());
	}
	else if(strncmp(RxBuff, "BATPOWER", 8)==0)
	{
		CommandLineInterface_printf("BATPOWER=%d\r\n", Batmon_getPowerMode());
	}
	else if(strncmp(RxBuff, "BATSTATB", 8)==0)
	{
		CommandLineInterface_printf("BATSTATB=%d\r\n", Batmon_getStatusBit());
	}
	else if(strncmp(RxBuff, "BATALARM", 8)==0)
	{
		CommandLineInterface_printf("BATALARM=%d\r\n", Batmon_getAlarmStatus());
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

bool Batmon_checkPresent()
{
	ret_code_t err_code;
	uint8_t data = 	IC_VERSION_REG;
	uint8_t regdata[2];

	if(I2C_twi_tx(LC709203F_ADDR, &data, 1, true) == NRF_SUCCESS)
	{
		LC709203F_PRESENT = true;

		err_code = I2C_twi_rx(LC709203F_ADDR, regdata, 2);
		APP_ERROR_CHECK(err_code);
		return true;
	}
	else
	{
		CommandLineInterface_printLine("Battery monitor not detected");
		return false;
	}
}

void Batmon_writeReg(BatMonRegisters reg, unsigned int data)
{
	ret_code_t err_code;
	uint8_t byteH = ((data >> 8) & 0xFF);
	uint8_t byteL = (data & 0xFF);
	uint8_t devaddress = LC709203F_ADDR<<1;

	if(LC709203F_PRESENT)
	{
		// Cannot use the standard 16 bit write function...needs to write a CRC
		uint8_t CRCdata[4] = {devaddress,reg,byteL,byteH};
		uint8_t CRC8 = computeCRC(CRCdata, 4);

		uint8_t regdata[4] = {reg,byteL,byteH,CRC8};

		err_code = I2C_twi_tx(LC709203F_ADDR, regdata, 4, false);
		APP_ERROR_CHECK(err_code);
	}
}

unsigned int Batmon_readReg(BatMonRegisters reg)
{
	if(LC709203F_PRESENT)
		return readReg16(LC709203F_ADDR, reg);
	else
		return -1;
}

static void Batmon_alarmb_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(pin == ALARMBPIN)
	{
		// Button is pushed (touch surface is touched)
		if((!nrf_drv_gpiote_in_is_set(ALARMBPIN)) && (prevstate == 1))
		{
			nrf_delay_ms(100); // brute force debouncing
			prevstate = 0;
			alarmstatus = 1;

			CommandLineInterface_printLine("ALARM: Low battery!");
		}

		// Button is released (touch surface is released)
		else if((nrf_drv_gpiote_in_is_set(ALARMBPIN)) && (prevstate == 0))
		{
			nrf_delay_ms(100); // brute force debouncing
			prevstate = 1;
			alarmstatus = 0;

			CommandLineInterface_printLine("ALARM: battery is ok.");
		}
	}
}

void Batmon_setup(uint8_t alarmbPin)
{
	Batmon_checkPresent();

	// Set fuel gage to normal operation mode
	Batmon_setPowerMode(1);

	// Set battery capacity to 2000 mAh
	Batmon_setAPA(0x2D);

	// Set battery profile to 3.7V/4.2V
	Batmon_setBatteryProfile(1);

	// Set thermistor mode provided by host over I2C
	Batmon_setStatusBit(1);

	// Set cell temperature to 25C
	Batmon_setCellTemperature(25.0);

	// Set up alarm pin
	ALARMBPIN = alarmbPin;
	Batmon_enableAlarmB();
}

void Batmon_enableAlarmB()
{
	uint32_t err_code;

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
#ifdef SMARTHALO_EE
	config.pull = NRF_GPIO_PIN_NOPULL;

#elif defined BOARD_PCA10040
	config.pull = NRF_GPIO_PIN_PULLUP;
#endif

	err_code = nrf_drv_gpiote_in_init(ALARMBPIN, &config, Batmon_alarmb_event_handler);
	APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_in_event_enable(ALARMBPIN,true);

	alarmstatus = 0;
}

void Batmon_disableAlarmB()
{
	nrf_drv_gpiote_in_uninit(ALARMBPIN);

	alarmstatus = 0;
}

unsigned int Batmon_getAlarmStatus()
{
	return alarmstatus;
}

float Batmon_getCellVoltage()
{
	return (float)Batmon_readReg(CELL_VOLT_REG)/1000.0;
}

unsigned int Batmon_getRSOC()
{
	return Batmon_readReg(RSOC_REG);
}

float Batmon_getITE()
{
	return (float)Batmon_readReg(ITE_REG)/10.0;
}

unsigned int Batmon_getICVersion()
{
	return Batmon_readReg(IC_VERSION_REG);
}

float Batmon_getCellTemperature()
{
	return ((float)Batmon_readReg(CELL_TEMP_REG)/10.0)-273.15;
}

void Batmon_setCellTemperature(float cellT)
{
	unsigned int cellT_int = (10.0*(cellT + 273.15)); // 0x0BA6 = 25.0C
	Batmon_writeReg(CELL_TEMP_REG, cellT_int);
}

unsigned int Batmon_getAlarmLowRSOC()
{
	return Batmon_readReg(ALARMLOWRSOC_REG);
}

void Batmon_setAlarmLowRSOC(unsigned int alarmLowRSOC)
{
	Batmon_writeReg(ALARMLOWRSOC_REG, alarmLowRSOC);
}

float Batmon_getAlarmLowCellVoltage()
{
	return (float)Batmon_readReg(ALARMLOWCELL_REG)/1000.0;
}

void Batmon_setAlarmLowCellVoltage(float alarmLowCellVoltage)
{
	Batmon_writeReg(ALARMLOWCELL_REG, alarmLowCellVoltage);
}

unsigned int Batmon_getAPA()
{
	return Batmon_readReg(APA_REG);
}

void Batmon_setAPA(unsigned int APA)
{
	Batmon_writeReg(APA_REG, APA);
}

unsigned int Batmon_getAPT()
{
	return Batmon_readReg(APT_REG);
}

void Batmon_setAPT(unsigned int APT)
{
	Batmon_writeReg(APT_REG, APT);
}

unsigned int Batmon_getPowerMode()
{
	return Batmon_readReg(ICPOWERMODE_REG);
}

void Batmon_setPowerMode(unsigned int powermode)
{
	Batmon_writeReg(ICPOWERMODE_REG, powermode);
}

unsigned int Batmon_getStatusBit()
{
	return Batmon_readReg(STATUSBIT_REG);
}

void Batmon_setStatusBit(unsigned int statusbit)
{
	Batmon_writeReg(STATUSBIT_REG, statusbit);
}

unsigned int Batmon_getBatteryProfile()
{
	return Batmon_readReg(BAT_PROFILE_R_REG);
}

void Batmon_setBatteryProfile(unsigned int batteryprofile)
{
	Batmon_writeReg(BAT_PROFILE_RW_REG, batteryprofile);
}

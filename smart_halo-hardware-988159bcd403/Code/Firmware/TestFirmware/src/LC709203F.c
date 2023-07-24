/*
 * LC709203F.c
 *
 *  Created on: Nov 15, 2016
 *      Author: sgelinas
 */

#include "LC709203F.h"
#include "I2C.h"
#include "CommandLineInterface.h"
#include "app_error.h"
#include "nrf_delay.h"

#include <string.h>
#include <stdbool.h>

#define LC709203F_ADDR         ((0x16 >> 1) & (0x7F))

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

uint8_t LC709203F_crc8_ccitt_update (uint8_t inCrc, uint8_t inData)
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

uint8_t LC709203F_computeCRC(uint8_t *msg, int length)
{
	uint8_t crc = 0, i;

	for (i = 0; i < length; i++)
		crc = LC709203F_crc8_ccitt_update(crc, msg[i]);

	return crc;
}

void LC709203F_setup()
{
	// Set fuel gage to normal operation mode
	LC709203F_setPowerMode(1);

	// Set battery capacity to 2000 mAh
	LC709203F_setAPA(0x2D);

	// Set battery profile to 3.7V/4.2V
	LC709203F_setBatteryProfile(1);

	// Set thermistor mode provided by host over I2C
	LC709203F_setStatusBit(1);

	// Set cell temperature to 25C
	LC709203F_setCellTemperature(25.0);
}

bool LC709203F_checkPresent()
{
	ret_code_t err_code;
	uint8_t data = IC_VERSION_REG;
	uint8_t regdata[2];

	if(I2C_twi_tx(LC709203F_ADDR, &data, 1, true) == NRF_SUCCESS)
	{
		err_code = I2C_twi_rx(LC709203F_ADDR, regdata, 2);
		APP_ERROR_CHECK(err_code);
		return true;
	}
	else
	{
		return false;
	}
}

void LC709203F_printHelp()
{
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

bool LC709203F_parseAndExecuteCommand(char * RxBuff, int cnt)
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
			LC709203F_setCellTemperature(valuef);
			CommandLineInterface_printf("BATCELLT=%.2fC\r\n", LC709203F_getCellTemperature());
		}
	}
	else if(sscanf(RxBuff, "BATAPA=%d", &value)==1)
	{
		if(value < APAMIN || value > APAMAX)
			CommandLineInterface_printf("Invalid BATAPA: must be between %d and %d\r\n", APAMIN, APAMAX);
		else
		{
			LC709203F_setAPA(value);
			CommandLineInterface_printf("BATAPA=%d\r\n", LC709203F_getAPA());
		}
	}
	else if(sscanf(RxBuff, "BATAPT=%d", &value)==1)
	{
		if(value < APTMIN || value > APTMAX)
			CommandLineInterface_printf("Invalid BATAPT: must be between %d and %d\r\n", APTMIN, APTMAX);
		else
		{
			LC709203F_setAPT(value);
			CommandLineInterface_printf("BATAPT=%d\r\n", LC709203F_getAPT());
		}
	}
	else if(sscanf(RxBuff, "BATLOWRSOC=%d", &value)==1)
	{
		if(value < ALARMRSOCMIN || value > ALARMRSOCMAX)
			CommandLineInterface_printf("Invalid BATLOWRSOC: must be between %d%% and %d%%\r\n", ALARMRSOCMIN, ALARMRSOCMAX);
		else
		{
			LC709203F_setAlarmLowRSOC(value);
			CommandLineInterface_printf("BATLOWRSOC=%d%%\r\n", LC709203F_getAlarmLowRSOC());
		}
	}
	else if(sscanf(RxBuff, "BATLOWCELLV=%d", &value)==1)
	{
		if(value < ALARMCELLMIN || value > ALARMCELLMAX)
			CommandLineInterface_printf("Invalid BATLOWCELLV: must be between %.1fV and %.1fV\r\n", ALARMCELLMIN, ALARMCELLMAX);
		else
		{
			LC709203F_setAlarmLowCellVoltage(value);
			CommandLineInterface_printf("BATLOWCELLV=%.1fV\r\n", LC709203F_getAlarmLowCellVoltage());
		}
	}
	else if(sscanf(RxBuff, "BATPOWER=%d", &value)==1)
	{
		if(value < POWERMODEMIN || value > POWERMODEMAX)
			CommandLineInterface_printf("Invalid BATPOWER: must be between %d and %d\r\n", POWERMODEMIN, POWERMODEMAX);
		else
		{
			LC709203F_setPowerMode(value);
			CommandLineInterface_printf("BATPOWER=%d\r\n", LC709203F_getPowerMode());
		}
	}
	else if(sscanf(RxBuff, "BATSTATB=%d", &value)==1)
	{
		if(value < STATUSBITMIN || value > STATUSBITMAX)
			CommandLineInterface_printf("Invalid BATSTATB: must be between %d and %d\r\n", STATUSBITMIN, STATUSBITMAX);
		else
		{
			LC709203F_setStatusBit(value);
			CommandLineInterface_printf("BATSTATB=%d\r\n", LC709203F_getStatusBit());
		}
	}
	else if(strncmp(RxBuff, "BATRSOC", 7)==0)
	{
		CommandLineInterface_printf("BATRSOC=%d%%\r\n", LC709203F_getRSOC());
	}
	else if(strncmp(RxBuff, "BATITE", 6)==0)
	{
		CommandLineInterface_printf("BATITE=%.1f%%\r\n", LC709203F_getITE());
	}
	else if(strncmp(RxBuff, "BATCELLV", 8)==0)
	{
		CommandLineInterface_printf("BATCELLV=%.3fV\r\n", LC709203F_getCellVoltage());
	}
	else if(strncmp(RxBuff, "BATCELLT", 8)==0)
	{
		CommandLineInterface_printf("BATCELLT=%.2fC\r\n", LC709203F_getCellTemperature());
	}
	else if(strncmp(RxBuff, "BATAPA", 6)==0)
	{
		CommandLineInterface_printf("BATAPA=%d\r\n", LC709203F_getAPA());
	}
	else if(strncmp(RxBuff, "BATAPT", 6)==0)
	{
		CommandLineInterface_printf("BATAPT=%d\r\n", LC709203F_getAPT());
	}
	else if(strncmp(RxBuff, "BATVER", 6)==0)
	{
		CommandLineInterface_printf("BATVER=%d\r\n", LC709203F_getICVersion());
	}
	else if(strncmp(RxBuff, "BATLOWRSOC", 10)==0)
	{
		CommandLineInterface_printf("BATLOWRSOC=%d%%\r\n", LC709203F_getAlarmLowRSOC());
	}
	else if(strncmp(RxBuff, "BATLOWCELLV", 11)==0)
	{
		CommandLineInterface_printf("BATLOWCELLV=%.3fV\r\n", LC709203F_getAlarmLowCellVoltage());
	}
	else if(strncmp(RxBuff, "BATPOWER", 8)==0)
	{
		CommandLineInterface_printf("BATPOWER=%d\r\n", LC709203F_getPowerMode());
	}
	else if(strncmp(RxBuff, "BATSTATB", 8)==0)
	{
		CommandLineInterface_printf("BATSTATB=%d\r\n", LC709203F_getStatusBit());
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void LC709203F_writeReg(LC709203F_Registers reg, unsigned int data)
{
	ret_code_t err_code;
	uint8_t byteH = ((data >> 8) & 0xFF);
	uint8_t byteL = (data & 0xFF);
	uint8_t devaddress = LC709203F_ADDR<<1;

	// Cannot use the standard 16 bit write function...needs to write a CRC
	uint8_t CRCdata[4] = {devaddress,reg,byteL,byteH};
	uint8_t CRC8 = LC709203F_computeCRC(CRCdata, 4);

	uint8_t regdata[4] = {reg,byteL,byteH,CRC8};

	err_code = I2C_twi_tx(LC709203F_ADDR, regdata, 4, false);
	APP_ERROR_CHECK(err_code);
}

unsigned int LC709203F_readReg(LC709203F_Registers reg)
{
	return readReg16(LC709203F_ADDR, reg);
}

float LC709203F_getCellVoltage()
{
	return (float)LC709203F_readReg(CELL_VOLT_REG)/1000.0;
}

unsigned int LC709203F_getRSOC()
{
	return LC709203F_readReg(RSOC_REG);
}

float LC709203F_getITE()
{
	return (float)LC709203F_readReg(ITE_REG)/10.0;
}

unsigned int LC709203F_getICVersion()
{
	return LC709203F_readReg(IC_VERSION_REG);
}

float LC709203F_getCellTemperature()
{
	return ((float)LC709203F_readReg(CELL_TEMP_REG)/10.0)-273.15;
}

void LC709203F_setCellTemperature(float cellT)
{
	unsigned int cellT_int = (10.0*(cellT + 273.15)); // 0x0BA6 = 25.0C
	LC709203F_writeReg(CELL_TEMP_REG, cellT_int);
}

unsigned int LC709203F_getAlarmLowRSOC()
{
	return LC709203F_readReg(ALARMLOWRSOC_REG);
}

void LC709203F_setAlarmLowRSOC(unsigned int alarmLowRSOC)
{
	LC709203F_writeReg(ALARMLOWRSOC_REG, alarmLowRSOC);
}

float LC709203F_getAlarmLowCellVoltage()
{
	return (float)LC709203F_readReg(ALARMLOWCELL_REG)/1000.0;
}

void LC709203F_setAlarmLowCellVoltage(float alarmLowCellVoltage)
{
	LC709203F_writeReg(ALARMLOWCELL_REG, alarmLowCellVoltage);
}

unsigned int LC709203F_getAPA()
{
	return LC709203F_readReg(APA_REG);
}

void LC709203F_setAPA(unsigned int APA)
{
	LC709203F_writeReg(APA_REG, APA);
}

unsigned int LC709203F_getAPT()
{
	return LC709203F_readReg(APT_REG);
}

void LC709203F_setAPT(unsigned int APT)
{
	LC709203F_writeReg(APT_REG, APT);
}

unsigned int LC709203F_getPowerMode()
{
	return LC709203F_readReg(ICPOWERMODE_REG);
}

void LC709203F_setPowerMode(unsigned int powermode)
{
	LC709203F_writeReg(ICPOWERMODE_REG, powermode);
}

unsigned int LC709203F_getStatusBit()
{
	return LC709203F_readReg(STATUSBIT_REG);
}

void LC709203F_setStatusBit(unsigned int statusbit)
{
	LC709203F_writeReg(STATUSBIT_REG, statusbit);
}

unsigned int LC709203F_getBatteryProfile()
{
	return LC709203F_readReg(BAT_PROFILE_R_REG);
}

void LC709203F_setBatteryProfile(unsigned int batteryprofile)
{
	LC709203F_writeReg(BAT_PROFILE_RW_REG, batteryprofile);
}

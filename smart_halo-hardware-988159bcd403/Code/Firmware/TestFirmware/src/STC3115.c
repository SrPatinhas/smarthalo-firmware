/*
 * STC3115.c
 *
 *  Created on: Nov 15, 2016
 *      Author: sgelinas
 */

#include "STC3115.h"
#include "I2C.h"
#include "CommandLineInterface.h"
#include "app_error.h"
#include "nrf_delay.h"

#include <string.h>
#include <stdbool.h>

#define STC3115_ADDR ((0xE0 >> 1) & (0x7F))
#define CONV_SOC  0.001953125
#define CONV_I    0.00588
#define CONV_V    0.0022
#define CONV_T    1
#define RSENSE    0.01

#define VM_CNF_DEF          379
#define CC_CNF_DEF          393
#define ALARM_SOC_DEF       20    	// 10%
#define ALARM_V_DEF         205   	// 3600 mV
#define CURRENT_THRES_DEF   3   	// 14 mA

#define ALARMRSOCMIN	0
#define ALARMRSOCMAX	200
#define ALARMCELLMIN	0.0
#define ALARMCELLMAX	4505.6

void STC3115_setup()
{
	// 1) Read OCV
	  int init_OCV = STC3115_readOCV();

	  // 2) Disable Operating Mode and Set parameters
	  STC3115_disableOperatingMode();
	  STC3115_setDefaultConfig();

	  // 3) Write OCV
	  STC3115_writeOCV(init_OCV);

	  // Start operation
	  STC3115_setOperatingMode();

	  nrf_delay_ms(500);

	  STC3115_checkStatus();
}

bool STC3115_checkPresent()
{
	unsigned char chipID = STC3115_readID();

	return (chipID == 0x14);
}

void STC3115_printHelp()
{
	CommandLineInterface_printLine("batrsoc:\t\tReturns the battery relative state of charge (%)");
	CommandLineInterface_printLine("batcellv:\t\tReturns the cell voltage (mV)");
	CommandLineInterface_printLine("batcelli:\t\tReturns the cell current (mA)");
	CommandLineInterface_printLine("batcellt:\t\tReturns the cell temperature (C)");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("batlowrscoc:\t\tReturns the alarmb low RSOC threshold (%)");
	CommandLineInterface_printLine("batlowrsoc=<value>:\tSets the alarmb low RSOC threshold (%)");
	CommandLineInterface_printLine("batlowcellv:\t\tReturns the alarmb low cell voltage (mV)");
	CommandLineInterface_printLine("batlowcellv=<value>:\tSets the alarmb low cell voltage (mV)");
	nrf_delay_ms(50);
}

bool STC3115_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	int value = 0;
	bool parsed = true;

	if(sscanf(RxBuff, "BATLOWRSOC=%d", &value)==1)
	{
		if(value < ALARMRSOCMIN || value > ALARMRSOCMAX)
			CommandLineInterface_printf("Invalid BATLOWRSOC: must be between %d%% and %d%%\r\n", ALARMRSOCMIN, ALARMRSOCMAX);
		else
		{
			// TODO: NOT IMPLEMENTED
		}
	}
	else if(sscanf(RxBuff, "BATLOWCELLV=%d", &value)==1)
	{
		if(value < ALARMCELLMIN || value > ALARMCELLMAX)
			CommandLineInterface_printf("Invalid BATLOWCELLV: must be between %.1fV and %.1fV\r\n", ALARMCELLMIN, ALARMCELLMAX);
		else
		{
			// TODO: NOT IMPLEMENTED
		}
	}
	else if(strncmp(RxBuff, "BATRSOC", 7)==0)
	{
		CommandLineInterface_printf("BATRSOC=%.1f%%\r\n", STC3115_getSOC());
	}
	else if(strncmp(RxBuff, "BATCELLV", 8)==0)
	{
		CommandLineInterface_printf("BATCELLV=%.3fV\r\n", STC3115_getVoltage());
	}
	else if(strncmp(RxBuff, "BATCELLI", 8)==0)
	{
		CommandLineInterface_printf("BATCELLI=%.3fmA\r\n", STC3115_getCurrent());
	}
	else if(strncmp(RxBuff, "BATCELLT", 8)==0)
	{
		CommandLineInterface_printf("BATCELLT=%.2fC\r\n", STC3115_getTemperature());
	}
	else if(strncmp(RxBuff, "BATLOWRSOC", 10)==0)
	{
		// TODO: NOT IMPLEMENTED
	}
	else if(strncmp(RxBuff, "BATLOWCELLV", 11)==0)
	{
		// TODO: NOT IMPLEMENTED
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void STC3115_checkStatus()
{
  STC3115_SOCMODE socmode = STC3115_getSOCMode();

  if(socmode == VOLT)
	  CommandLineInterface_printLine("SOCMODE: VOLT");
  else
	  CommandLineInterface_printLine("SOCMODE: COULOMBCTR");

  if(STC3115_isBatFail())
  {
	  CommandLineInterface_printLine("Detected a BATFAIL");
	  STC3115_clearBatFail();
  }
  if(STC3115_isPORdetected())
	  CommandLineInterface_printLine("Detected a POR");
  STC3115_clearPORdetect();

  if(STC3115_isAlarmSOC())
  {
	  CommandLineInterface_printLine("Detected a SOC Alarm");
	  STC3115_clearAlarmSOC();
  }

  if(STC3115_isAlarmVolt())
  {
	  CommandLineInterface_printLine("Detected a Volt Alarm");
	  STC3115_clearAlarmVolt();
  }
}

void STC3115_setDefaultConfig()
{
	STC3115_setDefaultVMConfig();
	STC3115_setDefaultCCConfig();
	STC3115_setDefaultAlarmSOCConfig();
	STC3115_setDefaultAlarmVoltageConfig();
	STC3115_setDefaultCurrentThresholdConfig();
	STC3115_setMixedMode();
}

void STC3115_writeReg(STC3115_Registers reg, unsigned char data)
{
	ret_code_t err_code;

	uint8_t regdata[2] = {reg, data};

	err_code = I2C_twi_tx(STC3115_ADDR, regdata, 2, false);
	APP_ERROR_CHECK(err_code);

}

unsigned char STC3115_readReg(STC3115_Registers reg)
{
	return readReg8(STC3115_ADDR, reg);
}

void STC3115_setOperatingMode()
{
  unsigned char regModeReg = STC3115_readMode();
  STC3115_writeReg(REG_MODE,(regModeReg) | (1 << 4));
}

void STC3115_disableOperatingMode()
{
  unsigned char regModeReg = STC3115_readMode();
  STC3115_writeReg(REG_MODE,(regModeReg) & ~(1 << 4));
}

void STC3115_setMixedMode()
{
  unsigned char regModeReg = STC3115_readMode();
  STC3115_writeReg(REG_MODE,(regModeReg) & ~(1 << 0));
}

void STC3115_forceCCMode()
{
  unsigned char regModeReg = STC3115_readMode();
  STC3115_writeReg(REG_MODE,(regModeReg) | (1 << 5));
}
void STC3115_setPowerSavingMode()
{
  unsigned char regModeReg = STC3115_readMode();
  STC3115_writeReg(REG_MODE,(regModeReg) | (1 << 0));
}

bool STC3115_isBatFail()
{
  unsigned char regCtrlReg = STC3115_readControl();

  return ((regCtrlReg >> 3) & 0x01)? true : false;
}

void STC3115_clearBatFail()
{
  unsigned char regCtrlReg = STC3115_readControl();
  STC3115_writeReg(REG_CTRL,(regCtrlReg) & ~(1 << 3));
}

bool STC3115_isPORdetected()
{
  unsigned char regCtrlReg = STC3115_readControl();

  return ((regCtrlReg >> 4) & 0x01)? true : false;
}

void STC3115_clearPORdetect()
{
  unsigned char regCtrlReg = STC3115_readControl();
  STC3115_writeReg(REG_CTRL,(regCtrlReg) & ~(1 << 4));
}

bool STC3115_isAlarmSOC()
{
  unsigned char regCtrlReg = STC3115_readControl();

  return ((regCtrlReg >> 5) & 0x01)? true : false;
}

bool STC3115_isAlarmVolt()
{
  unsigned char regCtrlReg = STC3115_readControl();

  return ((regCtrlReg >> 6) & 0x01)? true : false;
}

void STC3115_clearAlarmSOC()
{
  unsigned char regCtrlReg = STC3115_readControl();
  STC3115_writeReg(REG_CTRL,(regCtrlReg) & ~(1 << 5));
}

void STC3115_clearAlarmVolt()
{
  unsigned char regCtrlReg = STC3115_readControl();
  STC3115_writeReg(REG_CTRL,(regCtrlReg) & ~(1 << 6));
}

STC3115_SOCMODE STC3115_getSOCMode()
{
  unsigned char regCtrlReg = STC3115_readControl();

  return ((regCtrlReg >> 2) & 0x01)? VOLT : COULOMBCTR;
}

unsigned char STC3115_readMode()
{
	return STC3115_readReg(REG_MODE);
}

unsigned char STC3115_readControl()
{
  return STC3115_readReg(REG_CTRL);
}

unsigned char STC3115_readID()
{
  return STC3115_readReg(REG_ID);
}

unsigned int STC3115_readSOCRaw()
{
	// Must read 2 registers
	return readReg16(STC3115_ADDR, REG_SOC_L);
}

int STC3115_readVoltageRaw()
{
	return readReg16(STC3115_ADDR, REG_VOLTAGE_L);
}

int STC3115_readOCV()
{
	return readReg16(STC3115_ADDR, REG_OCV_L);
}

void STC3115_writeOCV(int ocv)
{
	unsigned char lowB = (unsigned char)(((unsigned int)ocv) & 0x00FF);
  	unsigned char highB = (unsigned char)(((unsigned int)ocv >> 8) & 0x00FF);

  	ret_code_t err_code;

	uint8_t regdata[3] = {REG_OCV_L, lowB, highB};

	err_code = I2C_twi_tx(STC3115_ADDR, regdata, 3, false);
	APP_ERROR_CHECK(err_code);
}

int STC3115_readCurrentRaw()
{
	return readSignedReg16(STC3115_ADDR, REG_CURRENT_L);
}

int STC3115_readTemperatureCRaw()
{
	return STC3115_readReg(REG_TEMPERATURE);
}

unsigned int STC3115_readVMConfig()
{
	return readReg16(STC3115_ADDR, REG_VM_CNF_L);
}

unsigned int STC3115_readCCConfig()
{
	return readReg16(STC3115_ADDR, REG_CC_CNF_L);
}

unsigned int STC3115_convert8Bto16BUnsigned(unsigned char lowB, unsigned char highB)
{
  return (unsigned int)(((unsigned int)highB << 8) | (unsigned int)lowB);
}

int STC3115_convert8Bto16BSigned(unsigned char lowB, unsigned char highB)
{
  return (int)(((unsigned int)highB << 8) | (unsigned int)lowB);
}

double STC3115_getSOC()
{
  return CONV_SOC * STC3115_readSOCRaw();
}

double STC3115_getVoltage()
{
  return CONV_V * STC3115_readVoltageRaw();
}

double STC3115_getCurrent()
{
  return (CONV_I * STC3115_readCurrentRaw()) / RSENSE;
}

double STC3115_getTemperature()
{
  return CONV_T * STC3115_readTemperatureCRaw();
}

void STC3115_setDefaultVMConfig()
{
    unsigned char lowB = (unsigned char)(((unsigned int)VM_CNF_DEF) & 0x00FF);
    unsigned char highB = (unsigned char)(((unsigned int)VM_CNF_DEF >> 8) & 0x00FF);

    ret_code_t err_code;

	uint8_t regdata[3] = {REG_VM_CNF_L, lowB, highB};

	err_code = I2C_twi_tx(STC3115_ADDR, regdata, 3, false);
	APP_ERROR_CHECK(err_code);
}

void STC3115_setDefaultCCConfig()
{
    unsigned char lowB = (unsigned char)(((unsigned int)CC_CNF_DEF) & 0x00FF);
    unsigned char highB = (unsigned char)(((unsigned int)CC_CNF_DEF >> 8) & 0x00FF);

    ret_code_t err_code;

    uint8_t regdata[3] = {REG_CC_CNF_L, lowB, highB};

	err_code = I2C_twi_tx(STC3115_ADDR, regdata, 3, false);
	APP_ERROR_CHECK(err_code);
}

void STC3115_setCurrentThreshold(unsigned char value)
{
	STC3115_writeReg(REG_CURRENT_THRES, value);
}

void STC3115_setDefaultAlarmSOCConfig()
{
    STC3115_writeReg(REG_ALARM_SOC,ALARM_SOC_DEF);
}

void STC3115_setDefaultAlarmVoltageConfig()
{
    STC3115_writeReg(REG_ALARM_VOLTAGE,ALARM_V_DEF);
}

void STC3115_setDefaultCurrentThresholdConfig()
{
    STC3115_writeReg(REG_CURRENT_THRES,CURRENT_THRES_DEF);
}

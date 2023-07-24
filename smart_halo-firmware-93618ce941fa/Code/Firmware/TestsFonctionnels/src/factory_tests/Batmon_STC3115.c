/*
 * Batmon_STC3115.c

 *
 *  Created on: Oct 26, 2016
 *      Author: Sean
 */
#include <stdbool.h>
#include <string.h>

#include "I2C.h"
#include "CommandLineInterface.h"

#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "Batmon_STC3115.h"

#define ALARMRSOCMIN	0x0000
#define ALARMRSOCMAX	0x0064
#define ALARMCELLMIN	0.0f
#define ALARMCELLMAX	4.33f
#define POWERMODEMIN	0x0000
#define POWERMODEMAX	0x0001
#define STATUSBITMIN	0x0000
#define STATUSBITMAX	0x0001
#define VM_CNF_DEF          379
#define CC_CNF_DEF          393
#define ALARM_SOC_DEF       20    // 10%
#define ALARM_V_DEF         205   // 3600 mV
#define CURRENT_THRES_DEF   3   // 14 mA

#define CONV_SOC  0.001953125
#define CONV_I    0.00588
#define CONV_V    0.0022
#define CONV_T    1
#define RSENSE    0.01

#define STC3115_ADDR     ((0xE0 >> 1) & (0x7F))

static uint8_t ALARMBPIN = 0;
static int prevstate = 1;
static unsigned int alarmstatus = 0;
static bool STC3115_PRESENT = false;

static void set_operating_mode(STC3115_GG_RUN_MODE Op_mode);
static void setDefaultConfig();
static void setDefaultVMConfig();
static void setDefaultCCConfig();
static void setDefaultAlarmSOCConfig();
static void setDefaultAlarmVoltageConfig();
static void setDefaultCurrentThresholdConfig();
static void setMixedMode();
static void writeOCV(int ocv);
static int readOCV();
static unsigned int convert8Bto16BUnsigned(unsigned char lowB, unsigned char highB);
static int convert8Bto16BSigned(unsigned char lowB, unsigned char highB);
static unsigned int readSOCRaw();
static int readCurrentRaw();
static int readTemperatureCRaw();
static double getSOC();
static double getVoltage();
static double getCurrent();
static double getTemperature();
static void read_and_print_all_registers();

void Batmon_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         BATTERY");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("batrsoc:\t\tReturns the battery relative state of charge (%)");
	CommandLineInterface_printLine("batcellv:\t\tReturns the cell voltage (mV)");
	CommandLineInterface_printLine("batcellt:\t\tReturns the cell temperature (C)");
	CommandLineInterface_printLine("batcurrent:\t\tReturns the battery output current (uA)");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("batver:\t\tReturns the battery monitor version");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("batalarm:\t\tReturns the alarm status (0=off, 1=on)");
	CommandLineInterface_printLine("batalarm=<value>:\tEnables or disables the alarm (0=off, 1=on)");
	CommandLineInterface_printLine("batlowrscoc:\t\tReturns the alarmb low RSOC threshold (%)");
	CommandLineInterface_printLine("batlowrsoc=<value>:\tSets the alarmb low RSOC threshold (%)");
	CommandLineInterface_printLine("batlowcellv:\t\tReturns the alarmb low cell voltage (mV)");
	CommandLineInterface_printLine("batlowcellv=<value>:\tSets the alarmb low cell voltage (mV)");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("batpower:\t\tReturns the power mode");
	CommandLineInterface_printLine("batpower=<value>:\tSets the power mode");
	CommandLineInterface_printLine("batstatb:\t\tReturns the status bit");
	CommandLineInterface_printLine("batstatb=<value>:\tSets the status bit");
	CommandLineInterface_printLine("printallreg:\tPrints the value of all the registers of the STC3115");
	nrf_delay_ms(50);
}

bool Batmon_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	int value = 0;
	float valuef = 0;
	bool parsed = true;

	if(sscanf(RxBuff, "BATLOWRSOC=%d", &value)==1)
	{
		if(value < ALARMRSOCMIN || value > ALARMRSOCMAX)
			CommandLineInterface_printf("Invalid BATLOWRSOC: must be between %d%% and %d%%\r\n", ALARMRSOCMIN, ALARMRSOCMAX);
		else
		{
			Batmon_setAlarmLowRSOC(value);
			CommandLineInterface_printf("BATLOWRSOC=%d%%\r\n", Batmon_getAlarmLowRSOC());
		}
	}
	else if(sscanf(RxBuff, "BATLOWCELLV=%f", &valuef)==1)
	{
		if(value < ALARMCELLMIN || value > ALARMCELLMAX)
			CommandLineInterface_printf("Invalid BATLOWCELLV: must be between %.1fV and %.1fV\r\n", ALARMCELLMIN, ALARMCELLMAX);
		else
		{
			Batmon_setAlarmLowCellVoltage(valuef);
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
	else if(strncmp(RxBuff, "BATCURRENT", 7)==0)
	{
		CommandLineInterface_printf("BATCURRENT=%f uA\r\n", getCurrent());
	}
	else if(strncmp(RxBuff, "BATCELLV", 8)==0)
	{
		CommandLineInterface_printf("BATCELLV=%.3fV\r\n", Batmon_getCellVoltage());
	}
	else if(strncmp(RxBuff, "BATCELLT", 8)==0)
	{
		CommandLineInterface_printf("BATCELLT=%.2fC\r\n", Batmon_getCellTemperature());
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
	else if(strncmp(RxBuff, "PRINTALLREG\r\n", 11)==0)
	{
		read_and_print_all_registers();
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
	uint8_t data = REG_ID;
	uint8_t regdata[2];

	if(I2C_twi_tx(STC3115_ADDR, &data, 1, true) == NRF_SUCCESS)
	{
		STC3115_PRESENT = true;

		err_code = I2C_twi_rx(STC3115_ADDR, regdata, 2);
		APP_ERROR_CHECK(err_code);
		return true;
	}
	else
	{
		CommandLineInterface_printLine("Battery monitor not detected");
		return false;
	}
}

void Batmon_writeReg(Reg_Gas_Gauge reg, unsigned int data)
{
	ret_code_t err_code;

	if(STC3115_PRESENT)
	{
		uint8_t regdata[2] = {reg,data};
		err_code = I2C_twi_tx(STC3115_ADDR, regdata, 2, false);
		APP_ERROR_CHECK(err_code);
	}
}

unsigned int Batmon_readReg(Reg_Gas_Gauge reg)
{
	if(STC3115_PRESENT)
		return readReg8(STC3115_ADDR, reg);
	else
		return -1;
}

void Batmon_setup(uint8_t alarmbPin)
{
	Batmon_checkPresent();

	int init_OCV = readOCV();

	set_operating_mode(GG_RUN_STANDBY);
	setDefaultConfig();

	writeOCV(init_OCV);
	set_operating_mode(GG_RUN_OPERATING_MODE);

	//read_and_print_all_registers();

	//Batmon_enableAlarmB(); //broken do not use

}

//used to set power saving voltage mode or mixed mode (CC active)
void Batmon_setPowerMode(STC3115_VMODE powermode)
{
	uint8_t stored_values = Batmon_readReg(REG_MODE);
	stored_values &= 0xFE; //clear VMODE bit
	stored_values |= powermode;
	Batmon_writeReg(REG_MODE, stored_values);
}

static void set_operating_mode(STC3115_GG_RUN_MODE Op_mode)
{
	uint8_t stored_values = Batmon_readReg(REG_MODE);
	stored_values &= 0xEF; //clear GG_RUN bit
	stored_values |= (Op_mode << 4);
	Batmon_writeReg(REG_MODE, stored_values);
}

static void setDefaultConfig()
{
  setDefaultVMConfig();
  setDefaultCCConfig();
  setDefaultAlarmSOCConfig();
  setDefaultAlarmVoltageConfig();
  setDefaultCurrentThresholdConfig();
  setMixedMode();
}

static void setDefaultVMConfig()
{
	unsigned char lowB = (unsigned char)(((unsigned int)VM_CNF_DEF) & 0x00FF);
	unsigned char highB = (unsigned char)(((unsigned int)VM_CNF_DEF >> 8) & 0x00FF);

	Batmon_writeReg(REG_VM_CNF_L, lowB);
	Batmon_writeReg(REG_VM_CNF_H, highB);
}

static void setDefaultCCConfig()
{
	unsigned char lowB = (unsigned char)(((unsigned int)CC_CNF_DEF) & 0x00FF);
	unsigned char highB = (unsigned char)(((unsigned int)CC_CNF_DEF >> 8) & 0x00FF);

	Batmon_writeReg(REG_CC_CNF_L, lowB);
	Batmon_writeReg(REG_CC_CNF_H, highB);
}

static void setDefaultAlarmSOCConfig()
{
	Batmon_writeReg(REG_ALARM_SOC, ALARM_SOC_DEF);
}

static void setDefaultAlarmVoltageConfig()
{
	Batmon_writeReg(REG_ALARM_VOLTAGE, ALARM_V_DEF);
}

static void setDefaultCurrentThresholdConfig()
{
	Batmon_writeReg(REG_CURRENT_THRES, CURRENT_THRES_DEF);
}

static void setMixedMode()
{
	Batmon_setPowerMode(COULOMBCTR);
}

static void writeOCV(int ocv)
{
	unsigned char lowB = (unsigned char)(((unsigned int)ocv) & 0x00FF);
	unsigned char highB = (unsigned char)(((unsigned int)ocv >> 8) & 0x00FF);

	Batmon_writeReg(REG_OCV_L, lowB);
	Batmon_writeReg(REG_OCV_H, highB);
}

static int readOCV()
{
	unsigned char c_L = Batmon_readReg(REG_OCV_L);
	unsigned char c_H = Batmon_readReg(REG_OCV_H);

	return convert8Bto16BSigned(c_L, c_H);
}

static unsigned int convert8Bto16BUnsigned(unsigned char lowB, unsigned char highB)
{
  return (unsigned int)(((unsigned int)highB << 8) | (unsigned int)lowB);
}

static int convert8Bto16BSigned(unsigned char lowB, unsigned char highB)
{
  	return (int16_t)((((uint16_t)highB << 8) &0xFF00) | (((uint16_t)lowB)&0x00FF));
}

static unsigned int readSOCRaw()
{
	unsigned char c_L = Batmon_readReg(REG_SOC_L);
	unsigned char c_H = Batmon_readReg(REG_SOC_H);

	return convert8Bto16BUnsigned(c_L, c_H);
}

static int readVoltageRaw()
{
	unsigned char c_L = Batmon_readReg(REG_VOLTAGE_L);
	unsigned char c_H = Batmon_readReg(REG_VOLTAGE_H);

	return convert8Bto16BSigned(c_L, c_H);
}

static int readCurrentRaw()
{
	unsigned char c_L = Batmon_readReg(REG_CURRENT_L);
	unsigned char c_H = Batmon_readReg(REG_CURRENT_H);

	return convert8Bto16BSigned(c_L, c_H);
}

static int readTemperatureCRaw()
{
	unsigned char c = Batmon_readReg(REG_TEMPERATURE);

	return (int)c;
}

static double getSOC()
{
  return CONV_SOC * readSOCRaw();
}

static double getVoltage()
{
  return CONV_V * readVoltageRaw();
}

//returns current in uA
static double getCurrent()
{
  return ((CONV_I * readCurrentRaw()) )/ RSENSE;
}

static double getTemperature()
{
  return CONV_T * readTemperatureCRaw();
}

static void read_and_print_all_registers()
{
	for(int i = 0; i < 64 ; ++i)
	{
		uint8_t stored_values = Batmon_readReg(i);
		printf("register %d = %d \r\n",i , stored_values);
		nrf_delay_ms(20);
	}
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

void Batmon_enableAlarmB()//broken do not use
{
	uint32_t err_code;

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);

	#ifdef BOARD_PCA10040
		config.pull = NRF_GPIO_PIN_PULLUP;
	#else
		config.pull = NRF_GPIO_PIN_NOPULL;
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
	return 0;
}

float Batmon_getCellVoltage()
{
	return getVoltage();
}

unsigned int Batmon_getRSOC()
{
	return getSOC();
}

//returns current in mA
float Batmon_getChargeCurrent()
{
	return getCurrent() /1000;
}

float Batmon_getITE()
{

	return 0;
}

unsigned int Batmon_getICVersion()
{
	return Batmon_readReg(REG_ID);
}

float Batmon_getCellTemperature()
{
	return getTemperature();
}

void Batmon_setCellTemperature(float cellT)
{

}

unsigned int Batmon_getAlarmLowRSOC()
{
	return Batmon_readReg(REG_ALARM_SOC)/2;
}

void Batmon_setAlarmLowRSOC(unsigned int alarmLowRSOC)
{
	Batmon_writeReg(REG_ALARM_SOC,alarmLowRSOC*2);
}

float Batmon_getAlarmLowCellVoltage()
{
	return Batmon_readReg(REG_ALARM_VOLTAGE)*0.0176f;
}

void Batmon_setAlarmLowCellVoltage(float alarmLowCellVoltage)
{
	Batmon_writeReg(REG_ALARM_VOLTAGE,alarmLowCellVoltage/0.0176f);
}

unsigned int Batmon_getAPA()
{

	return 0;
}

void Batmon_setAPA(unsigned int APA)
{

}

unsigned int Batmon_getAPT()
{

	return 0;
}

void Batmon_setAPT(unsigned int APT)
{

}

unsigned int Batmon_getPowerMode()
{
	uint8_t stored_values = Batmon_readReg(REG_MODE);
	stored_values &= 0x01; //keep VMODE bit
	return stored_values;
}

unsigned int Batmon_getStatusBit()
{
	uint8_t stored_values = Batmon_readReg(REG_CTRL);
	stored_values &= 0x01; //keep ALM bit
	return stored_values;
}

void Batmon_setStatusBit(unsigned int statusbit)
{
	uint8_t stored_values = Batmon_readReg(REG_CTRL);
	stored_values &= 0xFE; //clear ALM bit
	stored_values |= statusbit;
	Batmon_writeReg(REG_CTRL, stored_values);
}

unsigned int Batmon_getBatteryProfile()
{

	return 0;
}

void Batmon_setBatteryProfile(unsigned int batteryprofile)
{

}












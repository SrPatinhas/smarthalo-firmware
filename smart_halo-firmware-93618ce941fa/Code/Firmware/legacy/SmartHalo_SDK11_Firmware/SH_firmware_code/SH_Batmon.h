/*
 * SH_batmon.h
 *
 *  Created on: 2016-07-23
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_BATMON_H_
#define SH_FIRMWARE_CODE_SH_BATMON_H_


#define BATMON_ADDRESS         0b0001011

// LC709203F driver register addresses
typedef enum
{
	BEFORE_RSOC_REG	=		0X04,
	THERMISTORB_REG =		0x06,
	INITIAL_RSOC =			0x07,
	CELL_TEMP_REG =			0x08,
	CELL_VOLT_REG =			0x09,
	CURR_DIR_REG =			0x0A,
	APA_REG =				0x0B,
	APT_REG =				0x0C,
	RSOC_REG =				0x0D,
	ITE_REG =				0x0F,
	IC_VERSION_REG =		0x11,
	BAT_PROFILE_RW_REG =	0x12,
	ALARMLOWRSOC_REG =		0x13,
	ALARMLOWCELL_REG =		0x14,
	ICPOWERMODE_REG =		0x15,
	STATUSBIT_REG =			0x16,
	BAT_PROFILE_R_REG =		0x1A
} BatMonRegisters;

typedef enum
{
	OPERATIONAL_MODE =1,
	SLEEP_MODE = 2
}BatMonPowerMode;

typedef enum
{
	BATTERY_PROFILE1 = 1,//3.7V/4.2V
	BATTERY_PROFILE3 = 3,//3.8V/4.35V
	BATTERY_PROFILE4 = 4, //UR18650ZY (Panasonic)
	BATTERY_PROFILE2 = 5//ICR18650-26H (SAMSUNG)
}BatMonBatteryProfile;

typedef enum
{
	STATUS_BIT_I2C = 0,
	STATUS_BIT_THERMISTOR = 1

}BatMonStatusBit;

void SH_Batmon_setup(uint8_t alarmbPin);
void SH_Batmon_init();
void SH_Batmon_disable_alarmlowrsoc();
void SH_Batmon_test();
float SH_Batmon_getCellVoltage();
uint8_t SH_Batmon_getRSOC();
float SH_Batmon_getITE();
float SH_Batmon_getCellTemperature();
void SH_Batmon_setAlarmLowRSOC(unsigned int alarmLowRSOC);
void SH_Batmon_setAlarmLowCellVoltage(float alarmLowCellVoltage);


#endif /* SH_FIRMWARE_CODE_SH_BATMON_H_ */

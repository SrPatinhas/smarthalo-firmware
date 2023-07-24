/*
 * Batmon.h
 *
 *  Created on: Jun 3, 2016
 *      Author: sgelinas
 */

#ifndef SRC_BATMON_H_
#define SRC_BATMON_H_

#include "nrf.h"
#include "bsp.h"

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



void Batmon_setup(uint8_t alarmbPin);

void Batmon_printHelp();

bool Batmon_parseAndExecuteCommand(char * RxBuff, int cnt);

void Batmon_enableAlarmB();

void Batmon_disableAlarmB();

bool Batmon_checkPresent();

void Batmon_writeReg(BatMonRegisters reg, unsigned int data);

unsigned int Batmon_readReg(BatMonRegisters reg);

unsigned int Batmon_getAlarmStatus();

float Batmon_getCellVoltage();

unsigned int Batmon_getRSOC();

float Batmon_getITE();

unsigned int Batmon_getICVersion();

float Batmon_getCellTemperature();

void Batmon_setCellTemperature(float cellT);

unsigned int Batmon_getAlarmLowRSOC();

void Batmon_setAlarmLowRSOC(unsigned int alarmLowRSOC);

float Batmon_getAlarmLowCellVoltage();

void Batmon_setAlarmLowCellVoltage(float alarmLowCellVoltage);

unsigned int Batmon_getAPA();

void Batmon_setAPA(unsigned int APA);

unsigned int Batmon_getAPT();

void Batmon_setAPT(unsigned int APT);

unsigned int Batmon_getPowerMode();

void Batmon_setPowerMode(unsigned int powermode);

unsigned int Batmon_getStatusBit();

void Batmon_setStatusBit(unsigned int statusbit);

unsigned int Batmon_getBatteryProfile();

void Batmon_setBatteryProfile(unsigned int batteryprofile);

#endif /* SRC_BATMON_H_ */

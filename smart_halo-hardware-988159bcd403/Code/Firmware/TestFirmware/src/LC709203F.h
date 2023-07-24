/*
 * LC709203F.h
 *
 *  Created on: Nov 15, 2016
 *      Author: sgelinas
 */

#ifndef SRC_LC709203F_H_
#define SRC_LC709203F_H_

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
} LC709203F_Registers;

void LC709203F_setup();

bool LC709203F_checkPresent();

void LC709203F_printHelp();

bool LC709203F_parseAndExecuteCommand(char * RxBuff, int cnt);

float LC709203F_getCellVoltage();

unsigned int LC709203F_getRSOC();

float LC709203F_getITE();

unsigned int LC709203F_getICVersion();

float LC709203F_getCellTemperature();

void LC709203F_setCellTemperature(float cellT);

unsigned int LC709203F_getAlarmLowRSOC();

void LC709203F_setAlarmLowRSOC(unsigned int alarmLowRSOC);

float LC709203F_getAlarmLowCellVoltage();

void LC709203F_setAlarmLowCellVoltage(float alarmLowCellVoltage);

unsigned int LC709203F_getAPA();

void LC709203F_setAPA(unsigned int APA);

unsigned int LC709203F_getAPT();

void LC709203F_setAPT(unsigned int APT);

unsigned int LC709203F_getPowerMode();

void LC709203F_setPowerMode(unsigned int powermode);

unsigned int LC709203F_getStatusBit();

void LC709203F_setStatusBit(unsigned int statusbit);

unsigned int LC709203F_getBatteryProfile();

void LC709203F_setBatteryProfile(unsigned int batteryprofile);

void LC709203F_writeReg(LC709203F_Registers reg, unsigned int data);

unsigned int LC709203F_readReg(LC709203F_Registers reg);

#endif /* SRC_LC709203F_H_ */

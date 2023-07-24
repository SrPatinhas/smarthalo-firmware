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

void Batmon_setup(uint8_t alarmbPin, uint8_t chgPin, uint8_t pgPin);

void Batmon_printHelp();

bool Batmon_parseAndExecuteCommand(char * RxBuff, int cnt);

void Batmon_enableAlarmB();

void Batmon_disableAlarmB();

void Batmon_enableCHGInterrupt();

void Batmon_disableCHGInterrupt();

void Batmon_enablePGInterrupt();

void Batmon_disablePGInterrupt();

bool Batmon_checkPresent();

unsigned int Batmon_getAlarmStatus();

unsigned int Batmon_getCHGStatus();

unsigned int Batmon_getPGStatus();

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

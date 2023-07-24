/*
 * Batmon_STC3115.h
 *
 *  Created on: Oct 26, 2016
 *      Author: Sean
 */

#ifndef SRC_FACTORY_TESTS_BATMON_STC3115_H_
#define SRC_FACTORY_TESTS_BATMON_STC3115_H_

// STC3115 driver register addresses
typedef enum  Reg_Gas_Gauge
{
	REG_MODE			=	0,
	REG_CTRL			=	1,
	REG_SOC_L 			= 	2,
	REG_SOC_H			=	3,
	REG_COUNTER_L		=	4,
	REG_COUNTER_H		=	5,
	REG_CURRENT_L		=	6,
	REG_CURRENT_H		=	7,
	REG_VOLTAGE_L		=	8,
	REG_VOLTAGE_H		=	9,
	REG_TEMPERATURE		=	10,
	REG_CC_ADJ_HIGH		=	11,
	REG_VM_ADJ_HIGH		=	12,
	REG_OCV_L			=	13,
	REG_OCV_H			=	14,
	REG_CC_CNF_L		=	15,
	REG_CC_CNF_H		=	16,
	REG_VM_CNF_L		=	17,
	REG_VM_CNF_H		=	18,
	REG_ALARM_SOC		=	19,
	REG_ALARM_VOLTAGE	=	20,
	REG_CURRENT_THRES	=	21,
	REG_RELAX_COUNT		=	22,
	REG_RELAX_MAX		=	23,
	REG_ID				=	24,
	REG_CC_ADJ_LOW		=	25,
	REG_VM_ADJ_LOW		=	26,
	ACC_CC_ADJ_L		=	27,
	ACC_CC_ADJ_H		=	28,
	ACC_VM_ADJ_L		=	29,
	ACC_VM_ADJ_H		=	30
}Reg_Gas_Gauge;

typedef enum
{
  COULOMBCTR = 0,
  VOLT
} STC3115_VMODE;

typedef enum
{
  GG_RUN_STANDBY = 0,
  GG_RUN_OPERATING_MODE
} STC3115_GG_RUN_MODE;

void Batmon_setup(uint8_t alarmbPin);

void Batmon_printHelp();

bool Batmon_parseAndExecuteCommand(char * RxBuff, int cnt);

void Batmon_enableAlarmB();

void Batmon_disableAlarmB();

bool Batmon_checkPresent();

void Batmon_writeReg(Reg_Gas_Gauge reg, unsigned int data);

unsigned int Batmon_readReg(Reg_Gas_Gauge reg);

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

void Batmon_setPowerMode(STC3115_VMODE powermode);

unsigned int Batmon_getStatusBit();

void Batmon_setStatusBit(unsigned int statusbit);

unsigned int Batmon_getBatteryProfile();

void Batmon_setBatteryProfile(unsigned int batteryprofile);

float Batmon_getChargeCurrent();

#endif /* SRC_FACTORY_TESTS_BATMON_STC3115_H_ */

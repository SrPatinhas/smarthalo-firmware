/*
 * STC3115.h
 *
 *  Created on: Nov 15, 2016
 *      Author: sgelinas
 */

#ifndef SRC_STC3115_H_
#define SRC_STC3115_H_

#include "nrf.h"
#include "bsp.h"

// STC3115 driver register addresses
typedef enum
{
  REG_MODE = 0,
  REG_CTRL,
  REG_SOC_L,
  REG_SOC_H,
  REG_COUNTER_L,
  REG_COUNTER_H,
  REG_CURRENT_L,
  REG_CURRENT_H,
  REG_VOLTAGE_L,
  REG_VOLTAGE_H,
  REG_TEMPERATURE,
  REG_CC_ADJ_HIGH,
  REG_VM_ADJ_HIGH,
  REG_OCV_L,
  REG_OCV_H,
  REG_CC_CNF_L,
  REG_CC_CNF_H,
  REG_VM_CNF_L,
  REG_VM_CNF_H,
  REG_ALARM_SOC,
  REG_ALARM_VOLTAGE,
  REG_CURRENT_THRES,
  REG_RELAX_COUNT,
  REG_RELAX_MAX,
  REG_ID,
  REG_CC_ADJ_LOW,
  REG_VM_ADJ_LOW,
  ACC_CC_ADJ_L,
  ACC_CC_ADJ_H,
  ACC_VM_ADJ_L,
  ACC_VM_ADJ_H
} STC3115_Registers;

typedef enum
{
  COULOMBCTR = 0,
  VOLT
} STC3115_SOCMODE;

void STC3115_setup();

bool STC3115_checkPresent();

unsigned char STC3115_readID();

void STC3115_printHelp();

bool STC3115_parseAndExecuteCommand(char * RxBuff, int cnt);

unsigned char STC3115_readMode();

unsigned char STC3115_readControl();

unsigned int STC3115_readSOCRaw();

int STC3115_readVoltageRaw();

int STC3115_readCurrentRaw();

int STC3115_readTemperatureCRaw();

double STC3115_getSOC();

double STC3115_getVoltage();

double STC3115_getCurrent();

double STC3115_getTemperature();

bool STC3115_isBatFail();

void STC3115_clearBatFail();

bool STC3115_isPORdetected();

void STC3115_clearPORdetect();

void STC3115_setOperatingMode();

void STC3115_disableOperatingMode();

void STC3115_setMixedMode();

void STC3115_forceCCMode();

bool STC3115_isAlarmSOC();

bool STC3115_isAlarmVolt();

void STC3115_clearAlarmSOC();

void STC3115_clearAlarmVolt();

STC3115_SOCMODE STC3115_getSOCMode();

void STC3115_setCurrentThreshold(unsigned char value);

int STC3115_readOCV();

void STC3115_checkStatus();

void STC3115_setDefaultConfig();

void STC3115_writeOCV(int ocv);

void STC3115_setPowerSavingMode();

unsigned int STC3115_readVMConfig();

unsigned int STC3115_readCCConfig();

void STC3115_setDefaultVMConfig();

void STC3115_setDefaultCCConfig();

void STC3115_setDefaultAlarmSOCConfig();

void STC3115_setDefaultAlarmVoltageConfig();

void STC3115_setDefaultCurrentThresholdConfig();

unsigned int STC3115_convert8Bto16BUnsigned(unsigned char lowB, unsigned char highB);

int STC3115_convert8Bto16BSigned(unsigned char lowB, unsigned char highB);

#endif /* SRC_STC3115_H_ */

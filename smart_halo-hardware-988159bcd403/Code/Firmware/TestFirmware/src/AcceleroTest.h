/*
 * AcceleroTest.h
 *
 *  Created on: Jun 6, 2016
 *      Author: sgelinas
 */

#ifndef SRC_ACCELEROTEST_H_
#define SRC_ACCELEROTEST_H_

#include "nrf.h"
#include "bsp.h"
#include "SH_Accelero_Magneto_typedefs.h"

#include <stdbool.h>

void AcceleroTest_setup();

void AcceleroTest_printHelp();

bool AcceleroTest_parseAndExecuteCommand(char * RxBuff, int cnt);

void AcceleroTest_writeReg(AcceleroRegisters reg, unsigned char data);

unsigned char AcceleroTest_readReg(AcceleroRegisters reg);

unsigned char AcceleroTest_getID();

void AcceleroTest_getAccTemp(float *temp);

void AcceleroTest_getAccXYZ(float *aX, float *aY, float *aZ);

void AcceleroTest_setAccScale(AcceleroFS scale);

AcceleroFS AcceleroTest_getAccScale();

void AcceleroTest_setAccPowerMode(AcceleroPowerMode powermode);

AcceleroPowerMode AcceleroTest_getAccPowerMode();

bool AcceleroTest_selfTest();

#endif /* SRC_ACCELEROTEST_H_ */

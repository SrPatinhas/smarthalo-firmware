/*
 * MagnetoTest.h
 *
 *  Created on: Jun 6, 2016
 *      Author: sgelinas
 */

#ifndef SRC_MAGNETOTEST_H_
#define SRC_MAGNETOTEST_H_

#include "nrf.h"
#include "bsp.h"
#include "SH_Accelero_Magneto_typedefs.h"
#include <stdbool.h>


void MagnetoTest_setup();

void MagnetoTest_printHelp();

bool MagnetoTest_parseAndExecuteCommand(char * RxBuff, int cnt);

bool check_mag_acc_drivers();

unsigned char MagnetoTest_getID();

void MagnetoTest_writeReg(MagnetoRegisters reg, unsigned char data);

unsigned char MagnetoTest_readReg(MagnetoRegisters reg);

void MagnetoTest_getMagXYZ(float *mX, float *mY, float *mZ);

float MagnetoTest_getHeading();

void MagnetoTest_getMagMinMaxXYZ(unsigned int samples);

void MagnetoTest_getMagCalibOffset(float *mOffsetX, float *mOffsetY, float *mOffsetZ);

void MagnetoTest_setMagCalibOffset(float mOffsetX, float mOffsetY, float mOffsetZ);

bool MagnetoTest_selfTest();

#endif /* SRC_MAGNETOTEST_H_ */

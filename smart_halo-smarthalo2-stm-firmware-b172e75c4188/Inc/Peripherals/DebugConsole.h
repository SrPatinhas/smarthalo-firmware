///
/// \file 		DebugConsole.h
/// \brief 		[header file]
///				
/// \author 	NOVO
///
#ifndef __DEBUGCONSOLE__H
#define __DEBUGCONSOLE__H


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "main.h"
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define MAX_ARGS	15
#define MAX_LINE	80
#define MAX_FN		40



////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
void setTestMode_DebugConsole(bool isTestMode);
void readPhotoSensor_DebugConsole (int argc, char **argv);
void readAccSensor_DebugConsole (int argc, char **argv);
void readMagSensor_DebugConsole (int argc, char **argv);
void readTempSensor_DebugConsole (int argc, char **argv);
void readStateOfCharge_DebugConsole (int argc, char **argv);
void animation_DebugConsole (int argc, char **argv);
void animationOff_DebugConsole(int argc, char **argv);
void startTouchTest_DebugConsole (int argc, char **argv);
void calibrateTouch_DebugConsole (int argc, char **argv);
void swipeTest_DebugConsole (int argc, char **argv);
void tapTest_DebugConsole (int argc, char **argv);
void releaseTest_DebugConsole(int argc, char **argv);
void soundTest_DebugConsole (int argc, char **argv);
void ledsTest_DebugConsole (int argc, char **argv);
void frontTest_DebugConsole (int argc, char **argv);
void oledTest_DebugConsole (int argc, char **argv);
void readDeviceID_DebugConsole (int argc, char **argv);
void printDeviceID_DebugConsole(uint8_t * ID, uint8_t length);
void hardwareTest_DebugConsole (int argc, char **argv);
void pinTest_DebugConsole (int argc, char **argv);

#endif

/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Touch heartbeat detection and location sequence detection
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#ifndef TOUCHTEST_H_
#define TOUCHTEST_H_

#include "nrf.h"
#include "bsp.h"

void TouchTest_setup(uint8_t touchPin, uint8_t modePin);

void TouchTest_printHelp();

bool TouchTest_parseAndExecuteCommand(char * RxBuff, int cnt);

void TouchTest_startHearbeatTest(int duration_s);

void TouchTest_startTouchLocationTest(int duration_s);

bool get_touchtest_status();

#endif /* TOUCHTEST_H_ */

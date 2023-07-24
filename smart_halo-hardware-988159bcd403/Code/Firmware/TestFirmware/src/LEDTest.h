/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Set of functions to test Halo LEDS, Central LEDS and Front LEDS
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#ifndef LEDTEST_H_
#define LEDTEST_H_

#include "nrf.h"
#include "bsp.h"

// ISSI driver register addresses
typedef enum
{
	LED_EN_REG =         0x26,
	GLOBAL_EN_REG =      0x4A,
	UPDATE_REG =         0x25,
	SHUTDOWN_REG =       0x00,
	PWM_REG =            0x01,
	RESET_REG = 		 0x4F
} LEDRegisters;

void LEDTest_setup(uint8_t sdbPin, uint8_t frPin, uint8_t crPin, uint8_t cgPin, uint8_t cbPin);

void LEDTest_printHelp();

bool LEDTest_parseAndExecuteCommand(char * RxBuff, int cnt);

void LEDTest_allOff();

bool check_led_drivers();

void LEDTest_enableAll(int disable);

void LEDTest_showPixel(uint16_t n, int show);

void LEDTest_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);

void LEDTest_sweepRGBLEDs();

void LEDTest_setFrontLedBrightness(uint8_t brightness);

void LEDTest_showAllPixelColor(uint8_t r, uint8_t g, uint8_t b);

void LEDTest_showPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);

void LEDTest_showRangePixelColor(uint16_t m, uint16_t n, uint8_t r, uint8_t g, uint8_t b);

#endif /* LEDTEST_H_ */

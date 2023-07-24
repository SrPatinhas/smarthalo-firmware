/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Set of functions to control the BOOST output with EasyScale protocol
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/10/26
 *
 */

#include <stdint.h>

#include "EasyScale.h"
#include "nrf_delay.h"
#include "bsp.h"

// Global variables to store state parameters
static feedbackVoltagemV currentFeedbackVoltage = FV1229;
static uint8_t easyScalePin;
static int bitPeriodus = 20;

// Private Declarations
void EasyScale_sendByte(unsigned char byte);
void EasyScale_sendBit(unsigned int bit);

void EasyScale_setup(uint8_t enablePin)
{
	easyScalePin = enablePin;
	nrf_gpio_cfg_output(easyScalePin);

	// Enable EasyScale
	nrf_gpio_pin_write(easyScalePin,1);
	nrf_delay_us(100);

	nrf_gpio_pin_write(easyScalePin,0);
	nrf_delay_us(300);

	nrf_gpio_pin_write(easyScalePin,1);
}

// Must send Device address then databyte, MSB first.
// Data rate must be selected between ES_MIN_DATA_RATE AND ES_MAX_DATA_RATE
void EasyScale_setFeedbackVoltage(feedbackVoltagemV newValue)
{
	// Update Feedback voltage variable
	currentFeedbackVoltage = newValue;

	// Set up data byte
	ES_DataByte_t data;
	data.RFA = 0;
	data.addressbits = 0;
	data.newValue = currentFeedbackVoltage;

	// Send address byte
	EasyScale_sendByte(ES_DEVICE_ADDRESS);

	// Send data byte
	unsigned char databyte = ((unsigned char)(data.RFA) << 7) & 0x80;
	databyte |= ((unsigned char)(data.addressbits) << 5) & 0x60;
	databyte |= (unsigned char)(data.newValue) & 0x1F;

	EasyScale_sendByte(databyte);

	// End of transmission
	nrf_gpio_pin_write(easyScalePin, 1);
}

feedbackVoltagemV EasyScale_getFeedbackVoltage()
{
	return currentFeedbackVoltage;
}

float EasyScale_getOutputVoltage()
{
	return (float)currentFeedbackVoltage * (((float)(ES_R1)/(float)(ES_R2)) + 1.0);
}

// Send byte MSB first
void EasyScale_sendByte(unsigned char byte)
{
	// wait tstart before the byte
	nrf_gpio_pin_write(easyScalePin,1);
	nrf_delay_us(5);

	EasyScale_sendBit((byte >> 7) & 0x0001);
	EasyScale_sendBit((byte >> 6) & 0x0001);
	EasyScale_sendBit((byte >> 5) & 0x0001);
	EasyScale_sendBit((byte >> 4) & 0x0001);
	EasyScale_sendBit((byte >> 3) & 0x0001);
	EasyScale_sendBit((byte >> 2) & 0x0001);
	EasyScale_sendBit((byte >> 1) & 0x0001);
	EasyScale_sendBit((byte >> 0) & 0x0001);

	// Wait teos after the byte
	nrf_gpio_pin_write(easyScalePin,0);
	nrf_delay_us(5);
}

// 0 = Tl > 2*Th, 1 = Th > 2*Tl
void EasyScale_sendBit(unsigned int bit)
{
	nrf_gpio_pin_write(easyScalePin,0);

	bit?nrf_delay_us(bitPeriodus/4):nrf_delay_us(3*bitPeriodus/4);

	nrf_gpio_pin_write(easyScalePin,1);

	bit?nrf_delay_us(3*bitPeriodus/4):nrf_delay_us(bitPeriodus/4);
}

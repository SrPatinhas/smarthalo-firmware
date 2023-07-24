/*
 * battery.h
 *
 *  Created on: Nov 8, 2016
 *      Author: Sean
 */

#ifndef _BATTERY_H_
#define _BATTERY_H_

#if defined(PLATFORM_shv1x)

void bat_readData();

//the getXYZ functions return the last measured value. These are only valid if there has recently been a measurement
// ie if bat_readData(); has been recently called
//returns percentage
uint32_t bat_getSOC();
bool is_lowBat();

double bat_getVoltage();

//only call in mixed mode or else it will return garbage
double bat_getCurrent_uA();

double bat_getCurrent_mA();

int32_t bat_getTemperature();

//returns alarm status : b00 alarms are not trigerred, b01 SOC low alarm, b10 voltage low alarm, b11 both alarms
uint8_t bat_get_alarm_status();

bool bat_isUSBPlugged(void);
bool bat_isCharging(void);

void bat_init();

#else

#define bat_getVoltage() 0
#define bat_getCurrent_uA() 0

#define bat_getTemperature() 	0
#define bat_getSOC() 		0
#define bat_isUSBPlugged() 	false
#define bat_isCharging() 	false
#define bat_init() 			(void)0

#endif	//PLATFORM_shv1x

#endif //_BATTERY_H_

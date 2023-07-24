/*
 * SH_Factory_tests.h
 *
 *  Created on: May 18, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SH_FACTORY_TESTS_H_
#define SH_FIRMWARE_CODE_SH_FACTORY_TESTS_H_

#ifdef FACTORY_PROGRAMMED_FIRMWARE
#define LED_TEST
#endif

enum factory_commands{GetSerialNumber=0, SetSerialNumber,GetBootloaderVersion,\
	GetFirmwareVersion,GetSoftdeviceVersion,LoopbackTest,SetTxPower,GetTxPower,GetRSSI,PairingTest,SetLedColor,\
	TurnLedOn,SetFrontLedIntensity, HeartbeatTest,SetTouchTarget,Selftest,GetAccelerationXYZ,\
	FreefallTest,ActivityTriggerTest,GetMagnetometerXYZ,GetMaxAccelerationXYZ,GetMaxMagnetometerXYZ,\
	SetMagnetometerCalibrationOffset,GetCompassHeading,GetBatteryVoltage,GetBatterySOC,GetChargeCurrent,GetRailVoltage,\
	Play,ExitFactoryMode,ExitIntoShippingMode,EnterBootloader,NUMBER_OF_COMMANDS};
	//NUMBER_OF_COMMANDS is used to know how many commands are present in the enum

enum factory_parameter{lock=0,magnetic_key,PCBA,product,USB_interface,BLE_interface,ANT_interface,\
	front,back,left,right,acc,mag,NUMBER_OF_PARAMETERS};

void enter_factory_mode();

#endif /* SH_FIRMWARE_CODE_SH_FACTORY_TESTS_H_ */

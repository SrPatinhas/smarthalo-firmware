/*
 * SmartHaloFF.h
 *
 *  Created on: Oct 26, 2016
 *      Author: sgelinas
 */

#ifndef SRC_SMARTHALOFF_H_
#define SRC_SMARTHALOFF_H_

#define SMARTHALO_FF 1

// POWER SUPPLY
#define EN_VLED					24
#define EN_2_8V					23
#define EN_VPIEZO				22
#define VPIEZO_SENSE			5 // AIN3

// USB Charger
#define USB_CHARGING_N_PIN		20
#define USB_POWERGOOD_N_PIN		19

// USB Bridge
#define	UART_RXD_PIN			16
#define UART_TXD_PIN			14
#define UART_RTS_PIN			12
#define UART_CTS_PIN			15
#define USB_WAKEUP_N_PIN		13
#define USB_SLEEP_N_PIN			11
//#define RX_PIN_NUMBER  			UART_RXD_PIN
//#define TX_PIN_NUMBER  			UART_TXD_PIN
//#define CTS_PIN_NUMBER 			UART_CTS_PIN
//#define RTS_PIN_NUMBER 			UART_RTS_PIN

// I2C
#define I2C_SCL_PIN				31
#define I2C_SDA_PIN				30
#define I2C_SDB_PIN				4

// CENTRAL LED
#define CENTRAL_RED_PIN			29
#define CENTRAL_GREEN_PIN		27
#define CENTRAL_BLUE_PIN		28

// FRONT LED
#define	FRONTLED_PIN			3

// SOUND
#define PIEZO_DRIVE_PIN			2
#define PIEZO_VOLUME_PIN		EN_VPIEZO

// TOUCH
#define TOUCH_OUT_PIN			25
#define TOUCH_MODE_PIN			26

// BATMON
#define BATMON_ALARM_PIN		18

// ACCELERO/MAGNETO
#define ACCEL_INT_1_PIN			7
#define ACCEL_INT_2_PIN			8
#define MAG_INT_PIN				6

#endif /* SRC_SMARTHALOFF_H_ */

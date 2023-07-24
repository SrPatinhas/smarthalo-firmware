/*
 * SmartHaloEE.h
 *
 *  Created on: Jun 1, 2016
 *      Author: sgelinas
 */

#ifndef SRC_SMARTHALOEE_H_
#define SRC_SMARTHALOEE_H_

// POWER SUPPLY
#define EN_VLED					12
#define EN_2_8V					13
#define EN_VPIEZO				14
#define MCU_POWER_CYCLE			27

// USB Charger
#define USB_CHARGING_N_PIN		15
#define USB_POWERGOOD_N_PIN		17
#define USB_CHARGE_ISET2_PIN	16

// USB Bridge
#define	UART_RXD_PIN			18
#define UART_TXD_PIN			20
#define UART_RTS_PIN			23
#define UART_CTS_PIN			19
#define USB_WAKEUP_N_PIN		22
#define USB_SLEEP_N_PIN			24
//#define RX_PIN_NUMBER  			UART_RXD_PIN
//#define TX_PIN_NUMBER  			UART_TXD_PIN
//#define CTS_PIN_NUMBER 			UART_CTS_PIN
//#define RTS_PIN_NUMBER 			UART_RTS_PIN

// I2C
#define I2C_SCL_PIN				7
#define I2C_SDA_PIN				8
#define I2C_SDB_PIN				6

// CENTRAL LED
#define CENTRAL_RED_PIN			28
#define CENTRAL_GREEN_PIN		30
#define CENTRAL_BLUE_PIN		29

// FRONT LED
#define	FRONTLED_PIN			31

// SOUND
#define PIEZO_DRIVE_PIN			2
#define PIEZO_VOLUME_PIN		EN_VPIEZO

// TOUCH
#define TOUCH_OUT_PIN			26
#define TOUCH_MODE_PIN			25

// BATMON
#define BATMON_ALARM_PIN		11

// ACCELERO/MAGNETO
#define ACCEL_INT_1_PIN			3
#define ACCEL_INT_2_PIN			4
#define MAG_INT_PIN				5

#endif /* SRC_SMARTHALOEE_H_ */

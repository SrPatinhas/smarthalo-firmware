/*
 * SmartHaloEE.h
 *
 *  Created on: Oct 21, 2016
 *      Author: sgelinas
 */

#ifndef SRC_SHMP_DEVBOARD_H_
#define SRC_SHMP_DEVBOARD_H_

// POWER SUPPLY
#define EN_VLED					14
#define EN_2_8V					16
#define EN_VPIEZO				15

// USB Charger
#define USB_CHARGING_N_PIN		11
#define USB_POWERGOOD_N_PIN		12

// USB Bridge
#define	UART_RXD_PIN			24
#define UART_TXD_PIN			23
#define UART_RTS_PIN			8
#define UART_CTS_PIN			22
#define USB_WAKEUP_N_PIN		7
#define USB_SLEEP_N_PIN			6
#define RX_PIN_NUMBER  			UART_RXD_PIN
#define TX_PIN_NUMBER  			UART_TXD_PIN
#define CTS_PIN_NUMBER 			UART_CTS_PIN
#define RTS_PIN_NUMBER 			UART_RTS_PIN

// I2C
#define I2C_SCL_PIN				17
#define I2C_SDA_PIN				18
#define I2C_SDB_PIN				19

// CENTRAL LED
#define CENTRAL_RED_PIN			4
#define CENTRAL_GREEN_PIN		3
#define CENTRAL_BLUE_PIN		5

// FRONT LED
#define	FRONTLED_PIN			2

// SOUND
#define PIEZO_VINP				9
#define PIEZO_VINN				10
#define INV_IN					13
#define INV_OUT					11
#define PIEZO_DRIVE_PIN			PIEZO_VINP
#define PIEZO_VOLUME_PIN		EN_VPIEZO

// TOUCH
#define TOUCH_OUT_PIN			// TBD with peripheral boards connected (enable module in makefile)
#define TOUCH_MODE_PIN			// TBD with peripheral boards connected (enable module in makefile)

// BATMON
#define BATMON_ALARM_PIN		28 // TBD with peripheral boards connected (enable module in makefile)

// ACCELERO/MAGNETO
//#define ACCEL_INT_1_PIN		// TBD with peripheral boards connected (enable module in makefile)
//#define ACCEL_INT_2_PIN		// TBD with peripheral boards connected (enable module in makefile)
//#define MAG_INT_PIN			// TBD with peripheral boards connected (enable module in makefile)

// DEVBOARD SPECIFIC
#define LED1					26
#define LED2					27
#define LED3					12
#define LED4					11

#define BTN1					21
#define BTN2					13
#define BTN3					20
#define BTN4					25

#endif /* SRC_SHMP_DEVBOARD_H_ */

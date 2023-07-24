/*
 * PCA10040.h
 *
 *  Created on: Jun 1, 2016
 *      Author: sgelinas
 */

#ifndef SRC_PCA10040_H_
#define SRC_PCA10040_H_

#define PCA10040 1

//PIN0-1:		RESERVED XL CRYSTAL
//PIN2 - 4:		AVAILABLE
//PIN5 - 8: 	RESERVED INTERFACE MCU UART
//PIN9 - 12:	AVAILABLE
//PIN13 - 16: 	RESERVED BUTTONS 1-4
//PIN17 - 20: 	RESERVED LEDS 1-4
//PIN21: 		RESERVED RESET
//PIN22 - 25:	AVAILABLE
//PIN26 - 27:	RESERVED I2C EXT
//PIN28 - 31:	AVAILABLE


// POWER SUPPLY
#define EN_VLED					2
#define EN_2_8V					26
#define EN_VPIEZO				9 // Missing pin (same as POWER_CYCLE)
#define MCU_POWER_CYCLE			9

// USB Charger
#define USB_CHARGING_N_PIN		23
#define USB_POWERGOOD_N_PIN		25
#define USB_CHARGE_ISET2_PIN	29

// USB Bridge
#define	UART_RXD_PIN			8
#define UART_TXD_PIN			6
#define UART_RTS_PIN			5
#define UART_CTS_PIN			7
#define USB_WAKEUP_N_PIN		22
#define USB_SLEEP_N_PIN			24
//#define RX_PIN_NUMBER  			UART_RXD_PIN
//#define TX_PIN_NUMBER  			UART_TXD_PIN
//#define CTS_PIN_NUMBER 			UART_CTS_PIN
//#define RTS_PIN_NUMBER 			UART_RTS_PIN


// I2C
#define I2C_SCL_PIN				3
#define I2C_SDA_PIN				4
#define I2C_SDB_PIN				28

// CENTRAL LED
#define CENTRAL_RED_PIN			11
#define CENTRAL_GREEN_PIN		29
#define CENTRAL_BLUE_PIN		12

// FRONT LED
#define	FRONTLED_PIN			27  // LED2

// SOUND
#define PIEZO_DRIVE_PIN			13	// BUTTON1
#define PIEZO_VOLUME_PIN		28

// TOUCH
#define TOUCH_OUT_PIN			20	// BUTTON2
#define TOUCH_MODE_PIN			15 	// BUTTON3

// BATMON
#define BATMON_ALARM_PIN		24	// BUTTON4

// ACCELERO/MAGNETO
#define ACCEL_INT_1_PIN			30
#define ACCEL_INT_2_PIN			17	// LED1
#define MAG_INT_PIN				31

//LEDS Look Up Table (placeholder, BAD, from EE)
#define LEDS_LUT \
    28, 27, 29, 	/*LED12*/ \
    24, 25, 26, 	/*LED10*/ \
    21, 22, 23, 	/*LED8*/ \
    18, 19, 20, 	/*LED6*/ \
    15, 16, 17, 	/*LED4*/ \
    12, 13, 14, 	/*LED2*/ \
    11, 10,  9,		/*LED1*/ \
    49, 51, 50, 	/*LED3*/ \
    46, 48, 47, 	/*LED5*/ \
    43, 45, 44, 	/*LED7*/ \
    42, 40, 41, 	/*LED9*/ \
    38, 37, 39, 	/*LED11*/ \
    70, 72, 71, 	/*LED13*/ \
    68, 69, 67, 	/*LED16*/ \
    64, 66, 65, 	/*LED18*/ \
    61, 62, 63, 	/*LED20*/ \
    58, 59, 60, 	/*LED22*/ \
    55, 56, 57, 	/*LED24*/ \
    52, 53, 54, 	/*LED25*/ \
     5,  4,  3,		/*LED23*/ \
     8,  6,  7,		/*LED21*/ \
    36,  2,  1,		/*LED19*/ \
    33, 34, 35, 	/*LED17*/ \
    30, 31, 32, 	/*LED15*/ \

#endif /* SRC_PCA10040_H_ */

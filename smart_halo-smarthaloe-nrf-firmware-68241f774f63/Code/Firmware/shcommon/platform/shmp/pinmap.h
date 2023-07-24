/*
 * SmartHaloEE.h
 *
 *  Created on: Oct 21, 2016
 *      Author: sgelinas
 */

#ifndef SRC_SHMP_DEVBOARD_H_
#define SRC_SHMP_DEVBOARD_H_

// POWER SUPPLY
#define EN_VLED                 14
#define EN_2_8V                 16
#define EN_VPIEZO               15

// USB Charger
#define USB_CHARGING_N_PIN      11
#define USB_POWERGOOD_N_PIN     12

// USB Bridge
#define UART_RXD_PIN            24
#define UART_TXD_PIN            23
#define UART_RTS_PIN            8
#define UART_CTS_PIN            22
#define USB_WAKEUP_N_PIN        7
#define USB_SLEEP_N_PIN         6
#define RX_PIN_NUMBER           UART_RXD_PIN
#define TX_PIN_NUMBER           UART_TXD_PIN
#define CTS_PIN_NUMBER          UART_CTS_PIN
#define RTS_PIN_NUMBER          UART_RTS_PIN

// I2C
#define I2C_SCL_PIN             17
#define I2C_SDA_PIN             18
#define I2C_SDB_PIN             19

// CENTRAL LED
#define CENTRAL_RED_PIN         4
#define CENTRAL_GREEN_PIN       3
#define CENTRAL_BLUE_PIN        5

// FRONT LED
#define FRONTLED_PIN            2

// SOUND
#define PIEZO_VINP              9
#define PIEZO_VINN              10
#define INV_IN                  13
#define INV_OUT                 11
#define PIEZO_DRIVE_PIN         PIEZO_VINP
#define PIEZO_VOLUME_PIN        EN_VPIEZO

// TOUCH
#define TOUCH_OUT_PIN           // TBD with peripheral boards connected (enable module in makefile)
#define TOUCH_MODE_PIN          // TBD with peripheral boards connected (enable module in makefile)

// BATMON
#define BATMON_ALARM_PIN        28 // TBD with peripheral boards connected (enable module in makefile)

// ACCELERO/MAGNETO
#define ACCEL_INT_1_PIN       // TBD with peripheral boards connected (enable module in makefile)
#define ACCEL_INT_2_PIN       // TBD with peripheral boards connected (enable module in makefile)
#define MAG_INT_PIN           // TBD with peripheral boards connected (enable module in makefile)

// DEVBOARD SPECIFIC
#define LED1                    26
#define LED2                    27
#define LED3                    12
#define LED4                    11

#define BTN1                    21
#define BTN2                    13
#define BTN3                    20
#define BTN4                    25


//LEDS Look Up Table
#define LEDS_LUT \
    19, 20, 21,     /*LED12*/ \
    16, 17, 18,     /*LED10*/ \
    13, 14, 15,     /*LED8*/ \
    10, 11, 12,     /*LED6*/ \
     7,  8,  9,     /*LED4*/ \
     4,  6,  5,     /*LED2*/ \
    70, 72, 71,     /*LED1*/ \
    67, 69, 68,     /*LED3*/ \
    64, 66, 65,     /*LED5*/ \
    61, 63, 62,     /*LED7*/ \
    58, 60, 59,     /*LED9*/ \
    52, 54, 53,     /*LED11*/ \
    57, 56, 55,     /*LED13*/ \
    49, 51, 50,     /*LED16*/ \
    46, 48, 47,     /*LED18*/ \
    43, 45, 44,     /*LED20*/ \
    40, 42, 41,     /*LED22*/ \
    37, 39, 38,     /*LED24*/ \
     1,  2,  3,     /*LED25*/ \
    36, 35, 34,     /*LED23*/ \
    31, 32, 33,     /*LED21*/ \
    28, 29, 30,     /*LED19*/ \
    25, 26, 27,     /*LED17*/ \
    22, 23, 24,     /*LED15*/ \

#endif /* SRC_SHMP_DEVBOARD_H_ */

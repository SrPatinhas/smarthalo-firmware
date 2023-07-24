#ifndef _PINMAP_H_
#define _PINMAP_H_

#define SMARTHALO_EE 1

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

//LEDS Look Up Table
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


#endif

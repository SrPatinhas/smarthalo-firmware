/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef WT51822_H
#define WT51822_H

#define LED_START      3
#define LED_1          3
#define LED_2          4
#define LED_3          5
#define LED_STOP       5

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3

#define BUTTON_START   0
#define SW_1           0
#define SW_2           1
#define SW_3           2
#define BUTTON_STOP    2
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BSP_BUTTON_0   SW_1
#define BSP_BUTTON_1   SW_2
#define BSP_BUTTON_2   SW_3

#define BUTTONS_NUMBER 3
#define LEDS_NUMBER    3
#define BUTTONS_MASK   0x00000007

#define BSP_BUTTON_0_MASK (1<<BSP_BUTTON_0)
#define BSP_BUTTON_1_MASK (1<<BSP_BUTTON_1)
#define BSP_BUTTON_2_MASK (1<<BSP_BUTTON_2)

#define BUTTONS_LIST { SW_1, SW_2, SW_3 }
#define LEDS_LIST { LED_1, LED_2, LED_3 }

#define BSP_LED_0_MASK    (1<<LED_1)
#define BSP_LED_1_MASK    (1<<LED_2)
#define BSP_LED_2_MASK    (1<<LED_3)

#define LEDS_MASK      (BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK)
/* all LEDs are lit when GPIO is low */
#define LEDS_INV_MASK  LEDS_MASK


#define RX_PIN_NUMBER  13
#define TX_PIN_NUMBER  12
#define CTS_PIN_NUMBER 14
#define RTS_PIN_NUMBER 15
#define HWFC           true

#define SER_CON_RX_PIN              13
#define SER_CON_TX_PIN              12
#define SER_CON_CTS_PIN             14
#define SER_CON_RTS_PIN             15

#endif

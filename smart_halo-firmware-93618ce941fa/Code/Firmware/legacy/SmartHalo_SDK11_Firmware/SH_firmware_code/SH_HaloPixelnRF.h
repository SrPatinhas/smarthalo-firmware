/*
 * HaloPixelNRf.h
 *
 *  Created on: Jan 27, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_HALOPIXELNRF_H_
#define SH_HALOPIXELNRF_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>


// Choose the board to be programmed: LRTB_GVTG or LRTB_R98G
#define LRTB_R98G

// I2C 7-bit address must be right-shifted to the LSB (LSB is to indicate read/write but is unused)
#define ISSI1_ADDR         (0x78 >> 1) & (0x7F)
#define ISSI2_ADDR         (0x7E >> 1) & (0x7F)

// ISSI driver register addresses
/*#define LED_EN_REG         0x26
#define GLOBAL_EN_REG      0x4A
#define UPDATE_REG         0x25
#define SHUTDOWN_REG       0x00
#define PWM_REG            0x01
#define RESET_REG		   0x4F*/

//Maximum of bits for the buffer
#define HALOPIXEL_SEQ_WRITE_MAX		   		32

//SmartHalo specs
#define NUMBER_OF_LEDS		   			24
#define PIXELS_PER_LED					3

typedef enum ArrayPixelLed ArrayPixelLed;
enum ArrayPixelLed {ARRAY_RED_PIXEL=0, ARRAY_GREEN_PIXEL, ARRAY_BLUE_PIXEL};

//Current for the LEDs
typedef enum MaxCurrent MaxCurrent;
enum MaxCurrent {IMAX=0, IMAX_2, IMAX_3, IMAX_4};

//Set both ISSI drivers to normal operation mode
ret_code_t HaloPixel_begin();

//Sets the pixel color in the LEDs driver register for every LED
void HaloPixel_show();

//Sets the pixels colors of the n LED bit by bit
void HaloPixel_setPixelColor(uint8_t n, uint8_t r, uint8_t g, uint8_t b);

//Sets the pixels colors of the n LED bit by bit with gamma correction
void HaloPixel_setPixelColor_with_gamma_correction(uint8_t number_of_the_led, uint8_t r, uint8_t g, uint8_t b);

//Sets the color c of the n LED with a gamma correction
void HaloPixel_setColor_with_correction_gamma(uint8_t number_of_the_led, uint32_t RGB_color);

//Sets the color c of the n LED
void HaloPixel_setColor(uint8_t n, uint32_t c);

//Sets the current settings
void HaloPixel_init(uint16_t n, MaxCurrent currentSetting);

void HaloPixel_setBrightness(uint8_t b);

//Returns the color (32 bits) with RGB bytes
uint32_t HaloPixel_RGB_Color(uint8_t r, uint8_t g, uint8_t b);

//Sets the pixel color of the n LED in the LEDs driver register
void HaloPixel_showPixel(uint8_t n);

//Puts every LED color to black
void HaloPixel_clear();

#endif /* SH_HALOPIXELNRF_H_ */

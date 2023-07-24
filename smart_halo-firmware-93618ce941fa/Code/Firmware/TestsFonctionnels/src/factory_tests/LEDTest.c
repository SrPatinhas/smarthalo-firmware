/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Set of functions to test Halo LEDS, Central LEDS and Front LEDS
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#include <stdbool.h>
#include <string.h>


#include "I2C.h"
#include "pinmap.h"
#include "CommandLineInterface.h"
#include "PowerSupply.h"

#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#include "app_pwm.h"

//#include "leds.h"
#include "LEDTest.h"
// I2C 7-bit address must be right-shifted to the LSB (LSB is to indicate read/write but is unused)
#define ISSI1_ADDR         ((0x78 >> 1) & (0x7F))
#define ISSI2_ADDR         ((0x7E >> 1) & (0x7F))

static bool ISSI1_PRESENT = false;
static bool ISSI2_PRESENT = false;

typedef enum {MAX = 0, HALF = 1, THIRD = 2, FOURTH = 3} POWERMODE;

APP_PWM_INSTANCE(FR_R_PWM,2);                   // Create the instance "FR_R_PWM" using TIMER2.
APP_PWM_INSTANCE(C_GB_PWM,3);                   // Create the instance "C_GB_PWM" using TIMER3.

//extern app_pwm_t FR_R_PWM;
//extern app_pwm_t C_GB_PWM;

const uint8_t gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,254 };

POWERMODE powermode = MAX;

// Constants
#define NUMPIXELS          24

typedef enum {RGB, RBG, GBR, GRB, BRG, BGR, SPLIT} LEDORDER;

//#if (PLATFORM == pca10040)
//const unsigned char LEDDRIVERADDR[NUMPIXELS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
//                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
//const unsigned char LEDDRIVERIDX[NUMPIXELS]       = {16,13,10,7,4,1,
//                                                  34,31,28,25,22,19,
//                                                  16,13,10,7,4,1,
//                                                  34,31,28,25,22,19};
//const LEDORDER DRIVERORDER[NUMPIXELS]    =  {
//											  RGB,RGB,RGB,RGB,RGB,RGB,
//											  RGB,RGB,RGB,RGB,RGB,RGB,
//											  RGB,RGB,RGB,RGB,RGB,RGB,
//											  RGB,RGB,RGB,RGB,RGB,RGB,
//											};

#if defined(SMARTHALO_EE)
const unsigned char LEDDRIVERADDR[NUMPIXELS] = {
											   // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
											   ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
											   // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
                                               ISSI1_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
											   // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
											   // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
                                               ISSI2_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR
											  };

const unsigned char LEDDRIVERIDX[NUMPIXELS]    =  {
												  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
												  27,24,21,18,15,12,
												  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
												  9,13,10,7,4,1,
												  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
												  34,31,28,25,22,19,
												  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
												  16,3,6,1,33,30
											};
const LEDORDER DRIVERORDER[NUMPIXELS]    =  {
											  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
											  GRB,RGB,RGB,RGB,RGB,RGB,
											  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
											  BGR,RBG,RBG,RBG,GBR,GRB,
											  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
											  RBG,BRG,RBG,RGB,RGB,RGB,
											  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
											  RGB,BGR,GBR,SPLIT,RGB,RGB
											};

#elif defined(SMARTHALO_FF)
const unsigned char LEDDRIVERADDR[NUMPIXELS] = {
											   // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
											   ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
											   // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
											   // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
											   // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR
											  };

const unsigned char LEDDRIVERIDX[NUMPIXELS]    =  {
												  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
												  19,16,13,10,7,4,
												  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
												  34,31,28,25,22,16,
												  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
												  19,13,10,7,4,1,
												  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
												  1,34,31,28,25,22
											};
const LEDORDER DRIVERORDER[NUMPIXELS]    =  {
											  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
											  RGB,RGB,RGB,RGB,RGB,RBG,
											  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
											  RBG,RBG,RBG,RBG,RBG,RBG,
											  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
											  BGR,RBG,RBG,RBG,RBG,RBG,
											  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
											  RGB,BGR,RGB,RGB,RGB,RGB
											};
#endif


void LEDTest_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         LEDS");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("(0=all led, 25 = central led)");
	CommandLineInterface_printLine("led<x>r<y>G<y>B<y>:\t\tSets RGB <y> on led <x>");
	CommandLineInterface_printLine("led<x>-<z>r<y>g<y>b<y>:\tSets RGB <y> on led <x> to <z>");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("led<x>,<z>,[...]r<y>g<y>b<y>:\tSets RGB <y> on led <x>,<z>,[...]");
	CommandLineInterface_printLine("fled<x>:\t\t\tSets the front led brightness (0-255)");
	nrf_delay_ms(50);
}

bool LEDTest_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	bool invalid = false;
	bool parsed = true;
	char RxBuff2[64];
	int ledIndex1 = -2, ledIndex2 = -2, Rvalue = -1, Gvalue = -1, Bvalue = -1;
	char * pch = NULL;

	if(sscanf(RxBuff, "FLED%d", &Rvalue)==1)
	{
		if(Rvalue < 0 || Rvalue > 255)
		  CommandLineInterface_printLine("Invalid brightness value: must be between 0 to 255"), invalid = true;

		if(!invalid)
		{
		  CommandLineInterface_printf("Setting FLED to: %d\r\n", Rvalue);
		  LEDTest_setFrontLedBrightness(Rvalue);
		}
	}
	else if(sscanf(RxBuff, "LED%dR%dG%dB%d", &ledIndex1, &Rvalue, &Gvalue, &Bvalue)==4)
	{
		if(ledIndex1 < 0 || ledIndex1 > 25)
		  CommandLineInterface_printLine("Invalid LED: must be between 0 and 25. 0 will control all the leds at once"), invalid = true;
		if(Rvalue < 0 || Rvalue > 255)
		  CommandLineInterface_printLine("Invalid R value: must be between 0 to 255"), invalid = true;
		if(Gvalue < 0 || Gvalue > 255)
		  CommandLineInterface_printLine("Invalid G value: must be between 0 to 255"), invalid = true;
		if(Bvalue < 0 || Bvalue > 255)
		  CommandLineInterface_printLine("Invalid B value: must be between 0 to 255"), invalid = true;

		if(!invalid)
		{
			if(ledIndex1 == 0)
			{
			  CommandLineInterface_printf("Setting all LED to:");
			  LEDTest_showAllPixelColor(Rvalue, Gvalue, Bvalue);
			}
			else
			{
				CommandLineInterface_printf("Setting LED %d to:", ledIndex1);
				LEDTest_showPixelColor(ledIndex1-1, Rvalue, Gvalue, Bvalue);
			}
			CommandLineInterface_printf(" R %d G %d B %d\r\n", Rvalue, Gvalue, Bvalue);
		}
	}
	else if(sscanf(RxBuff, "LED%d-%dR%dG%dB%d", &ledIndex1, &ledIndex2, &Rvalue, &Gvalue, &Bvalue)==5)
	{
		if(ledIndex1 >= ledIndex2)
		{
		  CommandLineInterface_printLine("Invalid LED Range: First index must be lower then the second index"), invalid = true;
		}
		if(ledIndex1 < 1 || ledIndex1 > 25 || ledIndex2 < 1 || ledIndex2 > 25)
		  CommandLineInterface_printLine("Invalid LED Range: must be between 1 and 25"), invalid = true;
		if(Rvalue < 0 || Rvalue > 255)
		  CommandLineInterface_printLine("Invalid R value: must be between 0 to 255"), invalid = true;
		if(Gvalue < 0 || Gvalue > 255)
		  CommandLineInterface_printLine("Invalid G value: must be between 0 to 255"), invalid = true;
		if(Bvalue < 0 || Bvalue > 255)
		  CommandLineInterface_printLine("Invalid B value: must be between 0 to 255"), invalid = true;

		if(!invalid)
		 {
			CommandLineInterface_printf("Setting LED %d to %d: R %d G %d B %d\r\n", ledIndex1, ledIndex2, Rvalue, Gvalue, Bvalue);
			LEDTest_showRangePixelColor(ledIndex1-1, ledIndex2-1, Rvalue, Gvalue, Bvalue);
		 }
	}
	else if((strstr(RxBuff,",")!=NULL) && (strstr(RxBuff,"LED")!=NULL))
	{
		// save a copy
		strncpy(RxBuff2,RxBuff,sizeof(RxBuff));

		// Get the RGB values first
		pch = strtok(RxBuff2,",");

		while((pch != NULL) && (sscanf(pch,"%*dR%dG%dB%d", &Rvalue, &Gvalue, &Bvalue) != 3))
		{
		  pch = strtok(NULL,",");
		}

		if(Rvalue < 0 || Rvalue > 255)
		  CommandLineInterface_printLine("Invalid R value: must be between 0 to 255"), invalid = true;
		if(Gvalue < 0 || Gvalue > 255)
		  CommandLineInterface_printLine("Invalid G value: must be between 0 to 255"), invalid = true;
		if(Bvalue < 0 || Bvalue > 255)
		  CommandLineInterface_printLine("Invalid B value: must be between 0 to 255"), invalid = true;

		// Parse Led index per index
		pch = strtok(RxBuff,",");

		while(pch != NULL)
		{
		  sscanf(pch,"%d",&ledIndex1);

		  if(ledIndex1 < 1 || ledIndex1 > 25)
			  CommandLineInterface_printLine("Invalid LED Range: must be between 1 and 25");
		  else if(!invalid)
		  {
			  CommandLineInterface_printf("Setting LED %d to: R %d G %d B %d\r\n", ledIndex1, Rvalue, Gvalue, Bvalue);
			  LEDTest_showPixelColor(ledIndex1-1, Rvalue, Gvalue, Bvalue);
		  }

		  pch = strtok(NULL,",");
		}
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void LEDTest_HaloLEDsetup(uint8_t sdbPin)
{
	// Chip enable
	nrf_gpio_cfg_output(sdbPin);
	nrf_gpio_pin_write(sdbPin, 1);

	uint8_t buffer[2]={0,1};

	if(I2C_twi_tx(ISSI1_ADDR, buffer, 2, false) == NRF_SUCCESS)
		ISSI1_PRESENT = true;
	else
		CommandLineInterface_printLine("ISSI1 driver not detected");

	if(I2C_twi_tx(ISSI2_ADDR, buffer, 2, false) == NRF_SUCCESS)
		ISSI2_PRESENT = true;
	else
		CommandLineInterface_printLine("ISSI2 driver not detected");

	LEDTest_showRangePixelColor(0,23,0,0,0);
	LEDTest_enableAll(0);

	powermode=MAX;
}

void LEDTest_LEDPWM_setup(uint8_t frPin, uint8_t crPin, uint8_t cgPin, uint8_t cbPin)
{
	ret_code_t err_code;

	/* FLED PWM, 200Hz */
	app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000, frPin, crPin);
	app_pwm_config_t pwm2_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000, cgPin, cbPin);

	pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
	pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;
	pwm2_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
	pwm2_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;

	/* Initialize and enable PWM. */
	err_code = app_pwm_init(&FR_R_PWM,&pwm1_cfg,NULL);
	APP_ERROR_CHECK(err_code);
	app_pwm_enable(&FR_R_PWM);

	err_code = app_pwm_init(&C_GB_PWM,&pwm2_cfg,NULL);
	APP_ERROR_CHECK(err_code);
	app_pwm_enable(&C_GB_PWM);

	while (app_pwm_channel_duty_set(&FR_R_PWM, 0, 0) == NRF_ERROR_BUSY);
	while (app_pwm_channel_duty_set(&FR_R_PWM, 1, 0) == NRF_ERROR_BUSY);
	while (app_pwm_channel_duty_set(&C_GB_PWM, 0, 0) == NRF_ERROR_BUSY);
	while (app_pwm_channel_duty_set(&C_GB_PWM, 1, 0) == NRF_ERROR_BUSY);
}

void LEDTest_setup(uint8_t sdbPin, uint8_t frPin, uint8_t crPin, uint8_t cgPin, uint8_t cbPin)
{
	// Enable LED Supply
	if(!PowerSupply_getState(SUPPLY_VLED))
	{
		PowerSupply_enable(SUPPLY_VLED);
		nrf_delay_ms(100);
	}

 	LEDTest_HaloLEDsetup(sdbPin);

	LEDTest_LEDPWM_setup(frPin, crPin, cgPin, cbPin);
}

// Test function to perform a sweep, turning on R LEDs only, then G only, then B only
// 500 ms between each change
void LEDTest_sweepRGBLEDs()
{
	 //Enable or disable RGB LEDs one after the other
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, 100) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, 0) == NRF_ERROR_BUSY);

	for(int i=0;i<NUMPIXELS;i++)
	{
		LEDTest_setPixelColor(i,255,0,0);
		LEDTest_showPixel(i,1);
	}
	nrf_delay_ms(500);

	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, 0) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, 100) == NRF_ERROR_BUSY);

	for(int i=0;i<NUMPIXELS;i++)
	{
		LEDTest_setPixelColor(i,0,255,0);
		LEDTest_showPixel(i,1);
	}
	nrf_delay_ms(500);

	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, 100) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, 0) == NRF_ERROR_BUSY);

	for(int i=0;i<NUMPIXELS;i++)
	{
		LEDTest_setPixelColor(i,0,0,255);
		LEDTest_showPixel(i,1);
	}
	nrf_delay_ms(500);
}

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
void LEDTest_showPixel(uint16_t n, int show)
{
  if( n < 24)
  {
    // Powermode
    uint8_t ledreg = ((powermode & 0x03) << 1) | (show & 0x01);

    if(DRIVERORDER[n] != SPLIT )
    {
    	uint8_t buffer1[4] = {LED_EN_REG +LEDDRIVERIDX[n]-1,ledreg,ledreg,ledreg};
    	uint8_t buffer2[2] = {UPDATE_REG,0};

    	if(((LEDDRIVERADDR[n] == ISSI1_ADDR) && ISSI1_PRESENT) || ((LEDDRIVERADDR[n] == ISSI2_ADDR) && ISSI2_PRESENT))
    	{
    		I2C_twi_tx(LEDDRIVERADDR[n], buffer1, 4, false);
    		I2C_twi_tx(LEDDRIVERADDR[n], buffer2, 2, false);
    	}

    }
    else // SPLIT: special case need to write one by one
    {
    	uint8_t buffer1[2] = {LED_EN_REG,ledreg};
    	uint8_t buffer2[2] = {LED_EN_REG+1,ledreg};
    	uint8_t buffer3[2] = {LED_EN_REG+35,ledreg};
    	uint8_t buffer4[2] = {UPDATE_REG,0};

    	if(((LEDDRIVERADDR[n] == ISSI1_ADDR) && ISSI1_PRESENT) || ((LEDDRIVERADDR[n] == ISSI2_ADDR) && ISSI2_PRESENT))
		{
			I2C_twi_tx(LEDDRIVERADDR[n], buffer1, 2, false);
			I2C_twi_tx(LEDDRIVERADDR[n], buffer2, 2, false);
			I2C_twi_tx(LEDDRIVERADDR[n], buffer3, 2, false);
			I2C_twi_tx(LEDDRIVERADDR[n], buffer4, 2, false);
		}
    }
  }
}

bool check_led_drivers()
{
	uint8_t buffer[2]={0,1};
	bool is_connected = false;
	if(I2C_twi_tx(ISSI1_ADDR, buffer, 2, false) == NRF_SUCCESS)
	{}
	else
	{
		CommandLineInterface_printLine("ISSI1 driver not detected");
		is_connected =false;
	}

	if(I2C_twi_tx(ISSI2_ADDR, buffer, 2, false) == NRF_SUCCESS)
		{}
	else
	{
		CommandLineInterface_printLine("ISSI2 driver not detected");
		is_connected =false;
	}
	return 	is_connected;
}

// Default parameter enables LED on both drivers. Pass 1 to disable all LEDs
void LEDTest_enableAll(int disable)
{
  uint8_t buffer[2] = {GLOBAL_EN_REG,(uint8_t)disable};

  if(ISSI1_PRESENT)
	  I2C_twi_tx(ISSI1_ADDR, buffer, 2, false);
  if(ISSI2_PRESENT)
	  I2C_twi_tx(ISSI2_ADDR, buffer, 2, false);
}

void LEDTest_setFrontLedBrightness(uint8_t brightness)
{
	while(app_pwm_channel_duty_set(&FR_R_PWM, 0, (uint8_t)(100*((float)brightness/255))) == NRF_ERROR_BUSY); // Nordic uses duty in percent instead of integer values
}

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
void LEDTest_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  if ((n >= 0) && (n < 24))
  {
	if(((LEDDRIVERADDR[n] == ISSI1_ADDR) && ISSI1_PRESENT) || ((LEDDRIVERADDR[n] == ISSI2_ADDR) && ISSI2_PRESENT))
	{
		if(DRIVERORDER[n] == RGB)
		{
			uint8_t buffer[4] = {PWM_REG + LEDDRIVERIDX[n]-1,gamma[r],gamma[g],gamma[b]};
			I2C_twi_tx(LEDDRIVERADDR[n], buffer, 4, false);
		}
		else if(DRIVERORDER[n] == RBG)
		{
			uint8_t buffer[4] = {PWM_REG + LEDDRIVERIDX[n]-1,gamma[r],gamma[b],gamma[g]};
			I2C_twi_tx(LEDDRIVERADDR[n], buffer, 4, false);
		}
		else if(DRIVERORDER[n] == GBR)
		{
			uint8_t buffer[4] = {PWM_REG + LEDDRIVERIDX[n]-1,gamma[g],gamma[b],gamma[r]};
			I2C_twi_tx(LEDDRIVERADDR[n], buffer, 4, false);
		}
		else if(DRIVERORDER[n] == GRB)
		{
			uint8_t buffer[4] = {PWM_REG + LEDDRIVERIDX[n]-1,gamma[g],gamma[r],gamma[b]};
			I2C_twi_tx(LEDDRIVERADDR[n], buffer, 4, false);
		}
		else if(DRIVERORDER[n] == BRG)
		{
			uint8_t buffer[4] = {PWM_REG + LEDDRIVERIDX[n]-1,gamma[b],gamma[r],gamma[g]};
			I2C_twi_tx(LEDDRIVERADDR[n], buffer, 4, false);
		}
		else if(DRIVERORDER[n] == BGR)
		{
			uint8_t buffer[4] = {PWM_REG + LEDDRIVERIDX[n]-1,gamma[b],gamma[g],gamma[r]};
			I2C_twi_tx(LEDDRIVERADDR[n], buffer, 4, false);
		}
		else if(DRIVERORDER[n] == SPLIT)
		{
			uint8_t buffer1[2] = {PWM_REG, gamma[b]};
			uint8_t buffer2[2] = {PWM_REG+1, gamma[g]};
			uint8_t buffer3[2] = {PWM_REG+35, gamma[r]};

			I2C_twi_tx(LEDDRIVERADDR[n], buffer1, 2, false);
			I2C_twi_tx(LEDDRIVERADDR[n], buffer2, 2, false);
			I2C_twi_tx(LEDDRIVERADDR[n], buffer3, 2, false);
		}
	}
  }
  else if( n==24 ) // Central LED
  {
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, (uint8_t)(100*((float)gamma[r]/255))) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, (uint8_t)(100*((float)gamma[g]/255))) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, (uint8_t)(100*((float)gamma[b]/255))) == NRF_ERROR_BUSY);
  }
}

void LEDTest_showAllPixelColor(uint8_t r, uint8_t g, uint8_t b)
{
  for(int i=0;i<NUMPIXELS;i++)
  {
	  LEDTest_setPixelColor( i, r, g, b);
	  LEDTest_showPixel(i,1);
  }

  // Central LED
  while(app_pwm_channel_duty_set(&FR_R_PWM, 1, (uint8_t)(100*((float)gamma[r]/255))) == NRF_ERROR_BUSY);
  while(app_pwm_channel_duty_set(&C_GB_PWM, 0, (uint8_t)(100*((float)gamma[g]/255))) == NRF_ERROR_BUSY);
  while(app_pwm_channel_duty_set(&C_GB_PWM, 1, (uint8_t)(100*((float)gamma[b]/255))) == NRF_ERROR_BUSY);
}

void LEDTest_showPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
	LEDTest_setPixelColor(n, r, g, b);
	LEDTest_showPixel(n,1);
}

void LEDTest_showRangePixelColor(uint16_t m, uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  for(int i=m;i<=n;i++)
  {
	  LEDTest_setPixelColor(i, r, g, b);
	  LEDTest_showPixel(i,1);
  }
}

void LEDTest_showLogo()
{
	LEDTest_allOff();
	LEDTest_setPixelColor(0, 123, 77, 155);
	LEDTest_showPixel(0,1);
	LEDTest_setPixelColor(1, 87, 97, 171);
	LEDTest_showPixel(1,1);
	LEDTest_setPixelColor(2, 50, 118, 187);
	LEDTest_showPixel(2,1);
	LEDTest_setPixelColor(3, 14, 138, 203);
	LEDTest_showPixel(3,1);
	LEDTest_setPixelColor(4, 18, 145, 188);
	LEDTest_showPixel(4,1);
	LEDTest_setPixelColor(5, 22, 152, 174);
	LEDTest_showPixel(5,1);
	LEDTest_setPixelColor(6, 26, 159, 159);
	LEDTest_showPixel(6,1);
	LEDTest_setPixelColor(7, 29, 166, 144);
	LEDTest_showPixel(7,1);
	LEDTest_setPixelColor(8, 33, 173, 130);
	LEDTest_showPixel(8,1);
	LEDTest_setPixelColor(9, 37, 180, 115);
	LEDTest_showPixel(9,1);
	LEDTest_setPixelColor(10, 73, 187, 100);
	LEDTest_showPixel(10,1);
	LEDTest_setPixelColor(11, 110, 194, 86);
	LEDTest_showPixel(11,1);
	LEDTest_setPixelColor(12, 146, 201, 71);
	LEDTest_showPixel(12,1);
	LEDTest_setPixelColor(13, 182, 208, 56);
	LEDTest_showPixel(13,1);
	LEDTest_setPixelColor(14, 219, 215, 42);
	LEDTest_showPixel(14,1);
	LEDTest_setPixelColor(15, 255, 222, 27);
	LEDTest_showPixel(15,1);
	LEDTest_setPixelColor(16, 251, 188, 40);
	LEDTest_showPixel(16,1);
	LEDTest_setPixelColor(17, 247, 153, 54);
	LEDTest_showPixel(17,1);
	LEDTest_setPixelColor(18, 243, 119, 67);
	LEDTest_showPixel(18,1);
	LEDTest_setPixelColor(19, 240, 85, 80);
	LEDTest_showPixel(19,1);
	LEDTest_setPixelColor(20, 236, 50, 94);
	LEDTest_showPixel(20,1);
	LEDTest_setPixelColor(21, 232, 16, 107);
	LEDTest_showPixel(21,1);
	LEDTest_setPixelColor(22, 196, 36, 123);
	LEDTest_showPixel(22,1);
	LEDTest_setPixelColor(23, 159, 57, 139);
	LEDTest_showPixel(23,1);
}

void LEDTest_centralLedWhite()
{
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, 100) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, 100) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, 100) == NRF_ERROR_BUSY);
}

void LEDTest_centralLedOff()
{
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, 0) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, 0) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, 0) == NRF_ERROR_BUSY);
}

void LEDTest_centralLedR()
{
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, 100) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, 0) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, 0) == NRF_ERROR_BUSY);
}

void LEDTest_centralLedG()
{
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, 0) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, 100) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, 0) == NRF_ERROR_BUSY);
}

void LEDTest_centralLedB()
{
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, 0) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, 0) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, 100) == NRF_ERROR_BUSY);
}

void LEDTest_centralLedRGB(uint8_t r,uint8_t g,uint8_t b)
{
	while(app_pwm_channel_duty_set(&FR_R_PWM, 1, (uint8_t)(100*((float)gamma[r]/255))) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 0, (uint8_t)(100*((float)gamma[g]/255))) == NRF_ERROR_BUSY);
	while(app_pwm_channel_duty_set(&C_GB_PWM, 1, (uint8_t)(100*((float)gamma[b]/255))) == NRF_ERROR_BUSY);
}

void LEDTest_setAllWhite()
{
	LEDTest_allOff();
	LEDTest_centralLedWhite();

  for(int i=0;i<NUMPIXELS;i++)
  {
	  LEDTest_setPixelColor(i,255,255,255);
	  LEDTest_showPixel(i,1);
  }
}

void LEDTest_setHalfWhite()
{
	LEDTest_allOff();
	LEDTest_centralLedWhite();

  for(int i=0;i<NUMPIXELS/2;i++)
  {
	  LEDTest_setPixelColor(i,255,255,255);
	  LEDTest_showPixel(i,1);
  }
}


void LEDTest_allOff()
{
	LEDTest_centralLedOff();

  for(int i=0;i<NUMPIXELS;i++)
  {
	  LEDTest_setPixelColor(i,0,0,0);
	  LEDTest_showPixel(i,1);
  }
}

void LEDTest_updatePower()
{
  for(int i=0;i<NUMPIXELS;i++)
  {
	  LEDTest_showPixel(i,1);
  }
}

void LEDTest_smile()
{
	LEDTest_allOff();

	LEDTest_centralLedRGB(1,1,0);

  for(int i=1;i<NUMPIXELS/2-1;i++)
  {
	  LEDTest_setPixelColor(i,255,0,0);
	  LEDTest_showPixel(i,1);
  }

  LEDTest_setPixelColor(16,0,0,255);
  LEDTest_showPixel(15,1);
  LEDTest_setPixelColor(19,0,0,255);
  LEDTest_showPixel(20,1);
}


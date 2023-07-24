
#include "stdafx.h"
#include "SH_HaloPixelnRF.h"
#include "SH_TWI.h"
#include "nrf_soc.h"
#include "SH_pinMap.h"
#include "LEDTest.h"


#define RED_PIXEL							16
#define GREEN_PIXEL							8
#define SHIFT_FOR_PIXEL_COLOR				8
#define DO_UPDATE_REG						0
#define DO_RESET_REG						0
#define DO_SHUTDOWN_REG						0
#define ENABLE_ALL_LED						0
#define CLEAR_PIXEL							0
#define ARRAY_RED_PIXEL						0
#define ARRAY_GREEN_PIXEL					1
#define ARRAY_BLUE_PIXEL					2
#define RESCALING_BRIGHTNESS_MAX			255 //(8 bits)
#define SCALE_BRIGHTNESS_MAX				65535 //(32 bits)

#define SDB_PIN								I2C_SDB_PIN
#define HIGH								1

#define NUMBER_OF_DRIVERS				2

typedef enum {RGB, RBG, GBR, GRB, BRG, BGR, SPLIT} LEDORDER;

#if defined(LRTB_GVTG)

////const unsigned char DIODENAMES[NUMBER_OF_LEDS] = {D43,D42,D41,D40,D39,D38,
////                                             D37,D36,D35,D34,D33,D32,
////                                             D31,D30,D29,D28,D27,D26,
////                                             D25,D48,D47,D46,D45,D44};
//const unsigned char DRIVERADDR[NUMBER_OF_LEDS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
//                                               ISSI2_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
//const unsigned char DRIVERIDX[NUMBER_OF_LEDS]       = {19,16,13,10,7,4,
//                                                  1,34,31,28,25,22,
//                                                  19,16,13,10,7,4,
//                                                  1,34,31,28,25,22};
const unsigned char PIXELIDX[NUMBER_OF_LEDS]	  	   = {//ISSI1//
												  18,17,16,15,14,13,
												  12,11,10,9,8,7,
												  //ISSI2//
												  6,5,4,3,2,1,
												  0,23,22,21,20,19};
#elif defined(BOARD_PCA10040)
//const unsigned char DIODENAMES[NUMBER_OF_LEDS] = {LED19/42,LED20/43,LED21/44,LED22/45,LED23/46,LED24/47
//                                             LED25/48,LED26/49,LED27/50,LED28/51,LED29/52,LED6/53,
//                                             LED7/30,LED8/31,LED9/32,LED10/33,LED11/34,LED12/35,
//                                             LED13/36,LED14/37,LED15/38,LED16/39,LED17/40,LED18/41};


const unsigned char DRIVERADDR[NUMBER_OF_LEDS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
const unsigned char DRIVERIDX[NUMBER_OF_LEDS]       = {16,13,10,7,4,1,
                                                  34,31,28,25,22,19,
                                                  16,13,10,7,4,1,
                                                  34,31,28,25,22,19};
const unsigned char PIXELIDX[NUMBER_OF_LEDS]	  	   = {//ISSI1//
												  18,17,16,15,14,13,
												  12,11,10,9,8,7,
												  //ISSI2//
												  6,5,4,3,2,1,
												  0,23,22,21,20,19};
const LEDORDER LEDDRIVERORDER[NUMBER_OF_LEDS]    =  {
											  RGB,RGB,RGB,RGB,RGB,RGB,
											  RGB,RGB,RGB,RGB,RGB,RGB,
											  RGB,RGB,RGB,RGB,RGB,RGB,
											  RGB,RGB,RGB,RGB,RGB,RGB,
											};
#elif defined(SMARTHALO_EE)
const unsigned char DRIVERADDR[NUMBER_OF_LEDS] = {
											   // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
											   ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
											   // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
                                               ISSI1_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
											   // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
											   // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
                                               ISSI2_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR
											  };

const unsigned char DRIVERIDX[NUMBER_OF_LEDS]    =  {
												  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
												  27,24,21,18,15,12,
												  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
												  9,13,10,7,4,1,
												  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
												  34,31,28,25,22,19,
												  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
												  16,3,6,1,33,30
											};
const unsigned char DRIVERIDRED[NUMBER_OF_LEDS]    =  {
												  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
												  28,24,21,18,15,12,
												  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
												  11,13,10,7,6,2,
												  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
												  34,32,28,25,22,19,
												  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
												  16,5,8,36,33,30
											};
const unsigned char DRIVERIDGREEN[NUMBER_OF_LEDS]    =  {
												  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
												  27,25,22,19,16,13,
												  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
												  10,15,12,9,4,1,
												  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
												  36,33,30,26,23,20,
												  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
												  17,4,6,2,34,31
											};
const unsigned char DRIVERIDBLUE[NUMBER_OF_LEDS]    =  {
												  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
												  29,26,23,20,17,14,
												  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
												  9,14,11,8,5,3,
												  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
												  35,31,29,27,24,21,
												  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
												  18,3,7,1,35,32
											};






//const unsigned char PIXELIDX[NUMBER_OF_LEDS]	  	   = {//ISSI1//
//												  18,17,16,15,14,13,
//												  12,11,10,9,8,7,
//												  //ISSI2//
//												  6,5,4,3,2,1,

//												  0,23,22,21,20,19};
const LEDORDER LEDDRIVERORDER[NUMBER_OF_LEDS]    =  {
											  RGB,RGB,RGB,RGB,RGB,RGB,
											  RGB,RGB,RGB,RGB,RGB,RGB,
											  RGB,RGB,RGB,RGB,RGB,RGB,
											  RGB,RGB,RGB,RGB,RGB,RGB,
											};
//const LEDORDER LEDDRIVERORDER[NUMBER_OF_LEDS]    =  {
//											  // FRONT -> RIGHT: LED12,LED10,LED8,LED6,LED4,LED2
//											  GRB,RGB,RGB,RGB,RGB,RGB,
//											  // RIGHT -> BACK: LED1,LED3,LED5,LED7,LED9,LED11
//											  BGR,RBG,RBG,RBG,GBR,GRB,
//											  // BACK -> LEFT: LED13,LED16,LED18,LED20,LED22,LED24
//											  RBG,BRG,RBG,RGB,RGB,RGB,
//											  // LEFT -> FRONT: LED25,LED23,LED21,LED19,LED17,LED15
//											  RGB,SPLIT,GBR,SPLIT,RGB,RGB
//											};
#endif



/* Private functions prototypes */

//Sets the colors of a LED in the temporary register
void HaloPixel_showAllPixel(uint8_t number_of_the_led);

//RESET_REG -> reset all internal registers to default value (0 to reset)
static void HaloPixel_Reset_Register (uint8_t const * pdata);

//Sets software shutdown mode (0 to set software shutdown mode)
static void HaloPixel_ShutDown_Register (uint8_t const * pdata);

//UPDATE_REG sets all temporary values of pwm and led enable registers to permanent registers (0 to update)
static void HaloPixel_Update_Reg(uint8_t const * pdata);

//GLOBAL_REG -> sets all leds to on or off (0 for on)
static void HaloPixel_Global_Reg(uint8_t const * pdata);

//ENABLE ALL -> Puts all led to normal operation and output to current settings defined by user (0 to enable)
static void HaloPixel_enableAll(int disable);


static void HaloPixel_SDB_initialisation();

/* Variables globales priv�es */

//To know if an error has occured while transmitting to the drivers
static ret_code_t err_code_twi;


//static bool begun;          // true if begin() previously called

//Settings for the SmartHalo LEDs
static MaxCurrent currentSetting = IMAX_4;
static uint8_t pixels[NUMBER_OF_LEDS*PIXELS_PER_LED];       // Holds LED color values (3 or 4 bytes each)
static uint8_t brightness = 0;								//Brightness of LEDS on SmartHalo
static uint16_t number_of_LEDs = NUMBER_OF_LEDS;       		// Number of RGB LEDs in strip
//static uint16_t number_of_bytes = PIXELS_PER_LED * NUMBER_OF_LEDS;      		// Size of 'pixels' buffer below (3 or 4 bytes/pixel)

static const uint8_t correction_gamma[256] = {
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
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

ret_code_t HaloPixel_begin()
{
	//if(!(begun)){
	uint8_t data;

	//TO DO : INITIALIZE A PIN to HIGH for SDB
	HaloPixel_SDB_initialisation();

	//Reset Reg
	data= DO_RESET_REG;
	HaloPixel_Reset_Register(&data);

	//Shutdown Reg
	data= !(DO_SHUTDOWN_REG);
	HaloPixel_ShutDown_Register (&data);

	//Enable each LED to max current and power
	HaloPixel_enableAll(ENABLE_ALL_LED);


	APP_ERROR_CHECK(err_code_twi);

	//begun = true;}


	return err_code_twi;
}

static void HaloPixel_SDB_initialisation(){

	 nrf_gpio_cfg_output(SDB_PIN);
	 nrf_gpio_pin_write(SDB_PIN, HIGH);

}
static void HaloPixel_Reset_Register (uint8_t const * pdata){

	//Driver 1
	err_code_twi = SH_twi_write(ISSI1_ADDR, RESET_REG, pdata , 1, true);
	//Driver 2
	err_code_twi = SH_twi_write(ISSI2_ADDR, RESET_REG, pdata , 1, true);
	APP_ERROR_CHECK(err_code_twi);
}

static void HaloPixel_ShutDown_Register (uint8_t const * pdata){

	//Driver 1
	err_code_twi = SH_twi_write(ISSI1_ADDR, SHUTDOWN_REG, pdata , 1, true);
	//Driver 2
	err_code_twi = SH_twi_write(ISSI2_ADDR, SHUTDOWN_REG, pdata , 1, true);
	APP_ERROR_CHECK(err_code_twi);
}

static void HaloPixel_Update_Reg(uint8_t const * pdata){

	//Driver 1
	SH_twi_write(ISSI1_ADDR, UPDATE_REG, pdata , 1, true);
	//Driver2
	SH_twi_write(ISSI2_ADDR, UPDATE_REG, pdata , 1, true);

}

static void HaloPixel_Global_Reg(uint8_t const * pdata){

	//Driver 1
	err_code_twi = SH_twi_write(ISSI1_ADDR, PWM_REG, pdata , 1, true);
	//Driver2
	err_code_twi = SH_twi_write(ISSI2_ADDR, PWM_REG, pdata , 1, true);
	APP_ERROR_CHECK(err_code_twi);
}

void HaloPixel_show(){

	uint8_t data;

	//Sets the LEDs pixels color led by led
	for (int led=0; led<NUMBER_OF_LEDS; led++){

			HaloPixel_showAllPixel(led);

	}
	//Update Reg
	data= DO_UPDATE_REG;
	HaloPixel_Update_Reg(&data);
}

//Changes the color of the pixels of the led in the array
void HaloPixel_setPixelColor(uint8_t number_of_the_led, uint8_t r, uint8_t g, uint8_t b){

	if(number_of_the_led < number_of_LEDs)
	{
		if(brightness)
		{ // See notes in setBrightness()
			r = (r * brightness) >> SHIFT_FOR_PIXEL_COLOR;
			g = (g * brightness) >> SHIFT_FOR_PIXEL_COLOR;
			b = (b * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		}
		//uint8_t *p;
		if(LEDDRIVERORDER[number_of_the_led] == RGB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == RBG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = b;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = g;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GBR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = g;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = b;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GRB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = g;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = r;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BRG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = r;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = g;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BGR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == SPLIT)
		{
			uint8_t new_number_led = number_of_the_led;
			if(number_of_the_led ==19)
			{
				new_number_led = 21;
				//pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 19 g
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;	//good 21 b
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r; //good 19 b
			}
			else if(number_of_the_led ==21)
			{
				new_number_led = 19;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 21 g
				//pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
			}
			else
			{
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
			}
		}
	}

}
//Changes the color of the pixels of the led in the array with the gamma correction
void HaloPixel_setPixelColor_with_gamma_correction(uint8_t number_of_the_led, uint8_t r, uint8_t g, uint8_t b){

	if(number_of_the_led < number_of_LEDs)
	{
		if(brightness)
		{ // See notes in setBrightness()
			r = (r * brightness) >> SHIFT_FOR_PIXEL_COLOR;
			g = (g * brightness) >> SHIFT_FOR_PIXEL_COLOR;
			b = (b * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		}
		if(LEDDRIVERORDER[number_of_the_led] == RGB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[r];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[g];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] =correction_gamma[b];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == RBG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[r];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[b];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[g];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GBR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[g];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[b];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[r];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GRB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[g];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[r];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[b];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BRG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[b];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[r];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[g];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BGR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[b];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[g];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[r];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == SPLIT)
		{
			uint8_t new_number_led = number_of_the_led;
			if(number_of_the_led ==19)
			{
				new_number_led = 21;
				//pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 19 g
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;	//good 21 b
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r; //good 19 b
			}
			else if(number_of_the_led ==21)
			{
				new_number_led = 19;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;    //good      // R,G,B always stored
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 21 g
				//pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
			}
			else
			{
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
			}
		}

	}

}


void HaloPixel_setColor_with_correction_gamma(uint8_t number_of_the_led, uint32_t RGB_color){

	if(number_of_the_led < number_of_LEDs)
	{
		//uint8_t *copy_of_pixel;
		uint8_t r = (uint8_t)(RGB_color >> RED_PIXEL);
		uint8_t g = (uint8_t)(RGB_color >> GREEN_PIXEL);
		uint8_t b = (uint8_t)RGB_color;
		if(brightness)
		{ // See notes in setBrightness()
		  r = (r * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		  g = (g * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		  b = (b * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		}

		if(LEDDRIVERORDER[number_of_the_led] == RGB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[r];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[g];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] =correction_gamma[b];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == RBG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[r];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[b];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[g];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GBR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[g];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[b];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[r];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GRB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[g];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[r];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[b];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BRG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[b];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[r];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[g];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BGR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = correction_gamma[b];          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = correction_gamma[g];
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = correction_gamma[r];
		}
		else if(LEDDRIVERORDER[number_of_the_led] == SPLIT)
		{
			uint8_t new_number_led = number_of_the_led;
			if(number_of_the_led ==19)
			{
				new_number_led = 21;
				//pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 19 g
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;	//good 21 b
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r; //good 19 b
			}
			else if(number_of_the_led ==21)
			{
				new_number_led = 19;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 21 g
				//pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
			}
			else
			{
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
			}
		}
	}
}

//Changes the RGB color of the pixels of the led in the array
void HaloPixel_setColor(uint8_t number_of_the_led, uint32_t RGB_color)
{
	if(number_of_the_led < number_of_LEDs)
	{
		//uint8_t *copy_of_pixel;
		uint8_t r = (uint8_t)(RGB_color >> RED_PIXEL);
		uint8_t g = (uint8_t)(RGB_color >> GREEN_PIXEL);
		uint8_t b = (uint8_t)RGB_color;
		if(brightness)
		{ // See notes in setBrightness()
		  r = (r * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		  g = (g * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		  b = (b * brightness) >> SHIFT_FOR_PIXEL_COLOR;
		}

		if(LEDDRIVERORDER[number_of_the_led] == RGB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == RBG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = b;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = g;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GBR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = g;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = b;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == GRB)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = g;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = r;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BRG)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = r;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = g;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == BGR)
		{
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
			pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
		}
		else if(LEDDRIVERORDER[number_of_the_led] == SPLIT)
		{
			uint8_t new_number_led = number_of_the_led;
			if(number_of_the_led ==19)
			{
				new_number_led = 21;
				//pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 19 g
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;	//good 21 b
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r; //good 19 b
			}
			else if(number_of_the_led ==21)
			{
				new_number_led = 19;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
				pixels[new_number_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = r;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g; //good 21 g
				//pixels[new_number_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = b;
			}
			else
			{
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_RED_PIXEL] = b;          // R,G,B always stored
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_GREEN_PIXEL] = g;
				pixels[number_of_the_led * PIXELS_PER_LED + ARRAY_BLUE_PIXEL] = r;
			}
		}
  }
}

// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.  C'est la vie.
void HaloPixel_setBrightness(uint8_t brightness_level)
{
	// Stored brightness value is different than what's passed.
	// This simplifies the actual scaling math later, allowing a fast
	// 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
	// adding 1 here may (intentionally) roll over...so 0 = max brightness
	// (color values are interpreted literally; no scaling), 1 = min
	// brightness (off), 255 = just below max brightness.
	uint8_t newBrightness = brightness_level + 1;
	if(newBrightness != brightness)
	{
		// Compare against prior value
		// Brightness has changed -- re-scale existing data in RAM
		//uint8_t  color;
		//uint8_t	*ptr_of_pixels = pixels;
		uint8_t oldBrightness = brightness - 1; // De-wrap old brightness value
		uint16_t scale;

		if(oldBrightness == 0) scale = 0; // Avoid /0
		else if(brightness_level == RESCALING_BRIGHTNESS_MAX) scale = SCALE_BRIGHTNESS_MAX / oldBrightness;
		else scale = (((uint16_t)newBrightness << SHIFT_FOR_PIXEL_COLOR) - 1) / oldBrightness;
		for(uint16_t i=0; i<PIXELS_PER_LED*NUMBER_OF_LEDS; i++)
		{
			//color      = *ptr_of_pixels;
		  pixels[i] = (pixels[i] * scale) >> SHIFT_FOR_PIXEL_COLOR;
		}
		brightness = newBrightness;
	}
}

//Returns the RGB color depending on the r, g and b pixels
uint32_t HaloPixel_RGB_Color(uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint32_t)r << RED_PIXEL) | ((uint32_t)g <<  GREEN_PIXEL) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t HaloPixel_getPixelColor(uint16_t number_of_the_led) //const
{
	if(number_of_the_led >= number_of_LEDs) return 0; // Out of bounds, return no color.

	uint8_t *p;

	p = &pixels[number_of_the_led * PIXELS_PER_LED];
	if(brightness)
	{
		// Stored color was decimated by setBrightness().  Returned value
		// attempts to scale back to an approximation of the original 24-bit
		// value used when setting the pixel color, but there will always be
		// some error -- those bits are simply gone.  Issue is most
		// pronounced at low brightness levels.
		return (((uint32_t)(p[ARRAY_RED_PIXEL] << SHIFT_FOR_PIXEL_COLOR) / brightness) << RED_PIXEL) |
			 (((uint32_t)(p[ARRAY_GREEN_PIXEL] << SHIFT_FOR_PIXEL_COLOR) / brightness) <<  GREEN_PIXEL) |
			 ( (uint32_t)(p[ARRAY_BLUE_PIXEL] << SHIFT_FOR_PIXEL_COLOR) / brightness       );
	}
	else
	{
		// No brightness adjustment has been made -- return 'raw' color
		return ((uint32_t)p[ARRAY_RED_PIXEL] << RED_PIXEL) |
			 ((uint32_t)p[ARRAY_GREEN_PIXEL] <<  GREEN_PIXEL) |
			  (uint32_t)p[ARRAY_BLUE_PIXEL];
	}
}

//Puts the color of every LED to black
void HaloPixel_clear(){

	for (int led=0; led<number_of_LEDs; led++){

			HaloPixel_setPixelColor(led, CLEAR_PIXEL, CLEAR_PIXEL, CLEAR_PIXEL);
	}
}

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
void HaloPixel_showAllPixel(uint8_t number_of_the_led){

	uint8_t data;
	if(number_of_the_led < number_of_LEDs)
	{
		// Write RBG values into PWM registers
		// transmit to appropriate device

#ifdef BOARD_PCA10040
		size_t driver_address = DRIVERADDR[number_of_the_led];
		size_t led_pwm_register_address = PWM_REG + DRIVERIDX[number_of_the_led]-1;
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_RED_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address + ARRAY_RED_PIXEL, &data , 1, true);

			//Second pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_GREEN_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address + ARRAY_GREEN_PIXEL, &data , 1, true);

			//Third pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_BLUE_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address + ARRAY_BLUE_PIXEL, &data , 1, true);
#else

		size_t driver_address = DRIVERADDR[number_of_the_led];
		size_t led_pwm_register_address_red = PWM_REG + DRIVERIDRED[number_of_the_led]-1;
		size_t led_pwm_register_address_green = PWM_REG + DRIVERIDGREEN[number_of_the_led]-1;
		size_t led_pwm_register_address_blue = PWM_REG + DRIVERIDBLUE[number_of_the_led]-1;

		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_RED_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address_red , &data , 1, true);

			//Second pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_GREEN_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address_green , &data , 1, true);

			//Third pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_BLUE_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address_blue , &data , 1, true);
#endif
	}

}

// HELPER FUNCTIONS

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
//Sets the color of the LED and updates the registers
void HaloPixel_showPixel(uint8_t number_of_the_led)
{
	uint8_t data;
	if(number_of_the_led < number_of_LEDs)
	{
		// Write RBG values into PWM registers
		// transmit to appropriate device

#ifdef BOARD_PCA10040
		size_t driver_address = DRIVERADDR[number_of_the_led];
		size_t led_pwm_register_address = PWM_REG + DRIVERIDX[number_of_the_led]-1;
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_RED_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address + ARRAY_RED_PIXEL, &data , 1, true);

			//Second pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_GREEN_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address + ARRAY_GREEN_PIXEL, &data , 1, true);

			//Third pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_BLUE_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address + ARRAY_BLUE_PIXEL, &data , 1, true);
#else

		size_t driver_address = DRIVERADDR[number_of_the_led];
		size_t led_pwm_register_address_red = PWM_REG + DRIVERIDRED[number_of_the_led]-1;
		size_t led_pwm_register_address_green = PWM_REG + DRIVERIDGREEN[number_of_the_led]-1;
		size_t led_pwm_register_address_blue = PWM_REG + DRIVERIDBLUE[number_of_the_led]-1;

		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_RED_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address_red , &data , 1, true);

			//Second pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_GREEN_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address_green , &data , 1, true);

			//Third pixel
		data = (pixels[PIXELS_PER_LED*number_of_the_led+ARRAY_BLUE_PIXEL]);
		SH_twi_write(driver_address, led_pwm_register_address_blue , &data , 1, true);
#endif
	}
	  	//sd_nvic_critical_region_exit (false);

		data= DO_UPDATE_REG;
	HaloPixel_Update_Reg(&data);


}

//Enable all LEDs
static void HaloPixel_enableAll(int disable){

	uint8_t data;
	uint8_t LEDenable = !(disable);
	LEDenable |= currentSetting << 1; // Set current limit

	// Individual enables by memory space (not pixel number)
	//Driver 1
	for(int led=0; led<NUMBER_OF_LEDS; led++)
	{
		size_t driver_address = DRIVERADDR[led];
		size_t led_reg_register_address = LED_EN_REG + DRIVERIDX[led]-1;
		data= LEDenable;

		//First pixel
		err_code_twi = SH_twi_write(driver_address, led_reg_register_address + ARRAY_RED_PIXEL, &data , 1, true);
		//Second pixel
		err_code_twi = SH_twi_write(driver_address, led_reg_register_address + ARRAY_GREEN_PIXEL, &data , 1, true);
		//Third pixel
		err_code_twi = SH_twi_write(driver_address, led_reg_register_address + ARRAY_BLUE_PIXEL, &data , 1, true);

	}
#ifdef SMARTHALO_EE
	size_t driver_address = DRIVERADDR[21];
	size_t led_reg_register_address = LED_EN_REG + 35;
	data= LEDenable;

	//First pixel
	err_code_twi = SH_twi_write(driver_address, led_reg_register_address, &data , 1, true);

	driver_address = DRIVERADDR[19];
	led_reg_register_address = LED_EN_REG + 1;
	data= LEDenable;

	//First pixel
	err_code_twi = SH_twi_write(driver_address, led_reg_register_address, &data , 1, true);

	driver_address = DRIVERADDR[19];
	led_reg_register_address = LED_EN_REG ;
	data= LEDenable;

	//First pixel
	err_code_twi = SH_twi_write(driver_address, led_reg_register_address, &data , 1, true);
	//Second pixel
#endif

	//Update Reg
	data= DO_UPDATE_REG;
	HaloPixel_Update_Reg(&data);

	//Global Reg
	data=ENABLE_ALL_LED;
	HaloPixel_Global_Reg(&data);

}

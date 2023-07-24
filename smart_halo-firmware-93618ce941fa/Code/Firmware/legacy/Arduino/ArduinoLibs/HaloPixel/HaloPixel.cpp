#include "HaloPixel.h"

#include <Wire.h>

// Choose the board to be programmed: LRTB_GVTG or LRTB_R98G
#define LRTB_R98G

// I2C 7-bit address must be right-shifted to the LSB (LSB is to indicate read/write but is unused)
#define ISSI1_ADDR         (0x78 >> 1) & (0x7F)
#define ISSI2_ADDR         (0x7E >> 1) & (0x7F)

// ISSI driver register addresses
#define LED_EN_REG         0x26
#define GLOBAL_EN_REG      0x4A
#define UPDATE_REG         0x25
#define SHUTDOWN_REG       0x00
#define PWM_REG            0x01
#define RESET_REG		   0x4F

#define NUMPIXELS		   24
											  
#if defined(LRTB_GVTG)
////const unsigned char DIODENAMES[NUMPIXELS] = {D43,D42,D41,D40,D39,D38, 
////                                             D37,D36,D35,D34,D33,D32, 
////                                             D31,D30,D29,D28,D27,D26, 
////                                             D25,D48,D47,D46,D45,D44};
//const unsigned char DRIVERADDR[NUMPIXELS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
//                                               ISSI2_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
//const unsigned char DRIVERIDX[NUMPIXELS]       = {19,16,13,10,7,4,
//                                                  1,34,31,28,25,22,
//                                                  19,16,13,10,7,4,
//                                                  1,34,31,28,25,22};
const unsigned char PIXELIDX[NUMPIXELS]	  	   = {//ISSI1//
												  18,17,16,15,14,13,
												  12,11,10,9,8,7,
												  //ISSI2//
												  6,5,4,3,2,1,
												  0,23,22,21,20,19};
#elif defined(LRTB_R98G)
//const unsigned char DIODENAMES[NUMPIXELS] = {LED19/42,LED20/43,LED21/44,LED22/45,LED23/46,LED24/47 
//                                             LED25/48,LED26/49,LED27/50,LED28/51,LED29/52,LED6/53, 
//                                             LED7/30,LED8/31,LED9/32,LED10/33,LED11/34,LED12/35, 
//                                             LED13/36,LED14/37,LED15/38,LED16/39,LED17/40,LED18/41};
const unsigned char DRIVERADDR[NUMPIXELS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
const unsigned char DRIVERIDX[NUMPIXELS]       = {16,13,10,7,4,1,
                                                  34,31,28,25,22,19,
                                                  16,13,10,7,4,1,
                                                  34,31,28,25,22,19};       
const unsigned char PIXELIDX[NUMPIXELS]	  	   = {//ISSI1//
												  18,17,16,15,14,13,
												  12,11,10,9,8,7,
												  //ISSI2//
												  6,5,4,3,2,1,
												  0,23,22,21,20,19};												  
#endif												  

HaloPixel::HaloPixel(uint16_t n, MaxCurrent currentSetting) :
brightness(0), pixels(NULL), endTime(0), begun(false), currentSetting(currentSetting)
{
	updateLength(n);
}

HaloPixel::HaloPixel() :
brightness(0), pixels(NULL), endTime(0), begun(false), numLEDs(0), numBytes(0), currentSetting(IMAX)
{
}

HaloPixel::~HaloPixel()
{
	if(pixels)   free(pixels);
}
	
void HaloPixel::begin()
{
	//Set both ISSI drivers to normal operation mode
	Wire.begin();
	Wire.beginTransmission(ISSI1_ADDR);
	Wire.write(RESET_REG);
	Wire.write(0);
	Wire.endTransmission();
	Wire.beginTransmission(ISSI2_ADDR);
	Wire.write(RESET_REG);
	Wire.write(0);
	Wire.endTransmission();
	Wire.beginTransmission(ISSI1_ADDR);
	Wire.write(SHUTDOWN_REG);
	Wire.write(1);
	Wire.endTransmission();
	Wire.begin();
	Wire.beginTransmission(ISSI2_ADDR);
	Wire.write(SHUTDOWN_REG);
	Wire.write(1);
	Wire.endTransmission();
	enableAll();
	begun = true;
}

void HaloPixel::show()
{
	if(!pixels) return;

	// // Assigning pixel 1 by 1 regardless of their location in memory and associated driver
	// for(int i=0;i<numLEDs;i++)
	// {
		// // Write RBG values into PWM registers
		// Wire.beginTransmission(DRIVERADDR[i]); // transmit to appropriate device
		// // n is rgb pixel index [0-11]
		// Wire.write(PWM_REG + DRIVERIDX[i]-1); // R LED Address. Autoincrements to G then B on successive writes
		// Wire.write(pixels[3*i]);
		// Wire.write(pixels[3*i+1]);
		// Wire.write(pixels[3*i+2]);
		// Wire.endTransmission();    // stop transmitting		
	// }
	
	// Assigning pixel 1 by 1 driver per driver, in increasing memory space to optimise I2C comm
	Wire.beginTransmission(ISSI1_ADDR);
	Wire.write(PWM_REG);
	
	for(int i=0;i<numLEDs/2;i++)
	{		
		if(i==10) Wire.endTransmission(), Wire.beginTransmission(ISSI1_ADDR), Wire.write(PWM_REG + 3*i);// RSTART cause buffer is full
		

		Wire.write(pixels[3*PIXELIDX[i]]);	// R
		Wire.write(pixels[3*PIXELIDX[i]+1]);	// G
		Wire.write(pixels[3*PIXELIDX[i]+2]);	// B		
	}
	
	Wire.write(0); // UPDATE_REG
	Wire.endTransmission();    // stop transmitting	
	
	Wire.beginTransmission(ISSI2_ADDR);
	Wire.write(PWM_REG);
	
	for(int i=numLEDs/2;i<numLEDs;i++)
	{		
		if(i==(numLEDs/2+10)) Wire.endTransmission(), Wire.beginTransmission(ISSI2_ADDR), Wire.write(PWM_REG + 3*(i-numLEDs/2)); // RSTART cause buffer is full
		
		
		Wire.write(pixels[3*PIXELIDX[i]]);	// R
		Wire.write(pixels[3*PIXELIDX[i]+1]);	// G
		Wire.write(pixels[3*PIXELIDX[i]+2]);	// B		
	}
	
	Wire.write(0); // UPDATE_REG
	Wire.endTransmission();    // stop transmitting
}

void HaloPixel::setPixelColor(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
	if(n < numLEDs) 
	{
		if(brightness) 
		{ // See notes in setBrightness()
			r = (r * brightness) >> 8;
			g = (g * brightness) >> 8;
			b = (b * brightness) >> 8;
		}
		uint8_t *p;
		
		p = &pixels[n * 3];    // 3 bytes per pixel
		
		p[0] = r;          // R,G,B always stored
		p[1] = g;
		p[2] = b;
	}
}

void HaloPixel::setPixelColor(uint8_t n, uint32_t c)
{
	if(n < numLEDs) 
	{
		uint8_t *p;
		uint8_t r = (uint8_t)(c >> 16);
		uint8_t g = (uint8_t)(c >>  8);
		uint8_t b = (uint8_t)c;
		if(brightness) 
		{ // See notes in setBrightness()
		  r = (r * brightness) >> 8;
		  g = (g * brightness) >> 8;
		  b = (b * brightness) >> 8;
		}
		
		p = &pixels[n * 3];
		
		p[0] = r;
		p[1] = g;
		p[2] = b;
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
void HaloPixel::setBrightness(uint8_t b)
{
	// Stored brightness value is different than what's passed.
	// This simplifies the actual scaling math later, allowing a fast
	// 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
	// adding 1 here may (intentionally) roll over...so 0 = max brightness
	// (color values are interpreted literally; no scaling), 1 = min
	// brightness (off), 255 = just below max brightness.
	uint8_t newBrightness = b + 1;
	if(newBrightness != brightness) 
	{ 
		// Compare against prior value
		// Brightness has changed -- re-scale existing data in RAM
		uint8_t  c;
		uint8_t	*ptr = pixels;
		uint8_t oldBrightness = brightness - 1; // De-wrap old brightness value		
		uint16_t scale;
		
		if(oldBrightness == 0) scale = 0; // Avoid /0
		else if(b == 255) scale = 65535 / oldBrightness;
		else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
		for(uint16_t i=0; i<numBytes; i++) 
		{
		  c      = *ptr;
		  *ptr++ = (c * scale) >> 8;
		}
		brightness = newBrightness;
	}
}

void HaloPixel::clear()
{
	 memset(pixels, 0, numBytes);
}

void HaloPixel::updateLength(uint16_t n)
{
	if(pixels) free(pixels); // Free existing data (if any)

	// Allocate new data -- note: ALL PIXELS ARE CLEARED
	numBytes = n * 3;
	if((pixels = (uint8_t *)malloc(numBytes))) 
	{
		memset(pixels, 0, numBytes);
		numLEDs = n;
	} else 
	{
		numLEDs = numBytes = 0;
	}
}

uint8_t* HaloPixel::getPixels(void) const
{
	return pixels;
}

uint8_t HaloPixel::getBrightness(void) const
{
	return brightness - 1;
}

uint16_t HaloPixel::numPixels(void) const
{
	return numLEDs;
}

uint32_t HaloPixel::Color(uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t HaloPixel::getPixelColor(uint16_t n) const
{	
	if(n >= numLEDs) return 0; // Out of bounds, return no color.

	uint8_t *p;
	
	p = &pixels[n * 3];
	if(brightness)
	{
		// Stored color was decimated by setBrightness().  Returned value
		// attempts to scale back to an approximation of the original 24-bit
		// value used when setting the pixel color, but there will always be
		// some error -- those bits are simply gone.  Issue is most
		// pronounced at low brightness levels.
		return (((uint32_t)(p[0] << 8) / brightness) << 16) |
			 (((uint32_t)(p[1] << 8) / brightness) <<  8) |
			 ( (uint32_t)(p[2] << 8) / brightness       );
	} 
	else 
	{
		// No brightness adjustment has been made -- return 'raw' color
		return ((uint32_t)p[0] << 16) |
			 ((uint32_t)p[1] <<  8) |
			  (uint32_t)p[2];
	}
}
	
// HELPER FUNCTIONS
	
// Default parameter enables LED on both drivers. Pass 1 to disable all LEDs
void HaloPixel::enableAll(int disable)
{		
	uint8_t LEDenable = not(disable);
	LEDenable |= currentSetting << 1; // Set current limit
	
	// I2C Buffer limited to 32B : Need to restart

	Wire.beginTransmission(ISSI1_ADDR);    // Enable LED @ Max Current
	Wire.write(LED_EN_REG);
	
	// Individual enables by memory space (not pixel number)	
	for(int i=0;i<NUMPIXELS/2;i++)
	{		 
		if(i==10) Wire.endTransmission(), Wire.beginTransmission(ISSI1_ADDR), Wire.write(LED_EN_REG + 3*i); // RSTART cause buffer is full
				
		Wire.write(LEDenable);		
		Wire.write(LEDenable);
		Wire.write(LEDenable);
	}
	
	Wire.write(disable); // GLOBAL_EN_REG
	Wire.endTransmission();
	
	Wire.beginTransmission(ISSI2_ADDR);    // Enable LED @ Max Current
	Wire.write(LED_EN_REG);
	
	// Individual enables by memory space (not pixel number)	
	for(int i=0;i<NUMPIXELS/2;i++)
	{		 
		if(i==10) Wire.endTransmission(), Wire.beginTransmission(ISSI2_ADDR), Wire.write(LED_EN_REG + 3*i); // RSTART cause buffer is full
		
		
		Wire.write(LEDenable);
		Wire.write(LEDenable);
		Wire.write(LEDenable);		
	}
	
	Wire.write(disable); // GLOBAL_EN_REG
	Wire.endTransmission();
}

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
void HaloPixel::showPixel(uint8_t n)
{
	if(n < numLEDs) 
	{
	  // Write RBG values into PWM registers
	  Wire.beginTransmission(DRIVERADDR[n]); // transmit to appropriate device
	  // n is rgb pixel index [0-11]
	  Wire.write(PWM_REG + DRIVERIDX[n]-1); // R LED Address. Autoincrements to G then B on successive writes
	  Wire.write(pixels[3*n]);
	  Wire.write(pixels[3*n+1]);
	  Wire.write(pixels[3*n+2]);
	  Wire.endTransmission();    // stop transmitting	
	  Wire.beginTransmission(DRIVERADDR[n]);
	  Wire.write(UPDATE_REG);                    // Update registers
	  Wire.write(0);
	  Wire.endTransmission();  
	}
}
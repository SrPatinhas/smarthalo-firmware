#ifndef HALOPIXEL_H
#define HALOPIXEL_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

class HaloPixel
{
	public:
	
		enum MaxCurrent {IMAX=0, IMAX_2, IMAX_3, IMAX_4};
	
		HaloPixel(uint16_t n, MaxCurrent currentSetting=IMAX_4);
		HaloPixel();
		~HaloPixel();
	
		void begin();
		void show();
		void showPixel(uint8_t n);
		void setPixelColor(uint8_t n, uint8_t r, uint8_t g, uint8_t b);
		void setPixelColor(uint8_t n, uint32_t c);
		void setBrightness(uint8_t b);
		void clear();
		void updateLength(uint16_t n);
		
		uint8_t* getPixels(void) const;
		uint8_t getBrightness(void) const;
		uint16_t numPixels(void) const;
		static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
		uint32_t getPixelColor(uint16_t n) const;
		
	private:
	
		void enableAll(int disable=0);		
		
	private:
	
		boolean begun;          // true if begin() previously called
		uint16_t numLEDs;       // Number of RGB LEDs in strip
		uint16_t numBytes;      // Size of 'pixels' buffer below (3 or 4 bytes/pixel)		
		uint8_t brightness;
	    uint8_t *pixels;        // Holds LED color values (3 or 4 bytes each)		
		uint32_t endTime;       // Latch timing reference
		MaxCurrent currentSetting;
};

#endif 					// HALOPIXEL_H
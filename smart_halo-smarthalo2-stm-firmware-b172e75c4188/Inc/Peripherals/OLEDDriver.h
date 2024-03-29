/*!@file    OLEDDriver.h

 */
// ------------------------------------------------------------------------------------------------


#ifndef __OLEDDRIVER_H_
#define __OLEDDRIVER_H_

#include "main.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define SSD1305_LCDWIDTH                  128
#define SSD1305_LCDHEIGHT                 64
#define SIZEOF_BUFFER_OLED (SSD1305_LCDHEIGHT * SSD1305_LCDWIDTH / 8)
#define STARTLINE (SIZEOF_BUFFER_OLED - 128)

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================



// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
bool init_OLEDDriver(void);
bool setState_OLEDDriver(bool state);
bool display_OLEDDriver(uint8_t * buffer);
bool clearDisplay_OLEDDriver(void);
bool getBuffer_OLEDDriver(uint8_t * buffer);
bool getBufferLength_OLEDDriver(uint32_t * bufferLength);
bool setConstrast_OLEDDriver(uint8_t value);
bool setBrightness_OLEDDriver(uint8_t value);
bool setDisplayOff_OLEDDriver(void);

#endif  /* HALOLEDSDRIVER_H_ */

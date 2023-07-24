/*
 * OLEDDriver.c
 *
 *  Created on: July 03
 *      Author: JF Lemire
 */

#include <SystemUtilitiesTask.h>
#include "OLEDDriver.h"
#include "spi.h"
#include "reboot.h"
// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define SSD1305_SETLOWCOLUMN 0x00
#define SSD1305_SETHIGHCOLUMN 0x10
#define SSD1305_MEMORYMODE 0x20
#define SSD1305_SETCOLADDR 0x21
#define SSD1305_SETPAGEADDR 0x22
#define SSD1305_SETSTARTLINE 0x40

#define SSD1305_SETCONTRAST 0x81
#define SSD1305_SETBRIGHTNESS 0x82

#define SSD1305_SETLUT 0x91

#define SSD1305_SEGREMAP 0xA0
#define SSD1305_DISPLAYALLON_RESUME 0xA4
#define SSD1305_DISPLAYALLON 0xA5
#define SSD1305_NORMALDISPLAY 0xA6
#define SSD1305_INVERTDISPLAY 0xA7
#define SSD1305_SETMULTIPLEX 0xA8
#define SSD1305_DISPLAYDIM 0xAC
#define SSD1305_MASTERCONFIG 0xAD
#define SSD1305_DISPLAYOFF 0xAE
#define SSD1305_DISPLAYON 0xAF

#define SSD1305_SETPAGESTART 0xB0

#define SSD1305_COMSCANINC 0xC0
#define SSD1305_COMSCANDEC 0xC8
#define SSD1305_SETDISPLAYOFFSET 0xD3
#define SSD1305_SETDISPLAYCLOCKDIV 0xD5
#define SSD1305_SETAREACOLOR 0xD8
#define SSD1305_SETPRECHARGE 0xD9
#define SSD1305_SETCOMPINS 0xDA
#define SSD1305_SETVCOMLEVEL 0xDB
// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================
#define adagfx_swap(a, b) { uint8_t t = a; a = b; b = t; }

#define BLACK 0
#define WHITE 1

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// ================================================================================================
// ================================================================================================
//            PRIVATE ENUM DEFINITION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================
#define _ENABLE_OLED_VCC() HAL_GPIO_WritePin(OLED_VCC_PWR_EN_GPIO_Port, OLED_VCC_PWR_EN_Pin, GPIO_PIN_RESET)
#define _DISABLE_OLED_VCC() HAL_GPIO_WritePin(OLED_VCC_PWR_EN_GPIO_Port, OLED_VCC_PWR_EN_Pin, GPIO_PIN_SET)

#define _ENABLE_OLED_VDD() HAL_GPIO_WritePin(OLED_VDD_PWR_EN_GPIO_Port, OLED_VDD_PWR_EN_Pin, GPIO_PIN_SET)
#define _DISABLE_OLED_VDD() HAL_GPIO_WritePin(OLED_VDD_PWR_EN_GPIO_Port, OLED_VDD_PWR_EN_Pin, GPIO_PIN_RESET)

#define _OLED_CS_HIGH() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)
#define _OLED_CS_LOW() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)

#define _OLED_MODE_DATA() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
#define _OLED_MODE_CMD() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)

#define _OLED_RESET_HIGH() HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET)
#define _OLED_RESET_LOW() HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET)

// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
bool oledDriverCommand(uint8_t c);
bool oledDriverWriteDatas(uint8_t *data, uint16_t length, bool mode);
bool oledDriverDisplaySinglePage(uint8_t * buffer, uint8_t page, uint32_t length);
// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================
/**
 * @brief       Initialize the OLED
 * @details     Configure the OLED as per the datasheet
 * @return      bool: true if success, false otherwise.
 */
bool init_OLEDDriver(void)
{
    // Init sequence for 128x64 OLED module
    if (oledDriverCommand(0xAE) == false) return false;     //Set Display Off
    if (oledDriverCommand(0xd5) == false) return false;     //display divide ratio/osc. freq. mode
    if (oledDriverCommand(0x50) == false) return false;     //
    if (oledDriverCommand(0xA8) == false) return false;     //multiplex ration mode:63
    if (oledDriverCommand(0x3F) == false) return false;
    if (oledDriverCommand(0xD3) == false) return false;     //Set Display Offset
    if (oledDriverCommand(0x00) == false) return false;
    if (oledDriverCommand(0x40) == false) return false;     //Set Display Start Line
    if (oledDriverCommand(0xAD) == false) return false;     //DC-DC Control Mode Set
    if (oledDriverCommand(0x8A) == false) return false;     //DC-DC ON/OFF Mode Set
    if (oledDriverCommand(0xA1) == false) return false;     //Segment Remap
    if (oledDriverCommand(0xC8) == false) return false;     //Sst COM Output Scan Direction
    if (oledDriverCommand(0xdc) == false) return false;	  //row non-overlap /SEG Hiz Period Set
    if (oledDriverCommand(0x01) == false) return false;
    if (oledDriverCommand(0xDA) == false) return false;     //common pads hardware: alternative
    if (oledDriverCommand(0x12) == false) return false;
    if (oledDriverCommand(0x82) == false) return false;     //brightness control
    if (oledDriverCommand(0x7F) == false) return false;     //127 is 100%
    if (oledDriverCommand(0x81) == false) return false;     //contrast control
    if (oledDriverCommand(0x7F) == false) return false;     //127 is 50% which has the best appearance
    if (oledDriverCommand(0xd7) == false) return false;
    if (oledDriverCommand(0xD9) == false) return false;     //set pre-charge period
    if (oledDriverCommand(0x22) == false) return false;     //
    if (oledDriverCommand(0xDB) == false) return false;     //VCOM deselect level mode
    if (oledDriverCommand(0x2a) == false) return false;     //
    if (oledDriverCommand(0xA4) == false) return false;     //Set Entire Display On/Off
    if (oledDriverCommand(0xA6) == false) return false;     //Set Normal Display
    if (oledDriverCommand(0xAF) == false) return false;     //Set Display On
    return true;
}

/**
 * @brief       Set the state of the OLED
 * @details     Configure the state of the OLED, on or off
 * @param[in]	state: state to set
 * @return      bool: true if success, false otherwise.
 */
bool setState_OLEDDriver(bool state)
{
    if (state == true)
    {
        _DISABLE_OLED_VCC(); // Just in case...
        _ENABLE_OLED_VDD();	// Enable 2.8V
        _OLED_CS_HIGH();
        _OLED_MODE_CMD();	// D/C Low -> Command
        _OLED_RESET_LOW();	// Reset
        vTaskDelay(50);
        _OLED_RESET_HIGH();	// Out of Reset
        _ENABLE_OLED_VCC();	// Enable 12.5V
        vTaskDelay(100);
        init_OLEDDriver();
        clearDisplay_OLEDDriver();
    }
    else
    {
        oledDriverCommand(SSD1305_DISPLAYOFF);	//Send command AEh for display OFF.
        vTaskDelay(1);
        _DISABLE_OLED_VCC();	// Disable 12.5V
        vTaskDelay(100);
        _DISABLE_OLED_VDD();	// Disable 2.8V
    }
    return true;
}

/*! @brief  Convert src buffer from png to something the OLED can use
    @param[in]  src pointer to pixel data in png(?) format
    @param[out] dst pointer to an output buffer, suitable for passing to the OLED driver
 */
static void convertBuf(const uint8_t *src, uint8_t *dst)
{
    const int nrows = SSD1305_LCDHEIGHT / 8;
    const int ncols = SSD1305_LCDWIDTH;
    int row, col, srcidx;
    for (int i = 0; i < SIZEOF_BUFFER_OLED; i++) {
        row = i / ncols;
        col = i % ncols;
        srcidx = col * nrows + row;
        // consoleSend_Shell("%s: i: %d, srcidx: %d", __func__, i, srcidx);
        if (srcidx >= SIZEOF_BUFFER_OLED) {
            log_Shell("%s: index out of range, i: %d, srcidx: %d", __func__, i, srcidx);
            continue;
        }
        dst[i] = src[srcidx];
    }
}

/**
 * @brief       Pass an array of bytes to display on the OLED
 * @details     Display is 8 pages of 128 bytes
 * @param[in]	buffer: buffer to display
 * @return      bool: true if success, false otherwise.
 */
bool display_OLEDDriver(uint8_t * originalBuffer)
{
    uint8_t u8Page = 0;

    static uint8_t buffer[8*128];
    convertBuf(originalBuffer, buffer);

    if (buffer == NULL) return false;

    if (oledDriverCommand(0xaf) == false) return false;
    if (oledDriverCommand(0x40) == false) return false;

    for (; u8Page < 8; u8Page++)
    {
        if (oledDriverDisplaySinglePage(&(buffer[u8Page * 128]), u8Page, 128) == false) return false;
    }
    return true;
}
/**
 * @brief       Clear the OLED Display
 * @details     Clear all 8 pages
 * @return      bool: true if success, false otherwise.
 */
bool clearDisplay_OLEDDriver(void)
{
    uint8_t u8Page = 0;
    static uint8_t u8Buffer[128] = { 0 };
    if (oledDriverCommand(0xaf) == false) return false;
    if (oledDriverCommand(0x40) == false) return false;

    for (; u8Page < 8; u8Page++)
    {
        if (oledDriverDisplaySinglePage(u8Buffer, u8Page, 128) == false) return false;
    }

    _OLED_CS_HIGH();

    return true;
}
/**
 * @brief       Set the OLED Contrast
 * @details     Set and new OLED Contrast values between 0-255
 * @param[in]	value: constrast value
 * @return      bool: true if success, false otherwise.
 */
bool setConstrast_OLEDDriver(uint8_t value)
{
    if (oledDriverCommand(SSD1305_SETCONTRAST) == false) return false;
    if (oledDriverCommand(value) == false) return false;
    return true;
}
/**
 * @brief       Set the OLED Brightness
 * @details     Set the new OLED Brightness values between 0-255
 * @param[in]	value: Brightness value
 * @return      bool: true if success, false otherwise.
 */
bool setBrightness_OLEDDriver(uint8_t value)
{
    if (oledDriverCommand(SSD1305_SETBRIGHTNESS) == false) return false;
    if (oledDriverCommand(value) == false) return false;
    return true;
}
/**
 * @brief       Turn the OLED off
 * @details     Function to set display off.
 * @return      bool: true if success, false otherwise.
 */
bool setDisplayOff_OLEDDriver(void)
{
    if (oledDriverCommand(SSD1305_DISPLAYOFF) == false) return false;
    return true;
}
/**
 * @brief       OLEDDriverWriteData()
 * @details     Function to write data in SPI.
 * @public
 * @param[in]	pu8Data: buffer to write
 * @param[in]	u16Length: number of byte to write
 * @param[in]	bModeCmd: true is to set mode, false is data.
 * @return      bool: true if success, false otherwise.
 */
bool OLEDDriverWriteData(uint8_t *data, uint16_t length, bool bModeCmd)
{
    bool bResponse = false;
    if (data == false) return false;

    if (bModeCmd == false)
    {
        _OLED_MODE_DATA();
    }
    else
    {
        _OLED_MODE_CMD();
    }

    HAL_SPI_Write_DMA(&OLED_SPI, OLED_CS_GPIO_Port, OLED_CS_Pin,data , length, &bResponse);

    if (bResponse == false)
    {
        log_Shell("oups... spi oled");
#ifdef MEGABUG_HUNT
        while (1);
#else
        SOFT_CRASH(eOLEDDRIVER);
#endif
    }

    return true;
}
/**
 * @brief       OLEDDriverCommand()
 * @details     Function to write a command to the display
 * @public
 * @param[in]	u8Command: command.
 * @return      bool: true if success, false otherwise.
 */
bool oledDriverCommand(uint8_t u8Command)
{
    return OLEDDriverWriteData(&u8Command, 1, 1);
}
/**
 * @brief       OLEDDriverDisplaySinglePage()
 * @details     Function to display a single page (part a the complet buffer).
 * @public
 * @param[in]	pu8Buffer: buffer to display
 * @param[in]	u8Page: page to display
 * @param[in]	u32Length: numbe of byte to display.
 * @return      bool: true if success, false otherwise.
 */
bool oledDriverDisplaySinglePage(uint8_t * buffer, uint8_t page, uint32_t length)
{
    if (oledDriverCommand(SSD1305_SETPAGESTART + page) == false) return false;
    if (oledDriverCommand(0x00) == false) return false;
    if (oledDriverCommand(0x10) == false) return false;

    return OLEDDriverWriteData(buffer, length, 0);
}

/*
 * ST7701_driver.h
 *
 *  Created on: Nov. 29, 2020
 *      Author: Sean
 */

#ifndef DRIVERS_ST7701_DRIVER_H_
#define DRIVERS_ST7701_DRIVER_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"


 /*list of spi commands for st7701
  * See page 184 of ST7701_v1.2.pdf for details (driver datasheet) for common commands
  * See page 255 of ST7701_v1.2.pdf for details (driver datasheet) for command2 BKx
  *
  * As far as i can tell, BK commands are specialized settings that should not be played around with
  * except at initialization, and as such are a pain in the ass to access
 */
 typedef enum {
      NOP = 0x00,        // No-op
      SWRESET = 0x01,    // Software Reset
      RDDID = 0x04,      // Read Display ID
      RDNUMED = 0x05,    // Read Number of Errors on DSI
      RDRED = 0x06,      // Read the first pixel of Red Color
      RDGREEN = 0x07,    // Read the first pixel of Green Color
      RDBLUE = 0x08,     // Read the first pixel of Blue Color
      RDDPM = 0x0A,      // Read Display Power Mode
      RDDMADCTL = 0x0B,  // Read Display MADCTL
      RDDCOLMOD = 0x0C,  // Read Display Pixel Format
      RDDIM = 0x0D,      // Read Display Image Mode
      RDDSM = 0x0E,      // Read Display Signal Mode
      RDDSDR = 0x0F,     // Read Display Self-Diagnostic Result
      SLPIN = 0x10,      // Sleep in
      SLPOUT = 0x11,     // Sleep Out
      PTLON = 0x12,      // Partial Display Mode On
      NORON = 0x13,      // Normal Display Mode On
      INVOFF = 0x20,     // Display Inversion Off
      INVON = 0x21,      // Display Inversion On
      ALLPOFF = 0x22,    // All Pixel Off
      ALLPON = 0x23,     // All Pixel ON
      GAMSET = 0x26,     // Gamma Set
      DISPOFF = 0x28,    // Display Off
      DISPON = 0x29,     // Display On
      TEOFF = 0x34,      // Tearing Effect Line OFF
      TEON = 0x35,       // Tearing Effect Line ON
      MADCTL = 0x36,     // Display data access control
      IDMOFF = 0x38,     // Idle Mode Off
      IDMON = 0x39,      // Idle Mode On
      COLMOD = 0x3A,     // Interface Pixel Format
      GSL = 0x45,        // Get Scan Line
      WRDISBV = 0x51,    // Write Display Brightness
      RDDISBV = 0x52,    // Read Display Brightness Value
      WRCTRLD = 0x53,    // Write CTRL Display
      RDCTRLD = 0x54,    // Read CTRL Value Display
      WRCACE = 0x55,     // Write Content Adaptive Brightness Control and Color Enhancement
      RDCABC = 0x56,     // Read Content Adaptive Brightness Control
      WRCABCMB = 0x5E,   // Write CABC Minimum Brightness
      RDCABCMB = 0x5F,   // Read CABC Minimum Brightness
      RDABCSDR = 0x68,   // Read Automatic Brightness Control Self-Diagnostic Result
      RDBWLB = 0x70,     // Read Black/White Low Bits
      RDBkx = 0x71,      // Read Bkx
      RDBky = 0x72,      // Read Bky
      RDWx = 0x73,       // Read Wx
      RDWy = 0x74,       // Read Wy
      RDRGLB = 0x75,     // Read Red/Green Low Bits
      RDRx = 0x76,       // Read Rx
      RDRy = 0x77,       // Read Ry
      RDGx = 0x78,       // Read Gx
      RDGy = 0x79,       // Read Gy
      RDBALB = 0x7A,     // Read Blue/A Color Low Bits
      RDBx = 0x7B,       // Read Bx
      CND2BKxSEL = 0xFF, // Set Command2 mode for BK Register
  }CommandsGeneral;

  typedef enum {
      PVGAMCTRL = 0xB0, // Positive Voltage Gamma Control
      NVGAMCTRL = 0xB1, // Negative Voltage Gamma Control
      DGMEN = 0xB8,     // Digital Gamma Enable
      DGMLUTR = 0xB9,   // Digital Gamma Look-up Table for Red
      DGMLUTB = 0xBA,   // Digital Gamma Look-up Table for Blue
      PWMCLKSEL = 0xBC, // PWM CLK select
      LNESET = 0xC0,    // Display Line Setting
      PORCTRL = 0xC1,   // Porch Control
      INVSET = 0xC2,    // Inversion selection & Frame Rate Control
      RGBCTRL = 0xC3,   // RGB control
      PARCTRL = 0xC5,   // Partial Mode Control
      SDIR = 0xC7,      // X-direction Control
      PDOSET = 0xC8,    // Pseudo-Dot inversion diving setting
      COLCTRL = 0xCD,   // Color Control
      SRECTRL = 0xE0,   // Sunlight Readable Enhancement
      NRCTRL = 0xE1,    // Noise Reduce Control
      SECTRL = 0xE2,    // Sharpness Control
      CCCTRL = 0xE3,    // Color Calibration Control
      SKCTRL = 0xE4,    // Skin Tone Preservation CONTROL
  }BK0Command2;

  typedef enum {
      VRHS = 0xB0,     // Vop Amplitude setting
      VCOMS = 0xB1,    // VCOM amplitude setting
      VGHSS = 0xB2,    // VGH Voltage setting
      TESTCMD = 0xB3,  // TEST Command Setting
      VGLS = 0xB5,     // VGL Voltage setting
      PWCTRL1 = 0xB7,  // Power Control 1
      PWCTRL2 = 0xB8,  // Power Control 2
      PCLKS1 = 0xBA,   // Power pumping clk selection 1
      PCLKS3 = 0xBC,   // Power pumping clk selection 3
      SPD1 = 0xC1,     //  Source pre_drive timing set1
      SPD2 = 0xC2,     //  Source pre_drive timing set2
      MIPISET1 = 0xD0, // MIPI Setting 1
      MIPISET2 = 0xD1, // MIPI Setting 2
      MIPISET3 = 0xD2, // MIPI Setting 3
      MIPISET4 = 0xD3, // MIPI Setting 4
  }BK1Command2;

HAL_StatusTypeDef lcdInit_ST7701Driver(SPI_HandleTypeDef* hspi);

void displayAllWhitePixels_ST7701Driver(SPI_HandleTypeDef* hspi);
void displayAllBlackPixels_ST7701Driver(SPI_HandleTypeDef* hspi);
void turnOnDisplay_ST7701Driver(SPI_HandleTypeDef* hspi);
void turnOffDisplay_ST7701Driver(SPI_HandleTypeDef* hspi);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_ST7701_DRIVER_H_ */

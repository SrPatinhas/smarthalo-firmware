/*
 * ST7701_driver.c
 *
 *  Created on: Nov. 29, 2020
 *      Author: Sean Beitz
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include "ST7701_driver.h"
#include "stdio.h"
#include "main.h"

 /*
  *       DOCUMENTATION & REFERENCES:
  *       ST7701_v1.2.pdf
  */

#define COMMAND_BIT		0x00 //send command
#define PARAMETER_BIT	0x01 //send paramater value following a command

#define _LCD_DISABLE() HAL_GPIO_WritePin(LCD_nRESET_GPIO_Port, LCD_nRESET_Pin, GPIO_PIN_RESET)
#define _LCD_ENABLE() HAL_GPIO_WritePin(LCD_nRESET_GPIO_Port, LCD_nRESET_Pin, GPIO_PIN_SET)

 static HAL_StatusTypeDef writeCommand_ST7701Driver(SPI_HandleTypeDef* hspi, uint8_t command);
 static HAL_StatusTypeDef writeParameter_ST7701Driver(SPI_HandleTypeDef* hspi, uint8_t parameter);

 typedef enum {
  GENERAL_COMMAND  = 0x00,
  BK0COMMAND2      = 0x10,
  BK1COMMAND2      = 0x11,
 }commandsRegisterSetting_t;

HAL_StatusTypeDef lcdInit_ST7701Driver(SPI_HandleTypeDef* hspi)
{
	HAL_StatusTypeDef err_code = HAL_OK;

	/*_LCD_LDO_ENABLE();*/ //needs to be uncommented to turn on lcd ldo

	/* Screen reset initialization */
	_LCD_DISABLE();
	HAL_Delay(5);
	_LCD_ENABLE();

	HAL_Delay(300); //must wait for sleep mode to disable, can put to 200 ms if boot time too long

	writeCommand_ST7701Driver(hspi,CND2BKxSEL);
	writeParameter_ST7701Driver(hspi,0x77);
	writeParameter_ST7701Driver(hspi,0x01);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x10);

	writeCommand_ST7701Driver(hspi,0xC0);
	writeParameter_ST7701Driver(hspi,0x3B);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xC1);
	writeParameter_ST7701Driver(hspi,0x0B);	//VBP
	writeParameter_ST7701Driver(hspi,0x02);

	writeCommand_ST7701Driver(hspi,0xC2);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x02);

	writeCommand_ST7701Driver(hspi,0xC3);
	writeParameter_ST7701Driver(hspi,0x02);

	writeCommand_ST7701Driver(hspi,0xCC);
	writeParameter_ST7701Driver(hspi,0x10);

	writeCommand_ST7701Driver(hspi,0xCD);
	writeParameter_ST7701Driver(hspi,0x08);

	writeCommand_ST7701Driver(hspi,0xB0); //Positive Voltage Gamma Control
	writeParameter_ST7701Driver(hspi,0x02);
	writeParameter_ST7701Driver(hspi,0x13);
	writeParameter_ST7701Driver(hspi,0x1B);
	writeParameter_ST7701Driver(hspi,0x0D);
	writeParameter_ST7701Driver(hspi,0x10);
	writeParameter_ST7701Driver(hspi,0x05);
	writeParameter_ST7701Driver(hspi,0x08);
	writeParameter_ST7701Driver(hspi,0x07);
	writeParameter_ST7701Driver(hspi,0x07);
	writeParameter_ST7701Driver(hspi,0x24);
	writeParameter_ST7701Driver(hspi,0x04);
	writeParameter_ST7701Driver(hspi,0x11);
	writeParameter_ST7701Driver(hspi,0x0E);
	writeParameter_ST7701Driver(hspi,0x2C);
	writeParameter_ST7701Driver(hspi,0x33);
	writeParameter_ST7701Driver(hspi,0x1D);

	writeCommand_ST7701Driver(hspi,0xB1); //Negative Voltage Gamma Control
	writeParameter_ST7701Driver(hspi,0x05);
	writeParameter_ST7701Driver(hspi,0x13);
	writeParameter_ST7701Driver(hspi,0x1B);
	writeParameter_ST7701Driver(hspi,0x0D);
	writeParameter_ST7701Driver(hspi,0x11);
	writeParameter_ST7701Driver(hspi,0x05);
	writeParameter_ST7701Driver(hspi,0x08);
	writeParameter_ST7701Driver(hspi,0x07);
	writeParameter_ST7701Driver(hspi,0x07);
	writeParameter_ST7701Driver(hspi,0x24);
	writeParameter_ST7701Driver(hspi,0x04);
	writeParameter_ST7701Driver(hspi,0x11);
	writeParameter_ST7701Driver(hspi,0x0E);
	writeParameter_ST7701Driver(hspi,0x2C);
	writeParameter_ST7701Driver(hspi,0x33);
	writeParameter_ST7701Driver(hspi,0x1D);

	writeCommand_ST7701Driver(hspi,CND2BKxSEL);
	writeParameter_ST7701Driver(hspi,0x77);
	writeParameter_ST7701Driver(hspi,0x01);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x11);

	writeCommand_ST7701Driver(hspi,0xB0);
	writeParameter_ST7701Driver(hspi,0x5d);//5d

	writeCommand_ST7701Driver(hspi,0xB1); 	//VCOM amplitude setting
	writeParameter_ST7701Driver(hspi,0x43); //43

	writeCommand_ST7701Driver(hspi,0xB2); 	//VGH Voltage setting
	writeParameter_ST7701Driver(hspi,0x81);	//12V

	writeCommand_ST7701Driver(hspi,0xB3);
	writeParameter_ST7701Driver(hspi,0x80);

	writeCommand_ST7701Driver(hspi,0xB5); 	//VGL Voltage setting
	writeParameter_ST7701Driver(hspi,0x43);	//-8.3V

	writeCommand_ST7701Driver(hspi,0xB7);
	writeParameter_ST7701Driver(hspi,0x85);

	writeCommand_ST7701Driver(hspi,0xB8);
	writeParameter_ST7701Driver(hspi,0x20);

	writeCommand_ST7701Driver(hspi,0xC1);
	writeParameter_ST7701Driver(hspi,0x78);

	writeCommand_ST7701Driver(hspi,0xC2);
	writeParameter_ST7701Driver(hspi,0x78);

	writeCommand_ST7701Driver(hspi,0xD0);
	writeParameter_ST7701Driver(hspi,0x88);

	writeCommand_ST7701Driver(hspi,0xE0);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x02);

	writeCommand_ST7701Driver(hspi,0xE1);
	writeParameter_ST7701Driver(hspi,0x03);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x04);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x20);
	writeParameter_ST7701Driver(hspi,0x20);

	writeCommand_ST7701Driver(hspi,0xE2);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xE3);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x11);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xE4);
	writeParameter_ST7701Driver(hspi,0x22);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xE5);
	writeParameter_ST7701Driver(hspi,0x05);
	writeParameter_ST7701Driver(hspi,0xEC);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0x07);
	writeParameter_ST7701Driver(hspi,0xEE);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xE6);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x11);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xE7);
	writeParameter_ST7701Driver(hspi,0x22);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xE8);
	writeParameter_ST7701Driver(hspi,0x06);
	writeParameter_ST7701Driver(hspi,0xED);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0x08);
	writeParameter_ST7701Driver(hspi,0xEF);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xEB);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x40);
	writeParameter_ST7701Driver(hspi,0x40);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,0xED);
	writeParameter_ST7701Driver(hspi,0xFF);
	writeParameter_ST7701Driver(hspi,0xFF);
	writeParameter_ST7701Driver(hspi,0xFF);
	writeParameter_ST7701Driver(hspi,0xBA);
	writeParameter_ST7701Driver(hspi,0x0A);
	writeParameter_ST7701Driver(hspi,0xBF);
	writeParameter_ST7701Driver(hspi,0x45);
	writeParameter_ST7701Driver(hspi,0xFF);
	writeParameter_ST7701Driver(hspi,0xFF);
	writeParameter_ST7701Driver(hspi,0x54);
	writeParameter_ST7701Driver(hspi,0xFB);
	writeParameter_ST7701Driver(hspi,0xA0);
	writeParameter_ST7701Driver(hspi,0xAB);
	writeParameter_ST7701Driver(hspi,0xFF);
	writeParameter_ST7701Driver(hspi,0xFF);
	writeParameter_ST7701Driver(hspi,0xFF);

	writeCommand_ST7701Driver(hspi,0xEF);
	writeParameter_ST7701Driver(hspi,0x10);
	writeParameter_ST7701Driver(hspi,0x0D);
	writeParameter_ST7701Driver(hspi,0x04);
	writeParameter_ST7701Driver(hspi,0x08);
	writeParameter_ST7701Driver(hspi,0x3F);
	writeParameter_ST7701Driver(hspi,0x1F);

	writeCommand_ST7701Driver(hspi,CND2BKxSEL);
	writeParameter_ST7701Driver(hspi,0x77);
	writeParameter_ST7701Driver(hspi,0x01);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x13);

	writeCommand_ST7701Driver(hspi,0xEF);
	writeParameter_ST7701Driver(hspi,0x08);

	writeCommand_ST7701Driver(hspi,CND2BKxSEL);
	writeParameter_ST7701Driver(hspi,0x77);
	writeParameter_ST7701Driver(hspi,0x01);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,SLPOUT);

	HAL_Delay(120);

	writeCommand_ST7701Driver(hspi,DISPON);

	writeCommand_ST7701Driver(hspi,MADCTL);
	writeParameter_ST7701Driver(hspi,0x00);

	writeCommand_ST7701Driver(hspi,COLMOD);
	writeParameter_ST7701Driver(hspi,0x66);

    return err_code;
}

//HAL_StatusTypeDef lcdInit_ST7701Driver_old(SPI_HandleTypeDef* hspi)
//{
//	HAL_StatusTypeDef err_code = HAL_OK;
//
//	/* Screen reset initialization */
//	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
//	HAL_Delay(5);
//	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
//
//	HAL_Delay(10);// allows display to stabilize after hardware reset
//
//	//ensure all settings are blank
//	err_code = writeCommand_ST7701Driver(hspi, SWRESET);
//	HAL_Delay(10);
//	//wake up
//	err_code = writeCommand_ST7701Driver(hspi, SLPOUT);//page 201
//	HAL_Delay(300); //must wait for sleep mode to disable, can put to 200 ms if boot time too long
//
//	//BK0 settings : image settings
//	//display line settings
//	err_code = writeBK0Command2_ST7701Driver(hspi, LNESET);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3B);//page 267
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);//used to terminate parameter write commands
//	HAL_Delay(10);
//	//porch control
//	err_code = writeBK0Command2_ST7701Driver(hspi, PORCTRL);
//	err_code = writeParameter_ST7701Driver(hspi, FRONT_PORCH);//page 268
//	err_code = writeParameter_ST7701Driver(hspi, BACK_PORCH);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//	/*
//	 pub const TDOMode: Mode = Mode {
//	   clock: 16000,
//
//	   hdisplay: 480,
//	   hsync_start: 480 + 24,
//	   hsync_end: 480 + 24 + 6,
//	   htotal: 480 + 24 + 6 + 18,
//
//	   vdisplay: 480,
//	   vsync_start: 480 + 16,
//	   vsync_end: 480 + 16 + 4,
//	   vtotal: 480 + 16 + 4 + 10,
//
//	   width_mm: 69,
//	   height_mm: 139,
//	 };*/
//
//	//inversion selection
//	err_code = writeBK0Command2_ST7701Driver(hspi, INVSET);
//	err_code = writeParameter_ST7701Driver(hspi, 0x30);//page 269
//	err_code = writeParameter_ST7701Driver(hspi, 0x02);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//	// - nvm program active? todo?
//
//	//RGB mode control
//	err_code = writeBK0Command2_ST7701Driver(hspi, RGBCTRL);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0C);//page 270
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//color control
//	err_code = writeBK0Command2_ST7701Driver(hspi, COLCTRL);
//	err_code = writeParameter_ST7701Driver(hspi, 0x08);//page 274
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//positive gamma control
//	err_code = writeBK0Command2_ST7701Driver(hspi, PVGAMCTRL);
//	err_code = writeParameter_ST7701Driver(hspi, 0x02);//page 260
//	err_code = writeParameter_ST7701Driver(hspi, 0x13);
//	err_code = writeParameter_ST7701Driver(hspi, 0x1B);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0D);
//	err_code = writeParameter_ST7701Driver(hspi, 0x10);
//	err_code = writeParameter_ST7701Driver(hspi, 0x05);
//	err_code = writeParameter_ST7701Driver(hspi, 0x08);
//	err_code = writeParameter_ST7701Driver(hspi, 0x07);
//	err_code = writeParameter_ST7701Driver(hspi, 0x07);
//	err_code = writeParameter_ST7701Driver(hspi, 0x24);
//	err_code = writeParameter_ST7701Driver(hspi, 0x04);
//	err_code = writeParameter_ST7701Driver(hspi, 0x11);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0E);
//	err_code = writeParameter_ST7701Driver(hspi, 0x2C);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	err_code = writeParameter_ST7701Driver(hspi, 0x1D);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//negative gamma control, there is one too many in here? sequence copied from repo found at top of page
//	err_code = writeBK0Command2_ST7701Driver(hspi, NVGAMCTRL);
//	err_code = writeParameter_ST7701Driver(hspi, 0xB1);//page 262
//	err_code = writeParameter_ST7701Driver(hspi, 0x05);
//	err_code = writeParameter_ST7701Driver(hspi, 0x13);
//	err_code = writeParameter_ST7701Driver(hspi, 0x1B);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0D);
//	err_code = writeParameter_ST7701Driver(hspi, 0x11);
//	err_code = writeParameter_ST7701Driver(hspi, 0x05);
//	err_code = writeParameter_ST7701Driver(hspi, 0x08);
//	err_code = writeParameter_ST7701Driver(hspi, 0x07);
//	err_code = writeParameter_ST7701Driver(hspi, 0x07);
//	err_code = writeParameter_ST7701Driver(hspi, 0x24);
//	err_code = writeParameter_ST7701Driver(hspi, 0x04);
//	err_code = writeParameter_ST7701Driver(hspi, 0x11);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0E);
//	err_code = writeParameter_ST7701Driver(hspi, 0x2C);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	err_code = writeParameter_ST7701Driver(hspi, 0x1D);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//BK1 settings: power settings
//	//vop amplitude
//	err_code = writeBK1Command2_ST7701Driver(hspi, VRHS);//page 284
//	err_code = writeParameter_ST7701Driver(hspi, 0x5d);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//vcom amplitude
//	err_code = writeBK1Command2_ST7701Driver(hspi, VCOMS);//page 285
//	err_code = writeParameter_ST7701Driver(hspi, 0x43);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//vgh voltage
//	err_code = writeBK1Command2_ST7701Driver(hspi, VGHSS);//page 286
//	err_code = writeParameter_ST7701Driver(hspi, 0x81);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//test command setting
//	err_code = writeBK1Command2_ST7701Driver(hspi, TESTCMD);//page 287
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//vgl voltage
//	err_code = writeBK1Command2_ST7701Driver(hspi, VGLS);//page 288
//	err_code = writeParameter_ST7701Driver(hspi, 0x43);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//power control one
//	err_code = writeBK1Command2_ST7701Driver(hspi, PWCTRL1);//page 289
//	err_code = writeParameter_ST7701Driver(hspi, 0x45);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//power control two
//	err_code = writeBK1Command2_ST7701Driver(hspi, PWCTRL2);
//	err_code = writeParameter_ST7701Driver(hspi, 0x20);//page 290
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//pre drive timing 1
//	err_code = writeBK1Command2_ST7701Driver(hspi, SPD1);
//	err_code = writeParameter_ST7701Driver(hspi, 0x08);//page 293
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//pre drive timing 2
//	err_code = writeBK1Command2_ST7701Driver(hspi, SPD2);
//	err_code = writeParameter_ST7701Driver(hspi, 0x08);//page 294
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//
//	//Voodoo magic section
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE0);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x02);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE1);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0B);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0D);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0C);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0E);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE2);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	err_code = writeParameter_ST7701Driver(hspi, 0x64);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x66);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x65);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x67);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE3);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE4);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE5);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0C);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0E);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	err_code = writeParameter_ST7701Driver(hspi, 0x10);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	err_code = writeParameter_ST7701Driver(hspi, 0x12);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE6);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	err_code = writeParameter_ST7701Driver(hspi, 0x33);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE7);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xE8);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0D);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	err_code = writeParameter_ST7701Driver(hspi, 0x0F);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	err_code = writeParameter_ST7701Driver(hspi, 0x11);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	err_code = writeParameter_ST7701Driver(hspi, 0x13);
//	err_code = writeParameter_ST7701Driver(hspi, 0x78);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3C);
//	err_code = writeParameter_ST7701Driver(hspi, 0xA0);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xEB);
//	err_code = writeParameter_ST7701Driver(hspi, 0x02);
//	err_code = writeParameter_ST7701Driver(hspi, 0x02);
//	err_code = writeParameter_ST7701Driver(hspi, 0x39);
//	err_code = writeParameter_ST7701Driver(hspi, 0x39);
//	err_code = writeParameter_ST7701Driver(hspi, 0xEE);
//	err_code = writeParameter_ST7701Driver(hspi, 0x44);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xEC);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	err_code = writeBK1Command2_ST7701Driver(hspi, 0xED);
//	err_code = writeParameter_ST7701Driver(hspi, 0xFF);
//	err_code = writeParameter_ST7701Driver(hspi, 0xF1);
//	err_code = writeParameter_ST7701Driver(hspi, 0x04);
//	err_code = writeParameter_ST7701Driver(hspi, 0x56);
//	err_code = writeParameter_ST7701Driver(hspi, 0x72);
//	err_code = writeParameter_ST7701Driver(hspi, 0x3F);
//	err_code = writeParameter_ST7701Driver(hspi, 0xFF);
//	err_code = writeParameter_ST7701Driver(hspi, 0xFF);
//	err_code = writeParameter_ST7701Driver(hspi, 0xFF);
//	err_code = writeParameter_ST7701Driver(hspi, 0xFF);
//	err_code = writeParameter_ST7701Driver(hspi, 0xF3);
//	err_code = writeParameter_ST7701Driver(hspi, 0x27);
//	err_code = writeParameter_ST7701Driver(hspi, 0x65);
//	err_code = writeParameter_ST7701Driver(hspi, 0x40);
//	err_code = writeParameter_ST7701Driver(hspi, 0x1F);
//	err_code = writeParameter_ST7701Driver(hspi, 0xFF);
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
////	writeCommand_ST7701Driver(hspi,hspi,0xFF);
////	writeParameter_ST7701Driver(hspi,hspi,0x77);
////	writeParameter_ST7701Driver(hspi,hspi,0x01);
////	writeParameter_ST7701Driver(hspi,hspi,0x00);
////	writeParameter_ST7701Driver(hspi,hspi,0x00);
////	writeParameter_ST7701Driver(hspi,hspi,0x12);
////	writeCommand_ST7701Driver(hspi,hspi,0xD1);
////	writeParameter_ST7701Driver(hspi,hspi,0x81);
////	writeParameter_ST7701Driver(hspi,hspi,0x08);
////	writeParameter_ST7701Driver(hspi,hspi,0x03);
////	writeParameter_ST7701Driver(hspi,hspi,0x20);
////	writeParameter_ST7701Driver(hspi,hspi,0x08);
////	writeParameter_ST7701Driver(hspi,hspi,0x01);
////	writeParameter_ST7701Driver(hspi,hspi,0xA0);
////	writeParameter_ST7701Driver(hspi,hspi,0x01);
////	writeParameter_ST7701Driver(hspi,hspi,0xE0);
////	writeParameter_ST7701Driver(hspi,hspi,0xA0);
////	writeParameter_ST7701Driver(hspi,hspi,0x01);
////	writeParameter_ST7701Driver(hspi,hspi,0xE0);
////	writeParameter_ST7701Driver(hspi,hspi,0x03);
////	writeParameter_ST7701Driver(hspi,hspi,0x20);
////	writeCommand_ST7701Driver(hspi,hspi,0xD2);
////	writeParameter_ST7701Driver(hspi,hspi,0x08);
//
//	//General commands
//
//	//data control, scan direction
//	err_code = writeCommand_ST7701Driver(hspi, MADCTL);
//	err_code = writeParameter_ST7701Driver(hspi, 0x00);//page 214
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//interface pixel format
//	err_code = writeCommand_ST7701Driver(hspi, COLMOD);
//	err_code = writeParameter_ST7701Driver(hspi, 0x50);//page 218
//	//err_code = writeCommand_ST7701Driver(hspi, NOP);
//	HAL_Delay(10);
//
//	//turn on
//	err_code = writeCommand_ST7701Driver(hspi, DISPON);//page 210
//
//	return err_code;
//}



void displayAllWhitePixels_ST7701Driver(SPI_HandleTypeDef* hspi)
{
   writeCommand_ST7701Driver(hspi, ALLPON);
}

void displayAllBlackPixels_ST7701Driver(SPI_HandleTypeDef* hspi)
{
	while(hspi->State != HAL_SPI_STATE_READY);
	uint8_t pData[2] = {ALLPOFF, COMMAND_BIT};
    HAL_SPI_Transmit(hspi, pData, 1,0xFFFF);
}

void turnOnDisplay_ST7701Driver(SPI_HandleTypeDef* hspi)
{
	writeCommand_ST7701Driver(hspi, DISPON);
}

void turnOffDisplay_ST7701Driver(SPI_HandleTypeDef* hspi)
{
	writeCommand_ST7701Driver(hspi, DISPOFF);
}

static HAL_StatusTypeDef writeCommand_ST7701Driver(SPI_HandleTypeDef* hspi, uint8_t command)
{
	while(hspi->State != HAL_SPI_STATE_READY);
	uint8_t pData[2] = {GENERAL_COMMAND, }; //set general commands BK0 or special Bk1 commands2 register
	pData[0] = command;
    return HAL_SPI_Transmit(hspi, pData, 1,0xFFFF);
}

//parameters for commands must always be written after a writeCommand_ST7701Driver() or BKx command
static HAL_StatusTypeDef writeParameter_ST7701Driver(SPI_HandleTypeDef* hspi, uint8_t parameter)
{
	while(hspi->State != HAL_SPI_STATE_READY);
	uint8_t pData[2] = {parameter, PARAMETER_BIT};
    return HAL_SPI_Transmit(hspi, pData, 1,0xFFFF);
}

#ifdef __cplusplus
}
#endif

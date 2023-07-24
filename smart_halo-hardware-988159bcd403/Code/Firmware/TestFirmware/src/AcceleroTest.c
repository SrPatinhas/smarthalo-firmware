
#include "AcceleroTest.h"
#include "I2C.h"
#include "SmartHalo.h"
#include "CommandLineInterface.h"

#include "nrf_delay.h"
#include "app_error.h"
#include "SH_Accelero_Magneto_typedefs.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define REGISTER_AUTO_INCREMENT		 (1 << 7)

// Constants in gravitational acceleration units (mg)
#define ACC_SENS_2G_HR		0.98
#define ACC_SENS_4G_HR		1.95
#define ACC_SENS_8G_HR		3.9
#define ACC_SENS_16G_HR		11.72
#define ACC_SENS_2G			3.9
#define ACC_SENS_4G			7.82
#define ACC_SENS_8G			15.63
#define ACC_SENS_16G		46.9
#define ACC_SENS_2G_LP		15.63
#define ACC_SENS_4G_LP		31.26
#define ACC_SENS_8G_LP		62.52
#define ACC_SENS_16G_LP		187.58

// In digital 16 bits values
#define MIN_SELFTEST_VALUE		17
#define MAX_SELFEST_VALUE		360

static bool ACC_PRESENT = false;
static AcceleroPowerMode ACC_POW_MODE = POWERMODE_NORMAL;
static AcceleroFS ACC_FS = SCALE2G;
static Endianness ACC_ENDIAN = LITTLEENDIAN;
static float ACC_SENS = ACC_SENS_2G;
static float accX = 0;
static float accY = 0;
static float accZ = 0;
static float accTemp = 0;

void AcceleroTest_easyConfig();
void AcceleroTest_updateAccSensitivity();
Endianness AcceleroTest_getAccEndian();
float AcceleroTest_rawAccToFloat(uint8_t regL, uint8_t regH);

void AcceleroTest_checkPresent()
{
	ret_code_t err_code;
	uint8_t data = WHO_AM_I_A;
	uint8_t regdata;

	if(I2C_twi_tx(ACCELERO_ADDRESS, &data, 1, true) == NRF_SUCCESS)
	{
		ACC_PRESENT = true;

		err_code = I2C_twi_rx(ACCELERO_ADDRESS, &regdata, 1);
		APP_ERROR_CHECK(err_code);
	}
	else
	{
		CommandLineInterface_printLine("Accelerometer not detected");
	}
}

void AcceleroTest_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         ACCELERO");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("getaccelerationxyz:\t\tReturns the acceleration on all axes (mg)");
	CommandLineInterface_printLine("getaccelerationscale:\t\tReturns the accelerometer scale (g)");
	CommandLineInterface_printLine("getaccpowermode:\t\tReturns the accelerometer power mode");
	CommandLineInterface_printLine("getacctemperature:\t\tReturns the accelerometer temperature (C)");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("setaccelerationscale <scale>:\tSets the accelerometer scale (2G, 4G, 8G, 16G)");
	CommandLineInterface_printLine("setaccpowermode <mode>:\tSets the accelerometer power mode (NL, LP, HR)");
	CommandLineInterface_printLine("selftest <device>:\t\tRuns the selftest on the selected device (ACC, MAG)");
	nrf_delay_ms(50);
}

bool AcceleroTest_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	bool testPass = false;
	float aX, aY, aZ, accTemp;
	bool parsed = true;

	if(strncmp(RxBuff, "GETACCELERATIONXYZ", 18)==0)
	{
		AcceleroTest_getAccXYZ(&aX, &aY, &aZ);
		CommandLineInterface_printf("%5.2f mg, %5.2f mg, %5.2f mg\r\n", aX, aY, aZ);
	}
	else if(strncmp(RxBuff, "GETACCTEMPERATURE", 17)==0)
	{
		AcceleroTest_getAccTemp(&accTemp);
		CommandLineInterface_printf("%3.2f C\r\n", accTemp);
	}
	else if(strncmp(RxBuff, "GETACCELERATIONSCALE", 20)==0)
	{
		AcceleroFS scale = AcceleroTest_getAccScale();

		if(scale == SCALE2G)
			CommandLineInterface_printLine("+/-2g");
		else if(scale == SCALE4G)
			CommandLineInterface_printLine("+/-4g");
		else if(scale == SCALE8G)
			CommandLineInterface_printLine("+/-8g");
		else
			CommandLineInterface_printLine("+/-16g");
	}
	else if(strncmp(RxBuff, "GETACCPOWERMODE", 15)==0)
	{
		AcceleroPowerMode powermode = AcceleroTest_getAccPowerMode();

		if(powermode == POWERMODE_NORMAL)
			CommandLineInterface_printLine("normal");
		else if(powermode == POWERMODE_HIGHRESOLUTION)
			CommandLineInterface_printLine("high resolution");
		else
			CommandLineInterface_printLine("low power");
	}
	else if(strncmp(RxBuff, "SETACCELERATIONSCALE 2G", 23)==0)
	{
		AcceleroTest_setAccScale(SCALE2G);
		CommandLineInterface_printLine("Scale set to +/-2g");
	}
	else if(strncmp(RxBuff, "SETACCELERATIONSCALE 4G", 23)==0)
	{
		AcceleroTest_setAccScale(SCALE4G);
		CommandLineInterface_printLine("Scale set to +/-4g");
	}
	else if(strncmp(RxBuff, "SETACCELERATIONSCALE 8G", 23)==0)
	{
		AcceleroTest_setAccScale(SCALE8G);
		CommandLineInterface_printLine("Scale set to +/-8g");
	}
	else if(strncmp(RxBuff, "SETACCELERATIONSCALE 16G", 24)==0)
	{
		AcceleroTest_setAccScale(SCALE16G);
		CommandLineInterface_printLine("Scale set to +/-16g");
	}
	else if(strncmp(RxBuff, "SETACCPOWERMODE LP", 18)==0)
	{
		AcceleroTest_setAccPowerMode(POWERMODE_LOWPOWER);
		CommandLineInterface_printLine("Power mode set to low power");
	}
	else if(strncmp(RxBuff, "SETACCPOWERMODE NL", 18)==0)
	{
		AcceleroTest_setAccPowerMode(POWERMODE_NORMAL);
		CommandLineInterface_printLine("Power mode set to normal");
	}
	else if(strncmp(RxBuff, "SETACCPOWERMODE HR", 18)==0)
	{
		AcceleroTest_setAccPowerMode(POWERMODE_HIGHRESOLUTION);
		CommandLineInterface_printLine("Power mode set to high resolution");
	}
	else if(strncmp(RxBuff, "SELFTEST ACC", 12)==0)
	{
		CommandLineInterface_printLine("Starting accelerometer self test");
		testPass = AcceleroTest_selfTest();
		testPass ? CommandLineInterface_printLine("PASS") : CommandLineInterface_printLine("FAIL");
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void AcceleroTest_setup()
{
	AcceleroTest_checkPresent();

	// Set default config
	AcceleroTest_easyConfig();
}

void AcceleroTest_writeReg(AcceleroRegisters reg, unsigned char data)
{
	if(ACC_PRESENT)
		writeReg8(ACCELERO_ADDRESS, reg, data);
}

unsigned char AcceleroTest_readReg(AcceleroRegisters reg)
{
	if(ACC_PRESENT)
		return readReg8(ACCELERO_ADDRESS, reg);
	else
		return -1;
}

unsigned char AcceleroTest_getID()
{
	return AcceleroTest_readReg(WHO_AM_I_A);
}

void AcceleroTest_updateAccSensitivity()
{
	switch(ACC_FS)
	{
		case SCALE4G:
			if(ACC_POW_MODE == POWERMODE_LOWPOWER)
				ACC_SENS = ACC_SENS_4G_LP;
			else if(ACC_POW_MODE == POWERMODE_NORMAL)
				ACC_SENS = ACC_SENS_4G;
			else
				ACC_SENS = ACC_SENS_4G_HR;
			break;
		case SCALE8G:
			if(ACC_POW_MODE == POWERMODE_LOWPOWER)
				ACC_SENS = ACC_SENS_8G_LP;
			else if(ACC_POW_MODE == POWERMODE_NORMAL)
				ACC_SENS = ACC_SENS_8G;
			else
				ACC_SENS = ACC_SENS_8G_HR;
			break;
		case SCALE16G:
			if(ACC_POW_MODE == POWERMODE_LOWPOWER)
				ACC_SENS = ACC_SENS_16G_LP;
			else if(ACC_POW_MODE == POWERMODE_NORMAL)
				ACC_SENS = ACC_SENS_16G;
			else
				ACC_SENS = ACC_SENS_16G_HR;
			break;
		case SCALE2G:
		default:
			if(ACC_POW_MODE == POWERMODE_LOWPOWER)
				ACC_SENS = ACC_SENS_2G_LP;
			else if(ACC_POW_MODE == POWERMODE_NORMAL)
				ACC_SENS = ACC_SENS_2G;
			else
				ACC_SENS = ACC_SENS_2G_HR;
			break;
	}
}

void AcceleroTest_setAccScale(AcceleroFS scale)
{
	unsigned char regdata;

	if (scale != ACC_FS)
	{
		// Read relative registers and set only FS bits
		regdata = AcceleroTest_readReg(CTRL_REG4_A);
		regdata &= ~0x30;
		regdata |= (((unsigned char)scale << 4) & 0x30);
		AcceleroTest_writeReg(CTRL_REG4_A, regdata);

		ACC_FS = scale;
		AcceleroTest_updateAccSensitivity();
	}
}

AcceleroFS AcceleroTest_getAccScale()
{
	unsigned char regdata;

	regdata = AcceleroTest_readReg(CTRL_REG4_A);

	ACC_FS = (AcceleroFS)((regdata >> 4) & 0x03);

	return ACC_FS;
}

void AcceleroTest_setAccPowerMode(AcceleroPowerMode powermode)
{
	unsigned char regdata;

		if (powermode != ACC_POW_MODE)
		{
			// Read relative registers and set only HR and LPEn bits
			regdata = AcceleroTest_readReg(CTRL_REG1_A);
			regdata &= ~0x08;
			regdata |= (((unsigned char)powermode << 2) & 0x08);
			AcceleroTest_writeReg(CTRL_REG1_A, regdata);

			regdata = AcceleroTest_readReg(CTRL_REG4_A);
			regdata &= ~0x08;
			regdata |= (((unsigned char)powermode << 3) & 0x08);
			AcceleroTest_writeReg(CTRL_REG4_A, regdata);

			ACC_POW_MODE = powermode;
			AcceleroTest_updateAccSensitivity();
		}
}

AcceleroPowerMode AcceleroTest_getAccPowerMode()
{
	unsigned char regdata;

	regdata = AcceleroTest_readReg(CTRL_REG1_A);
	ACC_POW_MODE = (AcceleroPowerMode)((regdata >> 2) & 0x02);
	regdata = AcceleroTest_readReg(CTRL_REG4_A);
	ACC_POW_MODE |= (AcceleroPowerMode)((regdata >> 3) & 0x01);

	return ACC_POW_MODE;
}

void AcceleroTest_setAccEndian(Endianness endianness)
{
	unsigned char regdata;

	if (endianness != ACC_ENDIAN)
	{
		// Read relative registers and set only FS bits
		regdata = AcceleroTest_readReg(CTRL_REG4_A);
		regdata &= ~0x40;
		regdata |= (((unsigned char)endianness << 6) & 0x40);
		AcceleroTest_writeReg(CTRL_REG4_A, regdata);

		ACC_ENDIAN = endianness;
		AcceleroTest_updateAccSensitivity();
	}
}

Endianness AcceleroTest_getAccEndian()
{
	unsigned char regdata;

	regdata = AcceleroTest_readReg(CTRL_REG4_A);

	ACC_ENDIAN = (Endianness)((regdata >> 6) & 0x01);

	return ACC_ENDIAN;
}


// Block read: cannot use standard register read functions
void AcceleroTest_readAccTemp()
{
	if(ACC_PRESENT)
	{
		ret_code_t err_code;
		uint8_t data = REGISTER_AUTO_INCREMENT | OUT_TEMP_L_A; // MSB=1 to set address auto increment
		uint8_t regdata[2];

		err_code = I2C_twi_tx(ACCELERO_ADDRESS, &data, 1, true);
		APP_ERROR_CHECK(err_code);

		err_code = I2C_twi_rx(ACCELERO_ADDRESS, regdata, 2);
		APP_ERROR_CHECK(err_code);

		// Temperature only on 8 bits
		accTemp = (float)regdata[1] + 25.0;

	}
}

void AcceleroTest_getAccTemp(float *temp)
{
	assert(temp!=NULL);

	AcceleroTest_readAccTemp();

	*temp = accTemp;
}

// Block read: cannot use standard register read functions
void AcceleroTest_readAccSample()
{
	if(ACC_PRESENT)
	{
		ret_code_t err_code;
		uint8_t data = REGISTER_AUTO_INCREMENT | OUT_X_L_A; // MSB=1 to set address auto increment
		uint8_t regdata[6];

		err_code = I2C_twi_tx(ACCELERO_ADDRESS, &data, 1, true);
		APP_ERROR_CHECK(err_code);

		err_code = I2C_twi_rx(ACCELERO_ADDRESS, regdata, 6);
		APP_ERROR_CHECK(err_code);

		if( ACC_ENDIAN == LITTLEENDIAN )
		{
			accX = AcceleroTest_rawAccToFloat(regdata[0], regdata[1]);
			accY = AcceleroTest_rawAccToFloat(regdata[2], regdata[3]);
			accZ = AcceleroTest_rawAccToFloat(regdata[4], regdata[5]);
		}
		else
		{
			accX = AcceleroTest_rawAccToFloat(regdata[1], regdata[0]);
			accY = AcceleroTest_rawAccToFloat(regdata[3], regdata[2]);
			accZ = AcceleroTest_rawAccToFloat(regdata[5], regdata[4]);
		}
	}
}

float AcceleroTest_rawAccToFloat(uint8_t regL, uint8_t regH)
{
	int16_t reg16;

	// Resolution depends on the power mode (8, 10 or 12 bits)
	switch(ACC_POW_MODE)
	{
		case POWERMODE_HIGHRESOLUTION:
			reg16 = (regH << 4) | (regL >> 4);
			if( reg16 & 0x0800 )
				reg16 |= 0xF000;
			break;
		case POWERMODE_LOWPOWER:
			reg16 = regH;
			if( reg16 & 0x0080 )
				reg16 |= 0xFF00;
			break;
		case POWERMODE_NORMAL:
		default:
			reg16 = (regH << 2) | (regL >> 2);
			if( reg16 & 0x0200 )
				reg16 |= 0xFC00;
			break;
	}

	return (float)reg16;
}

void AcceleroTest_getAccXYZ(float *aX, float *aY, float *aZ)
{
	assert(aX!=NULL);assert(aY!=NULL);assert(aZ!=NULL);

	AcceleroTest_readAccSample();

	*aX = (float)(accX) * ACC_SENS;
	*aY = (float)(accY) * ACC_SENS;
	*aZ = (float)(accZ) * ACC_SENS;
}

// Based on Unico v4.4.2 Easyconfiguration (see Produit/Developement/AcceleroMagnetor/EasyConfig.doc)
// ODR = 100 Hz, high resolution mode, all axis enabled, FIFO disabled
// No interrupt on INT1 or INT2
void AcceleroTest_easyConfig()
{
	AcceleroTest_writeReg(TEMP_CFG_REG_A, 0xC0);
	AcceleroTest_writeReg(CTRL_REG1_A, 0x57);
	AcceleroTest_writeReg(CTRL_REG2_A, 0x00);
	AcceleroTest_writeReg(CTRL_REG3_A, 0x00);
	AcceleroTest_writeReg(CTRL_REG4_A, 0x88); // I2C instead of SPI default in easy config
	AcceleroTest_writeReg(CTRL_REG5_A, 0x00);
	AcceleroTest_writeReg(CTRL_REG6_A, 0x00);
	AcceleroTest_writeReg(REFERENCE_DATACAPTURE_A, 0x00);
	AcceleroTest_writeReg(FIFO_CTRL_REG_A, 0x00);
	AcceleroTest_writeReg(INT1_CFG_A, 0x00);
	AcceleroTest_writeReg(INT1_THS_A, 0x00);
	AcceleroTest_writeReg(INT1_DURATION_A, 0x00);
	AcceleroTest_writeReg(INT2_CFG_A, 0x00);
	AcceleroTest_writeReg(INT2_THS_A, 0x00);
	AcceleroTest_writeReg(INT2_DURATION_A, 0x00);
	AcceleroTest_writeReg(CLICK_CFG_A, 0x00);
	AcceleroTest_writeReg(CLICK_THS_A, 0x00);
	AcceleroTest_writeReg(TIME_LIMIT_A, 0x00);
	AcceleroTest_writeReg(TIME_LATENCY_A, 0x00);
	AcceleroTest_writeReg(Act_THS_A, 0x00);
	AcceleroTest_writeReg(Act_DUR_A, 0x00);

	// Update default settings
	AcceleroTest_getAccScale();
	AcceleroTest_getAccPowerMode();
	AcceleroTest_updateAccSensitivity();
	AcceleroTest_getAccEndian();
}

unsigned int AcceleroTest_selfTest_readNOST(float *accX_NOST_ave, float *accY_NOST_ave, float *accZ_NOST_ave)
{
	unsigned char status_a=0;
	unsigned int samplecnt=0;

	// Wait for stable output
	do
	{
		nrf_delay_ms(90);
		status_a = AcceleroTest_readReg(STATUS_REG_A);
	}
	while(!((status_a >> 3) & 1));

	// discard first sample
	AcceleroTest_readAccSample();

	// Average stable output on 5 measurements
	samplecnt = 0 ;

	for(int i = 0; i < 5; i++)
	{
		// Wait for new output
		do
		{
			nrf_delay_ms(20);
			status_a = AcceleroTest_readReg(STATUS_REG_A);
		}
		while(!((status_a >> 3) & 1));

		// Read a new sample
		AcceleroTest_readAccSample();
		*accX_NOST_ave += accX;
		*accY_NOST_ave += accY;
		*accZ_NOST_ave += accZ;
		samplecnt++;
	}

	// Compute average on NOST values
	assert(samplecnt>0); // divide by zero
	*accX_NOST_ave /= (int)samplecnt;
	*accY_NOST_ave /= (int)samplecnt;
	*accZ_NOST_ave /= (int)samplecnt;

	return samplecnt;
}

unsigned int AcceleroTest_selfTest_readST(float *accX_ST_ave, float *accY_ST_ave, float *accZ_ST_ave)
{
	unsigned char status_a=0;
	unsigned int samplecnt=0;

	// Wait for stable output
		do
		{
			nrf_delay_ms(90);
			status_a = AcceleroTest_readReg(STATUS_REG_A);
		}
		while(!((status_a >> 3) & 1));

		// discard first sample
		AcceleroTest_readAccSample();

		// Average stable output on 5 measurements
		samplecnt = 0;

		for(int i = 0; i < 5; i++)
		{
			// Wait for new output
			do
			{
				nrf_delay_ms(20);
				status_a = AcceleroTest_readReg(STATUS_REG_A);
			}
			while(!((status_a >> 3) & 1));

			// Read a new sample
			AcceleroTest_readAccSample();
			*accX_ST_ave += accX;
			*accY_ST_ave += accY;
			*accZ_ST_ave += accZ;
			samplecnt++;
		}

		// Compute average on NOST values
		assert(samplecnt>0); // divide by zero
		*accX_ST_ave /= (int)samplecnt;
		*accY_ST_ave /= (int)samplecnt;
		*accZ_ST_ave /= (int)samplecnt;

		return samplecnt;
}


// Based on page 25-26 of the datasheet: Only difference is SPI is NOT enabled
bool AcceleroTest_selfTest()
{
	float accX_NOST_ave=0, accY_NOST_ave=0, accZ_NOST_ave=0;
	float accX_ST_ave=0, accY_ST_ave=0, accZ_ST_ave=0;
	bool STpass=false;

	// Initialize sensor, turn on sensor, enable X/Y/Z axes, BDU=1, FS=2G, Normal mode, ODR = 100 Hz
	AcceleroTest_writeReg(CTRL_REG2_A, 0x00);
	AcceleroTest_writeReg(CTRL_REG3_A, 0x00);
	AcceleroTest_writeReg(CTRL_REG4_A, 0x80);
	AcceleroTest_writeReg(CTRL_REG1_A, 0x57);

	// Update settings for scale, power mode, etc.
	AcceleroTest_getAccScale();
	AcceleroTest_getAccPowerMode();
	AcceleroTest_updateAccSensitivity();
	AcceleroTest_getAccEndian();

	// Read initial average on all axes
	AcceleroTest_selfTest_readNOST(&accX_NOST_ave, &accY_NOST_ave, &accZ_NOST_ave);

	// Enable the self test 1
	AcceleroTest_writeReg(CTRL_REG4_A, 0x82);

	// Read self test average on all axes
	AcceleroTest_selfTest_readST(&accX_ST_ave, &accY_ST_ave, &accZ_ST_ave);

	// Disable self test and place in idle mode
	AcceleroTest_writeReg(CTRL_REG1_A, 0x00);
	AcceleroTest_writeReg(CTRL_REG4_A, 0x00);

	// Return PASS/FAIL based on results
	STpass = (abs(accX_ST_ave - accX_NOST_ave) >= MIN_SELFTEST_VALUE) && (abs(accX_ST_ave - accX_NOST_ave) <= MAX_SELFEST_VALUE);
	STpass &= (abs(accY_ST_ave - accY_NOST_ave) >= MIN_SELFTEST_VALUE) && (abs(accY_ST_ave - accY_NOST_ave) <= MAX_SELFEST_VALUE);
	STpass &= (abs(accZ_ST_ave - accZ_NOST_ave) >= MIN_SELFTEST_VALUE) && (abs(accZ_ST_ave - accZ_NOST_ave) <= MAX_SELFEST_VALUE);

	CommandLineInterface_printf("NOST\tX:%f, Y:%f, Z:%f\r\n",accX_NOST_ave,accY_NOST_ave,accZ_NOST_ave);
	CommandLineInterface_printf("ST\tX:%f, Y:%f, Z:%f\r\n",accX_ST_ave,accY_ST_ave,accZ_ST_ave);

	AcceleroTest_easyConfig();

	return STpass;
}

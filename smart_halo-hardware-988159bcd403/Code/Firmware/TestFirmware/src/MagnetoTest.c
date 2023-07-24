/*
 * MagnetoTest.c
 *
 *  Created on: Jun 6, 2016
 *      Author: sgelinas
 */

#include "MagnetoTest.h"
#include "AcceleroTest.h"
#include "I2C.h"
#include "SmartHalo.h"
#include "AcceleroTest.h"
#include "CommandLineInterface.h"
#include "SH_Accelero_Magneto_typedefs.h"
#include "nrf_delay.h"
#include "app_error.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



#define REGISTER_AUTO_INCREMENT		 (1 << 7)

// Constants in Gauss (mG)
#define MAG_SENS			1.5
#define MIN_SELFTEST_VALUE	15
#define MAX_SELFTEST_VALUE	500

static Endianness MAG_ENDIAN = LITTLEENDIAN;
static bool MAG_PRESENT = false;
static float magX = 0;
static float magY = 0;
static float magZ = 0;
static float magOffsetX = 0;
static float magOffsetY = 0;
static float magOffsetZ = 0;

void MagnetoTest_easyConfig();
void MagnetoTest_readMagOffset();
Endianness MagnetoTest_getMagEndian();

void MagnetoTest_checkPresent()
{
	ret_code_t err_code;
	uint8_t data = WHO_AM_I_M;
	uint8_t regdata;

	if(I2C_twi_tx(MAGNETO_ADDRESS, &data, 1, true) == NRF_SUCCESS)
	{
		MAG_PRESENT = true;

		err_code = I2C_twi_rx(MAGNETO_ADDRESS, &regdata, 1);
		APP_ERROR_CHECK(err_code);
	}
	else
	{
		CommandLineInterface_printLine("Magnetometer not detected");
	}
}

bool check_mag_acc_drivers()
{
	ret_code_t err_code;
	uint8_t data = WHO_AM_I_A;
	uint8_t regdata;
	bool is_connected = false;

	if(I2C_twi_tx(ACCELERO_ADDRESS, &data, 1, true) == NRF_SUCCESS)
	{
		err_code = I2C_twi_rx(ACCELERO_ADDRESS, &regdata, 1);
		APP_ERROR_CHECK(err_code);
	}
	else
	{
		CommandLineInterface_printLine("Accelerometer not detected");
		is_connected = true;
	}

	if(I2C_twi_tx(MAGNETO_ADDRESS, &data, 1, true) == NRF_SUCCESS)
	{
		err_code = I2C_twi_rx(MAGNETO_ADDRESS, &regdata, 1);
		APP_ERROR_CHECK(err_code);
	}
	else
	{
		CommandLineInterface_printLine("Magnetometer not detected");
		is_connected = true;
	}

	return is_connected;
}

void MagnetoTest_printHelp()
{

	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         MAG");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("getmagnetometerxyz:\t\t\tReturns the magnetic field (mG)");
	CommandLineInterface_printLine("getmagnetometercalibrationoffset:\tReturns the calibration offset");
	CommandLineInterface_printLine("setmagnetometercalibrationoffset <offsetx,offsety,offsetz>:\r\n\t\t\t\t\tSets the calibration offset");
	CommandLineInterface_printLine("getheading;\t\t\t\tReturns the heading toward north (deg)");
	CommandLineInterface_printLine("getmagminmaxxyz <samples>:\t\tReturns min/max magnetic field");
	CommandLineInterface_printLine("selftest <device>:\t\tRuns the selftest on the selected device (ACC, MAG)");
	nrf_delay_ms(50);
}

bool MagnetoTest_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	bool testPass = false;
	float mX, mY, mZ;
	float mOffX, mOffY, mOffZ;
	bool parsed = true;
	int value;

	if(strncmp(RxBuff, "GETMAGNETOMETERXYZ", 18)==0)
	{
		MagnetoTest_getMagXYZ(&mX, &mY, &mZ);
		CommandLineInterface_printf("%2.3f mG, %2.3f mG, %2.3f mG\r\n", mX, mY, mZ);
	}
	else if(strncmp(RxBuff, "GETMAGNETOMETERCALIBRATIONOFFSET", 32)==0)
	{
		MagnetoTest_getMagCalibOffset(&mOffX, &mOffY, &mOffZ);
		CommandLineInterface_printf("%2.3f mG, %2.3f mG, %2.3f mG\r\n", mOffX, mOffY, mOffZ);
	}
	else if(sscanf(RxBuff, "SETMAGNETOMETERCALIBRATIONOFFSET %f %f %f", &mOffX, &mOffY, &mOffZ)==3)
	{
		MagnetoTest_setMagCalibOffset(mOffX, mOffY, mOffZ);
		CommandLineInterface_printf("Calibration offset set to %2.3f mG, %2.3f mG, %2.3f mG\r\n", mOffX, mOffY, mOffZ);
	}
	else if(strncmp(RxBuff, "GETHEADING", 10)==0)
	{
		CommandLineInterface_printf("%3.1f degree\r\n", MagnetoTest_getHeading());
	}
	else if(strncmp(RxBuff, "SELFTEST MAG", 12)==0)
	{
		CommandLineInterface_printLine("Starting magnetometer self test");
		testPass = MagnetoTest_selfTest();
		testPass ? CommandLineInterface_printLine("PASS") : CommandLineInterface_printLine("FAIL");
	}
	else if(sscanf(RxBuff, "GETMAGMINMAXXYZ %d", &value)==1)
	{
		if(value < 0 || value > 500)
			CommandLineInterface_printf("Invalid samples: must be between %d and %d\r\n", 0, 100);
		else
		{
			MagnetoTest_getMagMinMaxXYZ(value);
		}
	}
	else
	{
		parsed = false;
	}
	return parsed;
}

void MagnetoTest_setup()
{
	MagnetoTest_checkPresent();

	// Set default config
	MagnetoTest_easyConfig();

	// Update default settings
	MagnetoTest_readMagOffset();
	MagnetoTest_getMagEndian();
}

void MagnetoTest_writeReg(MagnetoRegisters reg, unsigned char data)
{
	if(MAG_PRESENT)
		writeReg8(MAGNETO_ADDRESS, reg, data);
}

unsigned char MagnetoTest_readReg(MagnetoRegisters reg)
{
	if(MAG_PRESENT)
		return readReg8(MAGNETO_ADDRESS, reg);
	else
		return -1;
}

unsigned char MagnetoTest_getID()
{
	return MagnetoTest_readReg(WHO_AM_I_M);
}

void MagnetoTest_setMagEndian(Endianness endianness)
{
	unsigned char regdata;

	if (endianness != MAG_ENDIAN)
	{
		// Read relative registers and set only FS bits
		regdata = MagnetoTest_readReg(CFG_REG_C_M);
		regdata &= ~0x80;
		regdata |= (((unsigned char)endianness << 3) & 0x80);
		MagnetoTest_writeReg(CFG_REG_C_M, regdata);

		MAG_ENDIAN = endianness;
	}
}

Endianness MagnetoTest_getMagEndian()
{
	unsigned char regdata;

	regdata = MagnetoTest_readReg(CFG_REG_C_M);

	MAG_ENDIAN = (Endianness)((regdata >> 3) & 0x01);

	return MAG_ENDIAN;
}

// Block read: cannot use standard register read functions
void MagnetoTest_readMagSample()
{
	if(MAG_PRESENT)
	{
		ret_code_t err_code;

		uint8_t data = REGISTER_AUTO_INCREMENT | OUTX_L_REG_M; // MSB=1 to set address auto increment
		uint8_t regdata[6];

		err_code = I2C_twi_tx(MAGNETO_ADDRESS, &data, 1, true);
		APP_ERROR_CHECK(err_code);

		err_code = I2C_twi_rx(MAGNETO_ADDRESS, regdata, 6);
		APP_ERROR_CHECK(err_code);

		if(MAG_ENDIAN == LITTLEENDIAN)
		{
			magX = (float)((int16_t)(regdata[1] << 8) | regdata[0]);
			magY = (float)((int16_t)(regdata[3] << 8) | regdata[2]);
			magZ = (float)((int16_t)(regdata[5] << 8) | regdata[4]);
		}
		else
		{
			magX = (float)((int16_t)(regdata[0] << 8) | regdata[1]);
			magY = (float)((int16_t)(regdata[2] << 8) | regdata[3]);
			magZ = (float)((int16_t)(regdata[4] << 8) | regdata[5]);
		}
	}
}

// Block read: cannot use standard register read functions
void MagnetoTest_readMagOffset()
{
	if(MAG_PRESENT)
	{
		ret_code_t err_code;
		uint8_t data =  REGISTER_AUTO_INCREMENT | OFFSET_X_REG_L_M;  // MSB=1 to set address auto increment
		uint8_t regdata[6];

		err_code = I2C_twi_tx(MAGNETO_ADDRESS, &data, 1, true);
		APP_ERROR_CHECK(err_code);

		err_code = I2C_twi_rx(MAGNETO_ADDRESS, regdata, 6);
		APP_ERROR_CHECK(err_code);

		if(MAG_ENDIAN == LITTLEENDIAN)
		{
			magOffsetX = (float)((regdata[1] << 8) | regdata[0]);
			magOffsetY = (float)((regdata[3] << 8) | regdata[2]);
			magOffsetZ = (float)((regdata[5] << 8) | regdata[4]);
		}
		else
		{
			magOffsetX = (float)((regdata[0] << 8) | regdata[1]);
			magOffsetY = (float)((regdata[2] << 8) | regdata[3]);
			magOffsetZ = (float)((regdata[4] << 8) | regdata[5]);
		}
	}
}

// Block write: cannot use standard register write functions
void MagnetoTest_writeMagOffset()
{
	if(MAG_PRESENT)
	{
		ret_code_t err_code;
		uint8_t regdata[7];

		if(MAG_ENDIAN == LITTLEENDIAN)
		{
			regdata[0] =   REGISTER_AUTO_INCREMENT | OFFSET_X_REG_L_M;  // MSB=1 to set address auto increment
			regdata[1] = (int)(magOffsetX) & 0xFF;
			regdata[2] = ((int)(magOffsetX) >> 8) & 0xFF;
			regdata[3] = (int)(magOffsetY) & 0xFF;
			regdata[4] = ((int)(magOffsetY) >> 8) & 0xFF;
			regdata[5] = (int)(magOffsetZ) & 0xFF;
			regdata[6] = ((int)(magOffsetZ) >> 8) & 0xFF;
		}
		else
		{
			regdata[0] =   REGISTER_AUTO_INCREMENT | OFFSET_X_REG_L_M;  // MSB=1 to set address auto increment
			regdata[2] = (int)(magOffsetX) & 0xFF;
			regdata[1] = ((int)(magOffsetX) >> 8) & 0xFF;
			regdata[4] = (int)(magOffsetY) & 0xFF;
			regdata[3] = ((int)(magOffsetY) >> 8) & 0xFF;
			regdata[6] = (int)(magOffsetZ) & 0xFF;
			regdata[5] = ((int)(magOffsetZ) >> 8) & 0xFF;
		}

		err_code = I2C_twi_tx(MAGNETO_ADDRESS, regdata, 7, false);
		APP_ERROR_CHECK(err_code);
	}
}

void MagnetoTest_getMagXYZ(float *mX, float *mY, float *mZ)
{
	assert(mX!=NULL);assert(mY!=NULL);assert(mZ!=NULL);

	MagnetoTest_readMagSample();

	*mX = (float)(magX) * MAG_SENS;
	*mY = (float)(magY) * MAG_SENS;
	*mZ = (float)(magZ) * MAG_SENS;
}

void MagnetoTest_getMagCalibOffset(float *mOffsetX, float *mOffsetY, float *mOffsetZ)
{
	assert(mOffsetX!=NULL);assert(mOffsetY!=NULL);assert(mOffsetZ!=NULL);

	MagnetoTest_readMagOffset();

	*mOffsetX = (float)(magOffsetX) * MAG_SENS;
	*mOffsetY = (float)(magOffsetY) * MAG_SENS;
	*mOffsetZ = (float)(magOffsetZ) * MAG_SENS;
}

void MagnetoTest_setMagCalibOffset(float mOffsetX, float mOffsetY, float mOffsetZ)
{
	magOffsetX = ((mOffsetX) / MAG_SENS);
	magOffsetY = ((mOffsetY) / MAG_SENS);
	magOffsetZ = ((mOffsetZ) / MAG_SENS);

	MagnetoTest_writeMagOffset();
}

void MagnetoTest_getMagMinMaxXYZ(unsigned int samples)
{
	float mX, mY, mZ;
	float minX = 49150.5, minY = 49150.5, minZ = 49150.5;
	float maxX = -49152.0, maxY =-49152.0, maxZ = -49152.0;

	for(int i=0; i<samples; i++)
	{
		MagnetoTest_getMagXYZ(&mX,&mY,&mZ);

		if(mX < minX)
			minX = mX;
		else if(mX > maxX)
			maxX = mX;
		if(mY < minY)
			minY = mY;
		else if(mY > maxY)
			maxY = mY;
		if(mZ < minZ)
			minZ = mZ;
		else if(mZ > maxZ)
			maxZ = mZ;

		CommandLineInterface_printf("Min:%5.2f,%5.2f,%5.2f\t\tMax:%5.2f,%5.2f,%5.2f\r\n",minX,minY,minZ,maxX,maxY,maxZ);

		nrf_delay_ms(100);
	}
}

float MagnetoTest_getHeading()
{
	float mX, mY, mZ;
	float aX, aY, aZ;
	float Ex, Ey, Ez;
	float Nx, Ny, Nz;
	float mag;
	float heading;

	// Take a sample
	MagnetoTest_getMagXYZ(&mX, &mY, &mZ);
	AcceleroTest_getAccXYZ(&aX, &aY, &aZ);

	// Cross-product magnetic Vector Vs acceleration vector
	Ex = (mY * aZ) - (mZ * aY);
	Ey = (mZ * aX) - (mX * aZ);
	Ez = (mX * aY) - (mY * aX);

	// Normalize
	mag = sqrt((Ex*Ex) + (Ey*Ey) + (Ez*Ez));
	Ex = Ex/mag;
	Ey = Ey/mag;
	Ez = Ez/mag;

	// Cross-product acceleration vector Vs E vector
	Nx = (aY * Ez) - (aZ * Ey);
	Ny = (aZ * Ex) - (aX * Ez);
	Nz = (aX * Ey) - (aY * Ex);

	// Normalize
	mag = sqrt((Nx*Nx) + (Ny*Ny) + (Nz*Nz));
	Nx = Nx/mag;
	Ny = Ny/mag;
	Nz = Nz/mag;

	// Heading calculation from Y axis
	heading = atan2(-Ey,-Ny) * 180.0 / M_PI;

	if(heading < 0)
		heading += 360;

	return heading;
}

unsigned int MagnetoTest_selfTest_readNOST(float * magX_NOST_ave, float * magY_NOST_ave, float * magZ_NOST_ave)
{
	unsigned char status_m=0;
	unsigned int samplecnt=0;

	// Wait for stable output
	do
	{
		nrf_delay_ms(20);
		status_m = MagnetoTest_readReg(STATUS_REG_M);
	}
	while(!((status_m >> 3) & 1));

	// discard first sample
	MagnetoTest_readMagSample();

	// Average stable output on 50 measurements
	samplecnt = 0 ;

	for(int i = 0; i < 50; i++)
	{
		// Wait for new output
		do
		{
			nrf_delay_ms(20);
			status_m = MagnetoTest_readReg(STATUS_REG_M);
		}
		while(!((status_m >> 3) & 1));

		// Read a new sample
		MagnetoTest_readMagSample();
		*magX_NOST_ave += (float)magX * MAG_SENS;
		*magY_NOST_ave += (float)magY * MAG_SENS;
		*magZ_NOST_ave += (float)magZ * MAG_SENS;
		samplecnt++;
	}

	// Compute average on NOST values
	assert(samplecnt>0); // divide by zero
	*magX_NOST_ave /= (int)samplecnt;
	*magY_NOST_ave /= (int)samplecnt;
	*magZ_NOST_ave /= (int)samplecnt;

	return samplecnt;
}

unsigned int MagnetoTest_selfTest_readST(float * magX_ST_ave, float * magY_ST_ave, float * magZ_ST_ave)
{
	unsigned char status_m=0;
	unsigned int samplecnt=0;

	// Wait for stable output
	do
	{
		nrf_delay_ms(60);
		status_m = MagnetoTest_readReg(STATUS_REG_M);
	}
	while(!((status_m >> 3) & 1));

	// discard first sample
	MagnetoTest_readMagSample();

	// Average stable output on 50 measurements
	samplecnt = 0;

	for(int i = 0; i < 50; i++)
	{
		// Wait for new output
		do
		{
			nrf_delay_ms(20);
			status_m = MagnetoTest_readReg(STATUS_REG_M);
		}
		while(!((status_m >> 3) & 1));

		// Read a new sample
		MagnetoTest_readMagSample();
		*magX_ST_ave += (float)magX * MAG_SENS;
		*magY_ST_ave += (float)magY * MAG_SENS;
		*magZ_ST_ave += (float)magZ * MAG_SENS;
		samplecnt++;
	}

	// Compute average on NOST values
	assert(samplecnt>0); // divide by zero
	*magX_ST_ave /= (int)samplecnt;
	*magY_ST_ave /= (int)samplecnt;
	*magZ_ST_ave /= (int)samplecnt;

	return samplecnt;
}

// Based on Unico v4.4.2 Easyconfiguration (see Produit/Developement/AcceleroMagnetor/EasyConfig.doc)
// No hard-iron compensation, High resolution mode, ODR = 100 Hz, Continuous measurement
// Offset cancellation enabled, digital filter bypassed, Interrupt disabled
void MagnetoTest_easyConfig()
{
	MagnetoTest_writeReg(OFFSET_X_REG_L_M, 0x00);
	MagnetoTest_writeReg(OFFSET_X_REG_H_M, 0x00);
	MagnetoTest_writeReg(OFFSET_Y_REG_L_M, 0x00);
	MagnetoTest_writeReg(OFFSET_Y_REG_H_M, 0x00);
	MagnetoTest_writeReg(OFFSET_Z_REG_L_M, 0x00);
	MagnetoTest_writeReg(OFFSET_Z_REG_H_M, 0x00);
	MagnetoTest_writeReg(CFG_REG_A_M, 0x0C);
	MagnetoTest_writeReg(CFG_REG_B_M, 0x02);
	MagnetoTest_writeReg(CFG_REG_C_M, 0x00);
	MagnetoTest_writeReg(INT_CRTL_REG_M, 0xE0);
	MagnetoTest_writeReg(INT_THS_L_REG_M, 0x00);
	MagnetoTest_writeReg(INT_THS_H_REG_M, 0x00);
}

// Based on page 25-26 of the datasheet
bool MagnetoTest_selfTest()
{
	float magX_NOST_ave=0, magY_NOST_ave=0, magZ_NOST_ave=0;
	float magX_ST_ave=0, magY_ST_ave=0, magZ_ST_ave=0;
	bool STpass=false;

	// Initialize sensor, turn on sensor, BDU, Continous mode with offset cancellation at ODR = 100 Hz
	MagnetoTest_writeReg(CFG_REG_A_M, 0x8C);
	MagnetoTest_writeReg(CFG_REG_B_M, 0x02);
	MagnetoTest_writeReg(CFG_REG_C_M, 0x10);

	// Read initial average on all axes
	MagnetoTest_selfTest_readNOST(&magX_NOST_ave, &magY_NOST_ave, &magZ_NOST_ave);

	// Enable the self test
	MagnetoTest_writeReg(CFG_REG_C_M, 0x12);

	// Read self test average on all axes
	MagnetoTest_selfTest_readST(&magX_ST_ave, &magY_ST_ave, &magZ_ST_ave);

	// Disable self test and place in idle mode
	MagnetoTest_writeReg(CFG_REG_C_M, 0x10);
	MagnetoTest_writeReg(CFG_REG_A_M, 0x83);

	// Return PASS/FAIL based on results
	STpass = (abs(magX_ST_ave - magX_NOST_ave) >= MIN_SELFTEST_VALUE) && (abs(magX_ST_ave - magX_NOST_ave) <= MAX_SELFTEST_VALUE);
	STpass &= (abs(magY_ST_ave - magY_NOST_ave) >= MIN_SELFTEST_VALUE) && (abs(magY_ST_ave - magY_NOST_ave) <= MAX_SELFTEST_VALUE);
	STpass &= (abs(magZ_ST_ave - magZ_NOST_ave) >= MIN_SELFTEST_VALUE) && (abs(magZ_ST_ave - magZ_NOST_ave) <= MAX_SELFTEST_VALUE);

	CommandLineInterface_printf("NOST\tX:%f, Y:%f, Z:%f\r\n",magX_NOST_ave,magY_NOST_ave,magZ_NOST_ave);
	CommandLineInterface_printf("ST\tX:%f, Y:%f, Z:%f\r\n",magX_ST_ave,magY_ST_ave,magZ_ST_ave);

	return STpass;
}

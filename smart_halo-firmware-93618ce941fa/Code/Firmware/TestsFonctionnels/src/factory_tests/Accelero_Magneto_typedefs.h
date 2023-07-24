/*

 * SH_Accelero_Magneto_typedefs.h
 *
 *  Created on: 2016-02-22
 *      Author: SmartHalo
 */

#ifndef SH_ACCELERO_MAGNETO_TYPEDEFS_H_
#define SH_ACCELERO_MAGNETO_TYPEDEFS_H_

#define MAGNETO_ADDRESS 			((0x3C >> 1) & (0x7F))
#define ACCELERO_ADDRESS 			 ((0x32 >> 1) & (0x7F))

typedef enum
{
	OFFSET_X_REG_L_M =			0x45,
	OFFSET_X_REG_H_M =			0x46,
	OFFSET_Y_REG_L_M =			0x47,
	OFFSET_Y_REG_H_M =			0x48,
	OFFSET_Z_REG_L_M =			0x49,
	OFFSET_Z_REG_H_M =			0x4A,
	WHO_AM_I_M =				0x4F,
	CFG_REG_A_M	=				0x60,
	CFG_REG_B_M	=				0x61,
	CFG_REG_C_M	=				0x62,
	INT_CRTL_REG_M =			0x63,
	INT_SOURCE_REG_M =			0x64,
	INT_THS_L_REG_M =			0x65,
	INT_THS_H_REG_M	=			0x66,
	STATUS_REG_M =				0x67,
	OUTX_L_REG_M =				0x68,
	OUTX_H_REG_M =				0x69,
	OUTY_L_REG_M =				0x6A,
	OUTY_H_REG_M =				0x6B,
	OUTZ_L_REG_M =				0x6C,
	OUTZ_H_REG_M =				0x6D
} MagnetoRegisters;

typedef enum
{
	LITTLEENDIAN,
	BIGENDIAN
} Endianness;

typedef enum
{
	ODR_10_HZ = 10,
	ODR_20_HZ = 20,
	ODR_50_HZ = 50,
	ODR_100_HZ = 100
} MagnetoODRmode;

typedef enum
{
	POWER_DOWN_MODE_DATA_RATE = 0,
	FIRST_DATA_RATE = 1,
	SECOND_DATA_RATE = 10,
	THIRD_DATA_RATE	= 25,
	FOURTH_DATA_RATE = 50,
	FIFTH_DATA_RATE = 100,
	SIXTH_DATA_RATE	= 200,
	SEVENTH_DATA_RATE = 400,
	LOW_POWER_MODE_ONLY_DATA_RATE = 1620,
	HR_NORMAL_DATA_RATE = 1344,
	LOW_POWER_DATA_RATE = 5376
} AcceleroODRmode;

typedef enum
{
	STATUS_REG_AUX_A = 			0x07,
	OUT_TEMP_L_A =				0x0C,
	OUT_TEMP_H_A =				0x0D,
	INT_COUNTER_REG_A =			0x0E,
	WHO_AM_I_A =				0x0F,
	TEMP_CFG_REG_A =			0x1F,
	CTRL_REG1_A =				0x20,
	CTRL_REG2_A =				0x21,
	CTRL_REG3_A =				0x22,
	CTRL_REG4_A =				0x23,
	CTRL_REG5_A =				0x24,
	CTRL_REG6_A	=				0x25,
	REFERENCE_DATACAPTURE_A	=	0x26,
	STATUS_REG_A =				0x27,
	OUT_X_L_A =					0x28,
	OUT_X_H_A =					0x29,
	OUT_Y_L_A =					0x2A,
	OUT_Y_H_A =					0x2B,
	OUT_Z_L_A =					0x2C,
	OUT_Z_H_A =					0x2D,
	FIFO_CTRL_REG_A =			0x2E,
	FIFO_SRC_REG_A =			0x2F,
	INT1_CFG_A =				0x30,
	INT1_SRC_A =				0x31,
	INT1_THS_A =				0x32,
	INT1_DURATION_A =			0x33,
	INT2_CFG_A =				0x34,
	INT2_SRC_A =				0x35,
	INT2_THS_A =				0x36,
	INT2_DURATION_A =			0x37,
	CLICK_CFG_A =				0x38,
	CLICK_SRC_A	=				0x39,
	CLICK_THS_A	=				0x3A,
	TIME_LIMIT_A =				0x3B,
	TIME_LATENCY_A =			0x3C,
	TIME_WINDOW_A =				0x3D,
	Act_THS_A =					0x3E,
	Act_DUR_A =					0x3F
} AcceleroRegisters;

typedef enum
{
	POWERMODE_NORMAL,
	POWERMODE_HIGHRESOLUTION,
	POWERMODE_LOWPOWER
} AcceleroPowerMode;

typedef enum
{
	SCALE2G,
	SCALE4G,
	SCALE8G,
	SCALE16G
} AcceleroFS;

typedef enum{
	FS2G = 2,
	FS4G = 4,
	FS8G = 8,
	FS16G = 16
}AcceleroFullScaleValue;

typedef enum{
	INTTHS_16mg = 16,
	INTTHS_32mg = 32,
	INTTHS_62mg = 62,
	INTTHS_186mg = 186
}Accelero_Int_THS_LSB_Value;



//used for the accelelrometer/magnetometer
typedef enum Accelerometer_Power_Mode Accelerometer_Power_Mode;
enum Accelerometer_Power_Mode	{LOW_POWER_MODE = 0, NORMAL_MODE, HIGH_RESOLUTION_MODE};

//used for the accelelrometer/magnetometer
typedef enum Accelerometer_Magnetometer_System_mode Accelerometer_Magnetometer_System_mode;
enum Accelerometer_Magnetometer_System_mode	{CONTINUOUS_MODE = 0, SINGLE_MODE, IDLE_MODE};

#endif /* SH_ACCELERO_MAGNETO_TYPEDEFS_H_ */

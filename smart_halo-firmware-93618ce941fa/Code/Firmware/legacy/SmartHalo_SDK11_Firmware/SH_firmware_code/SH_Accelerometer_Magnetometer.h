/*

 * SH_Accelerometer_Magnetometer.h
 *
 *  Created on: 2016-02-22
 *      Author: SmartHalo
 */

#ifndef SH_ACCELEROMETER_MAGNETOMETER_H_
#define SH_ACCELEROMETER_MAGNETOMETER_H_

#include "SH_Includes.h"

#define ACCELEROMETER_DRIVER_ADDRESS				0b0011001
#define MAGNETOMETER_DRIVER_ADDRESS					0b0011110

//STATUS_REG_AUX_A
#define TOR											SEVENTH_BIT
#define TDA											THIRD_BIT

//TEMP_CFG_REG_A
#define TEMP_EN0									SEVENTH_BIT
#define TEMP_EN1									EIGHTH_BIT

//CTRL_REG1_A
#define ODR3										EIGHTH_BIT
#define ODR2										SEVENTH_BIT
#define ODR1										SIXTH_BIT
#define ODR0										FIFTH_BIT
#define LPen										FOURTH_BIT
#define Zen											THIRD_BIT
#define Yen											SECOND_BIT
#define Xen											FIRST_BIT

//CTRL_REG2_A
#define HPM1										EIGHTH_BIT
#define HPM0										SEVENTH_BIT
#define HPCF2										SIXTH_BIT
#define HPCF1										FIFTH_BIT
#define FDS											FOURTH_BIT
#define HPCLICK										THIRD_BIT
#define HPIS2										SECOND_BIT
#define HPIS1										FIRST_BIT

//CTRL_REG3_A
#define I1_CLICK									EIGHTH_BIT
#define I1_AOI1										SEVENTH_BIT
#define I1_AOI2										SIXTH_BIT
#define I1_DRDY1									FIFTH_BIT
#define I1_DRDY2									FOURTH_BIT
#define I1_WTM										THIRD_BIT
#define I1_OVERRUN									SECOND_BIT

//CTRL_REG4_A
#define BDU											EIGHTH_BIT
#define BLE											SEVENTH_BIT
#define FS1											SIXTH_BIT
#define FS0											FIFTH_BIT
#define HR											FOURTH_BIT
#define ST1											THIRD_BIT
#define ST0											SECOND_BIT
#define SPI_ENABLE									FIRST_BIT

//CTRL_REG5_A
#define BOOT										EIGHTH_BIT
#define FIFO_EN										SEVENTH_BIT
#define LIR_INT1									FOURTH_BIT
#define D4D_INT1									THIRD_BIT
#define LIR_INT2									SECOND_BIT
#define D4D_INT2									FIRST_BIT

//CTRL_REG6_A
#define I2_CLICKen									EIGHTH_BIT
#define I2_INT1										SEVENTH_BIT
#define I2_INT2										SIXTH_BIT
#define BOOT_I2										FIFTH_BIT
#define P2_ACT										FOURTH_BIT
#define H_LACTIVE									SECOND_BIT

//STATUS_REG_A
#define ZYXOR										EIGHTH_BIT
#define ZOR											SEVENTH_BIT
#define YOR											SIXTH_BIT
#define XOR											FIFTH_BIT
#define ZYXDA										FOURTH_BIT
#define ZDA											THIRD_BIT
#define YDA											SECOND_BIT
#define XDA											FIRST_BIT

//STATUS_REG_A
#define ZYXOR										EIGHTH_BIT
#define ZOR											SEVENTH_BIT
#define YOR											SIXTH_BIT
#define XOR											FIFTH_BIT
#define ZYXDA										FOURTH_BIT
#define ZDA											THIRD_BIT
#define YDA											SECOND_BIT
#define XDA											FIRST_BIT

//FIFO_CTRL_REG_A
#define FM1											EIGHTH_BIT
#define FM0											SEVENTH_BIT
#define TR											SIXTH_BIT
#define FTH4										FIFTH_BIT
#define FTH3										FOURTH_BIT
#define FTH2										THIRD_BIT
#define FTH1										SECOND_BIT
#define FTH0										FIRST_BIT

//FIFO_SRC_REG_A
#define WTM											EIGHTH_BIT
#define OVRN_FIFO									SEVENTH_BIT
#define EMPTY										SIXTH_BIT
#define FSSA										FIFTH_BIT
#define FSS3										FOURTH_BIT
#define FSS2										THIRD_BIT
#define FSS1										SECOND_BIT
#define FSS0										FIRST_BIT

//INT1(2)_CFG_A
#define AOI											EIGHTH_BIT
#define SIXD										SEVENTH_BIT
#define ZHIE_ZUPE									SIXTH_BIT
#define ZLIE_ZDOWNE									FIFTH_BIT
#define YHIE_YUPE									FOURTH_BIT
#define YLIE_YDOWNE									THIRD_BIT
#define XHIE_XUPE									SECOND_BIT
#define XLIE_XDOWNE									FIRST_BIT

//INT1(2)_SRC_A
#define IA											SEVENTH_BIT
#define ZH											SIXTH_BIT
#define ZL											FIFTH_BIT
#define YH											FOURTH_BIT
#define YL											THIRD_BIT
#define XH											SECOND_BIT
#define XL											FIRST_BIT

//INT1(2)_THS_A
#define THS6										SEVENTH_BIT
#define THS5										SIXTH_BIT
#define THS4										FIFTH_BIT
#define THS3										FOURTH_BIT
#define THS2										THIRD_BIT
#define THS1										SECOND_BIT
#define THS0										FIRST_BIT

//INT1(2)_DURATION_A
#define D6											SEVENTH_BIT
#define D5											SIXTH_BIT
#define D4											FIFTH_BIT
#define D3											FOURTH_BIT
#define D2											THIRD_BIT
#define D1											SECOND_BIT
#define D0											FIRST_BIT

//CLICK_CFG_A
#define ZD											SIXTH_BIT
#define ZS											FIFTH_BIT
#define YD											FOURTH_BIT
#define YS											THIRD_BIT
#define XD											SECOND_BIT
#define XS											FIRST_BIT

//CLICK_SRC_A
#define IA_CLICK									SIXTH_BIT
#define DClick										FIFTH_BIT
#define SClick										FOURTH_BIT
#define Z											THIRD_BIT
#define Y											SECOND_BIT
#define X											FIRST_BIT

//CLICK_THS_A
#define Ths6										SEVENTH_BIT
#define Ths5										SIXTH_BIT
#define Ths4										FIFTH_BIT
#define Ths3										FOURTH_BIT
#define Ths2										THIRD_BIT
#define Ths1										SECOND_BIT
#define Ths0										FIRST_BIT

//TIME_LIMIT_A
#define TLI6										SEVENTH_BIT
#define TLI5										SIXTH_BIT
#define TLI4										FIFTH_BIT
#define TLI3										FOURTH_BIT
#define TLI2										THIRD_BIT
#define TLI1										SECOND_BIT
#define TLI0										FIRST_BIT

//TIME_LATENCY_A
#define TLA7										EIGHTH_BIT
#define TLA6										SEVENTH_BIT
#define TLA5										SIXTH_BIT
#define TLA4										FIFTH_BIT
#define TLA3										FOURTH_BIT
#define TLA2										THIRD_BIT
#define TLA1										SECOND_BIT
#define TLA0										FIRST_BIT

//TIME_WINDOW_A
#define TW7											EIGHTH_BIT
#define TW6											SEVENTH_BIT
#define TW5											SIXTH_BIT
#define TW4											FIFTH_BIT
#define TW3											FOURTH_BIT
#define TW2											THIRD_BIT
#define TW1											SECOND_BIT
#define TW0											FIRST_BIT

//Act_THS_A
#define Acth6										SEVENTH_BIT
#define Acth5										SIXTH_BIT
#define Acth4										FIFTH_BIT
#define Acth3										FOURTH_BIT
#define Acth2										THIRD_BIT
#define Acth1										SECOND_BIT
#define Acth0										FIRST_BIT

//Act_DUR_A
#define ActD7										EIGHTH_BIT
#define ActD6										SEVENTH_BIT
#define ActD5										SIXTH_BIT
#define ActD4										FIFTH_BIT
#define ActD3										FOURTH_BIT
#define ActD2										THIRD_BIT
#define ActD1										SECOND_BIT
#define ActD0										FIRST_BIT

//CFG_REG_A(C)_M
#define COMP_TEMP_EN								SEVENTH_BIT
#define SOFT_RST									SIXTH_BIT
#define LP											FIFTH_BIT
#define ODR1_M										FOURTH_BIT
#define ODR0_M										THIRD_BIT
#define MD1											SECOND_BIT
#define MD0											FIRST_BIT

//CFG_REG_B_M
#define INT_on_DataOFF								FOURTH_BIT
#define Set_FREQ									THIRD_BIT
#define OFF_CANC									SECOND_BIT
#define LPF											FIRST_BIT

//CFG_REG_C_M
#define INT_MAG_PIN_S								SEVENTH_BIT
#define I2C_DIS										SIXTH_BIT
#define BDU_M										FIFTH_BIT
#define BLE_M										FOURTH_BIT
#define Self_test									SECOND_BIT
#define INT_MAG										FIRST_BIT

//INT_CTRL_REG_M
#define XIEN										EIGHTH_BIT
#define YIEN										SEVENTH_BIT
#define ZIEN										SIXTH_BIT
#define IEA											THIRD_BIT
#define IEL											SECOND_BIT
#define IEN											FIRST_BIT

//INT_SOURCE_REG_M
#define P_TH_S_X									EIGHTH_BIT
#define P_TH_S_Y									SEVENTH_BIT
#define P_TH_S_Z									SIXTH_BIT
#define N_TH_S_X									FIFTH_BIT
#define N_TH_S_Y									FOURTH_BIT
#define N_TH_S_Z									THIRD_BIT
#define MROI										SECOND_BIT
#define INT											FIRST_BIT

//INT_THS_L_REG_M
#define TH7											EIGHTH_BIT
#define THS6										SEVENTH_BIT
#define TH5											SIXTH_BIT
#define TH4											FIFTH_BIT
#define TH3											FOURTH_BIT
#define TH2											THIRD_BIT
#define TH1											SECOND_BIT
#define TH0											FIRST_BIT

//INT_THS_H_REG_M
#define TH7											EIGHTH_BIT
#define THS6										SEVENTH_BIT
#define TH5											SIXTH_BIT
#define TH4											FIFTH_BIT
#define TH3											FOURTH_BIT
#define TH2											THIRD_BIT
#define TH1											SECOND_BIT
#define TH0											FIRST_BIT

//STATUS_REG_M
#define Zyor										EIGHTH_BIT
#define zor											SEVENTH_BIT
#define yor											SIXTH_BIT
#define xor											FIFTH_BIT
#define Zyxda										FOURTH_BIT
#define zda											THIRD_BIT
#define yda											SECOND_BIT
#define xda											FIRST_BIT

//ODR_Accelerometer
#define POWER_DOWN_MODE_ACCELEROMETER				0b00000000
#define _1_HZ_MODE_ACCELEROMETER					0b00010000 	//ALL MODES
#define _10_HZ_MODE_ACCELEROMETER					0b00100000	//ALL MODES
#define _25_HZ_MODE_ACCELEROMETER					0b00110000	//ALL MODES
#define _50_HZ_MODE_ACCELEROMETER					0b01000000	//ALL MODES
#define _100_HZ_MODE_ACCELEROMETER					0b01010000	//ALL MODES
#define _200_HZ_MODE_ACCELEROMETER					0b01100000	//ALL MODES
#define _400_HZ_MODE_ACCELEROMETER					0b01110000	//ALL MODES
#define _1_620KHZ_MODE_ACCELEROMETER				0b10000000 //LOW-POWER MODE ONLY
#define _1_344KHZ_OR_5_376KHZ_MODE_ACCELEROMETER 	0b10010000 //LOW-POWER MODE 5.376KHZ AND NORMAL AND HR MODE 1.344KHZ

//INTERRUPT 2 DATA
#define INT_2_Z_UP									true
#define INT_2_Z_DOWN								false
#define INT_2_Y_UP									true
#define	INT_2_Y_DOWN								false
#define INT_2_X_UP									true
#define	INT_2_X_DOWN								false
#define INT_2_AOI_EVENT 							false //OR
#define INT_2_6D_DETECTION							false
#define INT_2_DURATION_VALUE 	 	 	 	 	 	0
#define INT_2_THS_VALUE								200

//FIFO STATES
#define FIFO_BYPASS									30
#define FIFO_MODE			                        31
#define FIFO_STREAM                                 32
#define FIFO_STOP_TRIG                              33
#define FIFO_READ									34
#define FIFO_RUN									35

/* FIFO MODES */
#define FIFO_MDE_BYPASS                             0x0
#define FIFO_MDE_MODE                               0x1
#define FIFO_STREAM_MODE                            0x2
#define FIFO_STR2FIFO_MODE                          0x3
#define FIFO_SHIFT                                  6
#define FIFO_MODE_MASK                              0xC0
#define FIFO_ENABLE									0x40

/* The FIFO has 256 triplets */
#define FIFO_NUM_ENTRIES                            32

#define SINGLE_CLICK								1
#define DOUBLE_CLICK								0

#define TEMPERATURE_OFFSET							25 //in deg c

#if SINGLE_CLICK
#define CLICK_CFG									0x15
#define CLICK_TIME_WINDOW							0x00
#endif

#if DOUBLE_CLICK
#define CLICK_CFG									0x2A
#define CLICK_TIME_WINDOW							0x30
#endif

#define CLICK_THS									0x10
#define CLICK_TIME_LATENCY							0x10
#define CLICK_TIME_LIMIT							0x20

#define BIT_SHIFT_DATA								8
#define BIT_SHIFT_FOR_12_BIT						4

#define ACCELEROMETER_BLOCK_DATA_UPDATE				false

#define ACT_DUR_A_LSB_INITIAL_VALUE					8
#define ACT_THS_A_LSB_INITIAL_VALUE 				128

void ACCELEROMETER_set_power_mode(Accelerometer_Power_Mode accelerometer_power_mode_value);
void ACCELEROMETER_set_full_scale_value(uint8_t full_scale_value);
void ACCELEROMETER_set_data_rate_configuration(uint16_t ODR_value);

void reboot_acclerometer();

void ACCELEROMETER_enable_x_axis(bool enable_x_axis);
void ACCELEROMETER_enable_y_axis(bool enable_y_axis);
void ACCELEROMETER_enable_z_axis(bool enable_z_axis);

void ACCELEROMETER_MAGNETOMETER_set_temperature_sensor(bool enable_temperature_sensor);
void ACCELEROMETER_Set_FIFO_Mode(uint8_t mode);
void ACCELEROMETER_initizalize_int2();
void ACCELEROMETER_initialize_int1_for_FIFO();
void ACCELEROMETER_set_bdu();
void SH_disable_interrupts_from_accelerometer();
void SH_enable_interrupts_accelerometer();


void SH_initialization_of_accelerometer22();

uint8_t check_register(size_t driver_address, size_t register_address);

void MAGNETOMETER_set_ODR(MagnetoODRmode ODR_value);
void MAGNETOMETER_set_system_mode(Accelerometer_Magnetometer_System_mode mode);
void MAGNETOMETER_set_low_power_mode(bool enable_low_power_mode);
void MAGNETOMETER_temperature_compensation();
void MAGNETOMETER_set_interrupt_for_drdy(bool enable_interrupts);

void SH_enable_click_detection(bool enable_click_detection);
void SH_initialize_click_detection();

bool ACCELEROMETER_get_axis_value();
int16_t ACCELEROMETER_x_axis_data();
int16_t ACCELEROMETER_y_axis_data();
int16_t ACCELEROMETER_z_axis_data();

bool MAGNETOMETER_get_axis_value();
int16_t MAGNETOMETER_x_axis_data();
int16_t MAGNETOMETER_y_axis_data();
int16_t MAGNETOMETER_z_axis_data();

int8_t ACCELEROMETER_MAGNETOMETER_retrieve_temperature_sensor_data();

void ACCELEROMETER_get_src_int2();
bool is_int2_z_high_value_event();
bool is_int2_z_low_value_event();
bool is_int2_y_high_value_event();
bool is_int2_y_low_value_event();
bool is_int2_x_high_value_event();
bool is_int2_x_low_value_event();


uint8_t Get_FIFO_Mode(void);
uint16_t FIFO_Get_Depth(void);
uint8_t FIFO_Set_Depth(uint8_t depth);
uint8_t FIFO_Get_Thr(void);
uint8_t FIFO_Get_Overrun(void);
uint8_t FIFO_Get_Stored(void);


#endif /* SH_ACCELEROMETER_MAGNETOMETER_H_ */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "platform.h"

#include "scheduler.h"
#include "bslink.h"
#include "dispatch.h"
#include "twi.h"
#include "alarm.h"
#include "matrix.h"
#include "calibration.h"
#include "leds.h"
#include "record.h"
#include "ui.h"
#include "touch.h"
#include "power.h"
#include "sound.h"
#include "bleapp.h"
#include "device.h"

#include "meters.h"

#if !defined(PLATFORM_pca10040) && !defined(PLATFORM_shmp)

bool met_agr;

typedef enum {
    ACC_MIN=0,
    ACC_MAX,
    MAG_XY_MIN,
    MAG_XY_MAX,
    MAG_Z_MIN,
    MAG_Z_MAX,
} met_st_range_t;

uint32_t met_st_range_agr[] = {
    17,
    360,
    15,
    500,
    15,
    500
};

uint32_t met_st_range_ctr[] = {
    70,
    1500,
    1000,
    3000,
    100,
    1000
};

uint32_t *met_st_range;

double met_acc_conv, met_mag_conv;

#define LSM303AGR_ACC_SENS_2G_HR        0.98
#define LSM303CTR_ACC_SENS_2G           0.061
#define LSM303AGR_MAG_SENS                  1.5
#define LSM303CTR_MAG_SENS                      0.58


#define MOVEMENT_MONITOR 15000

#define REGISTER_AUTO_INCREMENT      (1 << 7)

#define TWI_ADDR_AGR_ACC             ((0x32 >> 1) & (0x7F))
#define TWI_ADDR_AGR_MAG             ((0x3C >> 1) & (0x7F))

#define TWI_ADDR_CTR_ACC             ((0x3A >> 1) & (0x7F))
#define TWI_ADDR_CTR_MAG             ((0x3C >> 1) & (0x7F))

// CTR

#define CTR_WHO_AM_I_M                0x0F
#define CTR_CTRL_REG1_M               0x20
#define CTR_CTRL_REG2_M               0x21
#define CTR_CTRL_REG3_M               0x22
#define CTR_CTRL_REG4_M               0x23
#define CTR_CTRL_REG5_M               0x24
#define CTR_STATUS_REG_M              0x27
#define CTR_OUT_X_L_M                 0x28
#define CTR_OUT_X_H_M                 0x29
#define CTR_OUT_Y_L_M                 0x2A
#define CTR_OUT_Y_H_M                 0x2B
#define CTR_OUT_Z_L_M                 0x2C
#define CTR_OUT_Z_H_M                 0x2D
#define CTR_TEMP_L_M                  0x2E
#define CTR_TEMP_H_M                  0x2F
#define CTR_INT_CFG_M                 0x30
#define CTR_INT_SRC_M                 0x31
#define CTR_INT_THS_L_M               0x32
#define CTR_INT_THS_H_M               0x33

#define CTR_WHO_AM_I_A                 0x0F
#define CTR_ACT_THS_A                  0x1E
#define CTR_ACT_DUR_A                  0x1F
#define CTR_CTRL_REG1_A                0x20
#define CTR_CTRL_REG2_A                0x21
#define CTR_CTRL_REG3_A                0x22
#define CTR_CTRL_REG4_A                0x23
#define CTR_CTRL_REG5_A                0x24
#define CTR_CTRL_REG6_A                0x25
#define CTR_CTRL_REG7_A                0x26
#define CTR_STATUS_REG_A               0x27
#define CTR_OUT_X_L_A                  0x28
#define CTR_OUT_X_H_A                  0x29
#define CTR_OUT_Y_L_A                  0x2A
#define CTR_OUT_Y_H_A                  0x2B
#define CTR_OUT_Z_L_A                  0x2C
#define CTR_OUT_Z_H_A                  0x2D
#define CTR_FIFO_CTRL                  0x2E
#define CTR_FIFO_SRC                   0x2F
#define CTR_IG_CFG1_A                  0x30
#define CTR_IG_SRC1_A                  0x31
#define CTR_IG_THS_X1_A                0x32
#define CTR_IG_THS_Y1_A                0x33
#define CTR_IG_THS_Z1_A                0x34
#define CTR_IG_DUR1_A                  0x35
#define CTR_IG_CFG2_A                  0x36
#define CTR_IG_SRC2_A                  0x37
#define CTR_IG_THS2_A                  0x38
#define CTR_IG_DUR2_A                  0x39
#define CTR_XL_REFERENCE               0x3A
#define CTR_XH_REFERENCE               0x3B
#define CTR_YL_REFERENCE               0x3C
#define CTR_YH_REFERENCE               0x3D
#define CTR_ZL_REFERENCE               0x3E
#define CTR_ZH_REFERENCE               0x3

// AGR

#define AGR_OFFSET_X_REG_L_M          0x45
#define AGR_OFFSET_X_REG_H_M          0x46
#define AGR_OFFSET_Y_REG_L_M          0x47
#define AGR_OFFSET_Y_REG_H_M          0x48
#define AGR_OFFSET_Z_REG_L_M          0x49
#define AGR_OFFSET_Z_REG_H_M          0x4A
#define AGR_WHO_AM_I_M                0x4F
#define AGR_CFG_REG_A_M               0x60
#define AGR_CFG_REG_B_M               0x61
#define AGR_CFG_REG_C_M               0x62
#define AGR_INT_CRTL_REG_M            0x63
#define AGR_INT_SOURCE_REG_M          0x64
#define AGR_INT_THS_L_REG_M           0x65
#define AGR_INT_THS_H_REG_M           0x66
#define AGR_STATUS_REG_M              0x67
#define AGR_OUTX_L_REG_M              0x68
#define AGR_OUTX_H_REG_M              0x69
#define AGR_OUTY_L_REG_M              0x6A
#define AGR_OUTY_H_REG_M              0x6B
#define AGR_OUTZ_L_REG_M              0x6C
#define AGR_OUTZ_H_REG_M              0x6D

#define AGR_STATUS_REG_AUX_A          0x07
#define AGR_OUT_TEMP_L_A              0x0C
#define AGR_OUT_TEMP_H_A              0x0D
#define AGR_INT_COUNTER_REG_A         0x0E
#define AGR_WHO_AM_I_A                0x0F
#define AGR_TEMP_CFG_REG_A            0x1F
#define AGR_CTRL_REG1_A               0x20
#define AGR_CTRL_REG2_A               0x21
#define AGR_CTRL_REG3_A               0x22
#define AGR_CTRL_REG4_A               0x23
#define AGR_CTRL_REG5_A               0x24
#define AGR_CTRL_REG6_A               0x25
#define AGR_REFERENCE_DATACAPTURE_A   0x26
#define AGR_STATUS_REG_A              0x27
#define AGR_OUT_X_L_A                 0x28
#define AGR_OUT_X_H_A                 0x29
#define AGR_OUT_Y_L_A                 0x2A
#define AGR_OUT_Y_H_A                 0x2B
#define AGR_OUT_Z_L_A                 0x2C
#define AGR_OUT_Z_H_A                 0x2D
#define AGR_FIFO_CTRL_REG_A           0x2E
#define AGR_FIFO_SRC_REG_A            0x2F
#define AGR_INT1_CFG_A                0x30
#define AGR_INT1_SRC_A                0x31
#define AGR_INT1_THS_A                0x32
#define AGR_INT1_DURATION_A           0x33
#define AGR_INT2_CFG_A                0x34
#define AGR_INT2_SRC_A                0x35
#define AGR_INT2_THS_A                0x36
#define AGR_INT2_DURATION_A           0x37
#define AGR_CLICK_CFG_A               0x38
#define AGR_CLICK_SRC_A               0x39
#define AGR_CLICK_THS_A               0x3A
#define AGR_TIME_LIMIT_A              0x3B
#define AGR_TIME_LATENCY_A            0x3C
#define AGR_TIME_WINDOW_A             0x3D
#define AGR_Act_THS_A                 0x3E
#define AGR_Act_DUR_A                 0x3F


uint8_t met_acc_buffer[6];
uint8_t met_mag_buffer[6];
uint8_t met_acc_int1src[1];
uint8_t met_reg_aux[1];
uint8_t met_temp_buf[2];

#define TEMP_VALID 4
int16_t met_temp;

uint8_t met_reg_st_status_reg_a[1];
uint8_t met_st_acc_buffer[6];
uint8_t met_reg_st_status_reg_m[1];
uint8_t met_st_mag_buffer[6];


#define MET_MAG_DEFSTATE_AGR 0x90
#define MET_MAG_DEFSTATE_CTR 0x00
uint32_t met_mag_defstate;

/*
volatile uint8_t met_data_acc_init_agr[] = {
    TEMP_CFG_REG_A, 0xC0,
    CTRL_REG1_A, 0x57, //0x07
    CTRL_REG2_A, 0x02,
    CTRL_REG3_A, 0x00,
    CTRL_REG4_A, 0x88,
    CTRL_REG5_A, 0x00,
    CTRL_REG6_A, 0x20,
    REFERENCE_DATACAPTURE_A, 0x00,
    FIFO_CTRL_REG_A, 0x00,
    INT1_CFG_A, 0x00,
    INT1_THS_A, 0x00,
    INT1_DURATION_A, 0x00,
    INT2_CFG_A, 0x2A,
    INT2_THS_A, 0x02,
    INT2_DURATION_A, 0x04,
    CLICK_CFG_A, 0x00,
    CLICK_THS_A, 0x00,
    TIME_LIMIT_A, 0x00,
    TIME_LATENCY_A, 0x00,
    Act_THS_A, 0x00,
    Act_DUR_A, 0x00
};
*/

volatile uint8_t met_data_acc_init_agr[] = {
    AGR_TEMP_CFG_REG_A, 0xC0,
    AGR_CTRL_REG1_A, 0x57,
    AGR_CTRL_REG2_A, 0x01,
    AGR_CTRL_REG3_A, 0x40,
    AGR_CTRL_REG4_A, 0x88,
    AGR_CTRL_REG5_A, 0x00,
    AGR_CTRL_REG6_A, 0x00, //0x20
    AGR_REFERENCE_DATACAPTURE_A, 0x00,
    AGR_FIFO_CTRL_REG_A, 0x00,
    AGR_INT1_CFG_A, 0x2A, //0x00,
    AGR_INT1_THS_A, 0x02, //0x00,
    AGR_INT1_DURATION_A, 0x04, //0x00,
    AGR_INT2_CFG_A, 0x00, //0x2A,
    AGR_INT2_THS_A, 0x00, //0x02,
    AGR_INT2_DURATION_A, 0x00, //0x04,
    AGR_CLICK_CFG_A, 0x00,
    AGR_CLICK_THS_A, 0x00,
    AGR_TIME_LIMIT_A, 0x00,
    AGR_TIME_LATENCY_A, 0x00,
    AGR_Act_THS_A, 0x00,
    AGR_Act_DUR_A, 0x00
};

volatile uint8_t met_data_mag_init_agr[] = {
    AGR_OFFSET_X_REG_L_M, 0x00,
    AGR_OFFSET_X_REG_H_M, 0x00,
    AGR_OFFSET_Y_REG_L_M, 0x00,
    AGR_OFFSET_Y_REG_H_M, 0x00,
    AGR_OFFSET_Z_REG_L_M, 0x00,
    AGR_OFFSET_Z_REG_H_M, 0x00,
    AGR_CFG_REG_A_M, 0x93, //0x93
    AGR_CFG_REG_B_M, 0x01,
    AGR_CFG_REG_C_M, 0x00,
    AGR_INT_CRTL_REG_M, 0xE0,
    AGR_INT_THS_L_REG_M, 0x00,
    AGR_INT_THS_H_REG_M, 0x00
};

static app_twi_transfer_t const met_trans_init_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+6, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+8, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+10, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+12, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+14, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+16, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+18, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+20, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+22, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+24, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+26, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+28, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+30, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+32, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+34, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+36, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+38, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_init_agr+40, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+6, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+8, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+10, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+12, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+14, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+16, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+18, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+20, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_init_agr+22, 2, 0),
};
uint32_t met_trans_init_size_agr = sizeof(met_trans_init_agr)/sizeof(app_twi_transfer_t);


volatile uint8_t met_data_shutdown_agr[] = {
    AGR_TEMP_CFG_REG_A, 0x00,
    AGR_CTRL_REG1_A, 0x07,
    AGR_CFG_REG_A_M, 0x93,
};

static app_twi_transfer_t const met_trans_shutdown_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_shutdown_agr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_shutdown_agr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_shutdown_agr+4, 2, 0),
};
uint32_t met_trans_shutdown_size_agr = sizeof(met_trans_shutdown_agr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_mag_state_agr[] = {
    AGR_CFG_REG_A_M, 0x90,//0x90:on, 0x93:off
};

static app_twi_transfer_t const met_trans_mag_state_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_state_agr, 2, 0),
};
uint32_t met_trans_mag_state_size_agr = sizeof(met_trans_mag_state_agr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_acc_read_agr[] = {
    REGISTER_AUTO_INCREMENT | AGR_OUT_X_L_A
};

volatile uint8_t met_data_mag_read_agr[] = {
    REGISTER_AUTO_INCREMENT | AGR_OUTX_L_REG_M
};


volatile uint8_t met_data_acc_intsrc_read_agr[] = {
    AGR_INT1_SRC_A
};

volatile uint8_t met_data_reg_aux_read_agr[] = {
    AGR_STATUS_REG_AUX_A
};

volatile uint8_t met_data_temp_read_agr[] = {
    REGISTER_AUTO_INCREMENT | AGR_OUT_TEMP_L_A
};

static app_twi_transfer_t const met_trans_read_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_intsrc_read_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_ACC, met_acc_int1src, 1, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_acc_read_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_ACC, met_acc_buffer, 6, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_mag_read_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_MAG, met_mag_buffer, 6, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_reg_aux_read_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_ACC, met_reg_aux, 1, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_temp_read_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_ACC, met_temp_buf, 2, 0),
};
uint32_t met_trans_read_size_agr = sizeof(met_trans_read_agr)/sizeof(app_twi_transfer_t);

////

volatile uint8_t met_data_st_acc_init_agr[] = {
    AGR_CTRL_REG2_A, 0x00,
    AGR_CTRL_REG3_A, 0x10,
    AGR_CTRL_REG4_A, 0x80,
    AGR_CTRL_REG1_A, 0x57,
};
static app_twi_transfer_t const met_trans_st_acc_init_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_init_agr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_init_agr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_init_agr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_init_agr+6, 2, 0),
};
uint32_t met_trans_st_acc_init_size_agr = sizeof(met_trans_st_acc_init_agr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_st_acc_done_agr[] = {
    AGR_CTRL_REG2_A, 0x01,
    AGR_CTRL_REG3_A, 0x40,
    AGR_CTRL_REG4_A, 0x88,
    AGR_CTRL_REG1_A, 0x57,
};
static app_twi_transfer_t const met_trans_st_acc_done_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_done_agr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_done_agr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_done_agr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_done_agr+6, 2, 0),
};
uint32_t met_trans_st_acc_done_size_agr = sizeof(met_trans_st_acc_done_agr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_acc_stimuli_agr[] = {
    AGR_CTRL_REG4_A, 0x85,
};
static app_twi_transfer_t const met_trans_st_acc_stimuli_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_stimuli_agr, 2, 0),
};
uint32_t met_trans_st_acc_stimuli_size_agr = sizeof(met_trans_st_acc_stimuli_agr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_status_reg_a_agr[] = {
    AGR_STATUS_REG_A
};
static app_twi_transfer_t const met_trans_st_status_reg_a_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_status_reg_a_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_ACC, met_reg_st_status_reg_a, 1, 0),
};
uint32_t met_trans_st_status_reg_a_size_agr = sizeof(met_trans_st_status_reg_a_agr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_acc_read_agr[] = {
    REGISTER_AUTO_INCREMENT | AGR_OUT_X_L_A
};
static app_twi_transfer_t const met_trans_st_acc_read_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_ACC, met_data_st_acc_read_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_ACC, met_st_acc_buffer, 6, 0),
};
uint32_t met_trans_st_acc_read_size_agr = sizeof(met_trans_st_acc_read_agr)/sizeof(app_twi_transfer_t);

////

volatile uint8_t met_data_st_mag_init_agr[] = {
    AGR_CFG_REG_A_M, 0x8C,
    AGR_CFG_REG_B_M, 0x02,
    AGR_CFG_REG_C_M, 0x10,
};
static app_twi_transfer_t const met_trans_st_mag_init_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_mag_init_agr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_mag_init_agr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_mag_init_agr+4, 2, 0),
};
uint32_t met_trans_st_mag_init_size_agr = sizeof(met_trans_st_mag_init_agr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_st_mag_done_agr[] = {
    AGR_CFG_REG_C_M, 0x10,
    AGR_CFG_REG_A_M, 0x83,
};
static app_twi_transfer_t const met_trans_st_mag_done_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_mag_done_agr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_mag_done_agr+2, 2, 0),
};
uint32_t met_trans_st_mag_done_size_agr = sizeof(met_trans_st_mag_done_agr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_mag_stimuli_agr[] = {
    AGR_CFG_REG_C_M, 0x12,
};
static app_twi_transfer_t const met_trans_st_mag_stimuli_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_mag_stimuli_agr, 2, 0),
};
uint32_t met_trans_st_mag_stimuli_size_agr = sizeof(met_trans_st_mag_stimuli_agr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_status_reg_m_agr[] = {
    AGR_STATUS_REG_M
};
static app_twi_transfer_t const met_trans_st_status_reg_m_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_status_reg_m_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_MAG, met_reg_st_status_reg_m, 1, 0),
};
uint32_t met_trans_st_status_reg_m_size_agr = sizeof(met_trans_st_status_reg_m_agr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_mag_read_agr[] = {
    REGISTER_AUTO_INCREMENT | AGR_OUTX_L_REG_M
};
static app_twi_transfer_t const met_trans_st_mag_read_agr[] =
{
    APP_TWI_WRITE(TWI_ADDR_AGR_MAG, met_data_st_mag_read_agr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_AGR_MAG, met_st_mag_buffer, 6, 0),
};
uint32_t met_trans_st_mag_read_size_agr = sizeof(met_trans_st_mag_read_agr)/sizeof(app_twi_transfer_t);


//============================================================================
//

volatile uint8_t met_data_acc_init_ctr[] = {
    CTR_CTRL_REG1_A, 0x3F,
    CTR_CTRL_REG2_A, 0x01,
    CTR_CTRL_REG3_A, 0x08,
    CTR_CTRL_REG4_A, 0x04,
    CTR_CTRL_REG5_A, 0x00,
    CTR_CTRL_REG6_A, 0x00,
    CTR_CTRL_REG7_A, 0x00,
    CTR_FIFO_CTRL, 0x00,
    CTR_IG_CFG1_A, 0xAA, //0x00, //AOI flipped
    CTR_IG_THS_X1_A, 0x02,
    CTR_IG_THS_Y1_A, 0x02,
    CTR_IG_THS_Z1_A, 0x02,
    CTR_IG_DUR1_A, 0x04,
    CTR_IG_CFG2_A, 0x00,
    CTR_ACT_THS_A, 0x00,
    CTR_ACT_DUR_A, 0x00
};

volatile uint8_t met_data_mag_init_ctr[] = {
    CTR_CTRL_REG1_M, 0x90,
    CTR_CTRL_REG2_M, 0x60,
    CTR_CTRL_REG3_M, 0x03, //power down: 0x03
    CTR_CTRL_REG4_M, 0x00,
    CTR_CTRL_REG5_M, 0x00,
    CTR_INT_CFG_M, 0x00,
    CTR_INT_THS_L_M, 0x00,
    CTR_INT_THS_H_M, 0x00
};

static app_twi_transfer_t const met_trans_init_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+6, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+8, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+10, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+12, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+14, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+16, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+18, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+20, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+22, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+24, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_init_ctr+26, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+6, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+8, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+10, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+12, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+14, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_init_ctr+16, 2, 0),
};
uint32_t met_trans_init_size_ctr = sizeof(met_trans_init_ctr)/sizeof(app_twi_transfer_t);


volatile uint8_t met_data_shutdown_ctr[] = {
    CTR_CTRL_REG1_A, 0x07,
    CTR_CTRL_REG3_M, 0x03,
};


static app_twi_transfer_t const met_trans_shutdown_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_shutdown_ctr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_shutdown_ctr+2, 2, 0),
};
uint32_t met_trans_shutdown_size_ctr = sizeof(met_trans_shutdown_ctr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_mag_state_ctr[] = {
    CTR_CTRL_REG3_M, 0x00,//0x00:on, 0x03:off
};

static app_twi_transfer_t const met_trans_mag_state_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_state_ctr, 2, 0),
};
uint32_t met_trans_mag_state_size_ctr = sizeof(met_trans_mag_state_ctr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_acc_read_ctr[] = {CTR_OUT_X_L_A};

volatile uint8_t met_data_mag_read_ctr[] = {CTR_OUT_X_L_M};

volatile uint8_t met_data_temp_read_ctr[] = {CTR_TEMP_L_M,CTR_TEMP_H_M};

volatile uint8_t met_data_acc_intsrc_read_ctr[] = {CTR_IG_SRC1_A};

static app_twi_transfer_t const met_trans_read_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_intsrc_read_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_ACC, met_acc_int1src, 1, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_acc_read_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_ACC, met_acc_buffer, 6, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_mag_read_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_MAG, met_mag_buffer, 6, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_temp_read_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_MAG, met_temp_buf, 1, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_temp_read_ctr+1, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_MAG, met_temp_buf+1, 1, 0),
};
uint32_t met_trans_read_size_ctr = sizeof(met_trans_read_ctr)/sizeof(app_twi_transfer_t);

////

volatile uint8_t met_data_st_acc_init_ctr[] = {
    CTR_CTRL_REG1_A, 0x3F,
    CTR_CTRL_REG3_A, 0x01,
    CTR_CTRL_REG4_A, 0x04,
    CTR_CTRL_REG5_A, 0x00,
};
static app_twi_transfer_t const met_trans_st_acc_init_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_init_ctr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_init_ctr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_init_ctr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_init_ctr+6, 2, 0),
};
uint32_t met_trans_st_acc_init_size_ctr = sizeof(met_trans_st_acc_init_ctr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_st_acc_done_ctr[] = {
    CTR_CTRL_REG1_A, 0x3F,
    CTR_CTRL_REG3_A, 0x08,
    CTR_CTRL_REG4_A, 0x04,
    CTR_CTRL_REG5_A, 0x00,
};
static app_twi_transfer_t const met_trans_st_acc_done_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_done_ctr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_done_ctr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_done_ctr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_done_ctr+6, 2, 0),
};
uint32_t met_trans_st_acc_done_size_ctr = sizeof(met_trans_st_acc_done_ctr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_acc_stimuli_ctr[] = {
    CTR_CTRL_REG5_A, 0x04,
};
static app_twi_transfer_t const met_trans_st_acc_stimuli_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_stimuli_ctr, 2, 0),
};
uint32_t met_trans_st_acc_stimuli_size_ctr = sizeof(met_trans_st_acc_stimuli_ctr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_status_reg_a_ctr[] = {
    CTR_STATUS_REG_A
};
static app_twi_transfer_t const met_trans_st_status_reg_a_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_status_reg_a_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_ACC, met_reg_st_status_reg_a, 1, 0),
};
uint32_t met_trans_st_status_reg_a_size_ctr = sizeof(met_trans_st_status_reg_a_ctr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_acc_read_ctr[] = {
    CTR_OUT_X_L_A
};
static app_twi_transfer_t const met_trans_st_acc_read_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_ACC, met_data_st_acc_read_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_ACC, met_st_acc_buffer, 6, 0),
};
uint32_t met_trans_st_acc_read_size_ctr = sizeof(met_trans_st_acc_read_ctr)/sizeof(app_twi_transfer_t);

////

volatile uint8_t met_data_st_mag_init_ctr[] = {
    CTR_CTRL_REG1_M, 0xFC,
    CTR_CTRL_REG2_M, 0x60,
    CTR_CTRL_REG3_M, 0x00,
    CTR_CTRL_REG4_M, 0x0C,
    CTR_CTRL_REG5_M, 0x40,
};
static app_twi_transfer_t const met_trans_st_mag_init_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_init_ctr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_init_ctr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_init_ctr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_init_ctr+6, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_init_ctr+8, 2, 0),
};
uint32_t met_trans_st_mag_init_size_ctr = sizeof(met_trans_st_mag_init_ctr)/sizeof(app_twi_transfer_t);

volatile uint8_t met_data_st_mag_done_ctr[] = {
    CTR_CTRL_REG1_M, 0x90,
    CTR_CTRL_REG2_M, 0x60,
    CTR_CTRL_REG3_M, 0x03, //power down: 0x03
    CTR_CTRL_REG4_M, 0x00,
    CTR_CTRL_REG5_M, 0x00,
};
static app_twi_transfer_t const met_trans_st_mag_done_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_done_ctr, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_done_ctr+2, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_done_ctr+4, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_done_ctr+6, 2, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_done_ctr+8, 2, 0),
};
uint32_t met_trans_st_mag_done_size_ctr = sizeof(met_trans_st_mag_done_ctr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_mag_stimuli_ctr[] = {
    CTR_CTRL_REG1_M, 0xFD,
};
static app_twi_transfer_t const met_trans_st_mag_stimuli_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_stimuli_ctr, 2, 0),
};
uint32_t met_trans_st_mag_stimuli_size_ctr = sizeof(met_trans_st_mag_stimuli_ctr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_status_reg_m_ctr[] = {
    CTR_STATUS_REG_M
};
static app_twi_transfer_t const met_trans_st_status_reg_m_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_status_reg_m_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_MAG, met_reg_st_status_reg_m, 1, 0),
};
uint32_t met_trans_st_status_reg_m_size_ctr = sizeof(met_trans_st_status_reg_m_ctr)/sizeof(app_twi_transfer_t);

//

volatile uint8_t met_data_st_mag_read_ctr[] = {
    CTR_OUT_X_L_M
};
static app_twi_transfer_t const met_trans_st_mag_read_ctr[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_st_mag_read_ctr, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_MAG, met_st_mag_buffer, 6, 0),
};
uint32_t met_trans_st_mag_read_size_ctr = sizeof(met_trans_st_mag_read_ctr)/sizeof(app_twi_transfer_t);


//============================================================================

uint8_t met_reg_whoami[2];

volatile uint8_t met_data_whoami[] = {CTR_WHO_AM_I_M,AGR_WHO_AM_I_M};
static app_twi_transfer_t const met_trans_whoami[] =
{
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_whoami, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_MAG, met_reg_whoami, 1, 0),
    APP_TWI_WRITE(TWI_ADDR_CTR_MAG, met_data_whoami+1, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_ADDR_CTR_MAG, met_reg_whoami+1, 1, 0),
};
uint32_t met_trans_whoami_size = sizeof(met_trans_whoami)/sizeof(app_twi_transfer_t);

//============================================================================

app_twi_transfer_t const *met_trans_init = NULL;
app_twi_transfer_t const *met_trans_shutdown = NULL;
app_twi_transfer_t const *met_trans_mag_state = NULL;
app_twi_transfer_t const *met_trans_read = NULL;
uint32_t met_trans_init_size;
uint32_t met_trans_shutdown_size;
uint32_t met_trans_mag_state_size;
uint32_t met_trans_read_size;

volatile uint8_t *met_data_mag_state;

app_twi_transfer_t const *met_trans_st_acc_init;
app_twi_transfer_t const *met_trans_st_acc_done;
app_twi_transfer_t const *met_trans_st_acc_stimuli;
app_twi_transfer_t const *met_trans_st_status_reg_a;
app_twi_transfer_t const *met_trans_st_acc_read;
app_twi_transfer_t const *met_trans_st_mag_init;
app_twi_transfer_t const *met_trans_st_mag_done;
app_twi_transfer_t const *met_trans_st_mag_stimuli;
app_twi_transfer_t const *met_trans_st_status_reg_m;
app_twi_transfer_t const *met_trans_st_mag_read;

uint32_t met_trans_st_acc_init_size;
uint32_t met_trans_st_acc_done_size;
uint32_t met_trans_st_acc_stimuli_size;
uint32_t met_trans_st_status_reg_a_size;
uint32_t met_trans_st_acc_read_size;
uint32_t met_trans_st_mag_init_size;
uint32_t met_trans_st_mag_done_size;
uint32_t met_trans_st_mag_stimuli_size;
uint32_t met_trans_st_status_reg_m_size;
uint32_t met_trans_st_mag_read_size;


//============================================================================
//

#define MET_MOV_THS_ALARM 0.05
#define MET_MOV_THS_WAKE 0.02
#define MET_MOV_THS_TOUCH 0.01

#define CAL_MAXPTS 30

double met_cal_x[CAL_MAXPTS];
double met_cal_y[CAL_MAXPTS];
double met_cal_z[CAL_MAXPTS];
uint32_t met_cal_ptr = 0;
bool met_cal_enable = false;
bool met_cal_enRearm = false;

#define CAL_AUTO_MAXPTS 10

uint32_t met_auto_cal_ptr = 0;
//uint32_t met_auto_cal_cnt = 0;
bool met_cal_doSample = false;

//y[i] := y[i-1] + alpha * (x[i] - y[i-1])
#define LPF_ALPHA_ACC 0.1
#define LPF_ALPHA_MAG 0.5

//y[i] := a * (y[i-1] + x[i] - x[i-1])
#define HPF_ALPHA 0.5

float met_compass;

bool met_capture_enable = false;

#define MET_MAG_MAX (10*3*2)
uint8_t met_mag_buf[1+MET_MAG_MAX];
uint8_t *met_mag_payload = met_mag_buf+1;
uint32_t met_mag_ptr = 0;

#define CONV_mGtoA 0.00980665
#define PI 3.14159265

typedef struct {
    double center[3];
    double radii[3];
} met_settings_t;

met_settings_t met_settings __attribute__ ((aligned (4)));


#define SQR(x) ((x)*(x))
void met_nearestPoint(double *p, double *sqrdist, int *ptr) {
    *sqrdist = INFINITY;
    *ptr = 0;
    for(int i = 0; i < CAL_MAXPTS; i++) {
        if(!isnan(met_cal_x[i])) {
            double tst = SQR(met_cal_x[i]-p[0]) + SQR(met_cal_y[i]-p[1]) + SQR(met_cal_z[i]-p[2]);
            if(tst < *sqrdist) {
                *sqrdist = tst;
                *ptr = i;
            }
        } else {
            break;
        }
    }
}

//============================================================================
//

/*
// Auto calibration trial
// Not working, to improve: circle fitting with "known" radius

double met_mag_dip = -70.*PI/180.;

void met_auto_cal_do(double *p) {
    printf("cal: m_mag: %lf, %lf, %lf \r\n", p[0], p[1], p[2]);
    double sqrdist;
    int ptr;
    met_nearestPoint(p, &sqrdist, &ptr);
    if(sqrdist > 100) {
        ptr = met_auto_cal_ptr;
    } else {
        printf("cal: replace: %d \r\n", ptr);
    }
    met_cal_x[ptr] = p[0];
    met_cal_y[ptr] = p[1];
    met_cal_z[ptr] = p[2];
    //
    met_auto_cal_ptr++;
    met_auto_cal_ptr %= CAL_AUTO_MAXPTS;

    double mean_z = 0;

    int cnt = 0;
    for(int i = 0; i < CAL_AUTO_MAXPTS; i++) {
        if(!isnan(met_cal_z[i])) {
            cnt = i+1;
            //mean_z += met_cal_z[i];
        } else {
            break;
        }
    }

    if(cnt >= 4) {
        double auto_center[2];
        double auto_radius;
        bool res = cal_circle_fitting(met_cal_x,met_cal_y,cnt,auto_center,&auto_radius);
        if(res) {
            printf("cal_circle_fitting: %lf, %lf, %lf \r\n", auto_center[0], auto_center[1], auto_radius);

            double vp_sinA = sin(abs(met_mag_dip));
            double vp_cosA = cos(abs(met_mag_dip));
            double vp_a = auto_radius;
            double vp_c = vp_a / vp_sinA;
            double vp_z = copysign(vp_cosA, met_mag_dip) * vp_c;
            //printf("VP: %lf, %lf, %lf \r\n", auto_center[0], auto_center[1], mean_z + vp_z);

            double cal_x[CAL_AUTO_MAXPTS*2];
            double cal_y[CAL_AUTO_MAXPTS*2];
            double cal_z[CAL_AUTO_MAXPTS*2];

            for(int i = 0; i < cnt; i++) {
                cal_x[i] = met_cal_x[i];
                cal_x[i+cnt] = met_cal_x[i];
                cal_y[i] = met_cal_y[i];
                cal_y[i+cnt] = met_cal_y[i];
                cal_z[i] = met_cal_z[i];
                cal_z[i+cnt] = met_cal_z[i] - 2*vp_z;
            }

            //met_cal_x[0] = auto_center[0];
            //met_cal_y[0] = auto_center[1];
            //met_cal_z[0] = mean_z + vp_z;

            cal_do(cal_x, cal_y, cal_z, 2*cnt, met_settings.center, met_settings.radii);
            //rec_write(KEY_METSETTINGS, &met_settings, sizeof(met_settings_t), NULL);
            printf("met center: %lf, %lf, %lf\r\n", met_settings.center[0],met_settings.center[1],met_settings.center[2]);
            printf("met radii: %lf, %lf, %lf\r\n", met_settings.radii[0],met_settings.radii[1],met_settings.radii[2]);            
        }
    }
}

void met_cal_sample(void *ctx) {
    met_cal_doSample = true;
}
*/

//============================================================================
//

void met_compass_calibrate() {
    met_cal_enable = true;
    met_cal_ptr = 0;
    for(int i = 0; i < CAL_MAXPTS; i++) {
        met_cal_x[i] = NAN;
        met_cal_y[i] = NAN;
        met_cal_z[i] = NAN;
    }    
}

double met_hpf_acc[3*1] = {0,0,0};
double met_lpf_acc[3*1] = {0,0,0};
double met_lpf_mag[3*1] = {0,0,0};

bool met_movement_triggered_alarm = false;
bool met_movement_triggered_wake = false;
bool met_movement_triggered_touch = false;

void met_read(void *ctx);
void met_twi_read_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_done %d \r\n", result);
        sch_unique_oneshot(met_read, 100);
        return;
    }

    if(met_agr) {
        if(met_reg_aux[0] & TEMP_VALID) {
            met_temp = met_temp_buf[0] + (((int16_t)met_temp_buf[1]) << 8);
            met_temp >>= 8;
            met_temp += 25;
            //printf("met temp %d \r\n", met_temp);
        }
    } else {
        met_temp = met_temp_buf[0] + (((int16_t)met_temp_buf[1]) << 8);
        met_temp += 25*8;
        met_temp /= 8;
        //printf("met temp %d \r\n", met_temp);
    }

    //double mX, mY, mZ, mcX, mcY, mcZ;
    
    double m_mag[3*1];
    matrix_t mat_mag = mat_create(3,1,m_mag);
    double m_dir[3*1];
    matrix_t mat_dir = mat_create(3,1,m_dir);
    
    matrix_t mat_acc = mat_create(3,1,met_lpf_acc);
    double m_g[3*1] = {0,0,-1};
    matrix_t mat_g = mat_create(3,1,m_g);

    double m_acc[3*1];

    m_acc[0] = (*(int16_t*)(met_acc_buffer))>>((met_agr) ? 4 : 0);
    m_acc[1] = (*(int16_t*)(met_acc_buffer+2))>>((met_agr) ? 4 : 0);
    m_acc[2] = (*(int16_t*)(met_acc_buffer+4))>>((met_agr) ? 4 : 0);

    m_acc[0] *= met_acc_conv;
    m_acc[1] *= met_acc_conv;
    m_acc[2] *= met_acc_conv;

    m_acc[0] /= -1000;
    m_acc[1] /= -1000;
    m_acc[2] /= -1000;

    if(met_cal_enable && met_cal_ptr == CAL_MAXPTS) {
        met_cal_enable = false;
        if(met_cal_ptr >= 10) {
            cal_do(met_cal_x, met_cal_y, met_cal_z, met_cal_ptr, met_settings.center, met_settings.radii);
            rec_write(KEY_METSETTINGS, &met_settings, sizeof(met_settings_t), NULL);
            printf("met center: %lf, %lf, %lf\r\n", met_settings.center[0],met_settings.center[1],met_settings.center[2]);
            printf("met radii: %lf, %lf, %lf\r\n", met_settings.radii[0],met_settings.radii[1],met_settings.radii[2]);
            //leds_compass(200, 255, 10, 0);
            leds_anim_off(ANIM_CALIBRATE);
        }
    }

    double magsqr = m_acc[0]*m_acc[0] + m_acc[1]*m_acc[1] + m_acc[2]*m_acc[2];
    double magw = (magsqr > 1) ? (1./magsqr) : magsqr;

    if(magsqr > 0) {
        double distangle = (PI - acos(-1.* (m_acc[2] / sqrt(magsqr)))) / PI;
        double filtangle = 1./(1.+exp(-50.*(distangle-0.7)) );
        //printf("filter %f\r\n", filtangle);
        magw *= filtangle;
    }
    //magw = 1.0;

    // LPF
    met_lpf_acc[0] = met_lpf_acc[0] + magw * LPF_ALPHA_ACC * (m_acc[0] - met_lpf_acc[0]);
    met_lpf_acc[1] = met_lpf_acc[1] + magw * LPF_ALPHA_ACC * (m_acc[1] - met_lpf_acc[1]);
    met_lpf_acc[2] = met_lpf_acc[2] + magw * LPF_ALPHA_ACC * (m_acc[2] - met_lpf_acc[2]);

    //printf("met_hpf_acc: %9.6lf, %9.6lf, %9.6lf \r\n", met_lpf_acc[0], met_lpf_acc[1], met_lpf_acc[2]);
    //printf("m_acc: %9.6lf, %9.6lf, %9.6lf \r\n", m_acc[0], m_acc[1], m_acc[2]);

    // HPF
    static double m_acc_old[3*1] = {0,0,0};

    met_hpf_acc[0] = HPF_ALPHA * (met_hpf_acc[0] + m_acc[0] - m_acc_old[0]);
    met_hpf_acc[1] = HPF_ALPHA * (met_hpf_acc[1] + m_acc[1] - m_acc_old[1]);
    met_hpf_acc[2] = HPF_ALPHA * (met_hpf_acc[2] + m_acc[2] - m_acc_old[2]);

    m_acc_old[0] = m_acc[0];
    m_acc_old[1] = m_acc[1];
    m_acc_old[2] = m_acc[2];

    double acc_vector = sqrt(met_hpf_acc[0]*met_hpf_acc[0] + met_hpf_acc[1]*met_hpf_acc[1] + met_hpf_acc[2]*met_hpf_acc[2]);
    if(acc_vector > MET_MOV_THS_ALARM) {
        met_movement_triggered_alarm = true;
    }

    if(acc_vector > MET_MOV_THS_WAKE){
        met_movement_triggered_wake = true;
    }

    if(acc_vector > MET_MOV_THS_TOUCH){
        met_movement_triggered_touch = true;
    }

    //printf("met_hpf_acc: %9.6lf, %9.6lf, %9.6lf \r\n", met_hpf_acc[0], met_hpf_acc[1], met_hpf_acc[2]); 

    int16_t tmp;
    m_mag[0] = tmp = (*(int16_t*)(met_mag_buffer)); //x
    //offline dump
    //met_mag_payload[met_mag_ptr++] = ((tmp >> 8) & 0xff);
    //met_mag_payload[met_mag_ptr++] = (tmp & 0xff);

    m_mag[1] = tmp = (*(int16_t*)(met_mag_buffer+2)); //y
    //offline dump
    //met_mag_payload[met_mag_ptr++] = ((tmp >> 8) & 0xff);
    //met_mag_payload[met_mag_ptr++] = (tmp & 0xff);

    m_mag[2] = tmp = (*(int16_t*)(met_mag_buffer+4)); //z
    //offline dump
    //met_mag_payload[met_mag_ptr++] = ((tmp >> 8) & 0xff);
    //met_mag_payload[met_mag_ptr++] = (tmp & 0xff);

    m_mag[0] *= met_mag_conv;
    m_mag[1] *= met_mag_conv;
    m_mag[2] *= met_mag_conv;

    //printf("m_mag: %lf, %lf, %lf \r\n", m_mag[0], m_mag[1], m_mag[2]); 

    #define NEARDIST 100
    if(met_cal_enable && met_cal_ptr < CAL_MAXPTS) {
        double sqrdist;
        int ptr;
        met_nearestPoint(m_mag, &sqrdist, &ptr);
        if(sqrdist > SQR(NEARDIST)) {
            met_cal_x[met_cal_ptr] = m_mag[0];
            met_cal_y[met_cal_ptr] = m_mag[1];
            met_cal_z[met_cal_ptr] = m_mag[2];
            met_cal_ptr++;
        } else {
            printf("cal: replace: %d \r\n", ptr);
            met_cal_x[ptr] = m_mag[0];
            met_cal_y[ptr] = m_mag[1];
            met_cal_z[ptr] = m_mag[2];
        }
        leds_calibrate((float)met_cal_ptr/(float)CAL_MAXPTS);
    }

    /*
    if(!met_cal_enable && met_cal_doSample) {
        met_cal_doSample = false;
        met_auto_cal_do(m_mag);
    }*/
    magw = 1;
    met_lpf_mag[0] = m_mag[0] = met_lpf_mag[0] + magw * LPF_ALPHA_MAG * (m_mag[0] - met_lpf_mag[0]);
    met_lpf_mag[1] = m_mag[1] = met_lpf_mag[1] + magw * LPF_ALPHA_MAG * (m_mag[1] - met_lpf_mag[1]);
    met_lpf_mag[2] = m_mag[2] = met_lpf_mag[2] + magw * LPF_ALPHA_MAG * (m_mag[2] - met_lpf_mag[2]);

//offline dump
/*
    if(met_mag_ptr == MET_MAG_MAX) {
        if(met_capture_enable) {
            met_mag_buf[0] = NOTIFY_MAG;
            bslink_up_write(met_mag_buf, 1+MET_MAG_MAX);
        }
        met_mag_ptr = 0;
    }
*/

    m_mag[0] -= met_settings.center[0];
    m_mag[1] -= met_settings.center[1];
    m_mag[2] -= met_settings.center[2];

    //mcX = met_evecs[0][0] * mX + met_evecs[0][1] * mY + met_evecs[0][2] * mZ;
    //mcY = met_evecs[1][0] * mX + met_evecs[1][1] * mY + met_evecs[1][2] * mZ;
    //mcZ = met_evecs[2][0] * mX + met_evecs[2][1] * mY + met_evecs[2][2] * mZ;
    //mcX /= met_radii[0];
    //mcY /= met_radii[1];
    //mcZ /= met_radii[2];
    m_mag[0] /= (met_settings.radii[0]) ? met_settings.radii[0] : 1;
    m_mag[1] /= (met_settings.radii[1]) ? met_settings.radii[1] : 1;
    m_mag[2] /= (met_settings.radii[2]) ? met_settings.radii[2] : 1;

    //printf("mag: %lf, %lf, %lf \r\n", mcX, mcY, mcZ);   
    //printf("m_mag: %lf, %lf, %lf \r\n", m_mag[0], m_mag[1], m_mag[2]); 

    double m_R[3*3];
    matrix_t mat_R = mat_create(3,3,m_R);

    mat_getRotationFromVectors(mat_R, mat_acc, mat_g);
    //mat_identity(mat_R);
    mat_mult(mat_dir, mat_R, mat_mag);
    //printf("mag: %lf, %lf, %lf \r\n", m_dir[0], m_dir[1], m_dir[2]); 

    met_compass = atan2(m_dir[1], -m_dir[0]) * (180.0 / PI);

    //printf("angle: %lf \r\n", met_compass); 

    sch_unique_oneshot(met_read, 100);
}

//============================================================================
//

bool met_shutdown_mode = false;
bool met_int_polling_active = false;
bool met_selftest_active = false;

void met_read(void *ctx) {
    if(met_shutdown_mode || met_selftest_active) {
        return;
    }
    if(met_int_polling_active) {
        //printf("M\r\n");
        twi_transaction_do(met_trans_read, met_trans_read_size, met_twi_read_done, NULL);
    }
}

//============================================================================
//

void met_mag_enable() {
    met_data_mag_state[1] = met_mag_defstate;
    twi_transaction_do(met_trans_mag_state, met_trans_mag_state_size, NULL, NULL);
}

void met_mag_disable() {
    met_data_mag_state[1] = met_mag_defstate | 0x03;
    twi_transaction_do(met_trans_mag_state, met_trans_mag_state_size, NULL, NULL);
}

//============================================================================
//
/*
void met_pol_delayed_off(void *ctx) {
    met_mag_disable();
}

void met_pol_movement_off() {
    sch_unique_oneshot(met_pol_delayed_off, 15000);
}

void met_pol_movement_on() {
    met_mag_enable();
    sch_unique_cancel(met_pol_delayed_off);
}
*/
//============================================================================
//
/*
void met_twi_init_done(ret_code_t result, void *ctx) {
    //printf("met_twi_init_done\r\n");
    //sch_unique_oneshot(met_read, 100);
}
*/
//

//============================================================================
//

bool met_trigger_active = false;
bool met_trigger_wake = false;
bool met_trigger_touch = false;

bool met_isMoving() {
    return met_trigger_active;
}

void met_trigger_on_monitor(void *ctx) {
    uint8_t buf[2];
    uint32_t ptr = 0;
    buf[ptr++] = NOTIFY_MOVEMENT;
    buf[ptr++] = (met_trigger_active) ? 1 : 0;
    bslink_up_write(buf, ptr);
    if(met_trigger_active) {
        sch_unique_oneshot(met_trigger_on_monitor, MOVEMENT_MONITOR);
    //} else {
    //    printf("met mov off \r\n");
    }
}

void met_trigger_off() {
    //printf("MOV 0\r\n");
    alarm_movement_off();
}

void met_trigger_on() {
    //printf("MOV 1\r\n");
    alarm_movement_on();
    //
    if(!sch_unique_isScheduled(met_trigger_on_monitor)) {
        sch_unique_oneshot(met_trigger_on_monitor, 0);
    }
}

void met_movement_test(void *ctx) {
    static int activeCnt = 0;

    if(!met_int_polling_active) {
        met_int_polling_active = true;
        sch_unique_oneshot(met_read, 100);
        //
        //Validate movements
        activeCnt = 40;
        //
    }

    if(met_movement_triggered_touch){
        if(!met_trigger_touch){
            met_trigger_touch = true;
            touch_movement_on();
        }
    }else{
        if(met_trigger_touch){
            met_trigger_touch = false;
            touch_movement_off();
        }
    }

    if(met_movement_triggered_wake){
        if(!met_trigger_wake){
            met_trigger_wake = true;
            ui_onMovement();
            leds_movement_on();            
        }
    }else{
        if(met_trigger_wake){
            met_trigger_wake = false;
            leds_movement_off();
        }
    }

    if(met_movement_triggered_alarm && !sound_isActive()) {
        //
        if(!met_trigger_active) {
            met_trigger_active = true;
            met_trigger_on();
        }
    } else {
        if(met_trigger_active) {
            met_trigger_active = false;
            met_trigger_off();
        }
        //
        activeCnt--;
    }
    met_movement_triggered_wake = false;
    met_movement_triggered_touch = false;
    met_movement_triggered_alarm = false;

    if(activeCnt) {
        sch_unique_oneshot(met_movement_test, (met_trigger_active) ? 500 : 250);
    } else {
        met_int_polling_active = false;
        //printf("M SLEEP\r\n");
    }

}

//============================================================================
//

met_selftest_state_t met_selftest_state;
int nost_read;
int st_read;
int32_t met_st_acc_nost[3];
int32_t met_st_acc_st[3];
int32_t met_st_mag_nost[3];
int32_t met_st_mag_st[3];


//============================================================================
//

static void met_int_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    printf("M INT\r\n");
    //return;
    if(met_selftest_active) {
        met_selftest_state.int_success = true;
        return;
    }
    if(sound_isActive() && dev_isTravelMode()) {
        return;
    }

    if(!sch_unique_isScheduled(met_movement_test)) {
        sch_unique_oneshot(met_movement_test, 0);
    }
}

//============================================================================
//


void met_twi_read_st_done_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_done_done %d \r\n", result);
        return;
    }
    met_selftest_active = false;

    met_selftest_state.acc_success = true;
    met_selftest_state.mag_success = true;
    int16_t test;

    for(int i = 0; i < 3; i++) {
        met_st_acc_st[i] = (int32_t)((double)met_st_acc_st[i] * met_acc_conv) >> 3;
        met_st_acc_nost[i] = (int32_t)((double)met_st_acc_nost[i] * met_acc_conv) >> 3;
        met_st_mag_st[i] = (int32_t)((double)met_st_mag_st[i] * met_mag_conv) >> 6;
        met_st_mag_nost[i] = (int32_t)((double)met_st_mag_nost[i] * met_mag_conv) >> 6;
    }

    for(int i = 0; i < 3; i++) {
        test = met_selftest_state.res_acc[i] = abs(met_st_acc_st[i] - met_st_acc_nost[i]);
        met_selftest_state.acc_success &= ((met_st_range[ACC_MIN] <= test) && (test <= met_st_range[ACC_MAX]));
    }

    test = met_selftest_state.res_mag[0] = abs(met_st_mag_st[0] - met_st_mag_nost[0]);
    met_selftest_state.mag_success &= ((met_st_range[MAG_XY_MIN] <= test) && (test <= met_st_range[MAG_XY_MAX]));

    test = met_selftest_state.res_mag[1] = abs(met_st_mag_st[1] - met_st_mag_nost[1]);
    met_selftest_state.mag_success &= ((met_st_range[MAG_XY_MIN] <= test) && (test <= met_st_range[MAG_XY_MAX]));

    test = met_selftest_state.res_mag[2] = abs(met_st_mag_st[2] - met_st_mag_nost[2]);
    met_selftest_state.mag_success &= ((met_st_range[MAG_Z_MIN] <= test) && (test <= met_st_range[MAG_Z_MAX]));

    printf("Selftest: ACC %s %d, %d, %d\r\n", (met_selftest_state.acc_success) ? "OK" : "FAIL", met_selftest_state.res_acc[0], met_selftest_state.res_acc[1], met_selftest_state.res_acc[2]);
    printf("Selftest: MAG %s %d, %d, %d\r\n", (met_selftest_state.mag_success) ? "OK" : "FAIL", met_selftest_state.res_mag[0], met_selftest_state.res_mag[1], met_selftest_state.res_mag[2]);

    twi_transaction_do(met_trans_init, met_trans_init_size, NULL, NULL);

}


//-------------------------------------------------------------

void met_st_mag_test_zyxda(void *ctx);


//

void met_twi_read_st_mag_stimuli_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_stimuli_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }
    sch_unique_oneshot(met_st_mag_test_zyxda, 90);
}

//

void met_twi_read_st_mag_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }

    int16_t mag_x = (*(int16_t*)(met_st_mag_buffer));
    int16_t mag_y = (*(int16_t*)(met_st_mag_buffer+2));
    int16_t mag_z = (*(int16_t*)(met_st_mag_buffer+4));

    //printf("Selftest: %d, %d, %d\r\n", mag_x, mag_y, mag_z);

    if(nost_read) {
        nost_read--;
        if(!nost_read) {
            //next step
            twi_transaction_do(met_trans_st_mag_stimuli, met_trans_st_mag_stimuli_size, met_twi_read_st_mag_stimuli_done, NULL);
        } else {
            if(nost_read != 64) {
                met_st_mag_nost[0] += mag_x;
                met_st_mag_nost[1] += mag_y;
                met_st_mag_nost[2] += mag_z;
            }
            sch_unique_oneshot(met_st_mag_test_zyxda, 11);
        }
        return;
    }
    if(st_read) {
        st_read--;
        if(!st_read) {
            //done
            twi_transaction_do(met_trans_st_mag_done, met_trans_st_mag_done_size, met_twi_read_st_done_done, NULL);
        } else {
            if(st_read != 64) {
                met_st_mag_st[0] += mag_x;
                met_st_mag_st[1] += mag_y;
                met_st_mag_st[2] += mag_z;
            }
            sch_unique_oneshot(met_st_mag_test_zyxda, 11);
        }
        return;
    }

}

void met_twi_read_st_mag_test_zyxda_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_test_zyxda_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }
    if(met_reg_st_status_reg_m[0] & 0x08) {
        twi_transaction_do(met_trans_st_mag_read, met_trans_st_mag_read_size, met_twi_read_st_mag_done, NULL);
    } else {
        sch_unique_oneshot(met_st_mag_test_zyxda, 11);
    }
}

void met_st_mag_test_zyxda(void *ctx) {
    twi_transaction_do(met_trans_st_status_reg_m, met_trans_st_status_reg_m_size, met_twi_read_st_mag_test_zyxda_done, NULL);
}

//

void met_twi_read_st_mag_init_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_init_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }
    nost_read = 65;
    st_read = 65;
    met_st_mag_nost[0] = met_st_mag_nost[1] = met_st_mag_nost[2] = 0;
    met_st_mag_st[0] = met_st_mag_st[1] = met_st_mag_st[2] = 0;
    sch_unique_oneshot(met_st_mag_test_zyxda, 90);
}

//-------------------------------------------------------------

void met_st_acc_test_zyxda(void *ctx);

//

void met_twi_read_st_acc_done_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_init_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }
    twi_transaction_do(met_trans_st_mag_init, met_trans_st_mag_init_size, met_twi_read_st_mag_init_done, NULL);
}


//

void met_twi_read_st_acc_stimuli_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_stimuli_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }
    sch_unique_oneshot(met_st_acc_test_zyxda, 90);
}

//

void met_twi_read_st_acc_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }

    int16_t acc_x = (*(int16_t*)(met_st_acc_buffer)) >> ((met_agr) ? 6 : 0);
    int16_t acc_y = (*(int16_t*)(met_st_acc_buffer+2)) >> ((met_agr) ? 6 : 0);
    int16_t acc_z = (*(int16_t*)(met_st_acc_buffer+4)) >> ((met_agr) ? 6 : 0);

    //printf("Selftest: %d, %d, %d\r\n", acc_x, acc_y, acc_z);

    if(nost_read) {
        nost_read--;
        if(!nost_read) {
            //next step
            twi_transaction_do(met_trans_st_acc_stimuli, met_trans_st_acc_stimuli_size, met_twi_read_st_acc_stimuli_done, NULL);
        } else {
            if(nost_read != 8) {
                met_st_acc_nost[0] += acc_x;
                met_st_acc_nost[1] += acc_y;
                met_st_acc_nost[2] += acc_z;
            }
            sch_unique_oneshot(met_st_acc_test_zyxda, 11);
        }
        return;
    }
    if(st_read) {
        st_read--;
        if(!st_read) {
            //done
            twi_transaction_do(met_trans_st_acc_done, met_trans_st_acc_done_size, met_twi_read_st_acc_done_done, NULL);
        } else {
            if(st_read != 8) {
                met_st_acc_st[0] += acc_x;
                met_st_acc_st[1] += acc_y;
                met_st_acc_st[2] += acc_z;
            }
            sch_unique_oneshot(met_st_acc_test_zyxda, 11);
        }
        return;
    }

}

void met_twi_read_st_acc_test_zyxda_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_test_zyxda_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }
    if(met_reg_st_status_reg_a[0] & 0x08) {
        twi_transaction_do(met_trans_st_acc_read, met_trans_st_acc_read_size, met_twi_read_st_acc_done, NULL);
    } else {
        sch_unique_oneshot(met_st_acc_test_zyxda, 11);
    }
}

void met_st_acc_test_zyxda(void *ctx) {
    twi_transaction_do(met_trans_st_status_reg_a, met_trans_st_status_reg_a_size, met_twi_read_st_acc_test_zyxda_done, NULL);
}

//

void met_twi_read_st_acc_init_done(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_st_init_done %d \r\n", result);
        met_selftest_active = false;
        return;
    }
    nost_read = 9;
    st_read = 9;
    met_st_acc_nost[0] = met_st_acc_nost[1] = met_st_acc_nost[2] = 0;
    met_st_acc_st[0] = met_st_acc_st[1] = met_st_acc_st[2] = 0;
    sch_unique_oneshot(met_st_acc_test_zyxda, 90);
}

//-------------------------------------------------------------

void met_st_start(void *ctx) {
    twi_transaction_do(met_trans_st_acc_init, met_trans_st_acc_init_size, met_twi_read_st_acc_init_done, NULL);
}

//============================================================================
//

float met_getCompass() {
    return met_compass;
}


int16_t met_getTemperature() {
    return met_temp;
}

void met_shutdown() {
    met_shutdown_mode = true;
    twi_transaction_do(met_trans_shutdown, met_trans_shutdown_size, NULL, NULL);
}

void met_selftest_do(void) {
    met_selftest_active = true;
    met_selftest_state.acc_success = false;
    met_selftest_state.mag_success = false;
    met_selftest_state.int_success = false;
    met_selftest_state.agr = met_agr;
    sch_unique_oneshot(met_st_start, 100);
}

met_selftest_state_t *met_selftest_getState(void) {
    met_selftest_state.pending = met_selftest_active;
    return &met_selftest_state;
}


void met_evt_cb( ble_evt_t * p_ble_evt ) {
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GAP_EVT_CONNECTED:
            met_mag_enable();
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            met_mag_disable();
            break;
        default:
            break;
    }

}

void met_twi_read_whoami(ret_code_t result, void *ctx) {
    if(result != 0) {
        printf("met_twi_read_whoami %d \r\n", result);
        return;
    }
    //printf("met WhoamI: %d,%d\r\n", met_reg_whoami[0],met_reg_whoami[1]);

    if(met_reg_whoami[1] == 64) {

        printf("met lsm303agr\r\n");

        met_agr = true;

        met_trans_init = met_trans_init_agr;
        met_trans_shutdown = met_trans_shutdown_agr;
        met_trans_mag_state = met_trans_mag_state_agr;
        met_trans_read = met_trans_read_agr;

        met_trans_init_size = met_trans_init_size_agr;
        met_trans_shutdown_size = met_trans_shutdown_size_agr;
        met_trans_mag_state_size = met_trans_mag_state_size_agr;
        met_trans_read_size = met_trans_read_size_agr;

        met_data_mag_state = met_data_mag_state_agr;

        met_trans_st_acc_init = met_trans_st_acc_init_agr;
        met_trans_st_acc_done = met_trans_st_acc_done_agr;
        met_trans_st_acc_stimuli = met_trans_st_acc_stimuli_agr;
        met_trans_st_status_reg_a = met_trans_st_status_reg_a_agr;
        met_trans_st_acc_read = met_trans_st_acc_read_agr;
        met_trans_st_mag_init = met_trans_st_mag_init_agr;
        met_trans_st_mag_done = met_trans_st_mag_done_agr;
        met_trans_st_mag_stimuli = met_trans_st_mag_stimuli_agr;
        met_trans_st_status_reg_m = met_trans_st_status_reg_m_agr;
        met_trans_st_mag_read = met_trans_st_mag_read_agr;

        met_trans_st_acc_init_size = met_trans_st_acc_init_size_agr;
        met_trans_st_acc_done_size = met_trans_st_acc_done_size_agr;
        met_trans_st_acc_stimuli_size = met_trans_st_acc_stimuli_size_agr;
        met_trans_st_status_reg_a_size = met_trans_st_status_reg_a_size_agr;
        met_trans_st_acc_read_size = met_trans_st_acc_read_size_agr;
        met_trans_st_mag_init_size = met_trans_st_mag_init_size_agr;
        met_trans_st_mag_done_size = met_trans_st_mag_done_size_agr;
        met_trans_st_mag_stimuli_size = met_trans_st_mag_stimuli_size_agr;
        met_trans_st_status_reg_m_size = met_trans_st_status_reg_m_size_agr;
        met_trans_st_mag_read_size = met_trans_st_mag_read_size_agr;

        met_mag_defstate = MET_MAG_DEFSTATE_AGR;

        met_acc_conv = LSM303AGR_ACC_SENS_2G_HR;
        met_mag_conv = LSM303AGR_MAG_SENS;

        met_st_range = met_st_range_agr;

    } else if(met_reg_whoami[0] == 61) {

        printf("met lsm303ctr\r\n");

        met_agr = false;

        met_trans_init = met_trans_init_ctr;
        met_trans_shutdown = met_trans_shutdown_ctr;
        met_trans_mag_state = met_trans_mag_state_ctr;
        met_trans_read = met_trans_read_ctr;

        met_trans_init_size = met_trans_init_size_ctr;
        met_trans_shutdown_size = met_trans_shutdown_size_ctr;
        met_trans_mag_state_size = met_trans_mag_state_size_ctr;
        met_trans_read_size = met_trans_read_size_ctr;

        met_data_mag_state = met_data_mag_state_ctr;

        met_trans_st_acc_init = met_trans_st_acc_init_ctr;
        met_trans_st_acc_done = met_trans_st_acc_done_ctr;
        met_trans_st_acc_stimuli = met_trans_st_acc_stimuli_ctr;
        met_trans_st_status_reg_a = met_trans_st_status_reg_a_ctr;
        met_trans_st_acc_read = met_trans_st_acc_read_ctr;
        met_trans_st_mag_init = met_trans_st_mag_init_ctr;
        met_trans_st_mag_done = met_trans_st_mag_done_ctr;
        met_trans_st_mag_stimuli = met_trans_st_mag_stimuli_ctr;
        met_trans_st_status_reg_m = met_trans_st_status_reg_m_ctr;
        met_trans_st_mag_read = met_trans_st_mag_read_ctr;

        met_trans_st_acc_init_size = met_trans_st_acc_init_size_ctr;
        met_trans_st_acc_done_size = met_trans_st_acc_done_size_ctr;
        met_trans_st_acc_stimuli_size = met_trans_st_acc_stimuli_size_ctr;
        met_trans_st_status_reg_a_size = met_trans_st_status_reg_a_size_ctr;
        met_trans_st_acc_read_size = met_trans_st_acc_read_size_ctr;
        met_trans_st_mag_init_size = met_trans_st_mag_init_size_ctr;
        met_trans_st_mag_done_size = met_trans_st_mag_done_size_ctr;
        met_trans_st_mag_stimuli_size = met_trans_st_mag_stimuli_size_ctr;
        met_trans_st_status_reg_m_size = met_trans_st_status_reg_m_size_ctr;
        met_trans_st_mag_read_size = met_trans_st_mag_read_size_ctr;

        met_mag_defstate = MET_MAG_DEFSTATE_CTR;

        met_acc_conv = LSM303CTR_ACC_SENS_2G;
        met_mag_conv = LSM303CTR_MAG_SENS;

        met_st_range = met_st_range_ctr;

    } else {
        ERR_CHECK("MET WHOAMI UNKNOWN", 1);
    }

    twi_transaction_do(met_trans_init, met_trans_init_size, NULL, NULL);

}

uint8_t met_getHW() {
    return (met_agr) ? 1 : 0;
}

void met_init() {
    ret_code_t err;

    memset(&met_settings, 0, sizeof(met_settings_t));
    rec_read(KEY_METSETTINGS, &met_settings, sizeof(met_settings_t), NULL);
    printf("met center: %lf, %lf, %lf\r\n", met_settings.center[0],met_settings.center[1],met_settings.center[2]);
    printf("met radii: %lf, %lf, %lf\r\n", met_settings.radii[0],met_settings.radii[1],met_settings.radii[2]);

    //twi_transaction_do(met_trans_init, met_trans_init_size, NULL, NULL);
    twi_transaction_do(met_trans_whoami, met_trans_whoami_size, met_twi_read_whoami, NULL);

    if(!nrf_drv_gpiote_is_init())
    {
        err = nrf_drv_gpiote_init();
        ERR_CHECK("nrf_drv_gpiote_init", err);
    }

    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    config.pull = NRF_GPIO_PIN_PULLDOWN;
    err = nrf_drv_gpiote_in_init(ACCEL_INT_1_PIN, &config, met_int_handler);
    ERR_CHECK("nrf_drv_gpiote_in_init", err);
    nrf_drv_gpiote_in_event_enable(ACCEL_INT_1_PIN, true);

    bleapp_evt_register(met_evt_cb);

}

#endif

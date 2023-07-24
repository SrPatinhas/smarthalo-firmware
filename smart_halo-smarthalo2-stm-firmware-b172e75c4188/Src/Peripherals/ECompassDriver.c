/*
 * eCompassDriver.c
 *
 *  Created on: July 3
 *      Author: JF Lemire
 */


#include <ECompassDriver.h>
#include <SystemUtilitiesTask.h>
#include "lsm303agr_reg.h"
#include "i2c.h"

typedef struct
{
    lsm303agr_ctx_t dev_ctx_xl;
    lsm303agr_ctx_t dev_ctx_mg;
}oeCompassDriver_t, *poeCompassDriver_t;

oeCompassDriver_t oeCompassDriver;

static int32_t eCompassDriverWrite(void *handle, uint8_t u8Register, uint8_t *pu8Data, uint16_t u16Length);
static int32_t eCompassDriverRead(void *handle, uint8_t u8Register, uint8_t *pu8Data, uint16_t u16Length);
/**
 * @brief       Initialize the E Compass
 * @details     Configure the interrupts and sensitivity of the accelerometer, temperature and magnetometer
 * @return      bool: true if success, false otherwise.
 */
bool init_ECompassDriver(void)
{
    uint8_t u8WhoamI = 0;
    uint8_t u8Rst = 0;

    oeCompassDriver.dev_ctx_xl.write_reg = eCompassDriverWrite;
    oeCompassDriver.dev_ctx_xl.read_reg = eCompassDriverRead;
    oeCompassDriver.dev_ctx_xl.handle = (void*) LSM303AGR_I2C_ADD_XL;

    oeCompassDriver.dev_ctx_mg.write_reg = eCompassDriverWrite;
    oeCompassDriver.dev_ctx_mg.read_reg = eCompassDriverRead;
    oeCompassDriver.dev_ctx_mg.handle = (void*) LSM303AGR_I2C_ADD_MG;
    /*
     *  Check device ID
     */
    u8WhoamI = 0;
    lsm303agr_xl_device_id_get(&oeCompassDriver.dev_ctx_xl, &u8WhoamI);
    if (u8WhoamI != LSM303AGR_ID_XL) {
        log_Shell("lsm303agr_xl_device_id_get ERROR %d", u8WhoamI);
        // todo critical error. The axl is not detected.
        while (1);
    }

    u8WhoamI = 0;
    lsm303agr_mag_device_id_get(&oeCompassDriver.dev_ctx_mg, &u8WhoamI);
    if (u8WhoamI != LSM303AGR_ID_MG) {
        log_Shell("lsm303agr_mag_device_id_get ERROR %d", u8WhoamI);
        // todo critical error. The mag is not detected.
        while (1);
    }

    /*
     *  Restore default configuration for magnetometer
     */
    lsm303agr_mag_reset_set(&oeCompassDriver.dev_ctx_mg, PROPERTY_ENABLE);
    do {
        lsm303agr_mag_reset_get(&oeCompassDriver.dev_ctx_mg, &u8Rst);
    } while (u8Rst);

    // Configure the xl
    lsm303agr_xl_data_rate_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_XL_ODR_100Hz);
    // Set accelerometer full scale
    lsm303agr_xl_full_scale_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_2g);
    // High pass filtered data selection
    lsm303agr_xl_high_pass_on_outputs_set(&oeCompassDriver.dev_ctx_xl, PROPERTY_DISABLE);
    // Cut off frequency Configuration
    lsm303agr_xl_high_pass_bandwidth_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_AGGRESSIVE);
    // High pass filter mode
    lsm303agr_xl_high_pass_mode_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_NORMAL_WITH_RST);
    // High pass filter enables
    lsm303agr_xl_high_pass_int_conf_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_ON_INT1_GEN);
    // Enable interrupt 1
    lsm303agr_ctrl_reg3_a_t reg3_a;
    memset(&reg3_a, 0x00, sizeof(lsm303agr_ctrl_reg3_a_t));
    reg3_a.i1_aoi1 = PROPERTY_ENABLE;
    lsm303agr_xl_pin_int1_config_set(&oeCompassDriver.dev_ctx_xl, &reg3_a);
    // Set device in high resolution mode
    lsm303agr_xl_operating_mode_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_HR_12bit);
    // Set block data update, do not update until it's been read
    lsm303agr_xl_block_data_update_set(&oeCompassDriver.dev_ctx_xl, PROPERTY_ENABLE);
    // Disable SPI mode
    lsm303agr_xl_spi_mode_set(&oeCompassDriver.dev_ctx_xl, PROPERTY_DISABLE);

    // set trigger event
    lsm303agr_xl_fifo_trigger_event_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_INT1_GEN);
    // set FIFO Mode
    lsm303agr_xl_fifo_mode_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_BYPASS_MODE);

    // set interrupt configuration
    lsm303agr_int1_cfg_a_t int_1_cfg;
    memset(&int_1_cfg, 0x00, sizeof(lsm303agr_int1_cfg_a_t));
    int_1_cfg.xhie = PROPERTY_ENABLE;
    int_1_cfg.yhie = PROPERTY_ENABLE;
    int_1_cfg.zhie = PROPERTY_ENABLE;
    lsm303agr_xl_int1_gen_conf_set(&oeCompassDriver.dev_ctx_xl, &int_1_cfg);

    // set interrupt threshold
    lsm303agr_int1_ths_a_t int1_ths;
    memset(&int1_ths, 0x00, sizeof(lsm303agr_int1_ths_a_t));
    int1_ths.ths = 2;
    lsm303agr_xl_int1_gen_threshold_set(&oeCompassDriver.dev_ctx_xl, int1_ths.ths);

    // set interrupt duration
    lsm303agr_int1_duration_a_t int1_duration;
    memset(&int1_duration, 0x00, sizeof(lsm303agr_int1_duration_a_t));
    int1_duration.d = 4;
    lsm303agr_xl_int1_gen_duration_set(&oeCompassDriver.dev_ctx_xl, int1_duration.d);

    /*
     *  Enable Block Data Update
     */
    lsm303agr_mag_block_data_update_set(&oeCompassDriver.dev_ctx_mg, PROPERTY_ENABLE);
    /*
     * Set Output Data Rate
     */
    lsm303agr_mag_data_rate_set(&oeCompassDriver.dev_ctx_mg, LSM303AGR_MG_ODR_10Hz);
    /*
     * Set / Reset magnetic sensor mode
     */
    lsm303agr_mag_set_rst_mode_set(&oeCompassDriver.dev_ctx_mg, LSM303AGR_SENS_OFF_CANC_EVERY_ODR);
    /*
     * Enable temperature compensation on mag sensor
     */
    lsm303agr_mag_offset_temp_comp_set(&oeCompassDriver.dev_ctx_mg, PROPERTY_ENABLE);
    /*
     * Set magnetometer in continuous mode
     */
    lsm303agr_mag_operating_mode_set(&oeCompassDriver.dev_ctx_mg, LSM303AGR_CONTINUOUS_MODE);
    /*
     * Enable temperature sensor
     */
    lsm303agr_temperature_meas_set(&oeCompassDriver.dev_ctx_xl, LSM303AGR_TEMP_ENABLE);

    log_Shell("eCompass OK");

    return true;
}

/**
 * @brief       Get acceleromter Data
 * @details     Request accelerometer axis data from the e compass
 * @param[in]	pfX: handle on the variable to return X axis
 * @param[in]	pfY: handle on the variable to return Y axis
 * @param[in]	pfZ: handle on the variable to return Z axis
 * @param[out]	pbNewData: handle on the variable to indicate a new data
 * @return      bool: true if success, false otherwise.
 */
bool getXLData_ECompassDriver(float * pfX, float * pfY, float * pfZ, bool * pbNewData)
{
    lsm303agr_reg_t reg;
    axis3bit16_t data_raw_acceleration;

    if ((pfX == NULL) || (pfY == NULL) || (pfZ == NULL)) return false;

    *pbNewData = false;

    uint8_t ret = lsm303agr_xl_status_get(&oeCompassDriver.dev_ctx_xl, &reg.status_reg_a);

    if (reg.status_reg_a.zyxda) {
        /* Read accelerometer data */
        memset(data_raw_acceleration.u8bit, 0x00, 3 * sizeof(int16_t));
        lsm303agr_acceleration_raw_get(&oeCompassDriver.dev_ctx_xl, data_raw_acceleration.u8bit);
        *pfX = lsm303agr_from_fs_2g_hr_to_mg(data_raw_acceleration.i16bit[0]);
        *pfY = lsm303agr_from_fs_2g_hr_to_mg(data_raw_acceleration.i16bit[1]);
        *pfZ = lsm303agr_from_fs_2g_hr_to_mg(data_raw_acceleration.i16bit[2]);
        *pbNewData = true;
    }

    return ret == 0;
}
/**
 * @brief       Get magnetometer data
 * @details     Request magnetic data from the e compass
 * @param[in]	pfX: handle on the variable to return X axis
 * @param[in]	pfY: handle on the variable to return Y axis
 * @param[in]	pfZ: handle on the variable to return Z axis
 * @param[out]	pbNewData: handle on the variable to indicate a new data
 * @return      bool: true if success, false otherwise.
 */
bool getMAGData_ECompassDriver(float * pfX, float * pfY, float * pfZ, bool * pbNewData)
{
    lsm303agr_reg_t reg;
    axis3bit16_t data_raw_magnetic;

    if ((pfX == NULL) || (pfY == NULL) || (pfZ == NULL)) return false;

    *pbNewData = false;

    uint8_t ret = lsm303agr_mag_status_get(&oeCompassDriver.dev_ctx_mg, &reg.status_reg_m);

    if (reg.status_reg_m.zyxda) {
        /* Read accelerometer data */
        memset(data_raw_magnetic.u8bit, 0x00, 3 * sizeof(int16_t));
        lsm303agr_magnetic_raw_get(&oeCompassDriver.dev_ctx_mg, data_raw_magnetic.u8bit);
        *pfX = lsm303agr_from_lsb_to_mgauss(data_raw_magnetic.i16bit[0]);
        *pfY = lsm303agr_from_lsb_to_mgauss(data_raw_magnetic.i16bit[1]);
        *pfZ = lsm303agr_from_lsb_to_mgauss(data_raw_magnetic.i16bit[2]);
        *pbNewData = true;
    }

    return ret == 0;
}
/**
 * @brief       Get temperature data
 * @details     Request temperature data from the e compass
 * @public
 * @param[in]	pfTemp: percentage of the white led.
 * @param[out]	pbNewData: handle on the variable to indicate a new data
 * @return      bool: true if success, false otherwise.
 */
bool getTemperatureData_ECompassDriver(float * pfTemp, bool * pbNewData)
{
    lsm303agr_reg_t reg;
    axis3bit16_t data_raw_temperature;

    if (pfTemp == NULL) return false;

    *pbNewData = false;

    lsm303agr_temp_data_ready_get(&oeCompassDriver.dev_ctx_xl, &reg.byte);

    if (reg.byte) {
        /* Read accelerometer data */
        memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
        lsm303agr_temperature_raw_get(&oeCompassDriver.dev_ctx_xl, data_raw_temperature.u8bit);
        *pfTemp = (float)lsm303agr_from_lsb_hr_to_celsius(data_raw_temperature.i16bit[0]);
        *pbNewData = true;
    }

    return true;
}

/**
 * @brief       Write to the accelerometer/ecompass
 * @private
 * @param[in]	handle: Device addr.
 * @param[in]	u8Register: Register to access.
 * @param[in]	pu8Data: Handle on the buffer to write
 * @param[in]	u16Length: number of byte to write.
 * @return      zero on success
 */
static int32_t eCompassDriverWrite(void *handle, uint8_t u8Register, uint8_t *pu8Data, uint16_t u16Length) {
    uint32_t u32DeviceAddr = (uint32_t) handle;
    if (u32DeviceAddr == LSM303AGR_I2C_ADD_XL) {
        /* enable auto incremented in multiple read/write commands */
        u8Register |= 0x80;
    }

    if (pu8Data == NULL) return false;
    return !HAL_I2C_Write(&SENSOR_I2C, (uint8_t)u32DeviceAddr, u8Register, pu8Data, u16Length);
}

/**
 * @brief       Read from the accelerometer/ecompass
 * @private
 * @param[in]	handle: Device addr.
 * @param[in]	u8Register: Register to access.
 * @param[in]	pu8Data: Handle on the buffer to read
 * @param[in]	u16Length: number of byte to read.
 * @return      zero on success
 */
static int32_t eCompassDriverRead(void *handle, uint8_t u8Register, uint8_t *pu8Data, uint16_t u16Length) {
    uint32_t u32DeviceAddr = (uint32_t) handle;

    if (u32DeviceAddr == LSM303AGR_I2C_ADD_XL) {
        /* enable auto incremented in multiple read/write commands */
        u8Register |= 0x80;
    }
    return !HAL_I2C_Read(&SENSOR_I2C, (uint8_t)u32DeviceAddr, u8Register, pu8Data, u16Length);
}


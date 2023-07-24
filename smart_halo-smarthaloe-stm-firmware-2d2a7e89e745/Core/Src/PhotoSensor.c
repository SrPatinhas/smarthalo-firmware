// ------------------------------------------------------------------------------------------------
/*!@file    PhotoSensor.c
 */
// ------------------------------------------------------------------------------------------------
#include <PhotoSensor.h>
#include <string.h>
#include "stm32h7xx_hal.h"
//#include <SystemUtilitiesTask.h>
//#include "i2c.h"
// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define LTR329ALS01_SENSOR_ADDR (0x29 << 1)

#define LTR329ALS01_ALS_CONTR       0x80
#define LTR329ALS01_ALS_MEAS_RATE   0x85
#define LTR329ALS01_PART_ID         0x86
#define LTR329ALS01_MANUFAC_ID      0x87
#define LTR329ALS01_ALS_DATA_CH1_0  0x88
#define LTR329ALS01_ALS_DATA_CH1_1  0x89
#define LTR329ALS01_ALS_DATA_CH0_0  0x8A
#define LTR329ALS01_ALS_DATA_CH0_1  0x8B
#define LTR329ALS01_ALS_STATUS      0x8C

#define LTR329ALS01_PART_ID_DEFAULT 0xA0
#define LTR329ALS01_MANUFAC_ID_DEFAULT 0x05
// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================

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

// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================
typedef struct
{
    bool                bInitDone;
    bool                bAwake;
    eLTR329ALS01Gain    eGain;
}oPhotoSensor_t, *poPhotoSensor_t;

oPhotoSensor_t oPhotoSensor;
// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================

bool photoSensorWriteRegister(uint8_t u8DeviceAddr, uint8_t u8Register, uint8_t * pu8Data, uint16_t u16Length);
bool photoSensorReadRegister(uint8_t u8DeviceAddr, uint8_t u8Register, uint8_t * pu8Data, uint16_t u16Length);

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================
/**
 * @brief       Initialize the photo sensor
 * @details     Configure the photo sensor
 * @public
 * @param[in]   egain: Gain to apply
 * @param[out]  result: handle to return the result.
 * @return      bool: true if success, false otherwise.
 */
bool init_PhotoSensor(eLTR329ALS01Gain eGain, bool *result)
{
    uint8_t u8Buffer[2];

    if (oPhotoSensor.bInitDone == true)
        return true;

    oPhotoSensor.eGain = eGain;

    if (photoSensorReadRegister(LTR329ALS01_SENSOR_ADDR, LTR329ALS01_PART_ID, u8Buffer, 2) == false) return false;

    if ((u8Buffer[0] == LTR329ALS01_PART_ID_DEFAULT) &&
            (u8Buffer[1] == LTR329ALS01_MANUFAC_ID_DEFAULT))
    {
        //log_Shell("Light Sensor Init OK");
        oPhotoSensor.bInitDone = true;
        *result = true;
    }
    else
    {
        //log_Shell("Light Init ERROR");
        oPhotoSensor.bInitDone = false;
        *result = false;
    }

    // Update Gain register and activate
    u8Buffer[0] = (oPhotoSensor.eGain << 2 | 0x01);

    if (photoSensorWriteRegister(LTR329ALS01_SENSOR_ADDR, LTR329ALS01_ALS_CONTR, u8Buffer, 1) == false) return false;

    oPhotoSensor.bAwake = true;

    //vTaskDelay(30 / portTICK_RATE_MS);
    HAL_Delay(30);
#warning "swap this back when OS stable ^"

    //log_Shell("Photo Sensor Init Done");

    return true;
}

bool sleep_PhotoSensor(void)
{
    uint8_t value = 0;

    if (photoSensorWriteRegister(LTR329ALS01_SENSOR_ADDR, LTR329ALS01_ALS_CONTR, &value, 1) == false) {
        return false;
    }

    oPhotoSensor.bAwake = false;
    return true;
}

bool wake_PhotoSensor(void)
{
    uint8_t value;

    if (!oPhotoSensor.bInitDone) return false;

    // Update Gain register and activate
    value = (oPhotoSensor.eGain << 2 | 0x01);
    if (photoSensorWriteRegister(LTR329ALS01_SENSOR_ADDR, LTR329ALS01_ALS_CONTR, &value, 1) == false) {
        return false;
    }

    oPhotoSensor.bAwake = true;
    return true;
}

/**
 * @brief       Read the photo sensor data
 * @details     Get the photo data from the sensor
 * @public
 * @param[in]   channel0: handle on the variable for the channel 0
 * @param[in]   channel1: handle on the variable for the channel 1
 * @param[out]  result: handle to return the result.
 * @return      * bool: true if success, false otherwise.
 */
bool readData_PhotoSensor(uint16_t * channel0, uint16_t * channel1, bool * result)
{
    uint8_t u8Buffer[4];
    *result = false;
    if ((channel0 == NULL) || (channel1 == NULL) || (result == NULL)) return false;

    if (oPhotoSensor.bInitDone == false || oPhotoSensor.bAwake == false)return true;
    // Read values
    if (photoSensorReadRegister(LTR329ALS01_SENSOR_ADDR, LTR329ALS01_ALS_DATA_CH1_0, u8Buffer, 4) == false) return false;

    *channel0 = u8Buffer[0] + (u8Buffer[1] << 8);
    *channel1 = u8Buffer[2] + (u8Buffer[3] << 8);
    *result = true;
    return true;
}

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================
/**
 * @brief       PhotoSensorWriteRegister()
 * @details     Function to Write a register
 * @public
 * @param[in]   u8DeviceAddr: I2C Addr
 * @param[in]   u8Register: Register addr.
 * @param[in]   pu8Data: handle on the data to write
 * @param[in]   u16Length: number of byte to write
 * @return      bool: true if success, false otherwise.
 */
bool photoSensorWriteRegister(uint8_t u8DeviceAddr, uint8_t u8Register, uint8_t * pu8Data, uint16_t u16Length)
{
    //bool bResponse = false;
    if (pu8Data == NULL) return false;

    //if (HAL_I2C_Write(&LIGHT_SENSOR_I2C, u8DeviceAddr, u8Register, pu8Data, u16Length, &bResponse) == false) return false;

    //SBEITZ
    uint8_t reg_plus_data[5] = {u8Register};
    if(u16Length <= 4) memcpy(reg_plus_data+1,pu8Data,u16Length);
    else return false;
    HAL_I2C_Master_Transmit(&LIGHT_SENSOR_I2C, u8DeviceAddr, reg_plus_data, 2, 0xFFFF);
    //SBEITZ
#warning "swap this back ^ when i2c.c is integrated"
    return true;
}
/**
 * @brief       PhotoSensorReadRegister()
 * @details     Function to read a register
 * @public
 * @param[in]   u8DeviceAddr: I2C Addr
 * @param[in]   u8Register: Register addr.
 * @param[in]   pu8Data: handle on the data to read
 * @param[in]   u16Length: number of byte to read
 * @return      bool: true if success, false otherwise.
 */
bool photoSensorReadRegister(uint8_t u8DeviceAddr, uint8_t u8Register, uint8_t * pu8Data, uint16_t u16Length)
{
    //bool bResponse = false;
    if (pu8Data == NULL) return false;
   // if (HAL_I2C_Read(&LIGHT_SENSOR_I2C,  u8DeviceAddr, u8Register, pu8Data, u16Length, &bResponse) == false) return false;

   //SBEITZ
    HAL_I2C_Master_Transmit(&LIGHT_SENSOR_I2C, u8DeviceAddr, &u8Register, 1, 0xFFFF);
    HAL_I2C_Master_Receive(&LIGHT_SENSOR_I2C, u8DeviceAddr, pu8Data, u16Length, 0xFFFF);
    //SBEITZ
#warning "swap this back ^ when i2c.c is integrated"
    return true;
}


/*
 * HaloLedsDriver.c
 *
 *  Created on: June 21
 *      Author: JF Lemire
 */

#include <stdbool.h>
#include "HaloLedsDriver.h"
#include "i2c.h"
#include "semphr.h"
#include "Power.h"
#include "Shell.h"
#include "wwdg.h"

#define HALO_LEDS_NUMBER		72	/// Number of halo LEDs

#define IS31_LOW_I2C_ADD (0x3C << 1)
#define IS31_HIGH_I2C_ADD (0x3F << 1)

#define _ENABLE_LED_VLED() HAL_GPIO_WritePin(EN_VLED_GPIO_Port, EN_VLED_Pin, GPIO_PIN_SET)
#define _DISABLE_LED_VLED() HAL_GPIO_WritePin(EN_VLED_GPIO_Port, EN_VLED_Pin, GPIO_PIN_RESET)

#define _LED_SDB_HIGH() HAL_GPIO_WritePin(LED_SDB_GPIO_Port, LED_SDB_Pin, GPIO_PIN_SET)
#define _LED_SDB_LOW() HAL_GPIO_WritePin(LED_SDB_GPIO_Port, LED_SDB_Pin, GPIO_PIN_RESET)

#define	SHUTDOWN_REGISTER		0x00	/// Set software shutdown mode. 0 = shutdown, 1 = normal operation	
#define	PWM_REGISTER			0x01	/// 36 channels PWM duty cycle data register. The PWM Registers adjusts LED luminous intensity in 256 steps 
#define UPDATE_REGISTER			0x25	/// Load PWM Register and LED Control Register’s data. A write operation of “0000 0000” value to the Update Register to update


#define LED_CONTROL_REGISTER	0x26	/* Channel 1 to 36 enable bit and current setting.		D0 = LED State (0 = off, 1 = on) 
																								D2:D1 = 
																								00 IMAX
																								01 IMAX/2
																								10 IMAX/3
																								11 IMAX/4 	*/

#define GLOBAL_CONTROL_REGISTER	0x4A	/// Set all channels enable
#define RESET_REGISTER			0x4F	/// Reset all registers into default value 

/*used to swap between IMAX and IMAX/4 
(value at position 255/4 in gamma lookup table, minus offset for green and blue correction at lower current)*/
#define BRIGHTNESS_THRESHOLD 149 
///
const uint8_t lookUpTableArray[HALO_LEDS_NUMBER] = { 69, 71, 70, /*LED1*/
        66, 68, 67, /*LED3*/
        63, 65, 64, /*LED5*/
        60, 62, 61, /*LED7*/
        57, 59, 58, /*LED9*/
        54, 55, 56, /*LED11*/
        51, 53, 52, /*LED13*/
        48, 50, 49, /*LED16*/
        45, 47, 46, /*LED18*/
        42, 44, 43, /*LED20*/
        39, 41, 40, /*LED22*/
        36, 38, 37, /*LED24*/
        0, 1, 2, /*LED25*/
        35, 34, 33, /*LED23*/
        30, 31, 32, /*LED21*/
        27, 28, 29, /*LED19*/
        24, 25, 26, /*LED17*/
        21, 22, 23, /*LED15*/
        18, 19, 20, /*LED12*/
        15, 16, 17, /*LED10*/
        12, 13, 14, /*LED8*/
        9, 10, 11, /*LED6*/
        6, 7, 8, /*LED4*/
        3, 5, 4, /*LED2*/
};

// lookups & gamma
const uint8_t gammaLookUpTableHighCurrent[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
        2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
        5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
        10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
        17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
        25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
        37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
        51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
        69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
        90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
        115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
        144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
        177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
        215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

//used for the blue and green correction at lower current IMAX/4 setting, red uses the above table
const uint8_t gammaLookUpTableLowCurrent[] = {
0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0,
0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 1  , 1  , 1  , 1,
1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1  , 1,
1  , 2  , 2  , 2  , 2  , 2  , 2  , 2  , 3  , 3  , 3  , 3  , 3  , 3  , 3  , 3,
3  , 4  , 4  , 4  , 4  , 5  , 5  , 5  , 5  , 5  , 5  , 5  , 6  , 6  , 6  , 7,
7  , 7  , 7  , 7  , 7  , 8  , 8  , 9  , 9  , 9  , 9  , 9  , 10 , 10 , 11 , 11,
11 , 11 , 12 , 12 , 13 , 13 , 13 , 13 , 14 , 14 , 15 , 15 , 15 , 16 , 16 , 17,
17 , 17 , 18 , 18 , 19 , 19 , 19 , 20 , 21 , 21 , 21 , 22 , 23 , 23 , 23 , 24,
25 , 25 , 26 , 26 , 27 , 27 , 28 , 29 , 29 , 30 , 31 , 31 , 32 , 33 , 33 , 33,
34 , 35 , 36 , 37 , 37 , 38 , 39 , 39 , 40 , 41 , 41 , 42 , 43 , 44 , 45 , 45,
46 , 47 , 48 , 49 , 49 , 50 , 51 , 52 , 53 , 54 , 55 , 55 , 57 , 57 , 58 , 59,
60 , 61 , 62 , 63 , 64 , 65 , 66 , 67 , 68 , 69 , 70 , 71 , 73 , 73 , 75 , 76,
77 , 78 , 79 , 80 , 81 , 83 , 84 , 85 , 86 , 87 , 89 , 90 , 91 , 92 , 93 , 95,
96 , 97 , 99 , 100, 101, 103, 104, 105, 107, 108, 109, 111, 113, 114, 115, 117,
118, 120, 121, 123, 124, 126, 127, 129, 131, 132, 133, 135, 137, 139, 140, 142,
143, 145, 147, 149, 150, 152, 154, 155, 157, 159, 161, 163, 165, 166, 168, 170 };

typedef struct
{
    uint8_t ledsPWMArray[HALO_LEDS_NUMBER];
    haloLedDriverCurrent_t ledsPWMCurrentSetting[HALO_LEDS_NUMBER]; //tracks current setting for individual leds
    uint8_t ledRegister;	// This variable is required to exist longer than the scope's function.
}oHaloLeds_t, *poHaloLeds_t;

oHaloLeds_t oHaloLeds;

static bool oHaloPowerState; //true = IMAX, false = IMAX/4

static bool oHaloCurrentSetting;

static bool HaloLedsWriteRegister(uint8_t deviceAddr, uint8_t ledRegister, uint8_t * data, uint16_t length);

/**
 * @brief       Initialize/wakeup the LEDs
 * @details     Configure the LEDs drivers, current and operation
 * @return      bool: true if success, false otherwise.
 */
bool init_HaloLedsDriver(void)
{
    static bool beenHere = false;
    uint8_t u8Buffer[36];

    HAL_WWDG_Refresh(&hwwdg);
    
    _ENABLE_LED_VLED();
    _LED_SDB_LOW();
    vTaskDelay(10);
    _LED_SDB_HIGH();
    vTaskDelay(1);

    // Turn ON all LEDS
    // The LED Control Registers store the on or off state of each LED and set the output current.
    memset(oHaloLeds.ledsPWMCurrentSetting, IMAX, HALO_LEDS_NUMBER);  //   0x01 = Imax and LED on    |+|    0x03 = Imax/2 and LED on
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, LED_CONTROL_REGISTER, oHaloLeds.ledsPWMCurrentSetting, 36) == false) return false;
    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, LED_CONTROL_REGISTER, oHaloLeds.ledsPWMCurrentSetting, 36) == false) return false;

    // All PWM to 0x00
    // The PWM Registers adjusts LED luminous intensity in256 steps.
    memset(u8Buffer, 0x00, 36);
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, PWM_REGISTER, u8Buffer, 36) == false) return false;
    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, PWM_REGISTER, u8Buffer, 36) == false) return false;

    // A write operation of “0000 0000” value to the Update Register is required to update the registers
    oHaloLeds.ledRegister = 0x00;
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, UPDATE_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;
    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, UPDATE_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;

    //G_EN Global LED Enable
    //0 Normal operation
    //1 Shutdown all LEDs
    oHaloLeds.ledRegister = 0x00;
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, GLOBAL_CONTROL_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;
    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, GLOBAL_CONTROL_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;

    //SSD Software Shutdown Enable
    //0 Software shutdown mode
    //1 Normal operation
    oHaloLeds.ledRegister = 0x01;
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, SHUTDOWN_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;
    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, SHUTDOWN_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;

    if (!beenHere) {
        log_Shell("HaloLedsInit Done");
        beenHere = true;
    }

    oHaloPowerState = true;
    return true;
}

/*! @brief  Sleep the LEDs driver chip

            Two methods: kill the power or use low power mode
            Current decision is to kill the power

            There is no corresponding wake function as it would
            be identical to init_HaloLedsDriver()
 */
bool sleep_HaloLedsDriver(void)
{
    // Totally turn off the I31 chip
    _DISABLE_LED_VLED();
    _LED_SDB_LOW();
    oHaloPowerState = false;

    // OR just use software shutdown

    //SSD Software Shutdown Enable 
    //0 Software shutdown mode
    //1 Normal operation 
    // oHaloLeds.ledRegister = 0x00;
    // if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, SHUTDOWN_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;
    // if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, SHUTDOWN_REGISTER, &oHaloLeds.ledRegister, 1) == false) return false;

    return true;
}

/**
 * @brief       Update the LED values
 * @details     Set the LED values with an array of bytes, they are matched with a look up table and a gamma table to make the values linear.
 * @param[in]   buf: Handle on the buffer to write.
 * @return      bool: true if success, false otherwise.
 */
bool update_HaloLedsDriver(uint8_t * buf)
{
    bool retval = true;

    if (buf == NULL) return false;

    // If any of the LEDs are lit, poke power management
    // to "keep the lights on" -- but just once
    for (uint8_t *p = buf; p < buf + HALO_LEDS_NUMBER; p++) {
        if (*p) {
            stayAwake_Power(__func__);
            break;
        }
    }

    if (oHaloPowerState == false) return true;  // fake it

    for (int index = 0; index < HALO_LEDS_NUMBER; ++index) {
        if(buf[index] < BRIGHTNESS_THRESHOLD){

            buf[index] = buf[index]*255/BRIGHTNESS_THRESHOLD; 
            //choose a gamma correction table based on color (red has different current to brightness ratio at lower current)
            if(index%3 == 0) oHaloLeds.ledsPWMArray[lookUpTableArray[index]] = gammaLookUpTableHighCurrent[buf[index]]; //red
            else oHaloLeds.ledsPWMArray[lookUpTableArray[index]] = gammaLookUpTableLowCurrent[buf[index]]; //green and blue
           
            if ((oHaloLeds.ledsPWMCurrentSetting[lookUpTableArray[index]] == IMAX)){
                oHaloLeds.ledsPWMCurrentSetting[lookUpTableArray[index]] = IMAX_DIV_4;
                uint8_t driverAddress, ledChannel; 
                if(lookUpTableArray[index] > 35){//decides which led driver chip to talk to + channel based on led mapping
                    driverAddress = IS31_HIGH_I2C_ADD;
                    ledChannel = LED_CONTROL_REGISTER + lookUpTableArray[index] - 36;
                }else{
                    driverAddress = IS31_LOW_I2C_ADD; 
                    ledChannel = LED_CONTROL_REGISTER + lookUpTableArray[index];
                }
                HaloLedsWriteRegister(driverAddress, ledChannel, &oHaloLeds.ledsPWMCurrentSetting[lookUpTableArray[index]], 1);//may want to add a failure catch mechanism here
            }
        }else{ //higher than threshold
            oHaloLeds.ledsPWMArray[lookUpTableArray[index]] = gammaLookUpTableHighCurrent[buf[index]];
            if ((oHaloLeds.ledsPWMCurrentSetting[lookUpTableArray[index]] != IMAX)){
                oHaloLeds.ledsPWMCurrentSetting[lookUpTableArray[index]] = IMAX;
                uint8_t driverAddress, ledChannel; 
                if(lookUpTableArray[index] > 35){//decides which led driver chip to talk to + channel based on led mapping
                    driverAddress = IS31_HIGH_I2C_ADD;
                    ledChannel = LED_CONTROL_REGISTER + lookUpTableArray[index] - 36;
                }else{
                    driverAddress = IS31_LOW_I2C_ADD; 
                    ledChannel = LED_CONTROL_REGISTER + lookUpTableArray[index];
                }
                HaloLedsWriteRegister(driverAddress, ledChannel, &oHaloLeds.ledsPWMCurrentSetting[lookUpTableArray[index]], 1);//may want to add a failure catch mechanism here
            }
        }
    }  

    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, PWM_REGISTER, &oHaloLeds.ledsPWMArray[HALO_LEDS_NUMBER/2], HALO_LEDS_NUMBER/2) == false) retval = false;
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, PWM_REGISTER, oHaloLeds.ledsPWMArray, HALO_LEDS_NUMBER/2) == false) retval = false;

    oHaloLeds.ledRegister = 0x00;
    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, UPDATE_REGISTER, &oHaloLeds.ledRegister, 1) == false) retval = false;
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, UPDATE_REGISTER, &oHaloLeds.ledRegister, 1) == false) retval = false;

    return retval;
}

/**
 * @brief       HaloLedsWriteRegister()
 * @details     Function to write a Halo Led register.
 * @private
 * @return      bool: true if success, false otherwise.
 */
static bool HaloLedsWriteRegister(uint8_t deviceAddr, uint8_t ledRegister, uint8_t * data, uint16_t length)
{
    if (data == NULL) return false;

    if (HAL_I2C_Write_DMA(&HALO_LED_I2C, deviceAddr, ledRegister, data, length) == false) {
        log_Shell("%s: HAL_I2C_Write_DMA returned fail", __func__);
        return false;
    }
    return true;
}

/**
 * @brief       setCurrent_HaloLedsDriver()
 * @details     Function to swap current settings in the halo leds driver.
 * @private
 * @return      bool: true if success, false otherwise.
 */
bool setCurrent_HaloLedsDriver(haloLedDriverCurrent_t currentSetting)
{
    uint8_t u8Buffer[36];
    oHaloCurrentSetting = currentSetting == IMAX ? true : false; 

    memset(u8Buffer, currentSetting, 36);  //   0x01 = Imax and LED on    |+|    0x07 = Imax/4 and LED on
    if (HaloLedsWriteRegister(IS31_LOW_I2C_ADD, LED_CONTROL_REGISTER, u8Buffer, 36) == false) return false;
    if (HaloLedsWriteRegister(IS31_HIGH_I2C_ADD, LED_CONTROL_REGISTER, u8Buffer, 36) == false) return false;
    return true;
}

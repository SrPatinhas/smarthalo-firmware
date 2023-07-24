/*
 * I2C.c
 *
 *  Created on: Jun 3, 2016
 *      Author: sgelinas
 */

#include "I2C.h"
#include "app_util_platform.h"
#include "CommandLineInterface.h"
#include "nrf_delay.h"

// CONFIG TWI
#define MASTER_TWI_INST         0    //!< TWI interface used as a master accessing EEPROM memory

/**
 * @brief TWI master instance
 *
 * Instance of TWI master driver that would be used for communication with simulated
 * eeprom memory.
 */
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(MASTER_TWI_INST);

/**
 * @brief Initialize the master TWI
 *
 * Function used to initialize master TWI interface that would communicate with simulated EEPROM.
 *
 * @return NRF_SUCCESS or the reason of failure
 */
static ret_code_t I2C_twi_master_init(uint8_t sclPin, uint8_t sdaPin)
{
    ret_code_t ret;
    const nrf_drv_twi_config_t config =
    {
       .scl                = sclPin,
       .sda                = sdaPin,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };

    do
    {
        ret = nrf_drv_twi_init(&m_twi_master, &config, NULL, NULL);
        if(NRF_SUCCESS != ret)
        {
            break;
        }
        nrf_drv_twi_enable(&m_twi_master);
    }while(0);
    return ret;
}

void I2C_setup(uint8_t sclPin, uint8_t sdaPin)
{
	I2C_twi_master_init(sclPin, sdaPin);
}

void I2C_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         I2C");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("wc <device> <reg> <value>\tWrites char at <reg> for device at <device>");
	CommandLineInterface_printLine("rc <device> <reg>\t\tReturns char at <reg> from device at <device>");
	CommandLineInterface_printLine("ww <device> <reg> <value>\tWrites word at <reg> for device at <device>");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("rw <device> <reg>\t\tReturns word at <reg> from device at <device>");
	CommandLineInterface_printLine("wl <device> <reg> <value>\tWrites long at <reg> for device at <device>");
	CommandLineInterface_printLine("rl <device> <reg>\t\tReturns long at <reg> from device at <device>");
}

bool I2C_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	unsigned int deviceAddress = 0;
	unsigned int registerAddress = 0;
	unsigned int registerValue = 0;
	unsigned int registerValueH = 0;
	unsigned int registerValueL = 0;
	bool parsed = true;

	if(sscanf(RxBuff, "WL 0X%2x 0X%2x 0X%4x %4x\r\n", &deviceAddress, &registerAddress, &registerValueH, &registerValueL)==4)
	{
		CommandLineInterface_printf("Writing to device 0x%02X register 0x%02X long value 0x%04X %04X\r\n", deviceAddress, registerAddress, registerValueH, registerValueL);

		unsigned long registerValueLong = (registerValueH << 16) | registerValueL;
		writeReg32(deviceAddress, registerAddress, registerValueLong);
	}
	else if(sscanf(RxBuff, "WW 0X%2x 0X%2x 0X%4x\r\n", &deviceAddress, &registerAddress, &registerValue)==3)
	{
		CommandLineInterface_printf("Writing to device 0x%02X register 0x%02X short value 0x%04X\r\n", deviceAddress, registerAddress, registerValue);
		writeReg16(deviceAddress, registerAddress, registerValue);
	}
	else if(sscanf(RxBuff, "WC 0X%2x 0X%2x 0X%2x\r\n", &deviceAddress, &registerAddress, &registerValue)==3)
	{
		CommandLineInterface_printf("Writing to device 0x%02X register 0x%02X char value 0x%02X\r\n", deviceAddress, registerAddress, registerValue);
		writeReg8(deviceAddress, registerAddress, registerValue);
	}
	else if(sscanf(RxBuff, "RC 0X%2x 0X%2x\r\n", &deviceAddress, &registerAddress)==2)
	{
		registerValue = readReg8(deviceAddress, registerAddress);
		CommandLineInterface_printf("Read char 0x%02X from device 0x%02X register 0x%02X\r\n", registerValue, deviceAddress, registerAddress);
	}
	else if(sscanf(RxBuff, "RW 0X%2x 0X%2x\r\n", &deviceAddress, &registerAddress)==2)
	{
		registerValue = readReg16(deviceAddress, registerAddress);
		CommandLineInterface_printf("Read word 0x%04X from device 0x%02X register 0x%02X\r\n", registerValue, deviceAddress, registerAddress);
	}
	else if(sscanf(RxBuff, "RL 0X%2x 0X%2x\r\n", &deviceAddress, &registerAddress)==2)
	{
		unsigned long registerValueLong = readReg32(deviceAddress, registerAddress);
		registerValueH = (registerValueLong >> 16) & 0xFFFF;
		registerValueL = registerValueLong & 0xFFFF;
		CommandLineInterface_printf("Read long 0x%04X %04X from device 0x%02X register 0x%02X\r\n", registerValueH, registerValueL, deviceAddress, registerAddress);
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

inline void writeReg8(uint8_t address, uint8_t regaddress, uint8_t regdata)
{
	ret_code_t err_code;
	uint8_t data[2] = {regaddress,regdata};

	err_code = I2C_twi_tx(address, data, 2, false);
	APP_ERROR_CHECK(err_code);
}

inline void writeReg16(uint8_t address, uint8_t regaddress, uint16_t regdata)
{
	ret_code_t err_code;
	uint8_t byteH = ((regdata >> 8) & 0xFF);
	uint8_t byteL = (regdata & 0xFF);
	uint8_t data[3] = {regaddress,byteL,byteH};

	err_code = I2C_twi_tx(address, data, 3, false);
	APP_ERROR_CHECK(err_code);
}

inline void writeReg32(uint8_t address, uint8_t regaddress, uint32_t regdata)
{
	ret_code_t err_code;
	uint8_t WHbyteH = ((regdata >> 24) & 0xFF);
	uint8_t WHbyteL = ((regdata >> 16) & 0xFF);
	uint8_t WLbyteH = ((regdata >> 8) & 0xFF);
	uint8_t WLbyteL = (regdata & 0xFF);
	uint8_t data[5] = {regaddress,WLbyteL,WLbyteH,WHbyteL,WHbyteH};

	err_code = I2C_twi_tx(address, data, 5, false);
	APP_ERROR_CHECK(err_code);
}

inline uint8_t readReg8(uint8_t address, uint8_t regaddress)
{
	ret_code_t err_code;
	uint8_t data = regaddress;
	uint8_t regdata;

	err_code = I2C_twi_tx(address, &data, 1, true);
	APP_ERROR_CHECK(err_code);

	err_code = I2C_twi_rx(address, &regdata, 1);
	APP_ERROR_CHECK(err_code);

	return regdata;
}

inline uint16_t readReg16(uint8_t address, uint8_t regaddress)
{
	ret_code_t err_code;
	uint8_t data = regaddress;
	uint8_t regdata[2];
	uint16_t regvalue;

	err_code = I2C_twi_tx(address, &data, 1, true);
	APP_ERROR_CHECK(err_code);

	err_code = I2C_twi_rx(address, regdata, 2);
	APP_ERROR_CHECK(err_code);

	regvalue = (regdata[1] << 8) | regdata[0];

	return regvalue;
}

inline int16_t readSignedReg16(uint8_t address, uint8_t regaddress)
{
	ret_code_t err_code;
	uint8_t data = regaddress;
	uint8_t regdata[2];
	uint16_t regvalue;

	err_code = I2C_twi_tx(address, &data, 1, true);
	APP_ERROR_CHECK(err_code);

	err_code = I2C_twi_rx(address, regdata, 2);
	APP_ERROR_CHECK(err_code);

	regvalue = (int16_t)((uint16_t)regdata[1] << 8) | (uint16_t)regdata[0];

	return regvalue;
}

inline uint32_t readReg32(uint8_t address, uint8_t regaddress)
{
	ret_code_t err_code;
	uint8_t data = regaddress;
	uint8_t regdata[4];
	uint32_t regvalue;

	err_code = I2C_twi_tx(address, &data, 1, true);
	APP_ERROR_CHECK(err_code);

	err_code = I2C_twi_rx(address, regdata, 4);
	APP_ERROR_CHECK(err_code);

	regvalue = (regdata[3] << 24) | (regdata[2] << 16) | (regdata[1] << 8) | regdata[0];

	return regvalue;
}

ret_code_t I2C_twi_tx(uint8_t address, uint8_t const * pdata, uint8_t length, bool no_stop)
{
	return nrf_drv_twi_tx(&m_twi_master, address, pdata, length, no_stop);
}

ret_code_t I2C_twi_rx(uint8_t address, uint8_t * pdata, uint8_t length)
{
	return nrf_drv_twi_rx(&m_twi_master, address, pdata, length);
}

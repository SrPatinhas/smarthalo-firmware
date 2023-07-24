/*
 * I2C.h
 *
 *  Created on: Jun 3, 2016
 *      Author: sgelinas
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "nrf.h"
#include "bsp.h"
#include "nrf_drv_twi.h"
#include "app_error.h"

void I2C_setup(uint8_t sclPin, uint8_t sdaPin);

void I2C_printHelp();

bool I2C_parseAndExecuteCommand(char * RxBuff, int cnt);

void writeReg8(uint8_t address, uint8_t regaddress, uint8_t regdata);

void writeReg16(uint8_t address, uint8_t regaddress, uint16_t regdata);

void writeReg32(uint8_t address, uint8_t regaddress, uint32_t regdata);

uint8_t readReg8(uint8_t address, uint8_t regaddress);

uint16_t readReg16(uint8_t address, uint8_t regaddress);

uint32_t readReg32(uint8_t address, uint8_t regaddress);



// Should void using these functions as much as possible (only if custom write/read block is needed)
ret_code_t I2C_twi_tx(uint8_t address, uint8_t const * pdata, uint8_t length, bool no_stop);

ret_code_t I2C_twi_rx(uint8_t address, uint8_t * pdata, uint8_t length);

#endif /* SRC_I2C_H_ */

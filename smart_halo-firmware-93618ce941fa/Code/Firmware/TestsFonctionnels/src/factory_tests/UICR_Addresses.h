/*
 * UICR_Addresses.h
 *
 *  Created on: May 25, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SH_UICR_ADDRESSES_H_
#define SH_FIRMWARE_CODE_SH_UICR_ADDRESSES_H_
//#include "dfu_init.h"

// addresses from 0x10001080 to 0x1000108C are used for versions of SD, bootloader , app

typedef enum FLASH_address FLASH_address;
enum FLASH_address {APP_VALID_ADDRESS=0x7F000};


#define UICR_BASE_ADDRESS					0x10001080

#define UICR_BOOTLOADER_VERSION_ADDRESS		(UICR_BASE_ADDRESS+0x30)
#define UICR_SOFTDEVICE_ADDRESS				(UICR_BASE_ADDRESS+0x34)
#define UICR_APPLICATION_VERSION_ADDRESS	(UICR_BASE_ADDRESS+0x38)

//offset from 0x10001080 to allow for init packet checking
#define UICR_FACTORY_MODE_BOOL_ADDRESS		(UICR_BASE_ADDRESS+0x10)
#define UICR_SHIPPING_MODE_BOOL_ADDRESS		(UICR_BASE_ADDRESS+0x14)
#define UICR_ONE_HOUR_PAUSE_BOOL_ADDRESS	(UICR_BASE_ADDRESS+0x18)
#define UICR_PRODUCT_SERIAL_NUMBER_ADDRESS	(UICR_BASE_ADDRESS+0x1C)
#define UICR_LOCK_SERIAL_NUMBER_ADDRESS		(UICR_BASE_ADDRESS+0x20)
#define UICR_KEY_SERIAL_NUMBER_ADDRESS		(UICR_BASE_ADDRESS+0x24)
#define UICR_PCBA_SERIAL_NUMBER_ADDRESS		(UICR_BASE_ADDRESS+0x28)


uint32_t read_to_UICR(uint32_t UICR_address);

uint8_t write_to_UICR(uint32_t UICR_address, uint32_t value_to_write);


//writes to flash, use with extreme caution!
uint8_t write_to_FLASH(FLASH_address flash_address, uint32_t value_to_write);


#endif /* SH_FIRMWARE_CODE_SH_UICR_ADDRESSES_H_ */



#ifndef SH_TWI_H__
#define SH_TWI_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "sdk_errors.h"

typedef struct
{
	size_t         		driver_address;          	/**< Address of the driver to write/read into                    */
    size_t        		register_address;   		/**< Address of the register we want to write/read into */
    uint8_t * 			pdata;        				/**< What to write              */
    uint8_t				size;       				/**< Size of the data to write             */
} SH_twi_command_t;

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

//Disables the TWI instance
void SH_twi_disable(void);

//Enables the TWI instance
void SH_twi_enable(void);

// Initialization of the TWI (also enables the twi)
ret_code_t SH_twi_initialisation(void);

/**
 * Write data to Drivers
 *
 * Function uses TWI interface to write data into Drivers.
 *
 * @param     driver_address  		Address of the driver
 * @param     address  				Start address to write
 * @param[in] pdata 				Pointer to data to send
 * @param     size  				Byte count of data to send
 * @param     end_of_transmission 	Boolean indicates if its the last data to write
 *
 *
 */
ret_code_t SH_twi_write(size_t driver_address, size_t address, uint8_t const * pdata, uint8_t size, bool end_of_transmission);
ret_code_t SH_twi_write_three_registers(size_t driver_address, size_t address, uint8_t const * pdata, uint8_t size, bool end_of_transmission);
/**
 * Write data to Drivers
 * To use after the SH_twi_write that specifies which register to write data in
 *
 * Function uses TWI interface to write data into Drivers.
 *
 * @param     driver_address  		Address of the driver
 * @param[in] pdata 				Pointer to data to send
 * @param     size  				Byte count of data to send
 * @param     end_of_transmission 	Boolean indicates if its the last data to write
 *
 *
 *

ret_code_t SH_twi_write_again(size_t driver_address, uint8_t const * pdata, size_t size, bool end_of_transmission);
*/

/**
 * Read data from Drivers
 *
 * Function uses TWI interface to read data from Drivers.
 *
 * @param     driver_address  		Address of the driver
 * @param     address  				Start address to read
 * @param[in] pdata 				Pointer to buffer to put data in
 * @param     size  				Byte count of data to read
 *
 *
 */
ret_code_t SH_twi_read(size_t driver_address, size_t address, uint8_t * pdata, size_t size);

ret_code_t SH_twi_read_two_registers(size_t driver_address, size_t address, uint8_t * pdata, size_t size);

void SH_twi_disable(void);


void set_bit_in_register(size_t driver_address, size_t register_address, uint bit);
void clear_bit_in_register(size_t driver_address, size_t register_address, uint bit);
void modify_x_bit_in_register(size_t driver_address, size_t register_address, uint8_t modified_bit);
void set_register(size_t driver_address, size_t register_address, uint8_t new_data);
void set_three_registers(size_t driver_address, size_t register_address, uint8_t * new_data);
bool check_bit_in_register(size_t driver_address, size_t register_address, uint8_t bit);
uint8_t check_register(size_t driver_address, size_t register_address);

#endif /* SH_TWI_H_ */




#include "stdafx.h"
#include "SH_TWI.h"
#include "nrf_drv_twi.h"
#include "SH_HaloPixelnRF.h"
#include "app_util_platform.h"
#include "app_twi.h"
#include "SH_Accelerometer_Magnetometer.h"
#ifdef SMARTHALO_EE
#include "SH_Batmon.h"
#define ADDRESS_OF_BATTERY_SOC BATMON_ADDRESS
#else
#define ADDRESS_OF_BATTERY_SOC 0x00
#endif
#include "SH_queue_twi.h"
#include "SH_pinMap.h"

#define TWI_ERROR 0
#define TWI_SUCCESS 1

#define NUMBER_OF_TRIES_TWI_TRANSACTION 5


static bool write_is_done = true;
static bool read_is_done = true;
static bool write_error_with_data_nack = false;
static bool write_error_with_address_nack = false;
static bool read_error_with_data_nack = false;
static bool read_error_with_address_nack = false;
static bool twi_write_succeed = false;
static bool twi_read_succeed = false;

#define MAX_PENDING_TRANSACTIONS    50

#define READ(device_address, p_reg_addr, p_buffer, byte_cnt) \
    APP_TWI_WRITE(device_address, p_reg_addr, 1, APP_TWI_NO_STOP), \
    APP_TWI_READ (device_address, p_buffer, byte_cnt, 0)

#define WRITE(device_address, p_reg_addr_data, byte_cnt) \
    APP_TWI_WRITE(device_address, p_reg_addr_data, byte_cnt, 0)

static app_twi_t m_app_twi = APP_TWI_INSTANCE(0);

/**
 * @brief TWI master instance
 *
 * Instance of TWI master driver that would be used for communication with simulated
 * eeprom memory.
 */
//static const nrf_drv_twi_t SH_m_twi_master = NRF_DRV_TWI_INSTANCE(MASTER_TWI_INST);

//static void event_handler_for_sh_twi(nrf_drv_twi_evt_t const * p_event, void *p_context);
static void read_registers_handler (ret_code_t result, void * p_user_data);

static void write_registers_handler (ret_code_t result, void * p_user_data);

//Present configuration for the twi drivers
static nrf_drv_twi_config_t const SH_config_TWI = {
   .scl                = I2C_SCL_PIN,
   .sda                = I2C_SDA_PIN,
   .frequency          = NRF_TWI_FREQ_400K,
   .interrupt_priority = APP_IRQ_PRIORITY_LOWEST
};

/*void SH_twi_disable(void){
	nrf_drv_twi_disable(&SH_m_twi_master);
}

void SH_twi_enable(void){
	nrf_drv_twi_enable(&SH_m_twi_master);
}*/

ret_code_t SH_twi_initialisation(void)
{
	ret_code_t err_code;
    do
    {
        //ret = nrf_drv_twi_init(&SH_m_twi_master, &SH_config_TWI, NULL);
       //ret = nrf_drv_twi_init (&SH_m_twi_master, &SH_config_TWI, event_handler_for_sh_twi, NULL);
    	APP_TWI_INIT(&m_app_twi, &SH_config_TWI,  MAX_PENDING_TRANSACTIONS, err_code);


        //SH_twi_enable();

    }while(0);

    return err_code;
}

void SH_twi_disable(void)
{
	app_twi_uninit(&m_app_twi);
}

ret_code_t SH_twi_write(size_t driver_address, size_t address, uint8_t const * pdata, uint8_t size, bool end_of_transmission)
{

	//Returns error code if any
	    ret_code_t ret=0;
	    uint8_t addr8 = (uint8_t)address;
	    //uint8_t  driver_addr8 = (uint8_t)driver_address;
	    static uint8_t buffer[2]; //Addr + data
	    uint8_t number_of_try_to_write_twi = 0;


	    static app_twi_transfer_t const transfers_write_iss1[] =
	    {
	    		WRITE(ISSI1_ADDR, buffer, 2)
	    };
	    static app_twi_transfer_t const transfers_write_iss2[] =
	    {

	    		WRITE(ISSI2_ADDR, buffer, 2)
	    };
	    static app_twi_transfer_t const transfers_write_accelerometer[] =
	    {

	    		WRITE(ACCELEROMETER_DRIVER_ADDRESS, buffer, 2)
	    };
	    static app_twi_transfer_t const transfers_write_magnetometer[] =
	    {

	    		WRITE(MAGNETOMETER_DRIVER_ADDRESS, buffer, 2)
	    };
	    static app_twi_transfer_t const transfers_write_gas_gauge[] =
	    {

	    		WRITE(ADDRESS_OF_BATTERY_SOC, buffer, 2)
	    };

	    buffer[0] = (uint8_t)addr8;
	    memcpy(buffer+1, pdata, size);

	    static app_twi_transaction_t const transaction_write_iss1 =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_iss1,
	        .number_of_transfers = sizeof(transfers_write_iss1)/sizeof(transfers_write_iss1[0])
	    };
	    static app_twi_transaction_t const transaction_write_iss2 =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_iss2,
	        .number_of_transfers = sizeof(transfers_write_iss2)/sizeof(transfers_write_iss2[0])
	    };
	    static app_twi_transaction_t const transaction_write_accelerometer =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_accelerometer,
	        .number_of_transfers = sizeof(transfers_write_accelerometer)/sizeof(transfers_write_accelerometer[0])
	    };
	    static app_twi_transaction_t const transaction_write_magnetometer =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_magnetometer,
	        .number_of_transfers = sizeof(transfers_write_magnetometer)/sizeof(transfers_write_magnetometer[0])
	    };
	    static app_twi_transaction_t const transaction_write_gas_gauge =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_gas_gauge,
	        .number_of_transfers = sizeof(transfers_write_gas_gauge)/sizeof(transfers_write_gas_gauge[0])
	    };

	    write_is_done = false;

	    switch (driver_address){
	    case ISSI1_ADDR :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss1);
	    	break;
	    case ISSI2_ADDR :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss2);
	    	break;
	    case ACCELEROMETER_DRIVER_ADDRESS :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_accelerometer);
	    	break;
	    case MAGNETOMETER_DRIVER_ADDRESS :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_magnetometer);
	    	break;
	    case ADDRESS_OF_BATTERY_SOC :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_gas_gauge);
	    	break;
		default :
			write_is_done = true;
			break;
	    }

	    while (!write_is_done){

	    }

	    while ((
	    		write_error_with_data_nack | write_error_with_address_nack) & (number_of_try_to_write_twi <= NUMBER_OF_TRIES_TWI_TRANSACTION)){

	    	number_of_try_to_write_twi ++;

		    write_is_done = false;

		    switch (driver_address){
		    case ISSI1_ADDR :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss1);
		    	break;
		    case ISSI2_ADDR :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss2);
		    	break;
		    case ACCELEROMETER_DRIVER_ADDRESS :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_accelerometer);
		    	break;
		    case MAGNETOMETER_DRIVER_ADDRESS :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_magnetometer);
		    	break;
		    case ADDRESS_OF_BATTERY_SOC :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_gas_gauge);
		    	break;
			default :
				write_is_done = true;
				break;
		    }

		    while (!write_is_done){

		    }
	    }

	    if (twi_write_succeed){
	    	return ret;
	    }
	    else{
	    	return ret;
	    }

}

ret_code_t SH_twi_write_three_registers(size_t driver_address, size_t address, uint8_t const * pdata, uint8_t size, bool end_of_transmission)
{

	//Returns error code if any
	    ret_code_t ret=0;
	    uint8_t addr8 = (uint8_t)address;
	    //uint8_t  driver_addr8 = (uint8_t)driver_address;
	    static uint8_t buffer[4]; //Addr + data
	    uint8_t number_of_try_to_write_twi = 0;


	    static app_twi_transfer_t const transfers_write_iss1[] =
	    {
	    		WRITE(ISSI1_ADDR, buffer, 4)
	    };
	    static app_twi_transfer_t const transfers_write_iss2[] =
	    {

	    		WRITE(ISSI2_ADDR, buffer, 4)
	    };
	    static app_twi_transfer_t const transfers_write_accelerometer[] =
	    {

	    		WRITE(ACCELEROMETER_DRIVER_ADDRESS, buffer, 4)
	    };
	    static app_twi_transfer_t const transfers_write_magnetometer[] =
	    {

	    		WRITE(MAGNETOMETER_DRIVER_ADDRESS, buffer, 4)
	    };
	    static app_twi_transfer_t const transfers_write_gas_gauge[] =
	    {

	    		WRITE(ADDRESS_OF_BATTERY_SOC, buffer, 4)
	    };

	    buffer[0] = (uint8_t)addr8;
	    memcpy(buffer+1, pdata, size);

	    static app_twi_transaction_t const transaction_write_iss1 =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_iss1,
	        .number_of_transfers = sizeof(transfers_write_iss1)/sizeof(transfers_write_iss1[0])
	    };
	    static app_twi_transaction_t const transaction_write_iss2 =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_iss2,
	        .number_of_transfers = sizeof(transfers_write_iss2)/sizeof(transfers_write_iss2[0])
	    };
	    static app_twi_transaction_t const transaction_write_accelerometer =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_accelerometer,
	        .number_of_transfers = sizeof(transfers_write_accelerometer)/sizeof(transfers_write_accelerometer[0])
	    };
	    static app_twi_transaction_t const transaction_write_magnetometer =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_magnetometer,
	        .number_of_transfers = sizeof(transfers_write_magnetometer)/sizeof(transfers_write_magnetometer[0])
	    };
	    static app_twi_transaction_t const transaction_write_gas_gauge =
	    {
	        .callback            = write_registers_handler,
	        .p_user_data         = NULL,
	        .p_transfers         = transfers_write_gas_gauge,
	        .number_of_transfers = sizeof(transfers_write_gas_gauge)/sizeof(transfers_write_gas_gauge[0])
	    };

	    write_is_done = false;

	    switch (driver_address){
	    case ISSI1_ADDR :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss1);
	    	break;
	    case ISSI2_ADDR :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss2);
	    	break;
	    case ACCELEROMETER_DRIVER_ADDRESS :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_accelerometer);
	    	break;
	    case MAGNETOMETER_DRIVER_ADDRESS :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_magnetometer);
	    	break;
	    case ADDRESS_OF_BATTERY_SOC :
	    	ret = app_twi_schedule(&m_app_twi, &transaction_write_gas_gauge);
	    	break;
		default :
			write_is_done = true;
			break;
	    }

	    while (!write_is_done){

	    }

	    while ((
	    		write_error_with_data_nack | write_error_with_address_nack) & (number_of_try_to_write_twi <= NUMBER_OF_TRIES_TWI_TRANSACTION)){

	    	number_of_try_to_write_twi ++;

		    write_is_done = false;

		    switch (driver_address){
		    case ISSI1_ADDR :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss1);
		    	break;
		    case ISSI2_ADDR :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_iss2);
		    	break;
		    case ACCELEROMETER_DRIVER_ADDRESS :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_accelerometer);
		    	break;
		    case MAGNETOMETER_DRIVER_ADDRESS :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_magnetometer);
		    	break;
		    case ADDRESS_OF_BATTERY_SOC :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_write_gas_gauge);
		    	break;
			default :
				write_is_done = true;
				break;
		    }

		    while (!write_is_done){

		    }
	    }

	    if (twi_write_succeed){
	    	return ret;
	    }
	    else{
	    	return ret;
	    }

}

ret_code_t SH_twi_read_two_registers(size_t driver_address, size_t address, uint8_t * pdata, size_t size){

	//Returns error code if any
	ret_code_t ret=0;
    static uint8_t addr8;
    uint8_t const driver_addr8 = (uint8_t)driver_address;
    static uint8_t buffer [2];

		static app_twi_transfer_t const transfers_read_accelerometer[] =
		{

				READ(ACCELEROMETER_DRIVER_ADDRESS, &addr8, buffer, 2)
		};
		static app_twi_transfer_t const transfers_read_magnetometer[] =
		{

				READ(MAGNETOMETER_DRIVER_ADDRESS, &addr8, buffer, 2)
		};
		static app_twi_transfer_t const transfers_read_gas_gauge[] =
		{

				READ(ADDRESS_OF_BATTERY_SOC, &addr8, buffer, 2)
		};

		//transfers_read_gas_gauge.length = read_command.size;
		addr8 = (uint8_t)driver_address;

		static app_twi_transaction_t const transaction_read_accelerometer =
		{
			.callback            = read_registers_handler,
			.p_user_data         = NULL,
			.p_transfers         = transfers_read_accelerometer,
			.number_of_transfers = sizeof(transfers_read_accelerometer)/sizeof(transfers_read_accelerometer[0])
		};
		static app_twi_transaction_t const transaction_read_magnetometer =
		{
			.callback            = read_registers_handler,
			.p_user_data         = NULL,
			.p_transfers         = transfers_read_magnetometer,
			.number_of_transfers = sizeof(transfers_read_magnetometer)/sizeof(transfers_read_magnetometer[0])
		};
		static app_twi_transaction_t const transaction_read_gas_gauge =
		{
			.callback            = read_registers_handler,
			.p_user_data         = NULL,
			.p_transfers         = transfers_read_gas_gauge,
			.number_of_transfers = sizeof(transfers_read_gas_gauge)/sizeof(transfers_read_gas_gauge[0])
		};

		read_is_done = false;

		switch (driver_addr8){
		case ACCELEROMETER_DRIVER_ADDRESS :
			ret = app_twi_schedule(&m_app_twi, &transaction_read_accelerometer);
			break;
		case MAGNETOMETER_DRIVER_ADDRESS :
			ret = app_twi_schedule(&m_app_twi, &transaction_read_magnetometer);
			break;
		case ADDRESS_OF_BATTERY_SOC :
			ret = app_twi_schedule(&m_app_twi, &transaction_read_gas_gauge);
			break;
		default :
			read_is_done = true;
			break;
		}


		while (!read_is_done){

		}

		memcpy(pdata, buffer, size);

    return ret;
}

ret_code_t SH_twi_read(size_t driver_address, size_t address, uint8_t * pdata, size_t size){

	//Returns error code if any
	ret_code_t ret=0;
    static uint8_t addr8 = 0;
    uint8_t const driver_addr8 = (uint8_t)driver_address;
    static uint8_t buffer;
    //static uint8_t p_buffer[1];

		static app_twi_transfer_t transfers_read_accelerometer[] =
		{

				READ(ACCELEROMETER_DRIVER_ADDRESS, &addr8, &buffer, 1)
		};
		static app_twi_transfer_t transfers_read_magnetometer[] =
		{

				READ(MAGNETOMETER_DRIVER_ADDRESS, &addr8, &buffer, 1)
		};
		static app_twi_transfer_t transfers_read_gas_gauge[] =
		{

				READ(ADDRESS_OF_BATTERY_SOC, &addr8, &buffer, 1)
		};

		//transfers_read_gas_gauge.length = read_command.size;
		addr8 = (uint8_t)address;
		transfers_read_magnetometer[1].length = size;
		transfers_read_accelerometer[1].length = size;
		transfers_read_gas_gauge[1].length = size;
		uint8_t number_of_try_to_read_twi = 0;
		//buffer = pdata;

		static app_twi_transaction_t const transaction_read_accelerometer =
		{
			.callback            = read_registers_handler,
			.p_user_data         = NULL,
			.p_transfers         = transfers_read_accelerometer,
			.number_of_transfers = sizeof(transfers_read_accelerometer)/sizeof(transfers_read_accelerometer[0])
		};
		static app_twi_transaction_t const transaction_read_magnetometer =
		{
			.callback            = read_registers_handler,
			.p_user_data         = NULL,
			.p_transfers         = transfers_read_magnetometer,
			.number_of_transfers = sizeof(transfers_read_magnetometer)/sizeof(transfers_read_magnetometer[0])
		};
		static app_twi_transaction_t const transaction_read_gas_gauge =
		{
			.callback            = read_registers_handler,
			.p_user_data         = NULL,
			.p_transfers         = transfers_read_gas_gauge,
			.number_of_transfers = sizeof(transfers_read_gas_gauge)/sizeof(transfers_read_gas_gauge[0])
		};

		read_is_done = false;

		switch (driver_addr8){
		case ACCELEROMETER_DRIVER_ADDRESS :
			ret = app_twi_schedule(&m_app_twi, &transaction_read_accelerometer);
			break;
		case MAGNETOMETER_DRIVER_ADDRESS :
			ret = app_twi_schedule(&m_app_twi, &transaction_read_magnetometer);
			break;
		case ADDRESS_OF_BATTERY_SOC :
			ret = app_twi_schedule(&m_app_twi, &transaction_read_gas_gauge);
			break;
		default :
			read_is_done = true;
			break;
		}

		while (!read_is_done){
		}

	    while ((read_error_with_address_nack) & (number_of_try_to_read_twi <= NUMBER_OF_TRIES_TWI_TRANSACTION)){

	    	number_of_try_to_read_twi ++;

		    read_is_done = false;

		    switch (driver_address){

		    case ACCELEROMETER_DRIVER_ADDRESS :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_read_accelerometer);
		    	break;
		    case MAGNETOMETER_DRIVER_ADDRESS :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_read_magnetometer);
		    	break;
		    case ADDRESS_OF_BATTERY_SOC :
		    	ret = app_twi_schedule(&m_app_twi, &transaction_read_gas_gauge);
		    	break;
			default :
				read_is_done = true;
				break;
		    }

		    while (!read_is_done){

		    }
	    }

	    if (twi_read_succeed){
	    	*pdata = buffer;
	    	return ret;
	    }
	    else{

	    }

    return ret;
}

static void read_registers_handler (ret_code_t result, void * p_user_data)
{
	read_is_done = true;
	twi_read_succeed = false;
	read_error_with_address_nack = false;
	read_error_with_data_nack = true;

	if (result == NRF_SUCCESS){
		twi_read_succeed = true;
	}

	else if (result == SH_TWI_ERROR_WITH_DATA_NACK){
		read_error_with_data_nack = true;
		twi_read_succeed = true;
	}

	else if (result == SH_TWI_ERROR_WITH_ADDRESS_NACK){
		read_error_with_address_nack = true;
	}

}

static void write_registers_handler(ret_code_t result, void * p_user_data)
{
	write_is_done = true;
	twi_write_succeed = false;
	write_error_with_data_nack = false;
	write_error_with_address_nack = false;

	if (result == NRF_SUCCESS){
		twi_write_succeed = true;
	}

	else if (result == SH_TWI_ERROR_WITH_DATA_NACK){
		write_error_with_data_nack = true;

	}

	else if (result == SH_TWI_ERROR_WITH_ADDRESS_NACK){
		write_error_with_address_nack = true;
	}

}


//CHECK_BIT_IN_REGISTER
//
bool check_bit_in_register(size_t driver_address, size_t register_address, uint8_t bit){

	bool bit_value;
	uint8_t data;
	SH_twi_read(driver_address, register_address, &data, 1);
	bit_value = CHECK_BIT(data, bit);
	return bit_value;
}

//CHECK_REGISTER
//
uint8_t check_register(size_t driver_address, size_t register_address){

	uint8_t data;
	SH_twi_read(driver_address, register_address, &data, 1);
	return data;
}

//SET_BIT_IN_REGISTER
//
void set_bit_in_register(size_t driver_address, size_t register_address, uint bit){

	uint8_t data;
	SH_twi_read(driver_address, register_address, &data, 1);
	data |= 1 << bit;
	SH_twi_write(driver_address, register_address, &data, 1, true);
}

//SET_REGISTER
//
void set_register(size_t driver_address, size_t register_address, uint8_t new_data){

	uint8_t data = new_data;
	SH_twi_write(driver_address, register_address, &data, 1, true);
}

//SET_THREE_REGISTER
//
void set_three_registers(size_t driver_address, size_t register_address, uint8_t * new_data){

	uint8_t * data = new_data;
	SH_twi_write_three_registers(driver_address, register_address, data, 3, true);
}

//SET_X_BIT_IN_REGISTER
//
void modify_x_bit_in_register( size_t driver_address, size_t register_address, uint8_t modified_bit){

	uint8_t data;
	SH_twi_read(driver_address, register_address, &data, 1);
	data |= modified_bit;
	SH_twi_write(driver_address, register_address, &data, 1, true);
}

//CLEAR_BIT_IN_REGISTER
//
void clear_bit_in_register(size_t driver_address, size_t register_address, uint bit){

	uint8_t data;
	SH_twi_read(driver_address, register_address, &data, 1);
	data &= ~(1 << bit);
	SH_twi_write(driver_address, register_address, &data, 1, true);
}

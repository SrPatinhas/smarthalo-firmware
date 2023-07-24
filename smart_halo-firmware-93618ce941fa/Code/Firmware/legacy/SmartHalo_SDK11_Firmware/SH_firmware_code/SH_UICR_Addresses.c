


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif // BLE_DFU_APP_SUPPORT
#include "ble_conn_params.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "bsp_btn_ble.h"
#include "config_twi.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "nrf_twi.h"
#include "nrf_drv_twi.h"
#include "SH_HaloPixelnRF.h"
#include "SH_BLE.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "crc.h"
#include "SHP_Frames_and_commands.h"
#include "SH_Priority_queue.h"
#include "SH_Primary_state_machine.h"
#include "SH_typedefs.h"
#include "SH_Animations.h"
#include "SH_Task_handler.h"
#include "SH_TWI.h"
#include "nrf_drv_wdt.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_uart.h"
#include "nrf_drv_config.h"
#include "fstorage.h"
#include "nrf52_bitfields.h"
#include "nrf52.h"
#include "pca10040.h"
#include "SH_PWM_Center_LED.h"
#include "SH_TapCode.h"
#include "SH_PWM_FrontLight.h"
#include "SH_ADC_Read.h"
#include "app_fifo.h"
#include "SH_Piezo_sounds.h"
#include "SH_Tap_Qtouch.h"
#include "SH_Mouvement_Detection.h"
#include "SH_Magnetic_field_detection.h"
#include "SH_Factory_tests.h"
#include "SH_UICR_Addresses.h"
#include "nrf_nvmc.h"
#include "SH_pinMap.h"
#include "nrf_drv_gpiote.h"
#include "SH_GAP_GATT_Parameters.h"



static void reenable_SD_after_UICR_write();

static void reenable_SD_after_UICR_write()
{
	uint8_t err_code =0;
	 ble_enable_params_t ble_enable_params;
	err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
													PERIPHERAL_LINK_COUNT,
													&ble_enable_params);
	APP_ERROR_CHECK(err_code);

#ifdef BLE_DFU_APP_SUPPORT
	ble_enable_params.gatts_enable_params.service_changed = 1;
#endif // BLE_DFU_APP_SUPPORT
	//Check the ram settings against the used number of links
	CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

	// Enable BLE stack.
	err_code = softdevice_enable(&ble_enable_params);
	APP_ERROR_CHECK(err_code);

	//return err_code;
}


//can only write serial numbers once, or else program must be reflashed!
uint8_t write_to_UICR(uint32_t UICR_address, uint32_t value_to_write)
{
	uint8_t err_code =0;
	bool softD_was_enabled = false;

	//if already a serial number present, return error
	if(read_to_UICR(UICR_address) != -1)
	{
		return err_code = 255;
	}

	if(softdevice_handler_isEnabled())
	{
		softdevice_handler_sd_disable(); // can only access uicr if SoftDevice disabled
		softD_was_enabled = true;
	}
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
	*(uint32_t *)UICR_address = value_to_write; //new serial number
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

	//printf("%s",answer_OK);
	if(softD_was_enabled)
	{
		reenable_SD_after_UICR_write();
	}
	return err_code;
}



//use this with caution!
uint8_t write_to_FLASH(FLASH_address flash_address, uint32_t value_to_write)
{
	uint8_t err_code =0;
	//bool softD_was_enabled = false;

	if(softdevice_handler_isEnabled())
	{
		softdevice_handler_sd_disable(); // can only access flash if SoftDevice disabled
		//softD_was_enabled = true;
	}
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
	*(uint32_t *)flash_address = value_to_write; //new serial number
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

	//printf("%s",answer_OK);
//	if(softD_was_enabled)
//	{
//		reenable_SD_after_UICR_write();
//	}
	return err_code;
}






uint32_t read_to_UICR(uint32_t UICR_address)
{
	uint32_t serial_number_check = 0;
	memcpy(&serial_number_check,(uint32_t*)UICR_address, 4);
	//itoa(serial_number_check,uart_answer,10); //10 for decimal
	//printf("%s",uart_answer);
	return serial_number_check;
}
//
////outputs decimal value representation string!
//char* string_from_uint32_t(uint32_t value_to_convert)
//{
//	char output_string[11] = {0}; //longest possible string is 10 values plus \0 termination
//	value_to_convert
//
//
//}

















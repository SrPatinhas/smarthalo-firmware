/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
/** @example examples/ble_peripheral/ble_app_hrs/main.c
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include "SH_Includes.h"
#include "SH_Batmon.h"
#define APP_TIMER_MAX_TIMERS             (15+BSP_APP_TIMERS_NUMBER)                  /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                          /**< Size of timer operation queues. */

//#ifdef DEBUG_UART_MESSAGES
#define START_STRING                    "Start...\n\r"                                /**< The string that will be sent over the UART when the application starts. */
//#endif

#ifdef SMARTHALO_EE
//used for the board setup
static bool charging = false;
static bool powergood = false;
static bool usbsleep = false;
#endif

nrf_drv_wdt_channel_id m_channel_id;


//it must be determined if a priority task list is important
SHP_command_buffer_t SHP_cmd_buf_HIGH_PRIORITY = {
		.rear = -1
};

SHP_command_buffer_t SHP_cmd_buf_LOW_PRIORITY = {
		.rear = -1
};



void wdt_event_handler(void)
{


    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
	//63 us
}


#ifdef SMARTHALO_EE
void board_setup()
{
	// Disable all supplies

	enable_led_drivers();

	disable_touch_supply();

	disable_piezo_supply();

	// Self power cycle should be turned off
	nrf_gpio_cfg_output(MCU_POWER_CYCLE);
	nrf_gpio_pin_write(MCU_POWER_CYCLE, 0);

	// Set ISET2 H to enable 500 mA charge current
	nrf_gpio_cfg_output(USB_CHARGE_ISET2_PIN);
	nrf_gpio_pin_write(USB_CHARGE_ISET2_PIN, 1);

	// Check USB power
	nrf_gpio_cfg_input(USB_CHARGING_N_PIN, NRF_GPIO_PIN_NOPULL);
	charging = !(nrf_gpio_pin_read(USB_CHARGING_N_PIN));

	nrf_gpio_cfg_input(USB_POWERGOOD_N_PIN, NRF_GPIO_PIN_NOPULL);
	powergood = !(nrf_gpio_pin_read(USB_POWERGOOD_N_PIN));

	// Check USB bridge
	nrf_gpio_cfg_input(USB_SLEEP_N_PIN, NRF_GPIO_PIN_PULLUP);
	usbsleep = !(nrf_gpio_pin_read(USB_SLEEP_N_PIN));

	nrf_gpio_cfg_output(TOUCH_MODE_PIN);
	nrf_gpio_pin_write(TOUCH_MODE_PIN,0); // Low power mode

	// Wakeup USB bridge if sleeping
	if( usbsleep )
	{
		nrf_gpio_cfg_output(USB_WAKEUP_N_PIN);
		nrf_gpio_pin_write(USB_WAKEUP_N_PIN, 0);
	}
}
#endif

void usb_wake_up_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

}

void factory_initialisations()
{
	uint32_t err_code = 0;

    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    app_trace_init();
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;

    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    uart_init();
    app_uart_flush();

#ifdef SMARTHALO_EE
    board_setup();
#endif

     NRF_WDT->RR[0] = WDT_RR_RR_Reload;
     Bluetooth_setup();

    //Configure WDT.
	err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
	APP_ERROR_CHECK(err_code);
	nrf_drv_wdt_enable();

    uint8_t  start_string[] = START_STRING;
    printf("%s",start_string);


    I2C_setup( I2C_SCL_PIN,I2C_SDA_PIN);
    LEDTest_setup( I2C_SDB_PIN,  FRONTLED_PIN,  CENTRAL_RED_PIN,  CENTRAL_GREEN_PIN,  CENTRAL_BLUE_PIN);

    TouchTest_setup(TOUCH_OUT_PIN, TOUCH_MODE_PIN);
    AcceleroTest_setup();
    MagnetoTest_setup();
}


/*
uint8_t wbuf[] = "Hello!";

void writecb(ret_code_t err) {
    printf("writecb %d\r\n", (int)err);

    uint8_t buf[10];
    uint32_t readLen;
    err = rec_read(1, buf, 10, &readLen);
    printf("rec_read %d \r\n", (int)err);
    if(!err) {
        for(int i = 0; i < readLen; i++) {
            printf("%02X ", buf[i]);
        }
        printf("\r\n");
    }
}

void main_afterRecInit() {
    printf("main_afterRecInit \r\n");
    //uint32_t err;

    //rec_write(1, wbuf, 6, writecb);
    writecb(0);

}
*/

/**
 * @brief WDT events handler.
 */
int main(void)
{
	uint32_t err_code = 0 ;
    bool erase_bonds = false;
	nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
	set_factory_mode_status(false);
#ifdef FACTORY_PROGRAMMED_FIRMWARE


	uint32_t factory_check = 0;
	memcpy(&factory_check,(uint32_t*)UICR_FACTORY_MODE_BOOL_ADDRESS, 4);
	if(factory_check)
	{
		factory_initialisations();
		set_factory_mode_status(true);
		factory_test_mode();
		set_factory_mode_status(false);
	}
	uint32_t shipping_check = 0;
	memcpy(&shipping_check,(uint32_t*)UICR_SHIPPING_MODE_BOOL_ADDRESS, 4);
	if(shipping_check)
	{
		//Configure WDT.
		err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
		APP_ERROR_CHECK(err_code);
		err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
		APP_ERROR_CHECK(err_code);
		nrf_drv_wdt_enable();

		shipping_mode();
	}
#endif // FACTORY_PROGRAMMED_FIRMWARE
	set_factory_mode_status(false);

	////STANDARD OPERATION MODE////
    // Initialize.
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;

    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    uart_init(); //responsible for 1mA
    app_uart_flush();

#ifdef SMARTHALO_EE
    board_setup();
#endif
    initialise_bluetooth_stack_and_services(erase_bonds);
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;

    //Configure WDT.
	err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
	APP_ERROR_CHECK(err_code);
	nrf_drv_wdt_enable();

	// Start execution of BLE
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    SH_twi_initialisation(); //responsible for 1 mA of current
    HaloPixel_begin();
    pwm_center_led_init(); //responsible for 2 mA of current!
    pwm_front_led_init();
    front_light_timer_init();

#ifdef DEBUG_UART_MESSAGES
    uint8_t  start_string[] = START_STRING;
    printf("%s",start_string);
#endif


#ifndef TEST_HALO_BOX
    SH_TapCode_init();
    piezo_init();
    SH_Tap_Qtouch_init();
    SH_initialization_of_accelerometer();
    SH_Magnetometer_initialization();
    SH_Batmon_init();
#endif


    rec_init(NULL);

    //main program thread execute
    primary_state_machine(get_SHP_msg_buffer_address(), &SHP_cmd_buf_LOW_PRIORITY, &SHP_cmd_buf_HIGH_PRIORITY, get_m_nus());

    sd_nvic_SystemReset(); //only here as a precaution
	////STANDARD OPERATION MODE////
}

















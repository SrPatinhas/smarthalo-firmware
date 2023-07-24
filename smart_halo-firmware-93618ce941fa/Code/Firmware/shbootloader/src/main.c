/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include <stdint.h>
//#include "boards.h"
#include "nrf_mbr.h"
#include "nrf_bootloader.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_dfu_settings.h"
#include "nrf_dfu_utils.h"
#include "nrf_dfu.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_error.h"
#include "app_error_weak.h"
#include "nrf_bootloader_info.h"
#include "nrf_dfu_transport.h"
#include "app_timer.h"
#include "app_timer_appsh.h"

#include "platform.h"

#include "scheduler.h"
#include "blleds.h"
#include "bootinfo.h"
#include "keys.h"

#ifdef PRINTF_RTT
#include "SEGGER_RTT.h"
#endif

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("received a fault! id: 0x%08x, pc: 0x&08x\r\n", id, pc);
    NVIC_SystemReset();
}

void app_error_handler_bare(uint32_t error_code)
{
    (void)error_code;
    NRF_LOG_ERROR("received an error: 0x%08x!\r\n", error_code);
    NVIC_SystemReset();
}

platform_hw_t platform_hw;

platform_hw_t platform_getHW() {
    return platform_hw;
}

#ifdef PRINTF_UART

#define UART_TX_BUF_SIZE 256
#define UART_RX_BUF_SIZE 256

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        //APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

#endif

#define MAXLUM 25.f

void main_led_fade(bool in) {
    int tick = (in) ? 60 : 5;
    float step = (1.f/(float)tick);
    for(int i = 0; i < tick; i++) {
        float lvl;
        if(in) {
            lvl = step*MAXLUM*(float)i;
        } else {
            lvl = step*MAXLUM*(float)(tick-i);
        }
        leds_direct_set(0,lvl,0);
        nrf_delay_ms(16);
    }
}

bool main_touchTest() {
    if(platform_hw == HW_V12) {
        nrf_gpio_cfg_output(V12_EN_QTOUCH);
        nrf_gpio_pin_write(V12_EN_QTOUCH, 1);
    } else {
        nrf_gpio_cfg_output(V10_EN_2_8V);
        nrf_gpio_pin_write(V10_EN_2_8V, 1);
    }
    nrf_gpio_cfg_output(TOUCH_MODE_PIN);
    nrf_gpio_pin_write(TOUCH_MODE_PIN, 0);
    NRF_GPIO->PIN_CNF[TOUCH_OUT_PIN] = 0;

    leds_anim_pwr_on();
    main_led_fade(true);

    uint32_t cnt = 0;
    while(nrf_gpio_pin_read(TOUCH_OUT_PIN) && cnt<10) {
        cnt++;
        nrf_delay_ms(500);
    }

    main_led_fade(false);
    leds_anim_pwr_off();

    return (cnt>=10);
}

void HardFault_Handler(void)
{
    uint32_t *sp = (uint32_t *) __get_MSP(); // Get stack pointer
    static volatile uint32_t ia = 0;
    ia = sp[24/4]; // Get instruction address from stack
    UNUSED_VARIABLE(ia);
    while(1);
}

int main(void)
{
    uint32_t err;

    //HW version
    NRF_GPIO->PIN_CNF[HW0_PIN] = (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);
    NRF_GPIO->PIN_CNF[HW1_PIN] = (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);
    NRF_GPIO->PIN_CNF[HW2_PIN] = (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);

    bool hw0 = (nrf_gpio_pin_read(HW0_PIN) == 1);
    bool hw1 = (nrf_gpio_pin_read(HW1_PIN) == 1);
    bool hw2 = (nrf_gpio_pin_read(HW2_PIN) == 1);

    if(!hw2 && !hw1 && hw0) {
        platform_hw = HW_V12;
    } else {
        platform_hw = HW_V11;
    }

#ifdef PRINTF_UART
    if(platform_hw == HW_V11) {
        const app_uart_comm_params_t comm_params =
        {
              V10_UART_RXD_PIN,
              V10_UART_TXD_PIN,
              0,
              0,
              APP_UART_FLOW_CONTROL_DISABLED,
              false,
              UART_BAUDRATE_BAUDRATE_Baud115200
        };

        APP_UART_FIFO_INIT( &comm_params,
                             UART_RX_BUF_SIZE,
                             UART_TX_BUF_SIZE,
                             uart_error_handle,
                             APP_IRQ_PRIORITY_LOW,
                             err);

        APP_ERROR_CHECK(err);
    }
#endif
#ifdef PRINTF_RTT
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif

    (void) NRF_LOG_INIT(NULL);

    //Needed by touch test
    leds_init();

    uint32_t binfo = binfo_getWord();
    bool force_bl = nrf_dfu_init();
    bool app_valid = nrf_dfu_app_is_valid();
    bool touch = false;

    printf("In Bootloader %08X %08X%s\r\n", BOOTLOADER_START_ADDR, binfo, (app_valid) ? ", app" : ", no app");
    nrf_delay_ms(100);

    if(!force_bl && app_valid) {

        //CAN'T Jump to app if sd has been initialized, must reset
        if((NRF_POWER->GPREGRET & 1)) {
            NRF_POWER->GPREGRET = 0;
            nrf_bootloader_app_start(MAIN_APPLICATION_START_ADDR);
        }

        if (BINFO_IS_ALARM_ON(binfo))
        {
            nrf_bootloader_app_start(MAIN_APPLICATION_START_ADDR);
        }

        touch = main_touchTest();
        if(!touch) {
            if (!BINFO_IS_STAY_BL(binfo))
            {
                nrf_bootloader_app_start(MAIN_APPLICATION_START_ADDR);
            }
        }
    }
    NRF_POWER->GPREGRET = 0;

    sch_init();
    keys_init();

    err = nrf_dfu_transport_init();
    APP_ERROR_CHECK(err);

    if (BINFO_IS_STAY_BL(binfo)) {
        binfo_init();
        binfo = BINFO_STAY_BL(binfo, false);
        binfo_setWord(binfo, NULL);
    }

    sch_unique_oneshot(nrf_dfu_jumptoApp, 30000);

    leds_bl(false, 0);

    sch_run(); //Scheduler loop, never returns

}

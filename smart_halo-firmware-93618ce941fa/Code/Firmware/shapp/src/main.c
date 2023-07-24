
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"

//#include "nrf_dfu_settings.h"

#include "scheduler.h"
#include "bleapp.h"
#include "bslink.h"
#include "dispatch.h"
#include "auth.h"
#include "ui.h"
#include "alarm.h"
#include "device.h"
#include "twi.h"
#include "record.h"
#include "battery.h"
#include "sound.h"
#include "power.h"
#include "bootinfo.h"
#include "keys.h"
#include "test.h"
#include "exp.h"
#include "animscript.h"

#ifdef LEGACYTEST
#include "Factory_tests.h"
#endif

#ifdef PRINTF_RTT
#include "SEGGER_RTT.h"
#endif

#if defined(PRINTF_UART)

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

#define DBGOUT 28 //12

void HardFault_Handler(void)
{
    uint32_t *sp = (uint32_t *) __get_MSP(); // Get stack pointer
    static volatile uint32_t ia = 0;
    ia = sp[24/4]; // Get instruction address from stack
    UNUSED_VARIABLE(ia);
    while(1);
}

void main_test(void *ctx) {
    static bool trig = false;
    trig = !trig;
    nrf_gpio_pin_write(EN_VLED, (trig) ? 1 : 0);
}

void main_task_test(void *ctx) {
/*    static int cnt = 300;
    printf("A\r\n");
    cnt--;
    if(cnt) {
        sch_unique_oneshot(main_task_test, 16);
    } else {
        printf("Done\r\n");
    }*/
    uint8_t buf[1];
    uint32_t ptr = 0;
    buf[ptr++] = NOTIFY_TEST;
    bslink_up_write(buf, ptr);
    sch_unique_oneshot(main_task_test, 2000);
}

void main_poweroff_test(void *ctx) {
    bleapp_shutdown();
    //pwr_shutdown();
}

void main_stall_test(void *ctx) {
    while (1) {}
}

void main_after_rec() {
    //printf("main_after_rec \r\n");

    bleapp_init();
    bslink_init();
    disp_init();
    auth_init();
    device_init();
///*
    twi_init();
    sound_init();
    ui_init();
    bat_init();
    alarm_init();
    tst_init();
    exp_init();
    ascr_init();
//*/
#ifndef UTILITY_TESTPIEZO
    bleapp_advertising_start();
#endif
    
    printf("App Started!\r\n");
}

platform_hw_t platform_hw;

platform_hw_t platform_getHW() {
    return platform_hw;
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

    NRF_GPIO->PIN_CNF[HW0_PIN] = (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos);
    NRF_GPIO->PIN_CNF[HW1_PIN] = (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos);
    NRF_GPIO->PIN_CNF[HW2_PIN] = (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos);

    if(!hw2 && !hw1 && hw0) {
        platform_hw = HW_V12;
    } else {
        platform_hw = HW_V11;
    }

#if defined(PRINTF_UART)
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

        ERR_CHECK("uart_init", err);
    }
#endif
#ifdef PRINTF_RTT
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif

    printf("Reset ( reason: %04X ) \r\n", NRF_POWER->RESETREAS);
    NRF_POWER->RESETREAS = 0xFFFFFFFF; //clear flags

    printf("HW: %s \r\n", (platform_hw == HW_V12) ? "v1.2" : "v1.1");

    pwr_init();

    binfo_init();
    keys_init();

    err = nrf_mem_init();
    ERR_CHECK("nrf_mem_init", err);

    sch_init();

    ble_stack_init();

    rec_init(main_after_rec);

    //nrf_gpio_cfg_output(EN_VLED);
    //sch_register(main_test, 1.f/5.f, NULL);
    //sch_unique_oneshot(main_task_test, 5000);
    //sch_unique_oneshot(main_stall_test, 5000);
    //sch_unique_oneshot(main_poweroff_test, 5000);

    sch_run(); //Scheduler loop, never returns

    return 0;
}


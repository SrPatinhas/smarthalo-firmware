
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
#include "device.h"
#include "record.h"
#include "bootinfo.h"
#include "keys.h"
#include "test.h"
#include "uart.h"

#include "nrf_sdm.h"

#ifdef LEGACYTEST
#include "Factory_tests.h"
#endif

#ifdef PRINTF_RTT
#include "SEGGER_RTT.h"
#endif

#define DBGOUT 28 //12

/*
 * Copy pasted from SDK, replaced the NRF_LOG stuff with printf
 * and removed the call to nrf_strerror_get()
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    __disable_irq();
    NRF_BREAKPOINT_COND;
    printf("Fatal error\n");
    switch (id)
    {
        case NRF_FAULT_ID_SD_ASSERT:
            printf("SOFTDEVICE: ASSERTION FAILED\n");
            NRF_BREAKPOINT_COND;
            break;
        case NRF_FAULT_ID_APP_MEMACC:
            printf("SOFTDEVICE: INVALID MEMORY ACCESS\n");
            NRF_BREAKPOINT_COND;
            break;
        case NRF_FAULT_ID_SDK_ASSERT: {
            assert_info_t *p_info = (assert_info_t *)info;
            printf("ASSERTION FAILED at %s:%u\n", p_info->p_file_name,
                   p_info->line_num);
            NRF_BREAKPOINT_COND;
            break;
        }
        case NRF_FAULT_ID_SDK_ERROR: {
            error_info_t *p_info = (error_info_t *)info;
            printf(
                // "ERROR %u [%s] at %s:%u\r\nPC at: 0x%08x\n",
                "ERROR %u [not decoded] at %s:%u\r\nPC at: 0x%08x\n",
                p_info->err_code,
                // nrf_strerror_get(p_info->err_code),
                p_info->p_file_name, p_info->line_num, pc);
            printf("End of error report\n");
            NRF_BREAKPOINT_COND;
            break;
        }
        default:
            printf("UNKNOWN FAULT at 0x%08X\n", pc);
            NRF_BREAKPOINT_COND;
            break;
    }
#if NRF_MODULE_ENABLED(NRF_LOG_BACKEND_RTT)
    // To allow the buffer to be flushed by the host.
    nrf_delay_ms(100);
#endif
#ifdef NRF_DFU_DEBUG_VERSION
    NRF_BREAKPOINT_COND;
#endif
    NVIC_SystemReset();
}

void HardFault_Handler(void)
{
    printf("Hard fault!\n");
    nrf_delay_ms(1000);
    uint32_t *sp = (uint32_t *) __get_MSP(); // Get stack pointer
    static volatile uint32_t ia = 0;
    ia = sp[24/4]; // Get instruction address from stack
    UNUSED_VARIABLE(ia);
#ifdef DEBUG
    while(1);
#else
    NVIC_SystemReset();
#endif
}

void main_test(void *ctx) {
    static bool trig = false;
    trig = !trig;
    nrf_gpio_pin_write(EN_VLED, (trig) ? 1 : 0);
}

void main_task_test(void *ctx) {
    uint8_t buf[1];
    uint32_t ptr = 0;
    buf[ptr++] = NOTIFY_TEST;
    bslink_up_write(buf, ptr);
    sch_unique_oneshot(main_task_test, 2000);
}

void main_stall_test(void *ctx) {
    while (1) {}
}

void main_after_rec(ret_code_t err) {
    printf("main_after_rec \r\n");

    bleapp_init();
    bslink_init();
    disp_init();
    auth_init();
    device_init();
///*
    tst_init();
//*/
#ifndef UTILITY_TESTPIEZO
    bleapp_advertising_scan_start();
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

#ifdef PRINTF_RTT
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif

    printf("Reset ( reason: %04X ) \r\n", NRF_POWER->RESETREAS);
    NRF_POWER->RESETREAS = 0xFFFFFFFF; //clear flags

    printf("HW: %s \r\n", "v2.0");

    //binfo_init();
    keys_init();

    err = nrf_mem_init();
    ERR_CHECK("nrf_mem_init", err);

    sch_init();
    UartInit();
    
    printf("\r\n");
    
    ble_stack_init();

    printf("Device addr: [0]:%x [1]:%x\n",
           NRF_FICR->DEVICEADDR[0],
           NRF_FICR->DEVICEADDR[1]);

    rec_init();
    main_after_rec(err);
    sch_run(); //Scheduler loop, never returns

    return 0;

}


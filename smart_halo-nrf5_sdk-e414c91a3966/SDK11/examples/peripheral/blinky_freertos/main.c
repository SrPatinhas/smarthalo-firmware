/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example_freertos
 *
 * @brief Blinky FreeRTOS Example Application main file.
 *
 * This file contains the source code for a sample application using FreeRTOS to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_gpio.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_error.h"


#define TASK_DELAY        200    /**< Task delay. Delays a LED0 task for 200 ms */
#define TIMER_PERIOD      1000   /**< Timer period. LED1 timer will expire after 1000 ms */

/**@brief LED0 task entry function.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the task.
 */
static void vLed0Function (void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    for( ;; )
    {
        nrf_gpio_pin_toggle(BSP_LED_0);
        vTaskDelay(TASK_DELAY); // Delay a task for a given number of ticks

        // Tasks must be implemented to never return...
    }
}

/**@brief The function to call when the LED1 FreeRTOS timer expires.
 *
 * @param[in] pvParameter   Pointer that will be used as the parameter for the timer.
 */
static void vLed1Callback (void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    nrf_gpio_pin_toggle(BSP_LED_1);
}

int main(void)
{
    TaskHandle_t  xLed0Handle;       /**< Reference to LED0 toggling FreeRTOS task. */
    TimerHandle_t xLed1Handle;       /**< Reference to LED1 toggling FreeRTOS timer. */
    ret_code_t err_code;

    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    // Configure LED-pins as outputs
    nrf_gpio_cfg_output(BSP_LED_0);
    nrf_gpio_cfg_output(BSP_LED_1);
    nrf_gpio_cfg_output(BSP_LED_2);
    nrf_gpio_cfg_output(BSP_LED_3);
    nrf_gpio_pin_set(BSP_LED_0);
    nrf_gpio_pin_set(BSP_LED_1);
    nrf_gpio_pin_set(BSP_LED_2);
    nrf_gpio_pin_set(BSP_LED_3);

    UNUSED_VARIABLE(xTaskCreate( vLed0Function, "L0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &xLed0Handle ));    // LED0 task creation
    xLed1Handle = xTimerCreate( "L1", TIMER_PERIOD, pdTRUE, NULL, vLed1Callback );                                 // LED1 timer creation
    UNUSED_VARIABLE(xTimerStart( xLed1Handle, 0 ));                                                                // LED1 timer start

    /* Activate deep sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Start FreeRTOS scheduler.
    vTaskStartScheduler();

    while (true)
    {
        // FreeRTOS should not be here...
    }
}

/* Used in debug mode for assertions */
void assert_nrf_callback(uint16_t line_num, const uint8_t *file_name)
{
  while(1)
  {
    /* Loop forever */
  }
}

/**
 *@}
 **/

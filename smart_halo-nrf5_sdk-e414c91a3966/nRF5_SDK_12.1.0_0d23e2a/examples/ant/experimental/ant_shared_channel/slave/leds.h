
/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

/**@file
 * @defgroup leds
 * @{
 * @ingroup ant_auto_shared_channel
 *
 * @brief Header file containing platform specific LED functions
 */
#ifndef ANT_SHARED_CHANNEL_SLAVE_LEDS_H__
#define ANT_SHARED_CHANNEL_SLAVE_LEDS_H__

#if defined(BOARD_N5DK1)
    #include "n5sk_led.h"
#else
    #include "nrf_gpio.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define LED_INVALID     0xFF

static __INLINE void led_on(uint8_t led_num)
{
    if (led_num == LED_INVALID)
        return;

#if defined(BOARD_N5DK1)
    n5_io_turn_on_led(led_num);
#else
    nrf_gpio_pin_clear(led_num);
#endif
}

static __INLINE void led_off(uint8_t led_num)
{
    if (led_num == LED_INVALID)
        return;
#if defined(BOARD_N5DK1)
    n5_io_turn_off_led(led_num);
#else
    nrf_gpio_pin_set(led_num);
#endif
}

static __INLINE void led_toggle(uint8_t led_num)
{
    if (led_num == LED_INVALID)
        return;
#if defined(BOARD_N5DK1)
    n5_io_toggle_led(led_num);
#else
    nrf_gpio_pin_toggle(led_num);
#endif
}

static __INLINE void led_clear(void)
{

#if defined(BOARD_N5DK1)
    n5_io_clear_leds();
#else
    nrf_gpio_pin_set(BSP_LED_0);
    nrf_gpio_pin_set(BSP_LED_1);
#endif
}

static __INLINE void led_init(void)
{

#if defined(BOARD_N5DK1)
    n5_io_init_leds();
#else
    nrf_gpio_range_cfg_output(BSP_LED_0, BSP_LED_1);
    led_clear();
#endif
}

#if 0 // LEGACY STUFF BELOW

#define LED_INVALID     0xFF

static __INLINE void led_on(uint8_t led_num)
{
    if (led_num == LED_INVALID)
        return;

#if defined(BOARD_N5DK1)
    n5_io_turn_on_led(led_num);
#else
    nrf_gpio_pin_set(led_num);
#endif
}

static __INLINE void led_off(uint8_t led_num)
{
    if (led_num == LED_INVALID)
        return;
#if defined(BOARD_N5DK1)
    n5_io_turn_off_led(led_num);
#else
    nrf_gpio_pin_clear(led_num);
#endif
}

static __INLINE void led_toggle(uint8_t led_num)
{
    if (led_num == LED_INVALID)
        return;
#if defined(BOARD_N5DK1)
    n5_io_toggle_led(led_num);
#else
    nrf_gpio_pin_toggle(led_num);
#endif
}

static __INLINE void led_clear(void)
{

#if defined(BOARD_N5DK1)
    n5_io_clear_leds();
#else
    nrf_gpio_pin_clear(BSP_LED_0);
    nrf_gpio_pin_clear(BSP_LED_1);
#endif
}

static __INLINE void led_init(void)
{

#if defined(BOARD_N5DK1)
    n5_io_init_leds();
#else
    nrf_gpio_cfg_output(BSP_LED_0);
    nrf_gpio_cfg_output(BSP_LED_1);
    led_clear();
#endif
}

#endif // 0

#ifdef __cplusplus
}
#endif

#endif // ANT_SHARED_CHANNEL_SLAVE_LEDS_H__

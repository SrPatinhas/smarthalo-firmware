/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2015
All rights reserved.
*/

/**
 * @defgroup application Test Application
 * @brief Example of an application being debugged using debug messages.
 */

/**
 * @file application.c
 * @brief Example application source file
 * @ingroup application
 */


#include <stdint.h>
#include "string.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_error.h"
#include "ant_error.h"
#include "boards.h"
#include "bsp.h"
#include "ant_stack_config_defs.h"
#include "ant_channel_config.h"
#include "nrf_soc.h"
#include "debug.h"
#include "application.h"

// Channel configuration.
//ANT Channels
#define ANT_CHANNEL_NUMBER              0x00                        /**< ANT Channel 0. */

// Channel configuration.
#define RF_FREQUENCY                    66u                         /**< Channel RF Frequency = (2400 + 60)MHz */
#define CHANNEL_PERIOD                  8192u                       /**< Channel period 4 Hz. */
#define EXT_ASSIGN                      0x00                        /**< ANT Ext Assign. */

// Channel ID configuration.
#define CHAN_ID_DEV_TYPE                0x03u                       /**< Device type. */
#define CHAN_ID_DEV_NUM                 (NRF_FICR->DEVICEID[0])     /**< Device number. */
#define CHAN_ID_TRANS_TYPE              0x01u                       /**< Transmission type. */

// Miscellaneous defines.
#define ANT_CHANNEL_DEFAULT_NETWORK     0x00                        /**< ANT Network (default public network). */

// Data Page Numbers
#define DIGITALIO_DATA_PID              1u                          /**< Page number: digital data. */

static uint8_t m_broadcast_data[ANT_STANDARD_DATA_PAYLOAD_SIZE];    /**< Primary data transmit buffer. */
static uint8_t m_rx_input_pin_state = 0xFF;                         /**< State of received digital data, from the other node. */
static uint8_t m_tx_input_pin_state = 0xFF;                         /**< State of digital inputs in this node, for transmission. */


/**@brief Encode current state of buttons
 *
 * Configure bitfield encoding the state of the buttons
 * Bit 0 = 0 if button 0 is pressed
 * Bit 1 = 0 if button 1 is pressed
 * Bit 2 = 0 if button 2 is pressed
 * Bit 3 = 0 if button 3 is pressed
 * Bit 4-7 = 1 (unused)
 *
 * The state of each button is also transmitted in the
 * debug channel, with 0 = button pressed.
 */
static void encode_button_state(void)
{
    uint32_t err_code;
    bool     button0;
    bool     button1;
    bool     button2;
    bool     button3;

    err_code = bsp_button_is_pressed(0, &button0);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_button_is_pressed(1, &button1);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_button_is_pressed(2, &button2);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_button_is_pressed(3, &button3);
    APP_ERROR_CHECK(err_code);

    m_tx_input_pin_state = 0xFF;

    #ifdef INCLUDE_DEBUG_CHANNEL
    for(uint8_t i = ANT_DEBUG_FIELD_BUTTON_A_STATUS; i <= ANT_DEBUG_FIELD_BUTTON_D_STATUS; i++)
    {
        ad_set_debug_field(i,1);
    }
    #endif

    if (button0)
    {
        m_tx_input_pin_state &= 0xFE;
        #ifdef INCLUDE_DEBUG_CHANNEL
            ad_set_debug_field(ANT_DEBUG_FIELD_BUTTON_A_STATUS,0);
        #endif
    }
    if (button1)
    {
        m_tx_input_pin_state &= 0xFD;
        #ifdef INCLUDE_DEBUG_CHANNEL
            ad_set_debug_field(ANT_DEBUG_FIELD_BUTTON_B_STATUS,0);
        #endif
    }
    if (button2)
    {
        m_tx_input_pin_state &= 0xFB;
        #ifdef INCLUDE_DEBUG_CHANNEL
            ad_set_debug_field(ANT_DEBUG_FIELD_BUTTON_C_STATUS,0);
        #endif
    }
    if (button3)
    {
        m_tx_input_pin_state &= 0xF7;
        #ifdef INCLUDE_DEBUG_CHANNEL
            ad_set_debug_field(ANT_DEBUG_FIELD_BUTTON_D_STATUS,0);
        #endif
    }
}


/**@brief Formats page with current button state and sends data
 * Byte 0 = Page number (Digital I/O Data)
 * Byte 1-6 = Reserved
 * Byte 7 = State of digital inputs
 */
static void handle_transmit()
{
    uint32_t err_code;

    encode_button_state();

    m_broadcast_data[0] = DIGITALIO_DATA_PID;
    m_broadcast_data[1] = 0xFF;
    m_broadcast_data[2] = 0xFF;
    m_broadcast_data[3] = 0xFF;
    m_broadcast_data[4] = 0xFF;
    m_broadcast_data[5] = 0xFF;
    m_broadcast_data[6] = 0xFF;
    m_broadcast_data[7] = m_tx_input_pin_state;

    err_code = sd_ant_broadcast_message_tx(ANT_CHANNEL_NUMBER, ANT_STANDARD_DATA_PAYLOAD_SIZE, m_broadcast_data);
    APP_ERROR_CHECK(err_code);
}


/**@brief Turns on LEDs according to the contents of the received data page.
 */
static void set_led_state()
{
    uint8_t led_state_field = ~m_rx_input_pin_state;

    if (led_state_field & 1)
    {
        LEDS_ON(BSP_LED_0_MASK);
    }
    else
    {
        LEDS_OFF(BSP_LED_0_MASK);
    }

    if (led_state_field & 2)
    {
        LEDS_ON(BSP_LED_1_MASK);
    }
    else
    {
        LEDS_OFF(BSP_LED_1_MASK);
    }

    if (led_state_field & 4)
    {
        LEDS_ON(BSP_LED_2_MASK);
    }
    else
    {
        LEDS_OFF(BSP_LED_2_MASK);
    }

    if (led_state_field & 8)
    {
        LEDS_ON(BSP_LED_3_MASK);
    }
    else
    {
        LEDS_OFF(BSP_LED_3_MASK);
    }

    #ifdef INCLUDE_DEBUG_CHANNEL
    // Example use for Fast Debug Byte
    // Check the value actually written to LED A is correct by outputting to Fast Debug Byte (1 for on, 0 for off)
    ad_set_fast_debug_byte( ( LED_IS_ON(BSP_LED_0_MASK) != 0 ) & 1UL );
    #endif
}


void app_channel_setup(void)
{
    uint32_t err_code;

    #ifdef INCLUDE_DEBUG_CHANNEL
     //Init debug fields
     ad_set_debug_field(ANT_DEBUG_FIELD_TX_TOTAL_CH0, 0);
     ad_set_debug_field(ANT_DEBUG_FIELD_RX_TOTAL_CH0, 0);
     ad_set_debug_field(ANT_DEBUG_FIELD_COLLISIONS_CH0, 0);
    #endif

    ant_channel_config_t channel_config =
    {
        .channel_number     = ANT_CHANNEL_NUMBER,
        .channel_type       = CHANNEL_TYPE_MASTER,
        .ext_assign         = EXT_ASSIGN,
        .rf_freq            = RF_FREQUENCY,
        .transmission_type  = CHAN_ID_TRANS_TYPE,
        .device_type        = CHAN_ID_DEV_TYPE,
        .device_number      = CHAN_ID_DEV_NUM,
        .channel_period     = CHANNEL_PERIOD,
        .network_number     = ANT_CHANNEL_DEFAULT_NETWORK,
    };

    // Configure channel parameters
    err_code = ant_channel_init(&channel_config);
    APP_ERROR_CHECK(err_code);

    // Open channel.
    err_code = sd_ant_channel_open(ANT_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);
}


void app_channel_event_handler(ant_evt_t * p_ant_evt)
{
   ANT_MESSAGE * p_message = (ANT_MESSAGE*) p_ant_evt->msg.evt_buffer;

    switch(p_ant_evt->event)
    {
        case EVENT_RX:
            if(p_message->ANT_MESSAGE_aucPayload[0] == DIGITALIO_DATA_PID)
            {
                //Set LEDs according to Received Digital IO Data Page
                m_rx_input_pin_state = p_message->ANT_MESSAGE_aucPayload[7];
                set_led_state();
            }
            #ifdef INCLUDE_DEBUG_CHANNEL
            ad_increment_debug_field(ANT_DEBUG_FIELD_RX_TOTAL_CH0);
            #endif
            break;

        case EVENT_TX:
            // Transmit data on the reverse direction every channel period
            handle_transmit();
            #ifdef INCLUDE_DEBUG_CHANNEL
            ad_increment_debug_field(ANT_DEBUG_FIELD_TX_TOTAL_CH0);
            #endif
            break;

        case EVENT_CHANNEL_COLLISION:
            #ifdef INCLUDE_DEBUG_CHANNEL
            ad_increment_debug_field(ANT_DEBUG_FIELD_COLLISIONS_CH0);
            #endif
            break;

        default:
            break;
    }
}


#ifdef INCLUDE_DEBUG_CHANNEL
void app_custom_debug_command_handler(uint8_t const * const p_command)
{
    // Example of a custom command sent over the debug channel which causes the device to turn all its LEDs on.
    // Do not define any custom commands which begin with the debug message indicator: 0xF9 - they will be misinterpreted as a filter message.
    if(p_command[0] == 0x42)
    {
          LEDS_ON(LEDS_MASK);
    }
    else
    {
          LEDS_OFF(LEDS_MASK);
    }
}
#endif




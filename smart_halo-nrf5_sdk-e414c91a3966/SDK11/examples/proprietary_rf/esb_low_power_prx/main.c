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

#include "nrf_esb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_log.h"
#include "boards.h"
#include "nrf_log.h"

#define LED_ON          0
#define LED_OFF         1

void nrf_esb_error_handler(uint32_t err_code, uint32_t line)
{
#if DEBUG //lint -e553
    while(true);
#else
    NVIC_SystemReset();
#endif

}

#define APP_ERROR_CHECK(err_code) if(err_code) nrf_esb_error_handler(err_code, __LINE__);

//#define NRF_ESB_LEGACY

/*lint -save -esym(40, BUTTON_1) -esym(40, BUTTON_2) -esym(40, BUTTON_3) -esym(40, BUTTON_4) -esym(40, LED_1) -esym(40, LED_2) -esym(40, LED_3) -esym(40, LED_4) */

static nrf_esb_payload_t tx_payload;
static nrf_esb_payload_t rx_payload;
static uint8_t m_state[4];
const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;
uint8_t led_nr;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
            NRF_LOG("SUCCESS\r\n");
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            NRF_LOG("FAILED\r\n");
            (void) nrf_esb_flush_tx();
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS) ;
            NRF_LOG("Receiving packet: ");
            NRF_LOG_HEX_CHAR(rx_payload.data[0]);
            NRF_LOG("\r\n");

            switch (rx_payload.data[0] & 0xFUL)
            {
                case 0x1:
                    m_state[0] = !m_state[0];
                    nrf_gpio_pin_write(LED_1, m_state[0]);
                    break;

                case 0x2:
                    m_state[1] = !m_state[1];
                    nrf_gpio_pin_write(LED_2, m_state[1]);
                    break;

                case 0x4:
                    m_state[2] = !m_state[2];
                    nrf_gpio_pin_write(LED_3, m_state[2]);
                    break;

                case 0x8:
                    m_state[3] = !m_state[3];
                    nrf_gpio_pin_write(LED_4, m_state[3]);
                    break;
            }

            nrf_gpio_pin_write(LED_1, m_state[0]);
            nrf_gpio_pin_write(LED_2, m_state[1]);
            nrf_gpio_pin_write(LED_3, m_state[2]);
            nrf_gpio_pin_write(LED_4, m_state[3]);

            tx_payload.length = 1;
            tx_payload.data[0] = m_state[0] << 0
                               | m_state[1] << 1
                               | m_state[2] << 2
                               | m_state[3] << 3;
            (void) nrf_esb_write_payload(&tx_payload);

            NRF_LOG("Queue transmitt packet: ");
            NRF_LOG_HEX_CHAR(tx_payload.data[0]);
            NRF_LOG("\r\n");
            break;
    }



}


void clocks_start( void )
{
    // Start HFCLK and wait for it to start.
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}


uint32_t esb_init( void )
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

#ifndef NRF_ESB_LEGACY
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
#else // NRF_ESB_LEGACY
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_LEGACY_CONFIG;
#endif // NRF_ESB_LEGACY
    nrf_esb_config.selective_auto_ack       = 0;
    nrf_esb_config.payload_length           = 3;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;

    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

    tx_payload.length = 1;

    return NRF_SUCCESS;
}

void gpio_init( void )
{
    m_state[0] = LED_OFF;
    m_state[1] = LED_OFF;
    m_state[2] = LED_OFF;
    m_state[3] = LED_OFF;

    //nrf_gpio_range_cfg_output(8, 31);
    LEDS_CONFIGURE(LEDS_MASK);

    nrf_gpio_pin_write(LED_1, m_state[0]);
    nrf_gpio_pin_write(LED_2, m_state[1]);
    nrf_gpio_pin_write(LED_3, m_state[2]);
    nrf_gpio_pin_write(LED_4, m_state[3]);
}


uint32_t logging_init( void )
{
    uint32_t err_code;
    err_code = NRF_LOG_INIT();
    return err_code;
}


void power_manage( void )
{
    // WFE - SEV - WFE sequence to wait until a radio event require further actions.
    __WFE();
    __SEV();
    __WFE();
}


int main(void)
{
    uint32_t err_code;
    err_code = logging_init();
    APP_ERROR_CHECK(err_code);
    gpio_init();
    err_code = esb_init();
    APP_ERROR_CHECK(err_code);
    clocks_start();

    NRF_LOG("Enhanced ShockBurst Receiver Example running.\r\n");

    err_code = nrf_esb_start_rx();
    APP_ERROR_CHECK(err_code);

    tx_payload.data[0] = m_state[0] << 0
                       | m_state[1] << 1
                       | m_state[2] << 2
                       | m_state[3] << 3;
    err_code = nrf_esb_write_payload(&tx_payload);
    APP_ERROR_CHECK(err_code);

    while (true)
    {
        power_manage();
    }
}
/*lint -restore */

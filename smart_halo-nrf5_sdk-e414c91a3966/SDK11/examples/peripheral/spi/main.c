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

#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>

#if defined(BOARD_PCA10036) || defined(BOARD_PCA10040)
#define SPI_CS_PIN   29 /**< SPI CS Pin.*/
#elif defined(BOARD_PCA10028)
#define SPI_CS_PIN   4  /**< SPI CS Pin.*/
#else
#error "Example is not supported on that board."
#endif

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

#define TEST_STRING "Nordic"
static uint8_t       m_tx_buf[] = TEST_STRING;           /**< TX buffer. */
static uint8_t       m_rx_buf[sizeof(TEST_STRING)+1];    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    spi_xfer_done = true;
    NRF_LOG_PRINTF(" Transfer completed.\r\n");
    if (m_rx_buf[0] != 0)
    {
        NRF_LOG_PRINTF(" Received: %s\r\n",m_rx_buf);
    }
}

int main(void)
{
    LEDS_CONFIGURE(BSP_LED_0_MASK);
    LEDS_OFF(BSP_LED_0_MASK);

    APP_ERROR_CHECK(NRF_LOG_INIT());
    NRF_LOG_PRINTF("SPI example\r\n");

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG(SPI_INSTANCE);
    spi_config.ss_pin = SPI_CS_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler));

    while(1)
    {
        // Reset rx buffer and transfer done flag
        memset(m_rx_buf, 0, m_length);
        spi_xfer_done = false;

        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, m_length));

        while (!spi_xfer_done)
        {
            __WFE();
        }

        LEDS_INVERT(BSP_LED_0_MASK);
        nrf_delay_ms(200);
    }
}

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
 
#include "ble_dtm_app.h"
#include "ble_serialization.h"
#include "nrf_error.h"

uint32_t ble_dtm_init_req_enc(app_uart_stream_comm_params_t const * const p_uart_comm_params, uint8_t * const p_buf, uint32_t * const p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);
    SER_ASSERT_NOT_NULL(p_uart_comm_params);

    uint32_t index = 0;
    uint32_t buf_len = *p_buf_len;
    uint32_t err_code;

    err_code = uint8_t_enc(&p_uart_comm_params->tx_pin_no, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_uart_comm_params->rx_pin_no, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_uart_comm_params->baud_rate, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}


uint32_t ble_dtm_init_rsp_dec(uint8_t const * const p_buf, uint32_t packet_len, uint32_t * const p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t err_code;
    uint32_t index = 0;

    err_code = uint32_t_dec(p_buf, packet_len, &index, p_result_code);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT(packet_len == index, NRF_ERROR_INVALID_LENGTH);

    return err_code;
}

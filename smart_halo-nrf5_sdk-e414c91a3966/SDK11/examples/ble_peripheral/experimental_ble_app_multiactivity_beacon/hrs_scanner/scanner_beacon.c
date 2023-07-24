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
#include "scanner_beacon.h"
#include <stdio.h>
#include <string.h>

#include "app_error.h"
#include "app_util.h"


#define SCAN_INTERVAL_MS  100
#define WINDOW_LEN_MS    (SCAN_INTERVAL_MS / 4)
#define MAX_ADV_PACK_LEN (BLE_GAP_ADV_MAX_SIZE + BLE_GAP_ADDR_LEN + 2)
#define ADV_TYPE_LEN      2
#define BEACON_DATA_LEN   21

static struct
{
    ble_scan_beacon_evt_handler_t evt_handler;
    ble_srv_error_handler_t       error_handler;                      /**< Function to be called in case of an error. */
    ble_gap_scan_params_t         scan_params;
} m_beacon_scanner;

static void decode_ad_type_flags(uint8_t *buffer, uint8_t len, adv_packet_t *adv_packet)
{
    int i = 1;
    adv_packet->gap_ad_flags = buffer[i];
}


static void decode_ad_manuf_spec_data(uint8_t *buffer, uint8_t len, adv_packet_t *adv_packet)
{
    int i = 1;
    adv_packet->adv_data.manuf_id = uint16_decode(&buffer[i]);
    i += sizeof(uint16_t);
    adv_packet->adv_data.beacon_dev_type = buffer[i++];
    uint8_t data_len = buffer[i++];
    if (data_len == BEACON_DATA_LEN)
    {
        memcpy(&adv_packet->adv_data.uuid, &buffer[i], sizeof(ble_uuid128_t));
        i += sizeof(ble_uuid128_t);
        adv_packet->adv_data.major = uint16_decode(&buffer[i]);
        i += sizeof(uint16_t);
        adv_packet->adv_data.minor = uint16_decode(&buffer[i]);
        i += sizeof(uint16_t);
        adv_packet->adv_data.rssi = uint16_decode(&buffer[i]);
    }
    else
    {
        memset(&adv_packet->adv_data.uuid, 0, sizeof(ble_uuid128_t));
        adv_packet->adv_data.major = 0;
        adv_packet->adv_data.minor = 0;
        adv_packet->adv_data.rssi = 0;
    }
}


static uint32_t decode_advertising(uint8_t *buffer, uint8_t buffer_len, adv_packet_t *adv_packet)
{
    uint8_t i = 0; 
    
    do
    {
        uint8_t field_len = buffer[i++];
        if(buffer[i] == BLE_GAP_AD_TYPE_FLAGS)
        {
            if (field_len != ADV_TYPE_LEN)
            {
                return NRF_ERROR_NOT_FOUND;
            }
            decode_ad_type_flags(&buffer[i], field_len, adv_packet);
            i += field_len;
        }
        else if (buffer[i] == BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA)
        {
            decode_ad_manuf_spec_data(&buffer[i], field_len, adv_packet);
            i += field_len;
        }
    }while( i < buffer_len);

    return NRF_SUCCESS;    
}


void app_beacon_scanner_on_sys_evt(uint32_t event)
{
    // no implementation needed
}

void app_beacon_scanner_on_ble_evt(ble_evt_t * p_ble_evt)
{
    if (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT)
    {
        ble_scan_beacon_evt_t evt;
        uint32_t err_code = decode_advertising(p_ble_evt->evt.gap_evt.params.adv_report.data, p_ble_evt->evt.gap_evt.params.adv_report.dlen, &evt.rcv_adv_packet);
        if (err_code == NRF_SUCCESS)
        {
            if (m_beacon_scanner.evt_handler != NULL)
            {
                    evt.evt_type = BLE_SCAN_BEACON_ADVERTISER_FOUND;
                    m_beacon_scanner.evt_handler(&evt);
            }
        }
    }
}

void app_beacon_scanner_init(ble_beacon_scanner_init_t * p_init)
{
    m_beacon_scanner.evt_handler = p_init->evt_handler;
    m_beacon_scanner.error_handler= p_init->error_handler;
    
    memset(&m_beacon_scanner.scan_params, 0, sizeof(m_beacon_scanner.scan_params));
    m_beacon_scanner.scan_params.active      = false;
    m_beacon_scanner.scan_params.selective   = false;
    m_beacon_scanner.scan_params.p_whitelist = NULL;
    m_beacon_scanner.scan_params.interval    = MSEC_TO_UNITS(SCAN_INTERVAL_MS, UNIT_0_625_MS); 
    m_beacon_scanner.scan_params.window      = MSEC_TO_UNITS(WINDOW_LEN_MS, UNIT_0_625_MS);    
    m_beacon_scanner.scan_params.timeout     = 0; // disable timeout
}

void app_beacon_scanner_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_scan_start(&m_beacon_scanner.scan_params);
    if (err_code != NRF_SUCCESS)
    {
        if (m_beacon_scanner.error_handler != NULL)
        {
            m_beacon_scanner.error_handler(err_code);
        }
    }
}

void app_beacon_scanner_stop(void)
{
    uint32_t err_code;
    
    err_code = sd_ble_gap_scan_stop();
    if (err_code != NRF_SUCCESS)
    {
        if (m_beacon_scanner.error_handler != NULL)
        {
            m_beacon_scanner.error_handler(err_code);
        }
    }
}


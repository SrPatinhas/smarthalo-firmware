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
 
#include "advertiser_beacon.h"
#include <stdio.h>
#include <string.h>

#include "app_error.h"
#include "app_util.h"

#define ADV_DATA_BEACON_DATA_LEN (sizeof(ble_uuid128_t) + 2*sizeof(uint16_t) + sizeof(uint8_t))
#define ADV_DATA_MANUF_DATA_LEN  4 + ADV_DATA_BEACON_DATA_LEN 

#define ADV_TYPE_LEN             2

#define ADV_DATA_LEN             40

#define APP_DEVICE_TYPE          0x02                              /**< 0x02 refers to Beacon. */


static struct
{
    ble_uuid128_t           uuid;                               /** 128 proprietary service UUID to include in advertisement packets*/
    uint16_t                major;                              /** Major identifier to use for 'beacon'*/
    uint16_t                minor;                              /** Minor identifier to use for 'beacon'*/
    uint16_t                manuf_id;
    uint16_t                rssi;                               /** measured RSSI at 1 meter distance in dBm*/
    ble_gap_addr_t          beacon_addr;                        /** ble address to be used by the beacon*/
    ble_srv_error_handler_t error_handler;                      /**< Function to be called in case of an error. */
    ble_gap_adv_params_t    adv_params;
} m_beacon;

static uint8_t encode_adv_packet(uint8_t * p_data)
{
    uint8_t offset    = 0;

    // Adding advertising data: Flags
    p_data[offset++] =  ADV_TYPE_LEN;
    p_data[offset++] =  BLE_GAP_AD_TYPE_FLAGS;
    p_data[offset++] =  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // Adding advertising data: Manufacturer specific data.
    p_data[offset++] =  ADV_DATA_MANUF_DATA_LEN + 1;                          
    p_data[offset++] =  BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA;
    offset           += uint16_encode(m_beacon.manuf_id, &p_data[offset]);
    p_data[offset++] =  APP_DEVICE_TYPE;

    // Adding manufacturer specific data (beacon data).
    p_data[offset++] = ADV_DATA_BEACON_DATA_LEN; 
    memcpy(&p_data[offset], &m_beacon.uuid, sizeof(ble_uuid128_t));
    offset += sizeof(ble_uuid128_t);
    offset += uint16_encode(m_beacon.major, &p_data[offset]);
    offset += uint16_encode(m_beacon.minor, &p_data[offset]);
    p_data[offset++] = m_beacon.rssi;
   
    return offset;
}


void app_beacon_on_sys_evt(uint32_t event)
{
    // no implementation needed
}

void app_beacon_init(ble_beacon_init_t * p_init)
{         
    memcpy(&m_beacon.uuid, &p_init->uuid, sizeof(p_init->uuid));
    
    m_beacon.major         = p_init->major;
    m_beacon.minor         = p_init->minor;
    m_beacon.manuf_id      = p_init->manuf_id;
    m_beacon.rssi          = p_init->rssi;
    m_beacon.beacon_addr   = p_init->beacon_addr;
    m_beacon.error_handler = p_init->error_handler;
    
    m_beacon.adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_beacon.adv_params.p_peer_addr = &m_beacon.beacon_addr;
    m_beacon.adv_params.fp          = BLE_GAP_ADV_FP_ANY; 
    m_beacon.adv_params.p_whitelist = NULL;
    m_beacon.adv_params.interval    = p_init->adv_interval;
    m_beacon.adv_params.timeout     = 0; // disable timeout

    memset(&m_beacon.adv_params.channel_mask, 0, sizeof(m_beacon.adv_params.channel_mask));    
}

void app_beacon_start(void)
{
    uint32_t err_code;
    uint8_t adv_pdu[ADV_DATA_LEN];
    
    uint8_t adv_len = encode_adv_packet(&adv_pdu[0]);
    
    err_code = sd_ble_gap_adv_data_set(adv_pdu, adv_len, NULL, 0);
    
    if (err_code != NRF_SUCCESS)
    {
        if (m_beacon.error_handler != NULL)
        {
            m_beacon.error_handler(err_code);
        }
    } 
    
    err_code = sd_ble_gap_adv_start(&m_beacon.adv_params);
    
    if (err_code != NRF_SUCCESS)
    {
        if (m_beacon.error_handler != NULL)
        {
            m_beacon.error_handler(err_code);
        }
    } 
}

void app_beacon_stop(void)
{
    uint32_t err_code;
    err_code = sd_ble_gap_adv_stop();
    
    if (err_code != NRF_SUCCESS)
    {
        if (m_beacon.error_handler != NULL)
        {
            m_beacon.error_handler(err_code);
        }
    } 
}


/**
 * @file       shimano.h
 *
 * @brief      This file provides access to the bluetooth Shimano Information Service
 *
 * @author     Sean Beitz
 * @copyright  Copyright (c) 2021 SmartHalo Inc
 */

#ifndef _SHIMANO_H
#define _SHIMANO_H

#include <stdint.h>
#include "ble.h"
#include "ble_db_discovery.h"
#include "ble_srv_common.h"
#include "nrf_ble_gq.h"
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

//SHIMANO INFORMATION SERVICE UUID : 0x000018EF-5348-494D-414E-4F5F424C4500
#define SHIMANO_BASE_UUID      {{0x00, 0x45, 0x4C, 0x42, 0x5F, 0x4F, 0x4E, 0x41, 0x4D, 0x49, 0x48, 0x53, 0xEF, 0x18, 0x00, 0x00}}
#define SHIMANO_IS_UUID        0x18EF //Shimano Information Service uuid
#define SHIMANO_PI_CHAR_UUID   0x2AC1 //Shimano Periodic Information Characteristic uuid

#define BLE_SHIIS_C_BLE_OBSERVER_PRIO   2

//these reflect the values for the API communication
#define UI_SHIMANO_SPEEDO          36
#define UI_SHIMANO_ASSIST_MODE     37
#define UI_SHIMANO_ASSIST_LEVEL    38
#define UI_SHIMANO_BATTERY_LEVEL   39

// Forward declaration of the ble_shiis_t type.
typedef struct ble_shiis_c_s ble_shiis_c_t;

/**@brief   Macro for defining a ble_shiis_c instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_SHIIS_C_DEF(_name)                                                                        \
static ble_shiis_c_t _name;                                                                           \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_SHIIS_C_BLE_OBSERVER_PRIO,                                                   \
                     ble_shiis_c_on_ble_evt, &_name)

/**@brief Structure containing the handles related to the Shimano Information Service found on the peer. */
typedef struct
{
    uint16_t                bl_cccd_handle;  /**< Handle of the CCCD of the Periodic Information Characteristic. */
    uint16_t                bl_handle;       /**< Handle of the Periodic Information Characteristic, as provided by the SoftDevice. */
} ble_shiis_c_db_t;

/**@brief Shimano Information Service Client event type. */
typedef enum
{
    BLE_SHIIS_C_EVT_DISCOVERY_COMPLETE,  /**< Event indicating that the Shimano Information Service has been discovered at the peer. */
    BLE_SHIIS_C_EVT_NOTIFICATION,        /**< Event indicating that a notification of the Shimano Information characteristic has been received from the peer. */
    BLE_SHIIS_C_EVT_READ_RESP            /**< Event indicating that a read response on Shimano Information characteristic has been received from peer. */
} ble_shiis_c_evt_type_t;


/**@brief Shimano Information Service Client travel information parameters (exchanged data). */
typedef struct 
{
    uint8_t          assist_mode;
    int16_t          current_speed[2];
    uint8_t          assist_level;
    uint8_t          cadence;
    uint32_t         travel_time[4];
}ble_shiis_t;

/**@brief Shimano Information Service Client Event structure. */
typedef struct
{
    ble_shiis_c_evt_type_t evt_type;  /**< Event Type. */
    uint16_t conn_handle;           /**< Connection handle relevent to this event.*/
    union
    {
        ble_shiis_c_db_t shiis_db;         /**< Shimano Information Service related handles found on the peer device. This will be filled if the evt_type is @ref BLE_SHIIS_C_EVT_DISCOVERY_COMPLETE.*/
        ble_shiis_t shiis_data;
    } params;
} ble_shiis_c_evt_t;

typedef enum{
    ASSIST_OFF,
    ASSIST_ECO,
    ASSIST_NORMAL,
    ASSIST_HIGH,
    ASSIST_WALK_STOP,
    ASSIST_WALK_START,
    ASSIST_ENUM_COUNT
} shiis_assist_mode_t;

/**@brief   Event handler type.
 *
 * @details This is the type of the event handler that is to be provided by the application
 *          of the SHIIS module to receive events.
 */
typedef void (* ble_shiis_c_evt_handler_t) (ble_shiis_c_t * p_shiis_shiis_c, ble_shiis_c_evt_t * p_evt);

/**@brief   Shimano Information Service Client structure. */
struct ble_shiis_c_s
{
    uint8_t                     uuid_type;       //!< UUID type
    uint16_t                    conn_handle;     /**< Connection handle, as provided by the SoftDevice. */
    ble_shiis_c_db_t            peer_shiis_db;   /**< Handles related to SHIIS on the peer.*/
    ble_shiis_c_evt_handler_t   evt_handler;     /**< Application event handler to be called when there is an event related to the Shimano Information Service. */
    ble_srv_error_handler_t     error_handler;   /**< Function to be called in case of an error. */
    nrf_ble_gq_t              * p_gatt_queue;    /**< Pointer to the BLE GATT Queue instance. */
};

/**@brief   Shimano Information Service Client initialization structure. */
typedef struct
{
    ble_shiis_c_evt_handler_t   evt_handler;  /**< Event handler to be called by the Shimano Information Service Client module when there is an event related to the Shimano Information Service. */
    ble_srv_error_handler_t     error_handler;   /**< Function to be called in case of an error. */
    nrf_ble_gq_t              * p_gatt_queue; /**< Pointer to the BLE GATT Queue instance. */
} ble_shiis_c_init_t;


/**@brief      Function for initializing the Shimano Information Service Client module.
 *
 * @details    This function initializes the module and sets up datashiise discovery to discover
 *             the Shimano Information Service. After calling this function, call @ref ble_db_discovery_start
 *             to start discovery once a link with a peer has been established.
 *
 * @param[out] p_ble_shiis_c      Pointer to the Shimano Information Service Client structure.
 * @param[in]  p_ble_shiis_c_init Pointer to the Shimano Information Service initialization structure that contains
 *                              the initialization information.
 *
 * @retval     NRF_SUCCESS      Operation success.
 * @retval     NRF_ERROR_NULL   A parameter is NULL.
 * @retval	   err_code         Otherwise, an error code returned by @ref ble_db_discovery_evt_register.
 */
uint32_t ble_shiis_c_init(ble_shiis_c_t * p_ble_shiis_c, ble_shiis_c_init_t * p_ble_shiis_c_init);


/**@brief     Function for handling BLE events from the SoftDevice.
 *
 * @details   This function handles the BLE events received from the SoftDevice. If a BLE
 *            event is relevant to the Shimano Information Service Client module, the function uses the event's data to update
 *            interval variables and, if necessary, send events to the application.
 *
 * @note      This function must be called by the application.
 *
 * @param[in] p_ble_evt     Pointer to the BLE event.
 * @param[in] p_context     Pointer to the Shimano Information Service client structure.
 */
void ble_shiis_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


/**@brief   Function for enabling notifications on the Periodic Information Characteristic.
 *
 * @details This function enables the notification of the Periodic Information Characteristic at the
 *          peer by writing to the CCCD of the Periodic Information Characteristic.
 *
 * @param   p_ble_shiis_c Pointer to the Shimano Information Service client structure.
 *
 * @retval  NRF_SUCCESS     If the SoftDevice has been requested to write to the CCCD of the peer.
 * @retval  NRF_ERROR_NULL  Parameter is NULL.
 * @retval  err_code        Otherwise, an error code returned by the SoftDevice API @ref
 *                          sd_ble_gattc_write.
 */
uint32_t ble_shiis_c_bl_notif_enable(ble_shiis_c_t * p_ble_shiis_c);


/**@brief   Function for reading the Periodic Information Characteristic.
 *
 * @param   p_ble_shiis_c Pointer to the Shimano Information Service client structure.
 *
 * @retval  NRF_SUCCESS If the read request was successfully queued to be sent to peer.
 */
uint32_t ble_shiis_c_bl_read(ble_shiis_c_t * p_ble_shiis_c);


/**@brief     Function for handling events from the Datashiise Discovery module.
 *
 * @details   Call this function when you get a callback event from the Datashiise Discovery module.
 *            This function handles an event from the Datashiise Discovery module, and determines
 *            whether it relates to the discovery of Shimano Information Service at the peer. If it does, this function
 *			  calls the application's event handler to indicate that the Shimano Information Service was
 *            discovered at the peer. The function also populates the event with service-related
 *            information before providing it to the application.
 *
 * @param     p_ble_shiis_c Pointer to the Shimano Information Service client structure.
 * @param[in] p_evt Pointer to the event received from the Datashiise Discovery module.
 *
 */
void ble_shiis_on_db_disc_evt(ble_shiis_c_t * p_ble_shiis_c, ble_db_discovery_evt_t const * p_evt);


/**@brief     Function for assigning handles to this instance of shiis_c.
 *
 * @details   Call this function when a link has been established with a peer to
 *            associate the link to this instance of the module. This makes it
 *            possible to handle several links and associate each link to a particular
 *            instance of this module. The connection handle and attribute handles are
 *            provided from the discovery event @ref BLE_shiis_C_EVT_DISCOVERY_COMPLETE.
 *
 * @param[in] p_ble_shiis_c    Pointer to the SHIIS client structure instance for associating the link.
 * @param[in] conn_handle    Connection handle associated with the given SHIIS Client Instance.
 * @param[in] p_peer_handles Attribute handles on the shiis server you want this shiis client to
 *                           interact with.
 */
uint32_t ble_shiis_c_handles_assign(ble_shiis_c_t *    p_ble_shiis_c,
                                  uint16_t         conn_handle,
                                  ble_shiis_c_db_t * p_peer_handles);


#endif
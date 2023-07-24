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
 *
 * @defgroup ble_sdk_app_lns_main main.c
 * @{
 * @ingroup ble_sdk_app_lns
 * @brief Location and Navigation Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Location and Navigation service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_lns.h"
#include "ble_dis.h"
#include "sensorsim.h"
#include "app_timer.h"
#include "softdevice_handler.h"
#include "ble_conn_params.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "device_manager.h"
#include "pstorage.h"
#include "nrf_log.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT      1                                               /**< Include or not the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */
                                              
#define CENTRAL_LINK_COUNT                   0                                               /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT                1                                               /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define DEVICE_NAME                          "Nordic_LNS"                                    /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                    "NordicSemiconductor"                           /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                     40                                              /**< The advertising interval (in units of 0.625 ms; this value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS           180                                             /**< The advertising time-out in units of seconds. */

#define APPL_LOG(format, ...)                NRF_LOG_PRINTF("[APP] " format, ##__VA_ARGS__)  /**< Macro used to log debug information. */

#define APP_TIMER_PRESCALER                  0                                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS                 (6+BSP_APP_TIMERS_NUMBER)                       /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              4                                               /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL          APP_TIMER_TICKS(2400, APP_TIMER_PRESCALER)      /**< Battery level measurement interval (ticks). */
#define MIN_BATTERY_LEVEL                    81                                              /**< Minimum simulated battery level. */
#define MAX_BATTERY_LEVEL                    100                                             /**< Maximum simulated battery level. */
#define BATTERY_LEVEL_INCREMENT              1                                               /**< Increment between each simulated battery level measurement. */

#define LOC_AND_NAV_DATA_INTERVAL            APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)      /**< Location and Navigation data interval (ticks). */

#define SECOND_1_25_MS_UNITS                 800                                             /**< Definition of 1 second, when 1 unit is 1.25 ms. */
#define SECOND_10_MS_UNITS                   100                                             /**< Definition of 1 second, when 1 unit is 10 ms. */
#define MIN_CONN_INTERVAL                    MSEC_TO_UNITS(100, UNIT_1_25_MS)                /**< Minimum connection interval (100 ms). */
#define MAX_CONN_INTERVAL                    MSEC_TO_UNITS(250, UNIT_1_25_MS)                /**< Maximum connection interval (250 ms). */
#define SLAVE_LATENCY                        0                                               /**< Slave latency. */
#define CONN_SUP_TIMEOUT                     (4 * SECOND_10_MS_UNITS)                        /**< Connection supervisory time-out (4 seconds). Supervision time-out uses 10 ms units. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)      /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY        APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)      /**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT         3                                               /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                       1                                               /**< Perform bonding. */
#define SEC_PARAM_MITM                       0                                               /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                       0                                               /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS                   0                                               /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES            BLE_GAP_IO_CAPS_NONE                            /**< No I/O capabilities. */
#define SEC_PARAM_OOB                        0                                               /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE               7                                               /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE               16                                              /**< Maximum encryption key size. */

#define DEAD_BEEF                            0xDEADBEEF                                      /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static uint16_t                              m_conn_handle = BLE_CONN_HANDLE_INVALID;        /**< Handle of the current connection. */
static ble_bas_t                             m_bas;                                          /**< Structure used to identify the battery service. */
static ble_lns_t                             m_lns;                                          /**< Structure used to identify the location and navigation service. */

static dm_application_instance_t             m_app_handle;                                   /**< Application identifier allocated by Device Manager. */

static sensorsim_cfg_t                       m_battery_sim_cfg;                              /**< Battery Level sensor simulator configuration. */
static sensorsim_state_t                     m_battery_sim_state;                            /**< Battery Level sensor simulator state. */

APP_TIMER_DEF(m_battery_timer_id);                                                           /**< Battery timer. */
APP_TIMER_DEF(m_loc_and_nav_timer_id);                                                       /**< Location and navigation measurement timer. */

static ble_lns_loc_speed_t                   m_sim_location_speed;                           /**< Location and speed simulation. */
static ble_lns_pos_quality_t                 m_sim_position_quality;                         /**< Position measurement quality simulation. */
static ble_lns_navigation_t                  m_sim_navigation;                               /**< Navigation data structure simulation. */

static ble_uuid_t                            m_adv_uuids[] = {
                                                               {BLE_UUID_LOCATION_AND_NAVIGATION_SERVICE,  BLE_UUID_TYPE_BLE},
                                                               {BLE_UUID_BATTERY_SERVICE,                  BLE_UUID_TYPE_BLE},
                                                               {BLE_UUID_DEVICE_INFORMATION_SERVICE,       BLE_UUID_TYPE_BLE}
                                                             };

static const ble_lns_loc_speed_t initial_lns_location_speed = {
    .instant_speed_present   = true,
    .total_distance_present  = true,
    .location_present        = true,
    .elevation_present       = true,
    .heading_present         = true,
    .rolling_time_present    = true,
    .utc_time_time_present   = true,
    .position_status         = BLE_LNS_POSITION_OK,
    .data_format             = BLE_LNS_SPEED_DISTANCE_FORMAT_2D,
    .elevation_source        = BLE_LNS_ELEV_SOURCE_POSITIONING_SYSTEM,
    .heading_source          = BLE_LNS_HEADING_SOURCE_COMPASS,
    .instant_speed           = 12,         // = 1.2 meter/second
    .total_distance          = 2356,       // = 2356 meters/second
    .latitude                = -103123567, // = -10.3123567 degrees
    .longitude               = 601234567,  // = 60.1234567 degrees
    .elevation               = 1350,       // = 13.5 meter
    .heading                 = 2123,       // = 21.23 degrees
    .rolling_time            = 1,          // = 1 second
    .utc_time                = {
                                 .year    = 2015,
                                 .month   = 7,
                                 .day     = 8,
                                 .hours   = 12,
                                 .minutes = 43,
                                 .seconds = 33
                               }
};


static const ble_lns_pos_quality_t initial_lns_pos_quality = {
    .number_of_satellites_in_solution_present = true,
    .number_of_satellites_in_view_present     = true,
    .time_to_first_fix_present                = true,
    .ehpe_present                             = true,
    .evpe_present                             = true,
    .hdop_present                             = true,
    .vdop_present                             = true,
    .position_status                          = BLE_LNS_POSITION_OK,
    .number_of_satellites_in_solution         = 5,
    .number_of_satellites_in_view             = 6,
    .time_to_first_fix                        = 63,  // = 6.3 seconds
    .ehpe                                     = 100, // = 1 meter
    .evpe                                     = 123, // = 1.23 meter
    .hdop                                     = 123,
    .vdop                                     = 143
};

static const ble_lns_navigation_t initial_lns_navigation = {
    .remaining_dist_present       = true,
    .remaining_vert_dist_present  = true,
    .eta_present                  = true,
    .position_status              = BLE_LNS_POSITION_OK,
    .heading_source               = BLE_LNS_HEADING_SOURCE_COMPASS,
    .navigation_indicator_type    = BLE_LNS_NAV_TO_WAYPOINT,
    .waypoint_reached             = false,
    .destination_reached          = false,
    .bearing                      = 1234,   // = 12.34 degrees
    .heading                      = 2123,   // = 21.23 degrees
    .remaining_distance           = 532576, // = 53257.6 meters
    .remaining_vert_distance      = 123,    // = 12.3 meters
    .eta                          = {
                                      .year    = 2015,
                                      .month   = 7,
                                      .day     = 8,
                                      .hours   = 16,
                                      .minutes = 43,
                                      .seconds = 33
                                   }
};


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Callback function for errors in the Location Navigation Service.
 *
 * @details This function will be called in case of an error in the Location Navigation Service.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 */
static void lns_error_handler(uint32_t err_code)
{
    app_error_handler(DEAD_BEEF, 0, 0);
}


/**@brief Location Navigation event handler.
 *
 * @details This function will be called for all events of the Location Navigation Module that
 *          are passed to the application.
 *
 * @param[in]   p_evt   Event received from the Location Navigation Module.
 */
static void on_lns_evt(ble_lns_t const * p_lns, ble_lns_evt_t const * p_evt)
{
    switch(p_evt->evt_type)
    {
        case BLE_LNS_CTRLPT_EVT_INDICATION_ENABLED:
            APPL_LOG("Control Point: Indication enabled\r\n");
            break;

        case BLE_LNS_CTRLPT_EVT_INDICATION_DISABLED:
            APPL_LOG("Control Point: Indication disabled\r\n");
            break;

        case BLE_LNS_LOC_SPEED_EVT_NOTIFICATION_ENABLED:
            APPL_LOG("Location/Speed: Notification enabled\r\n");
            break;

        case BLE_LNS_LOC_SPEED_EVT_NOTIFICATION_DISABLED:
            APPL_LOG("Location/Speed: Notification disabled\r\n");
            break;

        case BLE_LNS_NAVIGATION_EVT_NOTIFICATION_ENABLED:
            APPL_LOG("Navigation: Notification enabled\r\n");
            break;

        case BLE_LNS_NAVIGATION_EVT_NOTIFICATION_DISABLED:
            APPL_LOG("Navigation: Notification disabled\r\n");
            break;

        default:
            break;
    }
}


ble_lncp_rsp_code_t on_ln_ctrlpt_evt(ble_lncp_t const * p_lncp, ble_lncp_evt_t const * p_evt)
{
    switch(p_evt->evt_type)
    {
        case LNCP_EVT_MASK_SET:
            APPL_LOG("LOC_SPEED_EVT: Feature mask set\r\n");
            break;

        case LNCP_EVT_TOTAL_DISTANCE_SET:
            APPL_LOG("LOC_SPEED_EVT: Set total distance: %d\r\n", p_evt->params.total_distance);
            break;

        case LNCP_EVT_ELEVATION_SET:
            APPL_LOG("LOC_SPEED_EVT: Set elevation: %d\r\n", p_evt->params.elevation);
            break;

        case LNCP_EVT_FIX_RATE_SET:
            APPL_LOG("POS_QUAL_EVT: Fix rate set to %d\r\n", p_evt->params.fix_rate);
            break;

        case LNCP_EVT_NAV_COMMAND:
            APPL_LOG("NAV_EVT: Navigation state changed to %d\r\n", p_evt->params.nav_cmd);
            break;

        case LNCP_EVT_ROUTE_SELECTED:
            APPL_LOG("NAV_EVT: Route selected %d\r\n", p_evt->params.selected_route);
            break;


        default:
            break;
    }

    return (LNCP_RSP_SUCCESS);
}


/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
static void battery_level_update(void)
{
    uint32_t err_code;
    uint8_t  battery_level;

    battery_level = (uint8_t)sensorsim_measure(&m_battery_sim_state, &m_battery_sim_cfg);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != BLE_ERROR_NO_TX_PACKETS) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for handling the Battery measurement timer time-out.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the time-out handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_level_update();
}


static void increment_time(ble_date_time_t * p_time)
{
    p_time->seconds++;
    if (p_time->seconds > 59)
    {
        p_time->seconds = 0;
        p_time->minutes++;
        if (p_time->minutes > 59)
        {
            p_time->minutes = 0;
            p_time->hours++;
            if (p_time->hours > 24)
            {
                p_time->hours = 0;
                p_time->day++;
                if (p_time->day > 31)
                {
                    p_time->day = 0;
                    p_time->month++;
                    if (p_time->month > 12)
                    {
                        p_time->year++;
                    }
                }
            }
        }
    }
}


static void navigation_simulation_update(void)
{
    // ugly updating of status
    m_sim_navigation.position_status           = (ble_lns_pos_status_type_t)
                                                 (
                                                    ( (uint32_t) m_sim_navigation.position_status + 1) %
                                                    ( (uint32_t) BLE_LNS_LAST_KNOWN_POSITION + 1)
                                                 );
    m_sim_navigation.heading_source            = (ble_lns_heading_source_t)
                                                 (
                                                    ( (uint32_t) m_sim_navigation.heading_source  + 1) %
                                                    ( (uint32_t) BLE_LNS_HEADING_SOURCE_COMPASS + 1)
                                                 );
    m_sim_navigation.navigation_indicator_type = (ble_lns_nav_indicator_type_t)
                                                 (
                                                    ( (uint32_t) m_sim_navigation.navigation_indicator_type + 1) %
                                                    ( (uint32_t) BLE_LNS_NAV_TO_DESTINATION + 1)
                                                 );

    m_sim_navigation.waypoint_reached    = !m_sim_navigation.waypoint_reached;
    m_sim_navigation.destination_reached = !m_sim_navigation.destination_reached;
    m_sim_navigation.bearing++;
    m_sim_navigation.heading++;
    m_sim_navigation.remaining_distance++;
    m_sim_navigation.remaining_vert_distance++;

    increment_time(&m_sim_navigation.eta);
}


static void position_quality_simulation_update(void)
{
    // ugly updating of status
    m_sim_position_quality.position_status = (ble_lns_pos_status_type_t)
                                             (
                                                ( (uint32_t) m_sim_position_quality.position_status + 1) %
                                                ( (uint32_t) BLE_LNS_LAST_KNOWN_POSITION + 1)
                                             );
    m_sim_position_quality.number_of_satellites_in_solution++;
    m_sim_position_quality.number_of_satellites_in_view++;
    m_sim_position_quality.time_to_first_fix++;
    m_sim_position_quality.ehpe++;
    m_sim_position_quality.evpe++;
    m_sim_position_quality.hdop++;
    m_sim_position_quality.vdop++;
}


/**@brief Provide simulated location and speed.
 */
static void loc_speed_simulation_update(void)
{
    // ugly updating of status
    m_sim_location_speed.position_status  = (ble_lns_pos_status_type_t)
                                            (
                                                ( (uint32_t) m_sim_location_speed.position_status + 1) %
                                                ( (uint32_t) BLE_LNS_LAST_KNOWN_POSITION + 1)
                                            );
    m_sim_location_speed.data_format      = (ble_lns_speed_distance_format_t)
                                            (
                                                ( (uint32_t) m_sim_location_speed.data_format + 1) %
                                                ( (uint32_t) BLE_LNS_SPEED_DISTANCE_FORMAT_3D + 1)
                                            );
    m_sim_location_speed.elevation_source = (ble_lns_elevation_source_t)
                                            (
                                                ( (uint32_t) m_sim_location_speed.elevation_source + 1) %
                                                ( (uint32_t) BLE_LNS_ELEV_SOURCE_OTHER + 1)
                                            );
    m_sim_location_speed.heading_source   = (ble_lns_heading_source_t)
                                            (
                                                ( (uint32_t) m_sim_location_speed.heading_source + 1) %
                                                ( (uint32_t) BLE_LNS_HEADING_SOURCE_COMPASS + 1)
                                            );
    m_sim_location_speed.total_distance++;
    m_sim_location_speed.latitude++;
    m_sim_location_speed.longitude++;
    m_sim_location_speed.elevation++;
    m_sim_location_speed.heading++;
    m_sim_location_speed.rolling_time++;

    increment_time(&m_sim_location_speed.utc_time);
}


/**@brief Location and navigation time-out handler.
 *
 * @details This function will be called each time the location and navigation measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the time-out handler.
 */
static void loc_and_nav_timeout_handler(void * p_context)
{
    uint32_t err_code;

    UNUSED_PARAMETER(p_context);

    loc_speed_simulation_update();
    position_quality_simulation_update();
    navigation_simulation_update();

    err_code = ble_lns_loc_speed_send(&m_lns);
    if (err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }

    err_code = ble_lns_navigation_send(&m_lns);
    if (err_code != NRF_ERROR_INVALID_STATE)
    {
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_loc_and_nav_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                loc_and_nav_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief GAP initialization.
 *
 * @details This function shall be used to set up all the necessary GAP (Generic Access Profile)
 *          parameters of the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_OUTDOOR_SPORTS_ACT_LOC_AND_NAV_DISP);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Initialize services that will be used by the application.
 *
 * @details Initialize the Location and Navigation, Battery and Device Information services.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_lns_init_t lns_init;
    ble_bas_init_t bas_init;
    ble_dis_init_t dis_init;

    memset(&lns_init, 0, sizeof(lns_init));

    lns_init.evt_handler        = on_lns_evt;
    lns_init.lncp_evt_handler   = on_ln_ctrlpt_evt;
    lns_init.error_handler      = lns_error_handler;

    lns_init.is_position_quality_present = true;
    lns_init.is_control_point_present    = true;
    lns_init.is_navigation_present       = true;

    lns_init.available_features     = BLE_LNS_FEATURE_INSTANT_SPEED_SUPPORTED                 |
                                      BLE_LNS_FEATURE_TOTAL_DISTANCE_SUPPORTED                |
                                      BLE_LNS_FEATURE_LOCATION_SUPPORTED                      |
                                      BLE_LNS_FEATURE_ELEVATION_SUPPORTED                     |
                                      BLE_LNS_FEATURE_HEADING_SUPPORTED                       |
                                      BLE_LNS_FEATURE_ROLLING_TIME_SUPPORTED                  |
                                      BLE_LNS_FEATURE_UTC_TIME_SUPPORTED                      |
                                      BLE_LNS_FEATURE_REMAINING_DISTANCE_SUPPORTED            |
                                      BLE_LNS_FEATURE_REMAINING_VERT_DISTANCE_SUPPORTED       |
                                      BLE_LNS_FEATURE_EST_TIME_OF_ARRIVAL_SUPPORTED           |
                                      BLE_LNS_FEATURE_NUM_SATS_IN_SOLUTION_SUPPORTED          |
                                      BLE_LNS_FEATURE_NUM_SATS_IN_VIEW_SUPPORTED              |
                                      BLE_LNS_FEATURE_TIME_TO_FIRST_FIX_SUPPORTED             |
                                      BLE_LNS_FEATURE_EST_HORZ_POS_ERROR_SUPPORTED            |
                                      BLE_LNS_FEATURE_EST_VERT_POS_ERROR_SUPPORTED            |
                                      BLE_LNS_FEATURE_HORZ_DILUTION_OF_PRECISION_SUPPORTED    |
                                      BLE_LNS_FEATURE_VERT_DILUTION_OF_PRECISION_SUPPORTED    |
                                      BLE_LNS_FEATURE_LOC_AND_SPEED_CONTENT_MASKING_SUPPORTED |
                                      BLE_LNS_FEATURE_FIX_RATE_SETTING_SUPPORTED              |
                                      BLE_LNS_FEATURE_ELEVATION_SETTING_SUPPORTED             |
                                      BLE_LNS_FEATURE_POSITION_STATUS_SUPPORTED;


    m_sim_location_speed   = initial_lns_location_speed;
    m_sim_position_quality = initial_lns_pos_quality;
    m_sim_navigation       = initial_lns_navigation;

    lns_init.p_location_speed   = &m_sim_location_speed;
    lns_init.p_position_quality = &m_sim_position_quality;
    lns_init.p_navigation       = &m_sim_navigation;

    lns_init.loc_nav_feature_security_req_read_perm  = SEC_OPEN;
    lns_init.loc_speed_security_req_cccd_write_perm  = SEC_OPEN;
    lns_init.position_quality_security_req_read_perm = SEC_OPEN;
    lns_init.navigation_security_req_cccd_write_perm = SEC_OPEN;
    lns_init.ctrl_point_security_req_write_perm      = SEC_OPEN;
    lns_init.ctrl_point_security_req_cccd_write_perm = SEC_OPEN;

    err_code = ble_lns_init(&m_lns, &lns_init);
    APP_ERROR_CHECK(err_code);

    ble_lns_route_t route1 = {.route_name = "Route one"};
    err_code = ble_lns_add_route(&m_lns, &route1);

    ble_lns_route_t route2 = {.route_name = "Route two"};
    err_code = ble_lns_add_route(&m_lns, &route2);

    // Initialize Battery Service
    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Initialize the simulators.
 */
static void sim_init(void)
{
    // battery simulation
    m_battery_sim_cfg.min          = MIN_BATTERY_LEVEL;
    m_battery_sim_cfg.max          = MAX_BATTERY_LEVEL;
    m_battery_sim_cfg.incr         = BATTERY_LEVEL_INCREMENT;
    m_battery_sim_cfg.start_at_max = true;

    sensorsim_init(&m_battery_sim_state, &m_battery_sim_cfg);
}


/**@brief Start application timers.
 */
static void application_timers_start(void)
{
    uint32_t err_code;

    // Start application timers
    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_loc_and_nav_timer_id, LOC_AND_NAV_DATA_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Connection Parameters Module handler.
 *
 * @details This function will be called for all events in the Connection Parameters Module that
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Connection Parameters module error handler.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Initialize the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = m_lns.loc_speed_handles.cccd_handle;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events that are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Application's BLE Stack event handler.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);

            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        default:
            break;
    }

    APP_ERROR_CHECK(err_code);
}


/**@brief Dispatches a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    ble_lns_on_ble_evt(&m_lns, p_ble_evt);
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}


/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the Device Manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    APP_ERROR_CHECK(event_result);
    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.lesc         = SEC_PARAM_LESC;
    register_param.sec_param.keypress     = SEC_PARAM_KEYPRESS;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Advertising functionality initialization.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and LEDs.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**@brief Application main function.
 */
int main(void)
{
    uint32_t err_code;
    bool erase_bonds;

    err_code = NRF_LOG_INIT();
    APP_ERROR_CHECK(err_code);

    // Initialize
    timers_init();
    buttons_leds_init(&erase_bonds);

    ble_stack_init();
    device_manager_init(erase_bonds);
    gap_params_init();
    advertising_init();
    services_init();
    sim_init();
    conn_params_init();

    // Start execution
    application_timers_start();

    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);

    APPL_LOG("Location and Navigation App started\r\n");

    // Enter main loop
    for (;;)
    {
        power_manage();
    }
}


/**
 * @}
 */


#ifndef _BLEAPP_H_
#define _BLEAPP_H_

//92540000-d6ed-4617-a4b4-d749f57710ce
#define BLE_UUID_SH1_BASE_UUID              {{0xce, 0x10, 0x77, 0xf5, 0x49, 0xd7, 0xb4, 0xa4, 0x17, 0x46, 0xed, 0xd6, 0x00, 0x00, 0x54, 0x92}}

#define CHAR_PERM_READ 1
#define CHAR_PERM_WRITE 2
#define CHAR_FEAT_NOTIFY 4
#define CHAR_FEAT_INDICATE 8

typedef void (*bleapp_evt_cb_t)( ble_evt_t * p_ble_evt );

/**@brief Function to reset device into bootloader
 */
void bleapp_jumptoBootloader(void);

/**@brief Function to disconnect from peer device
 */
void bleapp_kickout(void);

/**@brief Function for registering peripheral events outside of this file
 * 
 * @param[in] cb  callback to be registered
 */
void bleapp_evt_register(bleapp_evt_cb_t cb);

/**@brief Function for adding characteristics
 *  
 * @param[in] uuid       characteristic uuid
 * @param[in] data       charateristic attributes value
 * @param[in] data_len   length of data
 * @param[in] flags      read-write-notify permissions 
 * @param[in] p_handles  gatt characteristic handle
 * 
 */
uint32_t bleapp_char_add(uint16_t                        uuid,
                         uint8_t                       * data,
                         uint32_t                        data_len,
                         uint32_t flags,
                         ble_gatts_char_handles_t      * p_handles);

/**@brief Function for setting device advertised name
 *
 * @param[in]   *name      pointer to advertised name of device
 */
void bleapp_setName(uint8_t const * name);

/**@brief Function for initializing the advertising and the scanning.
 */
void bleapp_advertising_scan_start(void);

/**@brief Function to query peripheral connection status
 */
bool bleapp_isConnected(void);

/**@brief Function to query peripheral device RSSI level 
 * 
 * @details gives rssi to smartphone
 */
float bleapp_getRSSI(void);

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupts.
 */
void ble_stack_init(void);

/**@brief Function for initialising smarthalo bluetooth application
 */
void bleapp_init(void);

#endif

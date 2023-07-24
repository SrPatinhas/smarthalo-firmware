
#ifndef _BLEAPP_H_
#define _BLEAPP_H_

//92540000-d6ed-4617-a4b4-d749f57710ce
#define BLE_UUID_SH1_BASE_UUID              {{0xce, 0x10, 0x77, 0xf5, 0x49, 0xd7, 0xb4, 0xa4, 0x17, 0x46, 0xed, 0xd6, 0x00, 0x00, 0x54, 0x92}}

#define CHAR_PERM_READ 1
#define CHAR_PERM_WRITE 2
#define CHAR_FEAT_NOTIFY 4
#define CHAR_FEAT_INDICATE 8

typedef void (*bleapp_evt_cb_t)( ble_evt_t * p_ble_evt );

void bleapp_jumptoBootloader(void);
void bleapp_shutdown(void);
void bleapp_kickout(void);

void bleapp_evt_register(bleapp_evt_cb_t cb);
uint32_t bleapp_char_add(uint16_t                        uuid,
                         uint8_t                       * data,
                         uint32_t                        data_len,
//                         bool perm_read,
//                         bool perm_write,
//                         bool feature_notify,
                         uint32_t flags,
                         ble_gatts_char_handles_t      * p_handles);

void bleapp_setName(uint8_t const * name);

void bleapp_advertising_start(void);

bool bleapp_isConnected(void);

double bleapp_getRSSI(void);

void ble_stack_init(void);
void bleapp_init(void);

void bleapp_gap_params_update(void);

#endif

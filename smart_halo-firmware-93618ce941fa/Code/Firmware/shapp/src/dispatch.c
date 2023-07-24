
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ble_srv_common.h"
//#include "app_error.h"
#include "scheduler.h"
#include "bleapp.h"
#include "bslink.h"

#include "dispatch.h"


#define DISP_ENTRIES_MAX 10

typedef struct {
    disp_cmdfn_t dispatch;
    uint8_t service;
    uint8_t permission;
} disp_entry_t;

disp_entry_t disp_entries[DISP_ENTRIES_MAX];

uint8_t disp_permission = 0;

void disp_setPermissionMask(uint8_t permission) {
    disp_permission = permission;
}
uint8_t disp_getPermissionMask() {
    return disp_permission;
}
/*
void disp_evt_cb( ble_evt_t * p_ble_evt ) {
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GAP_EVT_CONNECTED:
            //printf("reset disp_permission\r\n");
            disp_permission = 0;
            break;
        default:
            break;
    }

}
*/
void disp_listen(uint8_t *buf, uint32_t len) {

    uint8_t reply[1];
    uint32_t ptr = 0;

    for(int i = 0; i < DISP_ENTRIES_MAX; i++) {
        if(disp_entries[i].dispatch && disp_entries[i].service == buf[0]) {
            if((disp_entries[i].permission == 0) || (disp_entries[i].permission & disp_permission)) {
                disp_entries[i].dispatch(buf+1, len-1);
            } else {
                reply[ptr++] = RET_DENIED;
                bslink_write(reply, ptr);
            }
            return;
        }
    }
    reply[ptr++] = RET_UNIMPLEMENTED;
    bslink_write(reply, ptr);

}

void disp_register(uint8_t service, uint8_t permission, disp_cmdfn_t dispatch) {
    for(int i = 0; i < DISP_ENTRIES_MAX; i++) {
        if(!disp_entries[i].dispatch) {
            disp_entries[i].dispatch = dispatch;
            disp_entries[i].service = service;
            disp_entries[i].permission = permission;
            break;
        }
    }

}

void disp_respond(comm_return_t type){
    uint8_t reply[1];
    uint32_t ptr = 0;
    reply[ptr++] = type;
    bslink_write(reply, ptr);         
}

void disp_init() {
    for(int i = 0; i < DISP_ENTRIES_MAX; i++) {
        disp_entries[i].dispatch = NULL;
        disp_entries[i].service = 0;
        disp_entries[i].permission = 0;
    }

    //bleapp_evt_register(disp_evt_cb);
	bslink_listen(disp_listen);
}
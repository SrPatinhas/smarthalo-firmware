#ifndef _DISPATCH_H
#define _DISPATCH_H

typedef enum {
    RET_OK = 0,
    RET_FAIL, //General fail or unknown
    RET_DENIED,
    RET_UNIMPLEMENTED,
} comm_return_t;

void disp_setPermissionMask(uint8_t permission);
uint8_t disp_getPermissionMask(void);

typedef void (*disp_cmdfn_t)(uint8_t *, uint32_t );

void disp_register(uint8_t service, uint8_t permission, disp_cmdfn_t dispatch);

void disp_respond(comm_return_t type);

void disp_init(void);

#endif
/*
    Magic Control Block
 */

#ifndef __MCB_H__
#define __MCB_H__

#include <stdbool.h>
#include <stdint.h>
#include "BootLoaderVersion.h"

/*! @brief  FW image metadata
*/
typedef struct {
    uint32_t    crc;
    uint32_t    len;
} fwMetadata_t;

#define SERIAL_LEN  12

/*! @brief  magic control block structure

    The magic control block (MCB) is a flags field
    and an array of firmware metadata structs
*/
typedef struct __attribute__ ((packed)) {
    uint32_t        flags;
    fwMetadata_t    fwData[3];
    blVersion_t     version;
    uint8_t         serial[SERIAL_LEN];
} mcb_t;

#define MCB_INTERNAL_IDX        0
#define MCB_EXT_UPDATE_IDX      1
#define MCB_EXT_BACKUP_IDX      2

#define MCB_CRC(x)      MCB->fwData[(x)].crc
#define MCB_LEN(x)      MCB->fwData[(x)].len

bool McbFirstboot();
bool McbUpdateGoldenCRC();
bool McbUpdateInteralMetadata(int idx);
bool McbUpdateMCBWithUpdate(uint32_t crc, uint32_t len);

extern mcb_t *MCB;

#endif // __MCB_H__
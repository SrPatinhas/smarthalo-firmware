/*!
    @file BootLoaderVersion.h

    Version number object and defines for the bootloader

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#ifndef __BOOTLOADERVERSION_H__
#define __BOOTLOADERVERSION_H__

#include <stdint.h>
#include "log.h"

#define BL_VERS_MAJOR       2
#define BL_VERS_MINOR       0
#define BL_VERS_REVISION    4
#define BL_VERS_COMMIT      0

typedef union __attribute__ ((packed)) {
    struct {
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        uint8_t blVersionCommit;
        uint8_t blVersionRevision;
        uint8_t blVersionMinor;
        uint8_t blVersionMajor;
#else
        uint8_t blVersionMajor;
        uint8_t blVersionMinor;
        uint8_t blVersionRevision;
        uint8_t blVersionCommit;
#endif
    };
    uint32_t    blVersion;
} blVersion_t;

void printBootLoaderVersion(blVersion_t *vers);

#endif // __BOOTLOADERVERSION_H__

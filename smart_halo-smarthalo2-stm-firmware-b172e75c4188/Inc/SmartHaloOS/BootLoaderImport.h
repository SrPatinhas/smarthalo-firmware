/*!
    @file BootLoaderImport.h

    Imports from boot loader -- selective copies of source code. This is a
    hack to get around that the bootloader repository is not linked to this
    one and making code sharing difficult. If/when the repositories get
    linked, this stuff should be replaced with direct includes of bootloader
    headers.

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019-2020 SmartHalo Inc
*/

#ifndef __BOOTLOADERIMPORT_H__
#define __BOOTLOADERIMPORT_H__

#include <stdint.h>

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

/*
 * Magic Control Block (mcb.h)
 */
#define MCB_MAGIC       0x8005800           // magic control block location

extern mcb_t *MCB;

// Bootloader flags (Boot.h)
typedef union {
    struct {
        uint32_t    halt:1;             ///< do not boot firmware and halt/low-power
        uint32_t    forceGolden:1;      ///< reinstall the Golden Firmware
        uint32_t    ignoreWatchdog:1;   ///< do not do anything special on watchdog resets
        uint32_t    normalReboot:1;     ///< false if firmware crashed
        uint32_t    armedBoot:1;        ///< true if alarm armed when rebooted
        uint32_t    disableWatchdog:1;  ///< true will not start watchdog when launching FW
        uint32_t    firstBoot:1;        ///< true set by bootloader to tell FW first ever boot
        uint32_t    reserved:7;
        uint32_t    softWD:1;           ///< true if software watchdog tripped
        uint32_t    crash:1;            ///< true if firmware crashed -- not reset by bootloader
        uint32_t    magic:16;           ///< magic number used to indicate correct initialization
    };
    uint32_t    all;
} BootFlags_t;

// boot reason bits (Boot.h)
typedef enum {
    eLPWRRST    = 0x01, // low power reset
    eWWDGRST    = 0x02, // window watchdog reset
    eIWDGRST    = 0x04, // independent watchdog reset
    eSFTRST     = 0x08, // software reset
    ePINRST     = 0x10, // external pin reset
    eBORRST     = 0x20, // brown out register reset
    eUNKNOWN    = 0x80, // none of the above
} eBootReason_t;

extern BootFlags_t *BF;
extern eBootReason_t *BR;
#endif // __BOOTLOADERIMPORT_H__


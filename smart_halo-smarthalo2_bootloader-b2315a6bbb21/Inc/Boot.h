/*!
    @file Boot.h

    Description here

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/
#ifndef __BOOT_H__
#define __BOOT_H__

#include "main.h"

/*  @brief  Boot flags

    This structure lives in RAM.
    Controls some boot behaviours, described below. These
    behaviours are active if the bit is set.
*/
typedef union {
    struct {
        uint32_t    halt:1;             ///< do not boot firmware and halt/low-power
        uint32_t    forceGolden:1;      ///< reinstall the Golden Firmware
        uint32_t    ignoreWatchdog:1;   ///< do not do anything special on watchdog resets
        uint32_t    normalReboot:1;     ///< false if firmware crashed
        uint32_t    armedBoot:1;        ///< true if alarm armed when rebooted
        uint32_t    disableWatchdog:1;  ///< true will not start watchdog when launching FW
        uint32_t    firstBoot:1;        ///< true set by bootloader to tell FW first ever boot
        uint32_t    reserved:9;
        uint32_t    magic:16;           ///< magic number used to indicate correct initialization
    };
    uint32_t    all;
} BootFlags_t;

/*  @brief  Boot reason bits

    As boot time, a reset register on the STM is read and the following bits
    are set accordingly.
*/
typedef enum {
    eLPWRRST    = 0x01, // low power reset
    eWWDGRST    = 0x02, // window watchdog reset
    eIWDGRST    = 0x04, // independent watchdog reset
    eSFTRST     = 0x08, // software reset
    ePINRST     = 0x10, // external pin reset
    eBORRST     = 0x20, // brown out register reset
    eUNKNOWN    = 0x80, // none of the above
} eBootReason_t;

#define BF_MAGIC    0xFEED

bool	BootInit(void);
bool    BootIsNormalBoot(void);
bool    BootValidateCRC32(uint32_t u32Expected, uint32_t u32Len, bool * pbCRCGood);
bool    BootResetBoard(bool halt);
void    BootJumpToUser();

extern BootFlags_t *BF;
extern eBootReason_t *BR;

#endif // __BOOT_H__

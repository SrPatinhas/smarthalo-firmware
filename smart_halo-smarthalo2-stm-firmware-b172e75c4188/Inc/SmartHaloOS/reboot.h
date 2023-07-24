/*!
    @file reboot.h

    Prototype for the polite reboot function

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#ifndef __REBOOT_H__
#define __REBOOT_H__

#include <stdint.h>

typedef enum {
    eRebootNot = 0,
    eRebootNormal,
    eRebootUnpaired,
    eRebootSystemUtilitiesStack,        // no longer used
    eRebootSystemUtilitiesLowBattery,
    eRebootSystemUtilitiesDFU,
    eRebootDisableWatchdog,
    eRebootShellHalt,
    eRebootShellReboot,
    eRebootShellForceGolden,
} eRebootReason_t;

typedef enum {
    eMAIN = 10,
    eQSPIFLASH,
    eSUBSCRIPTIONHELPER,
    eSYSTEMUTILITIES,
    eOLEDDRIVER,
    eANIMATIONSLIBRARY,
} eSourceFile_t;

#define SOFT_CRASH(f)    do { soft_crash((f) << 16 | __LINE__); } while (0)

void reboot(eRebootReason_t rebootreason);
void reboot_nosync(void);
void soft_crash(uint32_t arg);

#endif // __REBOOT_H__
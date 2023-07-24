/*!
    @file FirmwareUpdate.h

    Interface for firmware update function.

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#ifndef __FIRMWARE_UPDATE_H__
#define __FIRMWARE_UPDATE_H__

#define EXT_FLASH_BACKUP_START_OFFSET   0
#define EXPECTED_FIRMWARE_UPDATE_PAYLOAD_SIZE 64
#define EXT_FLASH_BACKUP_END_OFFSET     0x0003ffff
#define EXT_FLASH_BACKUP_LEN            (EXT_FLASH_BACKUP_END_OFFSET - EXT_FLASH_BACKUP_START_OFFSET)
#define EXT_FLASH_UPDATE_START_OFFSET   0x00040000
#define EXT_FLASH_UPDATE_END_OFFSET     0x0007ffff
#define EXT_FLASH_UPDATE_LEN            (EXT_FLASH_UPDATE_END_OFFSET - EXT_FLASH_UPDATE_START_OFFSET)

bool handleInstall_FirmwareUpdate(uint16_t u16length, void *payload);
void handleData_FirmwareUpdate(uint16_t u16length, void *payload);
void handleCRC_FirmwareUpdate(uint16_t u16length, void *payload);



#endif // __FIRMWARE_UPDATE_H__

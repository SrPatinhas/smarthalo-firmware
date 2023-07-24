/*!
    @file       SHftp.h

    @brief      Experimental over-the-air filesystem commands

    @details    Commands for the mobile app to manage assets (files)
                in the external flash filesystem.

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#ifndef __SHFTP_H__
#define __SHFTP_H__

#include <stdint.h>
#include "CommunicationTask.h"
#include "spiffs.h"

#define MAXFILENAME (32)    // including trailing NULL

/*! @brief  State structure for file transfers

            Common across SHftp and DFU
*/
typedef struct {
    uint32_t    crc;        ///< CRC of incoming file
    uint32_t    len;        ///< length of incoming file
    uint32_t    nextMsg;    ///< next expected packet
    spiffs_file fh;         ///< open filehandle for incoming file
    char        name[MAXFILENAME]; ///< filename of incoming file
    bool        complete;   ///< set to true when transfer is complete
} SHftpFile_t;

/*! @brief  message layout for EXP_FS_PUT command
*/
typedef struct __attribute__ ((packed)) {
    uint32_t    crc;        ///< CRC of incoming file
    uint32_t    len;        ///< length of incomping file
    char        name[];     ///< name of file (up to 31 characters)
} FsPutMsg_t;

/*! @brief  message structure of reply to EXP_FS_PUT command
*/
typedef struct __attribute__ ((packed)) {
    uint8_t     status;     ///< eCOM_RETURN_STATUS_OK, eCOM_RETURN_STATUS_FAIL, eCOM_RETURN_STATUS_UNNECESSARY
    uint16_t    msgId;      ///< msg/seq ID of next expected msg - big endian
} FsPutReply_t;

/*! @brief  message layout for EXP_FS_PUT_DATA command
*/
typedef struct __attribute__ ((packed)) {
    uint16_t    msgId;      ///< packet ID#, 16bit MSB
    uint8_t     len;        ///< length in bytes, 64 or less
    uint8_t     data[];     ///< generic pointer to data payload
} FsPutDataMsg_t;

/*! @brief  message layout for EXP_FS_DELETE command
*/
typedef struct __attribute__ ((packed)) {
    char    name[0];     ///< name of the file to be removed
} FsDeleteMsg_t;

// the BLE command interface
void handleFsPut_SHftp(uint16_t len, void *payload);        // EXP_FS_PUT
void handleFsPutData_SHftp(uint16_t len, void *payload);    // EXP_FS_PUT_DATA
void handleFsDelete_SHftp(uint16_t len, void *payload);     // EXP_FS_DELETE

// common code used by SHftp and DFU
eCOM_RETURN_STATUS_t startFile_SHftp(SHftpFile_t *fp, char *filename, uint32_t crc, uint32_t len);
eCOM_RETURN_STATUS_t writeData_SHftp(SHftpFile_t *fp, uint16_t len, void *payload);

#endif // __SHFTP_H__
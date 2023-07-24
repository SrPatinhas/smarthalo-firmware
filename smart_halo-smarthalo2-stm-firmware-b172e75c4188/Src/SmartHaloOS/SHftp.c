/*!
    @file       SHftp.c

    @brief		Experimental over-the-air filesystem commands

    @details    Commands for the mobile app to manage assets (files)
                in the external flash filesystem.

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
 */

#include <string.h>
#include "SHftp.h"
#include "CommunicationTask.h"
#include "Shell.h"
#include "FSUtil.h"
#include "SHftp.h"

#define EXPECTED_MSG_LENGTH (64)

static SHftpFile_t ftpState = {
        .fh = -1,
};

static void resetFtpState(SHftpFile_t *fp)
{
    memset(fp, 0, sizeof(*fp));
    fp->fh = -1;
}

/*! @brief      Send BLE response to EXP_FS_PUT command
    @param[IN]  msg/seq ID of next expected packet
    @return     returns result of BLE xmit attempt
 */
static bool fsPutSendAck(uint16_t nextMsg)
{
    FsPutReply_t pkt = {
            .status = eCOM_RETURN_STATUS_OK,
            .msgId = __REV16(nextMsg)
    };

    return sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_RESPONSE, sizeof(FsPutReply_t), (uint8_t *)&pkt);
}

/*! @brief      Generic routine to open/create file for incoming BLE xfer

                Common code for handling DEVICE_STM_DFU_CRC and EXP_FS_PUT commands.

    @param[in]  fp pointer to a state structure containing name, crc, length of incoming file
    @param[in]  filename name inside incoming BLE message
    @param[in]  crc file crc (decoded from incoming BLE message)
    @param[in]  len file length (decoded from incoming BLE message)
    @return     one of eCOM_RETURN_STATUS_OK, eCOM_RETURN_STATUS_UNNECESSARY or eCOM_RETURN_STATUS_FAIL
 */
eCOM_RETURN_STATUS_t startFile_SHftp(SHftpFile_t *fp, char *filename, uint32_t crc, uint32_t len)
{
    spiffs_stat statbuf;
    fsMeta_t    *meta = (fsMeta_t *)&statbuf.meta;

    // TODO check to see that we will have enough free space to receive the file

    // if we don't already have an open file handle,
    // check if the file exists
    if (fp->fh < 0 && stat(filename, &statbuf) == 0) {
        if (meta->crc == 0xffffffff) {
            // we already have the file but it's incomplete
            // we _might_ be able to recover here, but safer
            // to assume that what we have is garbage
            unlink(filename);
            log_Shell("%s: discarding %s", __func__, filename);
        } else if (statbuf.size == len && meta->crc == crc) {
            // already have it, tell the app but fill in the state structure anyway
            strcpy(fp->name, filename);
            fp->crc = crc;
            fp->len = len;
            fp->nextMsg = (statbuf.size / 64) + 1;
            fp->complete = true;
            log_Shell("%s: returning eCOM_RETURN_STATUS_UNNECESSARY", __func__);
            return eCOM_RETURN_STATUS_UNNECESSARY;
        }
    }

    // if new file name or metadata doesn't match,
    // close the in-progress file
    if (strcmp(fp->name, filename) || fp->crc != crc || fp->len != len) {
        if (fp->fh >= 0) {
            close(fp->fh);
            fp->fh = -1;
        }
    }

    // we have no open file, reset ftpState and open it
    if (fp->fh < 0) {
        strcpy(fp->name, filename);
        fp->crc = crc;
        fp->len = len;
        fp->nextMsg = 0;
        fp->fh = open(fp->name, O_RDWR | O_CREAT | O_TRUNC, 0);
        if (fp->fh < 0) {
            log_Shell("%s: open of %s failed", __func__, fp->name);
            return eCOM_RETURN_STATUS_FAIL;
        }
        // reset crc metadata here?
    }

    log_Shell("%s: returning OK: %ld", __func__, fp->nextMsg);
    return eCOM_RETURN_STATUS_OK;
}

/*! @brief      Handle EXP_FS_PUT message

                Decode EXP_FS_PUT message from the mobile app.
                Call startFile_SHftp to potentially open the named file.
                Send appropriate reply via BLE

    @param[IN]  len (ignored)
    @param[IN]  payload pointer to packet data
    @return     sends eCOM_RETURN_STATUS_OK, eCOM_RETURN_STATUS_FAIL,
                or eCOM_RETURN_STATUS_UNNECESSARY back to the mobile app
 */
void handleFsPut_SHftp(uint16_t len, void *payload)
{
    FsPutMsg_t  *msg = (FsPutMsg_t *)payload;
    uint32_t    crc = __REV(msg->crc);
    uint32_t    length = __REV(msg->len);
    char        name[MAXFILENAME];

    strlcpy(name, msg->name, sizeof(name));

    log_Shell("%s: CRC: %08lx, len: %lu, name: %s", __func__, crc, length, name);
    xxd_Shell(payload, len);

    eCOM_RETURN_STATUS_t re = startFile_SHftp(&ftpState, name, crc, length);
    switch (re) {
        case eCOM_RETURN_STATUS_UNNECESSARY:
        case eCOM_RETURN_STATUS_FAIL:
            genericResponse_CommunicationTask(re);
            break;
        case eCOM_RETURN_STATUS_OK:
            fsPutSendAck(ftpState.nextMsg);
            break;
        default:
            genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
            break; 
    }

    return;
}

/*! @brief  Receive a chunk (64bytes) of a file update

            This function is a generic to handle either DEVICE_STM_DFU_DATA
            or EXP_FS_PUT_DATA commands.
            It will be called repeatedly as chunks of a file are
            received over BLE.
            Upon receipt of the final packet, the CRC is validated and the
            file is closed.

    @param[IN]  fp pointer to the state structure
    @param[IN]  len packet length (ignored)
    @param[IN]  payload pointer to packet data
    @return     eCOM_RETURN_STATUS_OK or eCOM_RETURN_STATUS_FAIL
 */
eCOM_RETURN_STATUS_t writeData_SHftp(SHftpFile_t *fp, uint16_t len, void *payload)
{
    FsPutDataMsg_t  *msg = (FsPutDataMsg_t *)payload;
    uint16_t        msgId = __REV16(msg->msgId);
    uint32_t        left;
    int32_t         ret;
    bool            problem = false;
    extern bool     bBLEdebug;

    if (fp->fh < 0) {
        log_Shell("%s: receiving data packet but no open file", __func__);
        xxd_Shell(payload, len);
        problem = true;
        goto send_reply;
    }

    if (fp->nextMsg != msgId) {
        log_Shell("%s: failing fp->nextMsg: %lu, msgId: %u", __func__, fp->nextMsg, msgId);
        xxd_Shell(payload, len);
        problem = true;
        goto send_reply;
    }

    left = fp->len - ((msgId * EXPECTED_MSG_LENGTH) + msg->len);

    if (bBLEdebug) {
        log_Shell("%s: pkt len: %u, msgId: %u, msg->len: %u, left: %lu", __func__, len, msgId, msg->len, left);
        xxd_Shell(payload, len);
    }

    ret = write(fp->fh, msg->data, msg->len);
    if (ret < 0) {
        log_Shell("%s: write failed, returned %ld", __func__, ret);
        log_Shell("%s: removing and closing", __func__);
        if (fremove(fp->fh) < 0) {
            log_Shell("%s: fremove on fh: %d (%s) failed", __func__, fp->fh, fp->name);
        }
        // Experimentally, we know that fremove() closes/invalidates the file handle
        // If that semantic changes, we'll need to add a close(fp->fh) here
        resetFtpState(fp);
        problem = true;
        goto send_reply;
    }

    fp->nextMsg = msgId + 1;

    if (left == 0) {
        spiffs_stat statbuf;
        fsMeta_t    meta;
        uint32_t    vrfyCrc;

        fstat(fp->fh, &statbuf);
        if (statbuf.size == fp->len) {
            vrfyCrc = fcrc_FSUtil(fp->fh);
            if (vrfyCrc == fp->crc) {
                meta.crc = vrfyCrc;
                meta.version = 0;
                if ((ret = fupdateMeta_FSUtil(fp->fh, &meta)) < 0) {
                    log_Shell("%s: update metadata failed: %ld", __func__, ret);
                    problem = true;
                }
            } else {
                log_Shell("%s: crc doesn't match, got %08lx, expected %08lx", __func__, vrfyCrc, fp->crc);
                problem = true;
            }
        } else {
            log_Shell("%s: sizes don't match... %d vs %lu", __func__, statbuf.size, fp->len);
            problem = true;
        }
        ret = close(fp->fh);
        fp->fh = -1;
        if (ret < 0) {
            log_Shell("%s: close failed: %ld", __func__, ret);
            problem = true;
        } else {
            log_Shell("%s: closed %s", __func__, fp->name);
            fp->complete = true;
        }
    }
    send_reply:
    return problem ? eCOM_RETURN_STATUS_FAIL : eCOM_RETURN_STATUS_OK;
}

/*! @brief  Receive a chunk (64bytes) of a file update

            This function is called in response to EXP_FS_PUT_DATA commands.

    @param[IN]  len packet length (ignored)
    @param[IN]  payload pointer to packet data
    @return     eCOM_RETURN_STATUS_OK or eCOM_RETURN_STATUS_FAIL by BLE to mobile app
 */
void handleFsPutData_SHftp(uint16_t len, void *payload)
{
    genericResponse_CommunicationTask(writeData_SHftp(&ftpState, len, payload));
}

/*! @brief      Handle EXP_FS_DELETE message

                Delete the named file from the filesystem.

    @param[IN]  len packet length (ignored)
    @param[IN]  payload pointer to packet data
    @return     eCOM_RETURN_STATUS_OK or eCOM_RETURN_STATUS_FAIL
 */
void handleFsDelete_SHftp(uint16_t len, void *payload)
{
    int             res;
    char            file[MAXFILENAME];
    FsDeleteMsg_t   *msg = (FsDeleteMsg_t *)payload;

    strlcpy(file, msg->name, sizeof(file));

    log_Shell("%s: name: %s", __func__, file);
    xxd_Shell(payload, len);

    res = unlink(file);
    genericResponse_CommunicationTask(res == 0 ? eCOM_RETURN_STATUS_OK : eCOM_RETURN_STATUS_FAIL);
}

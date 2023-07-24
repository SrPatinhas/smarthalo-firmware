/*!
    @file FirmwareUpdate.c

    Code to handle firmware updates. Updates are driven by the
    following BLE commands:

        DEVICE_STM_DFU_CRC
        DEVICE_STM_DFU_DATA
        DEVICE_STM_DFU_INSTALL

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
 */

#include <CommunicationTask.h>
#include <stdbool.h>
#include <stdint.h>
#include <Shell.h>
#include "stm32l4xx_hal.h"
#include "main.h"
#include "crc.h"
#include "qspiflash.h"
#include <SystemUtilitiesTask.h>
#include "reboot.h"
#include "OSS/miniz.h"
#include "spiffs.h"
#include "FSUtil.h"
#include "FirmwareUpdate.h"
#include "SHftp.h"
#include "gzutil.h"

#define EXT_FLASH_MAX_WR    256
#define EXT_FLASH_MAX_RD    1024

#define UPDATE_FILE         "DFU.gz"

#define my_min(a,b) (((a) < (b)) ? (a) : (b))

/*! @brief  message structure for DEVICE_STM_DFU_CRC command
 */
struct CRCMsg {
    uint32_t    crc;        ///< CRC of incoming gzip file (big endian)
    uint32_t    len;        ///< total length of incoming gzip file (big endian)
};

/*! @brief  message structure of reply to DEVICE_STM_DFU_CRC command
 */
struct CRCAck {
    uint8_t     status;     ///< either eCOM_RETURN_STATUS_OK or eCOM_RETURN_STATUS_FAIL
    uint16_t    msgId;      ///< msg/seq ID of next expected msg - big endian
} __attribute__((packed));

/*! @brief  message layout for DEVICE_STM_DFU_DATA command
 */
struct DataMsg {
    uint16_t    packetID;   ///< packet ID#, 16bit MSB
    uint8_t     length;     ///< length in bytes, 64 or less
    uint8_t     data[];     ///< generic pointer to data payload
} __attribute__ ((packed));

/*! @brief  Private state data for firmware update
 */
typedef struct {
    SHftpFile_t gz;                         ///< state structure for incoming GZ file
    uint32_t    updateCRC;                  ///< cached CRC of incoming FW
    uint32_t    updateLen;                  ///< cached length of incoming FW
    uint8_t     inBuf[EXT_FLASH_MAX_RD];    ///< buffer for reading from ext flash
    uint32_t    addr;                       ///< current offset into external flash
    uint8_t     filename[256];              ///< original filename of gzip'ed firmware image
} DFUstate_t;

// Global because too big for stacks
static DFUstate_t   DFUstate;
#ifdef PARANOID
static uint8_t      scratchBuf[EXT_FLASH_MAX_RD];
#endif

/*! @brief  Reset the private state structure
 */
static void DFUresetState()
{
    // console_send("%s: resetting state, size: %d", __func__, sizeof(DFUstate_t));
    memset(&DFUstate, 0, sizeof(DFUstate));
    DFUstate.addr = EXT_FLASH_UPDATE_START_OFFSET;
    DFUstate.gz.fh = -1;
    strcpy(DFUstate.gz.name, UPDATE_FILE);
}

/*! @brief  Erase the update area of the external flash
 */
static void DFUeraseFlash()
{
    // console_send("%s: erasing FW update area", __func__);
    for (uint32_t addr = EXT_FLASH_UPDATE_START_OFFSET; addr < EXT_FLASH_UPDATE_END_OFFSET; addr += (64 * 1024)) {
        SPIFLASH_erase_SECTOR(addr);
    }
}

/*! @brief  Private function to write an uncompressed block (32KB) to external flash

            Incoming data from decompress comes 32KB at a time, has to be
            written 256 bytes at a time.

    @param[IN] len  length of incoming data
*/
static void DFUwriteBlock(uint8_t *data, uint32_t len)
{
    uint32_t    wl = EXT_FLASH_MAX_WR;
    uint8_t     *p = data;

    // XXX -- safely net shouldn't be needed
    if (DFUstate.addr == 0) DFUstate.addr = EXT_FLASH_UPDATE_START_OFFSET;

    for (int i = 0; i < len; i += EXT_FLASH_MAX_WR, p += EXT_FLASH_MAX_WR) {
        if (len - i < EXT_FLASH_MAX_WR) {
            wl = len - i;
        }
        if (i > DECOMP_OUT_BUFSZ) {
            log_Shell("%s: OMG i is out range: %d, should be no more than %d", __func__, i, DECOMP_OUT_BUFSZ);
            return;
        }
        if (DFUstate.addr < EXT_FLASH_UPDATE_START_OFFSET || DFUstate.addr > EXT_FLASH_UPDATE_END_OFFSET) {
            log_Shell("%s: OMG addr is out of range: %lx", __func__, DFUstate.addr);
            return;
        }
        if (SPIFLASH_write(DFUstate.addr, wl, p) != HAL_OK) {
            log_Shell("%s: OMG SPIFLASH_write failed", __func__);
        }
#ifdef PARANOID
        if (SPIFLASH_read(DFUstate.addr, wl, scratchBuf) != HAL_OK) {
            log_Shell("%s: OMG SPIFLASH_read failed", __func__);
        }
        if (memcmp(scratchBuf, p, wl)) {
            log_Shell("%s: OMG data doesn't match", __func__);
        }
#endif
        DFUstate.addr += EXT_FLASH_MAX_WR;
    }
}

/*! @brief  Perform the CRC validation of the uncompressed firmware

            Calculate the CRC on image in the update region of the external
            flash and compare that against the expected CRC (as received from
            the app).

            Additionally, on success, set some globals are visible to the
            bootloader.

    @return true on CRC match
 */
static bool DFUvalidateFW()
{
    uint32_t    addr, remaining, vrfyCrc = 0;
    extern volatile uint32_t __update_crc__, __update_len__;

    log_Shell("%s: crc: %lx, length: %lu", __func__, DFUstate.updateCRC, DFUstate.updateLen);
    log_Shell("%s: Resetting CRC engine", __func__);
    __HAL_CRC_DR_RESET(&hcrc);

    for (addr = EXT_FLASH_UPDATE_START_OFFSET, remaining = DFUstate.updateLen;
            addr < EXT_FLASH_UPDATE_START_OFFSET + DFUstate.updateLen;
            addr += EXT_FLASH_MAX_RD) {

        if (SPIFLASH_read(addr, EXT_FLASH_MAX_RD, DFUstate.inBuf) != HAL_OK) {
            log_Shell("%s: external flash read failure", __func__);
            DFUresetState();
            return false;
        }
        // Accumulate CRC as we read the chunks. There is no guarantee that our length is
        // a multiple of block size, thus the remaining
        vrfyCrc = HAL_CRC_Accumulate(&hcrc, (uint32_t *)DFUstate.inBuf, 
                (remaining >= EXT_FLASH_MAX_RD) ? EXT_FLASH_MAX_RD : remaining);
        remaining -= EXT_FLASH_MAX_RD;
    }

    // If the CRC doesn't match, erase the first sector of update area so the bootloader doesn't waste time looking at it
    if (DFUstate.updateCRC != ~vrfyCrc) {
        log_Shell("%s: OMG CRCs do not match! DFUstate.updateCRC: %08lx, vrfyCrc: %08lx", __func__, DFUstate.updateCRC, ~vrfyCrc);
        log_Shell("%s: erasing sector at: %x", __func__, EXT_FLASH_UPDATE_START_OFFSET);
        SPIFLASH_erase_SECTOR(EXT_FLASH_UPDATE_START_OFFSET);
        DFUresetState();
        return false;
    }

    // Set variables to signal bootloader with values from app
    __update_len__ = DFUstate.updateLen;
    __update_crc__ = DFUstate.updateCRC;

    return true;
}

/*! @brief      Send BLE response to DEVICE_STM_DFU_CRC command
    @param[IN]  command status/response code
    @param[IN]  msg/seq ID of next expected packet
    @return     returns result of BLE xmit attempt
 */
static bool DFUsendCRCAck(eCOM_RETURN_STATUS_t ack, uint16_t msgId)
{
    struct CRCAck pkt = { .status = ack, .msgId = __REV16(msgId) };

    return sendData_CommunicationTask(BLE_TYPE_MSG, BLE_TX_COMMAND_BLE_RESPONSE, sizeof(struct CRCAck), (uint8_t *)&pkt);
}

/*! @brief      Handle DEVICE_STM_DFU_CRC message

                The mobile app sends a DEVICE_STM_DFU_CRC command which
                contains the CRC and length of the firmware image that it
                wants to send. Our response contains the message/packet ID of
                the next packet we are expecting (to allow the restart of a
                failed firmware update). To support that, we cache the crc,len
                tuple and reset our state if the arguments do not match the
                cache.

    @param[IN]  payload pointer to packet data
 */
void handleCRC_FirmwareUpdate(uint16_t u16length, void *payload)
{
    struct CRCMsg   *msg = (struct CRCMsg *)payload;
    uint32_t crc = __REV(msg->crc);
    uint32_t length = __REV(msg->len);

    log_Shell("%s: CRC: %08lx, length: %lu", __func__, crc, length);
    xxd_Shell(payload, u16length);

    // We cannot have state structure in initialized data section because it's too big
    // to fit into the flash image -- so detect initial addr of zero here
    if (DFUstate.addr == 0) {
        DFUresetState();
    }

    // Check that the length isn't nonsense
    if (length < 100 || length > EXT_FLASH_UPDATE_LEN) {
        DFUsendCRCAck(eCOM_RETURN_STATUS_FAIL, 0);
        return;
    }

    eCOM_RETURN_STATUS_t ret = startFile_SHftp(&DFUstate.gz, UPDATE_FILE, crc, length);
    if (ret == eCOM_RETURN_STATUS_UNNECESSARY) {
        log_Shell("%s: re-seeding DFUstate: len: %ld, crc: %08lx, nextMsg: %ld\n",
                __func__, DFUstate.gz.len, DFUstate.gz.crc, DFUstate.gz.nextMsg);
    }

    DFUsendCRCAck(ret, DFUstate.gz.nextMsg);
}

/*! @brief  Receive a chunk (64bytes) of a FW update

            This function is called in response to DEVICE_STM_DFU_DATA commands.
            It will be called repeatedly as chunks of FW are
            received over BLE.
 */
void handleData_FirmwareUpdate(uint16_t u16length, void *payload)
{
    genericResponse_CommunicationTask(writeData_SHftp(&DFUstate.gz, u16length, payload));
}

/*! @brief  Decompress UPDATE_FILE and write it into the update area of the external flash
    @return true on sucess
 */
bool DFUdecompress()
{
    spiffs_file fh;
    int32_t     ret;
    gzHdr_t     header;
    gzFtr_t     footer;

    fh = open(UPDATE_FILE, O_RDONLY, 0);
    if (fh < 0) {
        log_Shell("%s: open returns %d", __func__, fh);
        return false;
    }

    // Read and parse gzip header
    ret = read(fh, &header, sizeof(header));
    if (ret < sizeof(header)) {
        log_Shell("%s:%d read returns %ld", __func__, __LINE__, ret);
        goto close_n_leave;
    }

    if (!(header.id1 == 0x1f && header.id2 == 0x8b)) {
        log_Shell("%s: incorrect magic: %x %x", __func__, header.id1, header.id2);
        char *p = (char *)&header;
        for (int i = 0; i < 10; i++) {
            log_Shell("header: %02x %c", *p, *p); p++;
        }
        goto close_n_leave;
    }

    if (header.flg != 0 && header.flg != 8) {
        log_Shell("%s: gzip header contains unimplemented flags", __func__);
        goto close_n_leave;
    }

    // We can cope with a filename, get it
    if (header.flg & 8) {
        uint8_t *p = DFUstate.filename;
        do {
            read(fh, p, 1);
        } while (*p++);
        log_Shell("%s: original FW filename: %s", __func__, DFUstate.filename);
    }

    // Note where we are, that's the start of the actual compressed data
    int32_t start = lseek(fh, 0, SEEK_CUR);
    // Read and parse the gzip footer (8 bytes at the end of a gzip file)
    int32_t end = lseek(fh, -sizeof(footer), SEEK_END);
    read(fh, &footer, sizeof(gzFtr_t));
    // Reset the read pointer to the start of compressed data
    lseek(fh, start, SEEK_SET);

    log_Shell("%s: start: %ld, end: %ld", __func__, start, end);
    log_Shell("%s: compressed len: %ld, uncompressed len: %ld, crc: %lx", __func__, end - start, footer.isize, footer.crc);

    DFUstate.updateLen = footer.isize;
    DFUstate.updateCRC = footer.crc;

    // Validate that the length makes sense
    if (footer.isize < 100 || footer.isize > EXT_FLASH_UPDATE_LEN) {
        log_Shell("%s: footer.isize is out of range", __func__);
        goto close_n_leave;
    }

    DFUeraseFlash();

    // Start the actual decompression

    uint32_t            inRemaining = end - start;
    uint32_t            inAvail = 0;
    uint32_t            inTotal = 0, outTotal = 0;
    uint8_t             *inNext = DFUstate.inBuf, *outNext;
    uint32_t            outAvail;
    decompEngine_t      *DE;

    // blocking call
    DE = getDecompressor_gzutil();
    outNext = DE->outBuf;
    outAvail = DE->len;

    for (;;) {
        size_t          inBytes, outBytes;
        tinfl_status    status;

        if (!inAvail) {
            uint32_t n = my_min(sizeof(DFUstate.inBuf), inRemaining);

            if (read(fh, DFUstate.inBuf, n) != n) {
                log_Shell("%s: read at line %d failed", __func__, __LINE__);
                goto close_n_leave;
            }

            inNext = DFUstate.inBuf;
            inAvail = n;
            inRemaining -= n;
        }

        inBytes = inAvail;
        outBytes = outAvail;
        status = tinfl_decompress(DE->inflator, inNext, &inBytes, DE->outBuf, outNext, &outBytes, (inRemaining ? TINFL_FLAG_HAS_MORE_INPUT : 0));

        inAvail -= inBytes;
        inNext += inBytes;
        inTotal += inBytes;

        outAvail -= outBytes;
        outNext += outBytes;
        outTotal += outBytes;

        if ((status <= TINFL_STATUS_DONE) || (!outAvail)) {
            // flush buffer
            DFUwriteBlock(DE->outBuf, DE->len - outAvail);
            outNext = DE->outBuf;
            outAvail = DE->len;
            // Debugging note:
            // You might be tempted to clear the output buffer here with a memset call...
            // Well don't.
            // It breaks the decompressed output in very subtle ways
        }

        if (status <= TINFL_STATUS_DONE) {
            if (status == TINFL_STATUS_DONE) {
                break;
            } else {
                log_Shell("%s: tinfl_decompress failed with status %d", __func__, status);
                goto close_n_leave;
            }
        }
    }

    log_Shell("%s: outTotal: %ld", __func__, outTotal);
    log_Shell("%s: all good", __func__);
    close(fh);
    releaseDecompressor_gzutil();

    return true;

    close_n_leave:
    close(fh);
    releaseDecompressor_gzutil();
    return false;
}

/*! @brief   Handle DEVICE_STM_DFU_INSTALL command

    If we have received an entire FW image (complete flag is set) then
    decompress and validate the CRC and reboot into the updated image.

    It is assumed that the caller will arrange for a future reboot on success.

    @param  payload and length are not used
    @return true if successful (and caller should reboot)
 */
bool handleInstall_FirmwareUpdate(uint16_t u16length, void *payload)
{
    log_Shell("%s: Got DEVICE_STM_DFU_INSTALL", __func__);
    xxd_Shell(payload, u16length);

    if (DFUstate.gz.complete && DFUdecompress() && DFUvalidateFW()) {
        genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
        return true;
    }

    log_Shell("%s: something wasn't right", __func__);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_FAIL);
    DFUresetState();
    return false;
}

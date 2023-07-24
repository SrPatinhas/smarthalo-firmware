/*!
    @file gzutil.c

    Interfaces for handling gzip data

    @author     Georg Nikodym
    @copyright  Copyright (c) 2020 SmartHalo Inc
*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "miniz.h"
#include "gzutil.h"
#include "main.h"
#include "semphr.h"
#include "spiffs.h"
#include "FSUtil.h"
#include "Shell.h"

static SemaphoreHandle_t   gzutilSemaphore = NULL;
static StaticSemaphore_t   gzUtilSemaphoreBuffer;

static decompEngine_t       decompEngine;
static uint8_t              buffer[DECOMP_OUT_BUFSZ];
static tinfl_decompressor   inflator;

/*! @brief  Calculate the size of the gzip header

    Performs some trivial validation to ensure that
    the received data is gzipped data.

    @return size of gzip header or zero on failure
*/
size_t gzipHeaderSize_gzutil(const void *data, size_t len)
{
    uint8_t *p, *start;
    const gzHdr_t *h = data;

    p = start = (uint8_t *)data;

    // if the caller hasn't even given a gzip header, give up
    if (len < sizeof(gzHdr_t))
        return 0;

    // not a gzip header
    if (!(h->id1 == 0x1f && h->id2 == 0x8b))
        return 0;

    // unimplemented flags
    if (h->flg != 0 && h->flg != 8)
        return 0;

    // skip header
    p += sizeof(gzHdr_t);

    // skip filename if there is one
    if (h->flg & 8)
        while (p - start <= len && *p++);

    return (size_t)(p - start);
}

/*! @brief  Return the CRC embedded in gzip data

    Gzip data has a header and a footer. The footer
    contains the CRC of the uncompressed data.
    This function does not validate that the data
    received is, in fact, gzip data... assumes
    that validation was done earlier.

    @param  data    pointer to some gzipped data
    @param  len     length of data (in bytes)
    @return CRC value of uncompressed data, zero on failure
*/
uint32_t gzipCRC_gzutil(const void *data, size_t len)
{
    // make sure that we at least got enough data to contain
    // gzip header and footer
    if (len < (sizeof(gzHdr_t) + sizeof(gzFtr_t))) return 0;

    const gzFtr_t *f = (gzFtr_t *)((uint8_t *)data + len - sizeof(gzFtr_t));

    return (f->crc);
}

/*! @brief  check to see if gzipped buffer matches file CRC

    Take a pointer (and len) to some gzipped data that may have
    already been decompressed into filename. Check that the CRC
    embedded in the gzip data matches the CRC of the file.

    @param  data        pointer to some gzipped data
    @param  len         length of data (in bytes)
    @param  filename    name of file that may contain decompressed data
    @return true if the data CRC matches the file CRC
*/
bool eqBufferAndFileCRC_gzutil(const void *data, size_t len, const char *filename)
{
    int         ret;
    spiffs_stat statbuf;
    fsMeta_t    *meta = (fsMeta_t *)&statbuf.meta;

    if (!waitForFS_SystemUtilities(filename)) return false;

    if ((ret = stat(filename, &statbuf)) != 0) return false;

    if (meta->crc != gzipCRC_gzutil(data, len)) return false;

    return true;
}

/*! @brief Decompress (gunzip) a memory buffer

    Assumes a small, in memory image, that will expand to no more than
    one OLED screen worth of pixels (1KB).

    @return true on successful decompression
*/
bool gunzipImage_gzutil(const void *gzimage, size_t len, uint8_t **buf, size_t *outSize)
{
    static uint8_t  image[1024];                // one OLED screen max
    uint32_t        inRemaining = len - 8;      // exclude footer
    uint32_t        inAvail;
    uint32_t        inTotal = 0, outTotal = 0;
    const uint8_t   *inNext = gzimage;
    uint8_t         *outNext;
    uint32_t        outAvail;
    size_t          hdrlen = gzipHeaderSize_gzutil(gzimage, len);
    decompEngine_t  *DE;

    if (hdrlen == 0) return false;

    inNext += hdrlen;
    inRemaining -= hdrlen;
    inAvail = inRemaining;

    DE = getDecompressor_gzutil();
    outNext = DE->outBuf;
    outAvail = DE->len;

    for (;;) {
        size_t          inBytes, outBytes;
        tinfl_status    status;

        inBytes = inAvail;
        outBytes = outAvail;

        status = tinfl_decompress(DE->inflator, inNext, &inBytes,
            DE->outBuf, outNext, &outBytes,
            (inRemaining ? TINFL_FLAG_HAS_MORE_INPUT : 0));

        inAvail -= inBytes;
        inNext += inBytes;
        inTotal += inBytes;

        outAvail -= outBytes;
        outNext += outBytes;
        outTotal += outBytes;

        if (status < TINFL_STATUS_DONE) {
            goto fail;
        }

        if (status == TINFL_STATUS_DONE)
            break;
    }

    *outSize = outTotal;
    // copy into image because DE->outBuf could be claimed by someone else
    // after we release the mutex...
    // not a complete solution, it still assumes the caller is going to
    // immediately copy this data elsewhere
    memcpy(image, DE->outBuf, outTotal < sizeof(image) ? outTotal : sizeof(image));
    *buf = image;

    releaseDecompressor_gzutil();
    return true;

fail:
    releaseDecompressor_gzutil();
    return false; 
}

/*! @brief  Decompress a buffer into a file
    @param  gzData      Pointer to gzipped data
    @param  gzLen       Length of gzipped data
    @param  filename    Name of file to put uncompressed data into
    @param  outSize     Optional size of uncompressed data
    @return True on success.
*/
bool gunzip2File_gzutil(const void *gzData, size_t gzLen, const char *filename, size_t *outSize)
{
    uint32_t        inRemaining = gzLen - 8;      // excluding footer
    uint32_t        inAvail;
    uint32_t        inTotal = 0, outTotal = 0;
    const uint8_t   *inNext = gzData;
    uint8_t         *outNext;
    uint32_t        outAvail;
    size_t          hdrlen = gzipHeaderSize_gzutil(gzData, gzLen);
    decompEngine_t  *DE;
    spiffs_file     fh;

    if (!waitForFS_SystemUtilities(filename)) return false;
    
    if (hdrlen == 0) return false;

    fh = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fh < 0) {
        log_Shell("%s: open of %s failed with %d", __func__, filename, fh);
        return false;
    }

    inNext += hdrlen;
    inRemaining -= hdrlen;
    inAvail = inRemaining;

    DE = getDecompressor_gzutil();
    outNext = DE->outBuf;
    outAvail = DE->len;

    for (;;) {
        size_t          inBytes, outBytes;
        int             wrLen;
        tinfl_status    status;

        inBytes = inAvail;
        outBytes = outAvail;

        status = tinfl_decompress(DE->inflator, inNext, &inBytes,
                                  DE->outBuf, outNext, &outBytes,
                                  (inRemaining ? TINFL_FLAG_HAS_MORE_INPUT : 0));

        inAvail -= inBytes;
        inNext += inBytes;
        inTotal += inBytes;

        outAvail -= outBytes;
        outNext += outBytes;
        outTotal += outBytes;

        if (status < TINFL_STATUS_DONE) {
            log_Shell("%s: tinfl_decompress failed with %d", __func__, status);
            goto leave_in_shame;
        }

        if (status >= TINFL_STATUS_DONE) {
            wrLen = write(fh, DE->outBuf, DE->len - outAvail);
            if (wrLen < 0) {
                log_Shell("%s: write error to %s return: %d", __func__, filename, wrLen);
                goto leave_in_shame;
            }
            outNext = DE->outBuf;
            outAvail = DE->len;
            if (status == TINFL_STATUS_DONE) break;
        }
    }

    if (outSize) *outSize = outTotal;

    uint32_t crc = gzipCRC_gzutil(gzData, gzLen);
    // calculate CRC from filesystem data
    fsMeta_t    meta = { 0 };
    meta.crc = fcrc_FSUtil(fh);
    // compare against gzip'ed data CRC
    if (crc != 0 && crc != meta.crc) {
        log_Shell("%s: crcs don't match, expected: %lx, actual: %lx", __func__, crc, meta.crc);
    }
    // unconditionally update fs with fs version of the CRC
    fupdateMeta_FSUtil(fh, &meta);

    close(fh);
    releaseDecompressor_gzutil();
    return true;

leave_in_shame:
    close(fh);
    releaseDecompressor_gzutil();
    return false; 
}

/*! @brief Get the handle to the decompression engine

    Because the state around decompression is quite large, we have
    just one that will be shared. The sharing is controlled
    by a mutex. Callers will block until this mutex becomes available.

    @return A pointer to a static decompressor state structure.
*/
decompEngine_t *getDecompressor_gzutil()
{
    if (gzutilSemaphore == NULL) {
        gzutilSemaphore = xSemaphoreCreateMutexStatic(&gzUtilSemaphoreBuffer);
    }

    xSemaphoreTake(gzutilSemaphore, portMAX_DELAY);

    memset(buffer, 0, sizeof(buffer));
    memset(&inflator, 0, sizeof(inflator));

    decompEngine.len        = sizeof(buffer);
    decompEngine.outBuf     = buffer;
    decompEngine.inflator   = &inflator;

    tinfl_init(decompEngine.inflator);

    return &decompEngine;
}

/*! @brief Release the decompression engine mutex
*/
void releaseDecompressor_gzutil()
{
    xSemaphoreGive(gzutilSemaphore);
}
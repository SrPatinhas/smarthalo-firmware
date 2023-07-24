/*!
    @file gzutil.h

    Interfaces for handling gzip data

    @author     Georg Nikodym
    @copyright  Copyright (c) 2020 SmartHalo Inc
*/

#ifndef __GZUTIL_H__
#define __GZUTIL_H__

#include <stddef.h>
#include <stdint.h>
#include "miniz.h"

#define DECOMP_OUT_BUFSZ    (1024 * 32)

typedef struct {
    tinfl_decompressor  *inflator;  ///< inflator state struct from miniz
    uint8_t             *outBuf;    ///< buffer for writing to ext flash
    uint32_t            len;        ///< sizeof outBuf
} decompEngine_t;

/*! @brief  gzip header memory layout

            Fully described in RFC1952 or http://www.zlib.org/rfc-gzip.html
*/
typedef struct {
    uint8_t     id1;        ///< magic #1, should be 0x1f
    uint8_t     id2;        ///< magic #2, should be 0x8b
    uint8_t     cm;         ///< compression method, should be 0x8
    uint8_t     flg;        ///< flags, we only accept bit 3 set
    uint32_t    mtime;      ///< modification time, we ignore
    uint8_t     xfl;        ///< extra flags, we ignore
    uint8_t     os;         ///< operating system, we ignore
} __attribute__ ((packed)) gzHdr_t;

/*! @brief  gzip footer memory layout
*/
typedef struct {
    uint32_t    crc;        ///< CRC2 of original data
    uint32_t    isize;      ///< original data size % 2^32
} __attribute__ ((packed)) gzFtr_t;

decompEngine_t *getDecompressor_gzutil();
void releaseDecompressor_gzutil();
size_t gzipHeaderSize_gzutil(const void *data, size_t len);
bool gunzipImage_gzutil(const void *gzimage, size_t len, uint8_t **buf, size_t *outSize);
bool gunzip2File_gzutil(const void *gzData, size_t gzLen, const char *filename, size_t *outSize);
uint32_t gzipCRC_gzutil(const void *data, size_t len);
bool eqBufferAndFileCRC_gzutil(const void *data, size_t len, const char *filename);

#endif // __GZUTIL_H__
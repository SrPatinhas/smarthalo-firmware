/*!
    @file       FSUtil.c

    @detail     File operation wrappers

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#include "FSUtil.h"
#include "crc.h"

#define EXT_FLASH_MAX_RD    1024

extern spiffs       rootFS;
static uint8_t      scratchBuf[EXT_FLASH_MAX_RD];

// Dumb wrappers for standard file operations
// Two things are being achieved here:
//  1. allows other code to not need (too much) special SPIFFS related stuff
//  2. hides global rootFS

spiffs_file open(const char *path, spiffs_flags flags, spiffs_mode mode)
{
    return SPIFFS_open(&rootFS, path, flags, mode);
}

s32_t close(spiffs_file fh)
{
    return SPIFFS_close(&rootFS, fh);
}

s32_t read(spiffs_file fh, void *buf, s32_t len)
{
    return SPIFFS_read(&rootFS, fh, buf, len);
}

s32_t write(spiffs_file fh, void *buf, s32_t len)
{
    return SPIFFS_write(&rootFS, fh, buf, len);
}

s32_t lseek(spiffs_file fh, s32_t offs, int whence)
{
    return SPIFFS_lseek(&rootFS, fh, offs, whence);
}

s32_t unlink(const char *path)
{
    return SPIFFS_remove(&rootFS, path);
}

s32_t stat(const char *path, spiffs_stat *s)
{
    return SPIFFS_stat(&rootFS, path, s);
}

s32_t fstat(spiffs_file fh, spiffs_stat *s)
{
    return SPIFFS_fstat(&rootFS, fh, s);
}

spiffs_DIR *opendir(const char *name, spiffs_DIR *d)
{
    return SPIFFS_opendir(&rootFS, name, d);
}

s32_t fremove(spiffs_file fh)
{
    return SPIFFS_fremove(&rootFS, fh);
}

/*! @brief  FS info provides two numbers of bytes, total and used
*/
s32_t fsinfo_FSUtil(uint32_t *total, uint32_t *used)
{
    return SPIFFS_info(&rootFS, (u32_t *)total, (u32_t *)used);
}

/*! @brief  flush any pending writes
*/
s32_t fsync(spiffs_file fh)
{
    return SPIFFS_fflush(&rootFS, fh);
}

/*! @brief  Calculate CRC of a file, given an open file handle
*/
uint32_t fcrc_FSUtil(spiffs_file fh)
{
    int         ret;
    int32_t     pos;
    uint32_t    crc = 0xffffffff;

    __HAL_CRC_DR_RESET(&hcrc);
    memset(scratchBuf, 0xff, sizeof(scratchBuf));

    pos = lseek(fh, 0, SEEK_CUR);
    lseek(fh, 0, SEEK_SET);

    while ((ret = read(fh, scratchBuf, sizeof(scratchBuf))) > 0) {
        // short read, zero the 
        if (ret < sizeof(scratchBuf))
            memset(scratchBuf + ret, 0xff, sizeof(scratchBuf) - ret);
        crc = HAL_CRC_Accumulate(&hcrc, (uint32_t *)scratchBuf, ret);
    }
    lseek(fh, pos, SEEK_SET);

    return ~crc;
}

/*! @brief  Calculate CRC of a file given a name
*/
uint32_t crc_FSUtil(const char *path)
{
    spiffs_file fh;
    uint32_t    crc = 0xffffffff;   // AKA -1

    fh = open(path, O_RDONLY, 0);
    if (fh < 0) {
        printf("%s: open failed with %d\n", __func__, fh);
        return crc;
    }
    crc = fcrc_FSUtil(fh);
    close(fh);
    return ~crc;
}

/*! @brief  Update the metadata associated with the named file
*/
s32_t updateMeta_FSUtil(const char *name, void *meta)
{
    return SPIFFS_update_meta(&rootFS, name, meta);
}

/*! @brief  Update the metadata associated with an open file handle
*/
s32_t fupdateMeta_FSUtil(spiffs_file fh, void *meta)
{
    return SPIFFS_fupdate_meta(&rootFS, fh, meta);
}

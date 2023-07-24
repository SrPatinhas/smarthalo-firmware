/*!
    @file       FSUtil.h

    @brief     Flash filesystem support

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#ifndef __FSUTIL_H
#define __FSUTIL_H

#include "spiffs.h"

// standard-ish POSIX wrappers
spiffs_file open(const char *path, spiffs_flags flags, spiffs_mode mode);
s32_t close(spiffs_file fh);
s32_t read(spiffs_file fh, void *buf, s32_t len);
s32_t write(spiffs_file fh, void *buf, s32_t len);
s32_t lseek(spiffs_file fh, s32_t offs, int whence);
s32_t unlink(const char *path);
s32_t stat(const char *path, spiffs_stat *s);
s32_t fstat(spiffs_file fh, spiffs_stat *s);
spiffs_DIR *opendir(const char *name, spiffs_DIR *d);
s32_t fremove(spiffs_file fh);
s32_t fsync(spiffs_file fh);

// no wrapper required, simple macro replacement is adequate
#define closedir    SPIFFS_closedir
#define readdir     SPIFFS_readdir

// Some other equivalents
#define DIR         spiffs_DIR
#define dirent      spiffs_dirent

#ifndef SEEK_SET
#define SEEK_SET    SPIFFS_SEEK_SET
#endif
#ifndef SEEK_CUR
#define SEEK_CUR    SPIFFS_SEEK_CUR
#endif
#ifndef SEEK_END
#define SEEK_END    SPIFFS_SEEK_END
#endif

#define O_RDONLY    SPIFFS_O_RDONLY
#define O_WRONLY    SPIFFS_O_WRONLY
#define O_RDWR      SPIFFS_O_RDWR
#define O_CREAT     SPIFFS_O_CREAT
#define O_TRUNC     SPIFFS_O_TRUNC

// Non-POSIX things
s32_t       fsinfo_FSUtil(uint32_t *total, uint32_t *used);
uint32_t    crc_FSUtil(const char *path);
uint32_t    fcrc_FSUtil(spiffs_file fh);
s32_t       updateMeta_FSUtil(const char *name, void *meta);
s32_t       fupdateMeta_FSUtil(spiffs_file fh, void *meta);

/*! @brief  metadata attached to files in the filesystem

            SPIFFS supports the idea of having some number of bytes
            reserved for metadata for each file. This structure
            defines the layout/purpose of those bytes
*/
typedef struct {
    uint32_t    crc;        ///< CRC32 of the file
    uint32_t    version;    ///< optional version
} fsMeta_t;

#endif // __FSUTIL_H
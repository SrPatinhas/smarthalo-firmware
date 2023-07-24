/*
 * SH2 specific spiffs config
 */

#ifndef _SH2_SPIFFS_CONFIG_H_
#define _SH2_SPIFFS_CONFIG_H_

// machine specific typedefs
typedef signed int      s32_t;
typedef unsigned int    u32_t;
typedef signed short    s16_t;
typedef unsigned short  u16_t;
typedef signed char     s8_t;
typedef unsigned char   u8_t;

#define SPIFFS_TYPES_OVERRIDE
typedef u32_t spiffs_block_ix;
typedef u32_t spiffs_page_ix;
typedef u32_t spiffs_obj_id;
typedef u32_t spiffs_span_ix;

#define SPIFFS_BUFFER_HELP              0
#define SPIFFS_CACHE                    1
#define SPIFFS_CACHE_WR                 1
#define SPIFFS_CACHE_STATS              1
#define SPIFFS_PAGE_CHECK               1
#define SPIFFS_GC_MAX_RUNS              5
#define SPIFFS_GC_STATS                 1
#define SPIFFS_GC_HEUR_W_DELET          (5)
#define SPIFFS_GC_HEUR_W_USED           (-1)
#define SPIFFS_GC_HEUR_W_ERASE_AGE      (50)
#define SPIFFS_OBJ_NAME_LEN             (32)
#define SPIFFS_OBJ_META_LEN             (8)
#define SPIFFS_COPY_BUFFER_STACK        (64)
#define SPIFFS_USE_MAGIC                (1)
#define SPIFFS_USE_MAGIC_LENGTH         (1)

void fsLock_SystemUtilities();
#define SPIFFS_LOCK(fs) (fsLock_SystemUtilities())
void fsUnlock_SystemUtilities();
#define SPIFFS_UNLOCK(fs) (fsUnlock_SystemUtilities())

#define SPIFFS_SINGLETON 1

#define SPIFFS_RESERVED_BY_BOOTLOADER     (1024*512)
#define SPIFFS_CFG_PHYS_SZ(ignore)        (1024*1024*2 - SPIFFS_RESERVED_BY_BOOTLOADER)
#define SPIFFS_CFG_PHYS_ERASE_SZ(ignore)  (64*1024)
#define SPIFFS_CFG_PHYS_ADDR(ignore)      SPIFFS_RESERVED_BY_BOOTLOADER
#define SPIFFS_CFG_LOG_PAGE_SZ(ignore)    (256)
#define SPIFFS_CFG_LOG_BLOCK_SZ(ignore)   (64*1024)

#define SPIFFS_ALIGNED_OBJECT_INDEX_TABLES       0

#define SPIFFS_HAL_CALLBACK_EXTRA         0

#define SPIFFS_FILEHDL_OFFSET                 0

#define SPIFFS_READ_ONLY                      0

#define SPIFFS_TEMPORAL_FD_CACHE              1
#define SPIFFS_TEMPORAL_CACHE_HIT_SCORE       4

#define SPIFFS_IX_MAP                         1

#define SPIFFS_NO_BLIND_WRITES                0

#define SPIFFS_TEST_VISUALISATION         1

#endif /* _SH2_SPIFFS_CONFIG_H_ */

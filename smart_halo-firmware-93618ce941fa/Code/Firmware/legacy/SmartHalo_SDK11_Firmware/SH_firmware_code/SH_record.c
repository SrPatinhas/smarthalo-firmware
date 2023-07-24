
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "app_util_platform.h"
#include "app_twi.h"

#include "fds.h"

#include "SH_record.h"

#define FILE_ID     0x1234

typedef void (*rec_evt_cb_t)( ret_code_t err );

rec_evt_cb_t rec_evt_cb = NULL;
rec_cb_t rec_cb = NULL;

void rec_evt_handler(fds_evt_t const * const p_fds_evt)
{
    printf("rec_evt_handler %d %d \r\n", p_fds_evt->id, (int)p_fds_evt->result);
    rec_evt_cb_t cb = rec_evt_cb;
    rec_evt_cb = NULL;
    if(cb) {
        cb(p_fds_evt->result);
        //sch_oneshot_ctx((sch_cb_ctx_t)cb, 0, (void*)p_fds_evt->result);
    }
    /*switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result != FDS_SUCCESS)
            {
                // Initialization failed.
            }
            break;
        default:
            break;
    }*/
}

ret_code_t rec_read(uint16_t key, uint8_t *buf, uint32_t bufLen, uint32_t *readLen) {

    ret_code_t err = 0;

    fds_flash_record_t  flash_record;
    fds_record_desc_t   record_desc;
    memset(&record_desc, 0, sizeof(fds_record_desc_t));
    fds_find_token_t    ftok;
    memset(&ftok, 0, sizeof(fds_find_token_t));

    if(!buf) {
        return false;
    }

    err = fds_record_find(FILE_ID, key, &record_desc, &ftok);
    if (err != FDS_SUCCESS)
    {
        printf("fds_record_find %d \r\n", (int)err);
        return err;
    }

    err = fds_record_open(&record_desc, &flash_record);
    if (err != FDS_SUCCESS)
    {
        printf("fds_record_open %d \r\n", (int)err);
        return false;
    }

    uint32_t reclen = flash_record.p_header->tl.length_words * 4;
    uint32_t efflen = MIN(reclen, bufLen);
    if(readLen) {
        *readLen = efflen;
    }
    memcpy(buf, flash_record.p_data, efflen);

    err = fds_record_close(&record_desc);
    if (err != FDS_SUCCESS)
    {
        printf("fds_record_close %d \r\n", (int)err);
        return false;
    }

    return err;

}



void rec_write_done(ret_code_t err) {
    rec_cb_t cb = rec_cb;
    rec_cb = NULL;
    if(cb) {
        cb(err);
    }
}

void rec_write(uint16_t key, uint8_t *buf, uint32_t len, rec_cb_t cb) {

    fds_record_t        record;
    fds_record_chunk_t  record_chunk;
    // Set up data.
    record_chunk.p_data         = buf;
    record_chunk.length_words   = (len>>2) + ((len%4) ? 1 : 0);
    // Set up record.
    record.file_id                  = FILE_ID;
    record.key               = key;
    record.data.p_chunks       = &record_chunk;
    record.data.num_chunks   = 1;
      
    rec_evt_cb = rec_write_done;
    rec_cb = cb;

    fds_record_desc_t   record_desc;
    memset(&record_desc, 0, sizeof(fds_record_desc_t));
    fds_find_token_t    ftok;
    memset(&ftok, 0, sizeof(fds_find_token_t));

    if (fds_record_find(FILE_ID, key, &record_desc, &ftok) == FDS_SUCCESS)
    {
        printf("rec_write fds_record_update\r\n");
        ret_code_t ret = fds_record_update(&record_desc, &record);
        if (ret != FDS_SUCCESS)
        {
            rec_evt_cb = NULL;
            rec_cb = NULL;
            cb(ret);
            return;
        }
    } else {
        printf("rec_write fds_record_write\r\n");
        ret_code_t ret = fds_record_write(&record_desc, &record);
        if (ret != FDS_SUCCESS)
        {
            rec_evt_cb = NULL;
            rec_cb = NULL;
            cb(ret);
            return;
        }
    }

}


void rec_gb_done(ret_code_t err) {
    rec_cb_t cb = rec_cb;
    rec_cb = NULL;
    if(cb) {
        cb(err);
    }
}

void rec_gb_do(ret_code_t err) {
    if (err != FDS_SUCCESS)
    {
        printf("fds_init cb %d \r\n", (int)err);
        APP_ERROR_CHECK(err);
    }
    rec_evt_cb = rec_gb_done;
    fds_gc();
}

void rec_init(rec_cb_t cb) {
    rec_cb = cb;

    ret_code_t err_code;

    err_code = fds_register(rec_evt_handler);
    printf("fds_register %d \r\n", (int)err_code);
    APP_ERROR_CHECK(err_code);

    rec_evt_cb = rec_gb_do;
    err_code = fds_init();
    printf("fds_init %d \r\n", (int)err_code);
    APP_ERROR_CHECK(err_code);

}
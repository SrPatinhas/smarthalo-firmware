
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "fds.h"

#include "app_util_platform.h"
#include "app_twi.h"
#include "mem_manager.h"

#include "scheduler.h"

#include "record.h"

#define FILE_ID     0x1234

#define REC_OPQUEUELEN  4

typedef void (*rec_evt_cb_t)( ret_code_t err );

typedef enum { REQ_OP_NONE, REQ_OP_INIT, REQ_OP_WR, REQ_OP_DEL } rec_op_type_t;

typedef struct {
    rec_op_type_t op;
    //bool doGC;
    uint32_t retries;
    void *buf; 
    uint32_t len; 
    rec_cb_t cb;
    uint32_t key; 
} rec_op_pending_t;

rec_op_pending_t rec_op_queue[REC_OPQUEUELEN];
uint32_t rec_op_queue_in = 0;
uint32_t rec_op_queue_out = 0;

rec_op_pending_t *rec_op_peek() {
    if(rec_op_queue_in == rec_op_queue_out) {
        return NULL;
    }
    return &rec_op_queue[rec_op_queue_out];
}

rec_op_pending_t *rec_op_alloc() {
    uint32_t in = rec_op_queue_in;
    uint32_t next = (rec_op_queue_in+1)%REC_OPQUEUELEN;
    if(next == rec_op_queue_out) {
        return NULL;
    }
    rec_op_queue_in = next;
    return &rec_op_queue[in];
}

void rec_op_pop() {
    if(rec_op_queue_in == rec_op_queue_out) {
        return;
    }
    rec_op_queue[rec_op_queue_out].op = REQ_OP_NONE;
    rec_op_queue_out++;
    rec_op_queue_out %= REC_OPQUEUELEN;
}

rec_evt_cb_t rec_evt_cb = NULL;
//rec_cb_t rec_cb = NULL;

void rec_evt_handler(fds_evt_t const * const p_fds_evt)
{
    //printf("rec_evt_handler %d %d \r\n", p_fds_evt->id, p_fds_evt->result);
    rec_evt_cb_t cb = rec_evt_cb;
    rec_evt_cb = NULL;
    if(cb) {
        //cb(p_fds_evt);
        sch_oneshot_ctx((sch_cb_ctx_t)cb, 0, (void*)p_fds_evt->result);
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

ret_code_t rec_read(uint16_t key, void *buf, uint32_t bufLen, uint32_t *readLen) {

    ret_code_t err = 0;

    fds_flash_record_t  flash_record;
    fds_record_desc_t   record_desc;
    memset(&record_desc, 0, sizeof(fds_record_desc_t));
    fds_find_token_t    ftok;
    memset(&ftok, 0, sizeof(fds_find_token_t));

    if(!buf) {
        return -1;
    }

    err = fds_record_find(FILE_ID, key, &record_desc, &ftok);
    if (err != FDS_SUCCESS)
    {
        //printf("fds_record_find %d \r\n", err);
        return err;
    }

    err = fds_record_open(&record_desc, &flash_record);
    if (err != FDS_SUCCESS)
    {
        //printf("fds_record_open %d \r\n", err);
        return err;
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
        //printf("fds_record_close %d \r\n", err);
        return err;
    }

    return err;

}


void rec_op_do(void *ctx);

void rec_op_done(ret_code_t err) {
    rec_op_pending_t *entry = rec_op_peek();
    if(!entry) {
        printf("rec_write_done entry NULL\r\n");
        return;
    }
    rec_cb_t cb = entry->cb;
    rec_op_pop();
    //printf("rec_op_done %08X\r\n", cb);

    if(cb) {
        sch_oneshot_ctx((sch_cb_ctx_t)cb, 0, (void*)err);
    }

    sch_unique_oneshot(rec_op_do, 0);
}


void rec_op_gc_done(ret_code_t err) {
    //printf("rec_op_gc_done! %d\r\n", err);
    sch_unique_oneshot(rec_op_do, 0);
}

void rec_op_write(rec_op_pending_t *entry) {
    ret_code_t err;
    fds_record_t        record;
    fds_record_chunk_t  record_chunk;
    // Set up data.
    record_chunk.p_data         = entry->buf;
    record_chunk.length_words   = ((entry->len)>>2) + (((entry->len)%4) ? 1 : 0);
    // Set up record.
    record.file_id                  = FILE_ID;
    record.key               = entry->key;
    record.data.p_chunks       = &record_chunk;
    record.data.num_chunks   = 1;

    fds_record_desc_t   record_desc;
    memset(&record_desc, 0, sizeof(fds_record_desc_t));
    fds_find_token_t    ftok;
    memset(&ftok, 0, sizeof(fds_find_token_t));

    rec_evt_cb = rec_op_done;
    if (fds_record_find(FILE_ID, entry->key, &record_desc, &ftok) == FDS_SUCCESS)
    {
        err = fds_record_update(&record_desc, &record);
    } else {
        err = fds_record_write(&record_desc, &record);
    }

    if (err == FDS_ERR_NO_SPACE_IN_FLASH) {
        //printf("rec_op_write no space\r\n");
        if(entry->retries) {
            entry->retries--;
            rec_evt_cb = rec_op_gc_done;
            err = fds_gc();
        } else{
            rec_op_done(err);            
        }
    } else if (err != FDS_SUCCESS) {
        //printf("rec_op_write err %d\r\n", err);
        rec_evt_cb = NULL;
        if(entry->cb) {
            entry->cb(err);
        }
        rec_op_done(err);
    }
}

void rec_op_del(rec_op_pending_t *entry) {
    
    rec_evt_cb = rec_op_done;

    fds_record_desc_t   record_desc;
    memset(&record_desc, 0, sizeof(fds_record_desc_t));
    fds_find_token_t    ftok;
    memset(&ftok, 0, sizeof(fds_find_token_t));

    ret_code_t ret = fds_record_find(FILE_ID, entry->key, &record_desc, &ftok);
    if (ret == FDS_SUCCESS)
    {
        ret = fds_record_delete(&record_desc);
        if (ret != FDS_SUCCESS)
        {
            rec_evt_cb = NULL;
            if(entry->cb) {
                entry->cb(ret);
            }
        }
    } else {
        rec_evt_cb = NULL;
        if(entry->cb) {
            entry->cb(ret);
        }
    }
    rec_op_done(ret);
}

void rec_op_do(void *ctx) {

    rec_op_pending_t *entry = rec_op_peek();
    if(!entry) {
        return;
    }

    if(entry->op == REQ_OP_WR) {
        //check if the data is different from what we're writing
        uint8_t rbuf[entry->len];
        memset(rbuf,0,entry->len);
        uint32_t read_len = 0;
        ret_code_t read_err = rec_read(entry->key, &rbuf, entry->len, &read_len);
        //when there's a read error it's likely the entry doesn't exist yet
        bool isWriteRequired = read_err ? true : false;
        if(!isWriteRequired){
            uint8_t writeBuf[entry->len];
            memcpy(writeBuf, entry->buf, entry->len);
            for(int i = 0; i<entry->len; i++){
                if(rbuf[i] != writeBuf[i]){
                    isWriteRequired = true;
                    break;
                }
            }
        }

        if(isWriteRequired){
            rec_op_write(entry);
        }else{
            rec_op_done(FDS_SUCCESS);
        }
    }
    if(entry->op == REQ_OP_DEL) {
        rec_op_del(entry);
    }

}

void rec_write(uint16_t key, void *buf, uint32_t len, rec_cb_t cb) {

    bool pending = (rec_op_peek() != NULL);

    rec_op_pending_t *entry = rec_op_alloc();
    if(entry) {
        entry->op = REQ_OP_WR;
        entry->retries = 2; //gc seems to need 2 call 
        entry->key = key;
        entry->buf = buf;
        entry->len = len;
        entry->cb = cb;

        if(!pending) {
            sch_unique_oneshot(rec_op_do, 0);
        }
    }

}


void rec_delete(uint16_t key, rec_cb_t cb) {
    bool pending = (rec_op_peek() != NULL);

    rec_op_pending_t *entry = rec_op_alloc();
    if(entry) {
        entry->op = REQ_OP_DEL;
        entry->retries = 2;
        entry->key = key;
        entry->buf = NULL;
        entry->len = 0;
        entry->cb = cb;

        if(!pending) {
            sch_unique_oneshot(rec_op_do, 0);
        }
    }

}


void rec_gc_do(ret_code_t err) {
    if (err != FDS_SUCCESS)
    {
        printf("fds_init cb %d \r\n", err);
        APP_ERROR_CHECK(err);
    }
    printf("rec_gc_do \r\n");
    rec_evt_cb = rec_op_done;
    fds_gc();
}

void rec_init(rec_cb_t cb) {

    ret_code_t err_code;
    
    for(int i = 0; i < REC_OPQUEUELEN; i++) {
        rec_op_queue[i].op = REQ_OP_NONE;
    }

    err_code = fds_register(rec_evt_handler);
    printf("fds_register %d \r\n", err_code);
    APP_ERROR_CHECK(err_code);

    //enqueue
    rec_op_pending_t *entry = rec_op_alloc();
    if(entry) {
        entry->op = REQ_OP_INIT;
        entry->retries = 1;
        entry->cb = cb;
    }

    rec_evt_cb = rec_op_done;//rec_gc_do;
    err_code = fds_init();
    printf("fds_init %d \r\n", err_code);
    APP_ERROR_CHECK(err_code);

}
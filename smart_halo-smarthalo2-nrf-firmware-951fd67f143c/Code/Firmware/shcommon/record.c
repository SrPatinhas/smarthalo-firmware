
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "fds.h"

#include "app_util_platform.h"
#include "mem_manager.h"

#include "scheduler.h"

#include "record.h"

#define FILE_ID     0x1234

#define MAX_ENTRY_BYTES 40

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

static uint32_t m_data[MAX_ENTRY_BYTES/4] __ALIGN(sizeof(uint32_t));
static fds_record_t m_record = 
{
    .file_id = FILE_ID,
    .key = 0, //0 is illegal, need to update it., 
    .data.p_data = &m_data,
    .data.length_words = 0,
};
static bool volatile m_fds_initialized;

rec_op_pending_t rec_op_queue[REC_OPQUEUELEN];
uint32_t rec_op_queue_in = 0;
uint32_t rec_op_queue_out = 0;

static char const * fds_evt_str[] =
{
    "FDS_EVT_INIT",
    "FDS_EVT_WRITE",
    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD",
    "FDS_EVT_DEL_FILE",
    "FDS_EVT_GC",
};

const char *fds_err_str(ret_code_t ret)
{
    /* Array to map FDS return values to strings. */
    static char const * err_str[] =
    {
        "FDS_ERR_OPERATION_TIMEOUT",
        "FDS_ERR_NOT_INITIALIZED",
        "FDS_ERR_UNALIGNED_ADDR",
        "FDS_ERR_INVALID_ARG",
        "FDS_ERR_NULL_ARG",
        "FDS_ERR_NO_OPEN_RECORDS",
        "FDS_ERR_NO_SPACE_IN_FLASH",
        "FDS_ERR_NO_SPACE_IN_QUEUES",
        "FDS_ERR_RECORD_TOO_LARGE",
        "FDS_ERR_NOT_FOUND",
        "FDS_ERR_NO_PAGES",
        "FDS_ERR_USER_LIMIT_REACHED",
        "FDS_ERR_CRC_CHECK_FAILED",
        "FDS_ERR_BUSY",
        "FDS_ERR_INTERNAL",
    };

    return err_str[ret - NRF_ERROR_FDS_ERR_BASE];
}

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
    if (p_fds_evt->result == NRF_SUCCESS)
    {
        printf("Event: %s received (NRF_SUCCESS)\r\n",
                      fds_evt_str[p_fds_evt->id]);
    }
    else
    {
        printf("Event: %s received (%s)\r\n",
                      fds_evt_str[p_fds_evt->id],
                      fds_err_str(p_fds_evt->result));
    }

    rec_evt_cb_t cb = rec_evt_cb;
    rec_evt_cb = NULL;
    if(cb) {
        sch_oneshot_ctx((sch_cb_ctx_t)cb, 0, (void*)p_fds_evt->result);
    }

    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result == NRF_SUCCESS)
            {
                m_fds_initialized = true;
                printf("Initialized!\n");
            }
            break;

        case FDS_EVT_WRITE:
        {
            if (p_fds_evt->result == NRF_SUCCESS)
            {
                printf("Record ID:\t0x%04x\r\n",  p_fds_evt->write.record_id);
                printf("File ID:\t0x%04x\r\n",    p_fds_evt->write.file_id);
                printf("Record key:\t0x%04x\r\n", p_fds_evt->write.record_key);
            }
        } break;

        case FDS_EVT_DEL_RECORD:
        {
            if (p_fds_evt->result == NRF_SUCCESS)
            {
                printf("Record ID:\t0x%04x\r\n",  p_fds_evt->del.record_id);
                printf("File ID:\t0x%04x\r\n",    p_fds_evt->del.file_id);
                printf("Record key:\t0x%04x\r\n", p_fds_evt->del.record_key);
            }
        } break;

        default:
            break;
    }
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
    if (err != NRF_SUCCESS)
    {
        printf("Record not found succesfully\n");
        printf("fds_record_find %d \r\n", err);
        return err;
    }

    err = fds_record_open(&record_desc, &flash_record);
    if (err != NRF_SUCCESS)
    {
        printf("fds_record_open %d \r\n", err);
        return err;
    }

    uint32_t reclen = flash_record.p_header->length_words * 4;
    uint32_t efflen = MIN(reclen, bufLen);
    if(readLen) {
        *readLen = efflen;
    }
    memcpy(buf, flash_record.p_data, efflen);

    err = fds_record_close(&record_desc);
    if (err != NRF_SUCCESS)
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
    // Set up data.
    memcpy(m_data, entry->buf, entry->len);
    m_record.data.length_words = ((entry->len+3)/sizeof(uint32_t) + (((entry->len+3)%sizeof(uint32_t)) ? 1 : 0));

    // Set up m_record.m_dummy_cfgm_dummy_cfgm_dummy_cfg
    m_record.key = entry->key;

    fds_record_desc_t   record_desc = {0};
    fds_find_token_t    ftok = {0};

    rec_evt_cb = rec_op_done;
    err = fds_record_find(FILE_ID, entry->key, &record_desc, &ftok); 
    if (err == NRF_SUCCESS){
        err = fds_record_update(&record_desc, &m_record);
        printf("Update ret: %d\n", err);
    } else {
        err = fds_record_write(&record_desc, &m_record);
        printf("Write ret: %d\n", err);
    }

    if (err == FDS_ERR_NO_SPACE_IN_FLASH) {
        printf("rec_op_write no space\r\n");
        if(entry->retries) {
            entry->retries--;
            rec_evt_cb = rec_op_gc_done;
            err = fds_gc();
        } else{
            printf("Err not space in flash\n");
            rec_op_done(err);            
        }
    } else if (err != NRF_SUCCESS) {
        printf("rec_op_write err %s\r\n", fds_err_str(err));
        rec_evt_cb = NULL;
        if(entry->cb) {
            entry->cb(err);
        }
        printf("Err Not success %d\n", err);
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
    if (ret == NRF_SUCCESS)
    {
        ret = fds_record_delete(&record_desc);
        if (ret != NRF_SUCCESS)
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
            rec_op_done(NRF_SUCCESS);
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
    if (err != NRF_SUCCESS)
    {
        printf("fds_init cb %d \r\n", err);
        APP_ERROR_CHECK(err);
    }
    printf("rec_gc_do \r\n");
    rec_evt_cb = rec_op_done;
    fds_gc();
}

/**@brief   Sleep until an event is received. */
static void power_manage(void)
{
#ifdef SOFTDEVICE_PRESENT
    (void) sd_app_evt_wait();
#else
    __WFE();
#endif
    printf("Initializing\n");
}

/**@brief   Wait for fds to initialize. */
static void wait_for_fds_ready(void)
{
    while (!m_fds_initialized)
    {
        power_manage();
    }
}

void rec_init() {

    ret_code_t err_code;
    
    for(int i = 0; i < REC_OPQUEUELEN; i++) {
        rec_op_queue[i].op = REQ_OP_NONE;
    }

    err_code = fds_register(rec_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = fds_init();
    APP_ERROR_CHECK(err_code);

    /* Wait for fds to initialize. */
    wait_for_fds_ready();        

    fds_stat_t stat = {0};

    err_code = fds_stat(&stat);
    APP_ERROR_CHECK(err_code);

    printf("Found %d valid records.\r\n", stat.valid_records);
    printf("Found %d dirty records (ready to be garbage collected).\r\n", stat.dirty_records);
}
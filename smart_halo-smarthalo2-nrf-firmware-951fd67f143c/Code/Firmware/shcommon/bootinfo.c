
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "nrf_mbr.h"
#include "nrf_fstorage.h"
#include "nrf_dfu_flash.h"
#include "nrf_fstorage_sd.h"

#include "bootinfo.h"

#define MASK_OUTDATED 0x80000000

#define INVALIDATE 0x7fffffff

#define NOT_OUTDATED(x) ((x)&MASK_OUTDATED)

uint32_t bootInfo[MBR_PAGE_SIZE_IN_WORDS] __attribute__ ((section(".bootInfo")))
                                              __attribute__((used));


/* Initialize an fstorage instance using the nrf_fstorage_sd backend.
 * nrf_fstorage_sd uses the SoftDevice to write to flash. This implementation can safely be
 * used whenever there is a SoftDevice, regardless of its status (enabled/disabled). */
nrf_fstorage_api_t * p_fs_api = &nrf_fstorage_sd;

// Function prototypes
static void fs_evt_handler(nrf_fstorage_evt_t * evt);

// FS_REGISTER_CFG(nrf_fstorage_t binfo_fs_config) =
// {
//     .evt_handler       = fs_evt_handler,            // Function for event callbacks.
//     .start_addr   = (uint32_t*)bootInfo,
//     .end_addr     = (uint32_t*)bootInfo + MBR_PAGE_SIZE_IN_WORDS*sizeof(uint32_t)
// };

NRF_FSTORAGE_DEF(nrf_fstorage_t binfo_fs_config) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = fs_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr   = (uint32_t)&bootInfo,
    .end_addr     = (uint32_t)&bootInfo + MBR_PAGE_SIZE_IN_WORDS*sizeof(uint32_t)
};

static void fs_evt_handler(nrf_fstorage_evt_t * evt)
{
	printf("fs_evt_handler %08X %08X %08X\r\n", evt->id, evt->p_src);
    if (evt->p_param)
    {
       ((dfu_flash_callback_t)evt->p_param)(evt);
    }
}

uint32_t binfo_getWord() {
    for(int i = 0; i < MBR_PAGE_SIZE_IN_WORDS; i++) {
        if(NOT_OUTDATED(bootInfo[i])) {
        	return bootInfo[i] & 0x7FFFFFFF;
        }
    }
    return 0x7FFFFFFF;
}

binfo_cb_t binfo_cb = NULL;
uint32_t binfo_w;
uint32_t binfo_invalidate = INVALIDATE;

static void binfo_setWord_writeDone(nrf_fstorage_evt_t const * const evt, ret_code_t result)
{
	//printf("writeDone binfo_cb: %08X\r\n", binfo_cb);
	if(binfo_cb) {
		binfo_cb();
		binfo_cb = NULL;
	}
}

static void binfo_setWord_invalidateDone(nrf_fstorage_evt_t const * const evt, ret_code_t result)
{
	uint32_t err;
    for(int i = 0; i < MBR_PAGE_SIZE_IN_WORDS; i++) {
        if(NOT_OUTDATED(bootInfo[i])) {
		    err = nrf_fstorage_write(&binfo_fs_config, (uint32_t) &bootInfo[i], &binfo_w, sizeof(bootInfo[i]), binfo_setWord_writeDone);
		    if (err != NRF_SUCCESS)
		    {
		    	printf("invalidateDone fs_store: %d\r\n", err);
		    	binfo_cb = NULL;
		    }
	        return;
        }
    }
   	binfo_cb = NULL;
}

void binfo_setWord(uint32_t w, binfo_cb_t cb) {
	uint32_t err;
	if(binfo_cb) {
		return;
	}
   	binfo_cb = cb;
   	binfo_w = w | MASK_OUTDATED;
    for(int i = 0; i < MBR_PAGE_SIZE_IN_WORDS; i++) {
        if(NOT_OUTDATED(bootInfo[i])) {
        	if(i == MBR_PAGE_SIZE_IN_WORDS-1) {
        		break;
        	}
		    err = nrf_fstorage_write(&binfo_fs_config, (uint32_t) &bootInfo[i], &binfo_invalidate, sizeof(bootInfo[i]), binfo_setWord_invalidateDone);
		    if (err != NRF_SUCCESS)
		    {
		    	printf("binfo_setWord fs_store: %d\r\n", err);
		    	binfo_cb = NULL;
		    }
	        return;
        }
    }
    //erase all
    err = nrf_fstorage_erase(&binfo_fs_config, (uint32_t) bootInfo, sizeof(bootInfo), binfo_setWord_invalidateDone);
    if (err != NRF_SUCCESS)
    {
    	printf("binfo_setWord fs_erase: %d\r\n", err);
	   	binfo_cb = NULL;
    }
}

/*
uint32_t testbuf[MBR_PAGE_SIZE_IN_WORDS-4];
void binfo_test() {
    for(int j = 0; j < MBR_PAGE_SIZE_IN_WORDS-4; j++) {
        testbuf[j] = 0x7fffabcd;
    }
	//fs_store(&binfo_fs_config, bootInfo, testbuf, MBR_PAGE_SIZE_IN_WORDS-4, NULL);
    //fs_erase(&binfo_fs_config, bootInfo, 1, NULL);
}
*/

/*
void binfo_dump() {
    for(int i = 0; i < (4*4); i+=4) {
        for(int j = 0; j < 4; j++) {
            printf("%08X ", bootInfo[i+j]);
        }
        printf("\r\n");
    }
    printf("...\r\n");
    for(int i = MBR_PAGE_SIZE_IN_WORDS-(4*4); i < MBR_PAGE_SIZE_IN_WORDS; i+=4) {
        for(int j = 0; j < 4; j++) {
            printf("%08X ", bootInfo[i+j]);
        }
        printf("\r\n");
    }
}
*/

void binfo_init() {
	nrf_fstorage_init(&binfo_fs_config, p_fs_api, NULL);
}
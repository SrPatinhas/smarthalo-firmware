
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "nrf_mbr.h"
#include "fstorage.h"

#include "bootinfo.h"

#define MASK_OUTDATED 0x80000000

#define INVALIDATE 0x7fffffff

#define NOT_OUTDATED(x) ((x)&MASK_OUTDATED)

uint32_t bootInfo[MBR_PAGE_SIZE_IN_WORDS] __attribute__ ((section(".bootInfo")))
                                              __attribute__((used));

// Function prototypes
static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result);

FS_REGISTER_CFG(fs_config_t binfo_fs_config) =
{
    .callback       = fs_evt_handler,            // Function for event callbacks.
    .p_start_addr   = (uint32_t*)bootInfo,
    .p_end_addr     = (uint32_t*)bootInfo + MBR_PAGE_SIZE_IN_WORDS*sizeof(uint32_t)
};


static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result)
{
	//printf("fs_evt_handler %08X %08X %08X\r\n", evt->id, evt->p_context, result);
    if (evt->p_context)
    {
        ((fs_cb_t)evt->p_context)(evt, result);
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

static void binfo_setWord_writeDone(fs_evt_t const * const evt, fs_ret_t result)
{
	//printf("writeDone binfo_cb: %08X\r\n", binfo_cb);
	if(binfo_cb) {
		binfo_cb();
		binfo_cb = NULL;
	}
}

static void binfo_setWord_invalidateDone(fs_evt_t const * const evt, fs_ret_t result)
{
	uint32_t err;
    for(int i = 0; i < MBR_PAGE_SIZE_IN_WORDS; i++) {
        if(NOT_OUTDATED(bootInfo[i])) {
		    err = fs_store(&binfo_fs_config, &bootInfo[i], &binfo_w, 1, binfo_setWord_writeDone);
		    if (err != FS_SUCCESS)
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
		    err = fs_store(&binfo_fs_config, &bootInfo[i], &binfo_invalidate, 1, binfo_setWord_invalidateDone);
		    if (err != FS_SUCCESS)
		    {
		    	printf("binfo_setWord fs_store: %d\r\n", err);
		    	binfo_cb = NULL;
		    }
	        return;
        }
    }
    //erase all
    err = fs_erase(&binfo_fs_config, bootInfo, 1, binfo_setWord_invalidateDone);
    if (err != FS_SUCCESS)
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
	fs_init();
}
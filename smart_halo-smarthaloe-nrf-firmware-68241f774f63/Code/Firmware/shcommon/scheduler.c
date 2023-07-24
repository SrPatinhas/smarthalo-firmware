
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "platform.h"

#include "app_scheduler.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "nrf_sdm.h"
// #include "nrf_drv_wdt.h"
//#include "mem_manager.h"

#include "scheduler.h"

#define FPU_EXCEPTION_MASK 0x0000009F
void FPU_IRQHandler(void)
{
    uint32_t *fpscr = (uint32_t *)(FPU->FPCAR+0x40);
    (void)__get_FPSCR();
    *fpscr = *fpscr & ~(FPU_EXCEPTION_MASK);
}

#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVENT_DATA_SIZE, sizeof(void *))
#define SCHED_QUEUE_SIZE                10

#define SCH_TIMER_CNT APP_TIMER_OP_QUEUE_SIZE-1 //APP_TIMER_OP_QUEUE_SIZE

typedef struct {
	uint32_t id;
	sch_cb_ctx_t cb;
	void *ctx;
	bool active;
	bool unique;
	bool repeat;
	uint32_t ms;
} sch_entry_t;

app_timer_t sch_timers[SCH_TIMER_CNT];
sch_entry_t sch_entries[SCH_TIMER_CNT];

int32_t sch_entry_running;

#if 0 //!defined(DEBUG) && !defined(BOOTLOADER) 
nrf_drv_wdt_channel_id sch_wdt_channel_id;
#endif

void sch_run(void) {

    while (1)
    {
		app_sched_execute();
	    uint32_t err_code = sd_app_evt_wait();
	    if(err_code) {
	    	printf("sch_run, %ld\r\n", err_code);
		    APP_ERROR_CHECK(err_code);
	    }
#if 0 //!defined(DEBUG) && !defined(BOOTLOADER) 
	    //
	    // Would like to WDT feed before WFE to include interrupt code in the 2 sec window
	    // But cannot because of Errata [88] WDT: Increased current consumption when configured to pause in System ON idle
	    // WDT paused in WFE
	    nrf_drv_wdt_channel_feed(sch_wdt_channel_id);
	    //
#endif	    
	    //printf("A\r\n");
    }

}

void sch_timer_cb(void * ctx) {
	sch_entry_t *entry = (sch_entry_t *)ctx;

	sch_entry_running = (int32_t)entry->id;
	if(entry->cb) {
		entry->cb(entry->ctx);
	}
	if(entry->active) {
		if(entry->repeat) {
			app_timer_id_t timer_id = &sch_timers[entry->id];
			app_timer_stop(timer_id);
			app_timer_start(timer_id, APP_TIMER_TICKS(entry->ms), entry);
		} else {
			entry->active = false;		
		}
	}
	sch_entry_running = -1;

}

static void sch_bypass_cb(void * p_event_data, uint16_t event_size)
{
    app_timer_event_t * p_timer_event = (app_timer_event_t *)p_event_data;
    APP_ERROR_CHECK_BOOL(event_size == sizeof(app_timer_event_t));
    p_timer_event->timeout_handler(p_timer_event->p_context);
}

void sch_cancel(int32_t handle) {
	if(handle < 0 || handle >= SCH_TIMER_CNT) {
		return;
	}
	sch_entry_t *entry = &sch_entries[handle];
	app_timer_id_t timer_id = &sch_timers[handle];
	app_timer_stop(timer_id);
	entry->unique = false;
	entry->repeat = false;
	entry->cb = NULL;
	entry->ctx = NULL;
	entry->ms = 0;
	//
	entry->active = false;
}

bool sch_unique_isScheduled(sch_cb_ctx_t cb) {
	for(int i = 0; i < SCH_TIMER_CNT; i++) {
		if(sch_entries[i].active && sch_entries[i].cb == cb) {
			return true;
		}
	}
	return false;
}

void sch_unique_cancel(sch_cb_ctx_t cb) {
	for(int i = 0; i < SCH_TIMER_CNT; i++) {
		if(sch_entries[i].active && sch_entries[i].cb == cb) {
			sch_cancel(i);
			break;
		}
	}
}

int32_t sch_schedule(bool unique, bool repeat, sch_cb_ctx_t cb, uint32_t ms, void *ctx) {
	if(unique) {
		sch_unique_cancel(cb);
	}

	if(ms == 0) {
	    app_timer_event_t timer_event;
	    timer_event.timeout_handler = cb;
	    timer_event.p_context       = ctx;
	    return app_sched_event_put(&timer_event, sizeof(timer_event), sch_bypass_cb);
	}

	int32_t ptr = -1;
	CRITICAL_REGION_ENTER();
	for(int i = 0; i < SCH_TIMER_CNT; i++) {
		if((i != sch_entry_running) && !sch_entries[i].active) {
			ptr = i;
			sch_entries[ptr].active = true;
			break;
		}
	}
	CRITICAL_REGION_EXIT();

	if(ptr != -1) {
		//printf("sch %d\r\n", ptr);
		sch_entries[ptr].unique = unique;
		sch_entries[ptr].repeat = repeat;
		sch_entries[ptr].cb = cb;
		sch_entries[ptr].ctx = ctx;
		sch_entries[ptr].ms = ms;

		app_timer_id_t timer_id = &sch_timers[ptr];
		app_timer_start(timer_id, APP_TIMER_TICKS(ms), &sch_entries[ptr]);
		return ptr;
	} else {
		ERR_CHECK("SCH: out of resources", 1);
		return -1;
	}

}

int32_t sch_register(sch_cb_ctx_t cb, float freq, void *ctx) {
	uint32_t ms = (uint32_t)(1000. / freq);
	return sch_schedule(false, true, cb, ms, ctx);
}

int32_t sch_oneshot(sch_cb_ctx_t cb, uint32_t ms) {
	return sch_schedule(false, false, cb, ms, NULL);
}

int32_t sch_oneshot_ctx(sch_cb_ctx_t cb, uint32_t ms, void *ctx) {
	return sch_schedule(false, false, cb, ms, ctx);
}

void sch_unique_oneshot(sch_cb_ctx_t cb, uint32_t ms) {
	sch_schedule(true, false, cb, ms, NULL);
}

void sch_unique_oneshot_ctx(sch_cb_ctx_t cb, uint32_t ms, void *ctx) {
	sch_schedule(true, false, cb, ms, ctx);
}

#if 0 //!defined(DEBUG) && !defined(BOOTLOADER) 
void sch_wdt_event_handler(void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
    while(1) {}
}
#endif

void sch_init() {

    NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(FPU_IRQn);

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
	app_timer_init();
	sch_entry_running = -1;

	for(int i = 0; i < SCH_TIMER_CNT; i++) { //SCH_TIMER_CNT
		app_timer_id_t timer_id = &sch_timers[i];
		app_timer_create(&timer_id, APP_TIMER_MODE_SINGLE_SHOT, &sch_timer_cb);
		sch_entries[i].id = i;
		sch_entries[i].active = false;
		sch_entries[i].unique = false;
		sch_entries[i].repeat = false;
		sch_entries[i].cb = NULL;
		sch_entries[i].ctx = NULL;
		sch_entries[i].ms = 0;
	}

#if 0 //!defined(DEBUG) && !defined(BOOTLOADER) 
	uint32_t err_code;
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, sch_wdt_event_handler);
	ERR_CHECK("nrf_drv_wdt_init", err_code);
    err_code = nrf_drv_wdt_channel_alloc(&sch_wdt_channel_id);
	ERR_CHECK("nrf_drv_wdt_channel_alloc", err_code);
    nrf_drv_wdt_enable();
#endif

}

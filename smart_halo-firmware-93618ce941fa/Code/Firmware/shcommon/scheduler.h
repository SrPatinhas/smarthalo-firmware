
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          20                                          /**< Size of timer operation queues. */

typedef void (*sch_cb_ctx_t)( void * );

int32_t sch_register(sch_cb_ctx_t cb, float freq, void *ctx);
int32_t sch_oneshot(sch_cb_ctx_t cb, uint32_t ms);
int32_t sch_oneshot_ctx(sch_cb_ctx_t cb, uint32_t ms, void *ctx);
void sch_cancel(int32_t handle);
void sch_unique_oneshot(sch_cb_ctx_t cb, uint32_t ms);
void sch_unique_oneshot_ctx(sch_cb_ctx_t cb, uint32_t ms, void *ctx);
void sch_unique_cancel(sch_cb_ctx_t cb);
bool sch_unique_isScheduled(sch_cb_ctx_t cb);

void sch_init(void);

void sch_run(void);

#endif

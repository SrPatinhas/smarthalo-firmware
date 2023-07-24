/**
  ******************************************************************************
  * File Name          : OSWrappers.cpp
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#include <cassert>
#include <cmsis_os2.h>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/hal/OSWrappers.hpp>

static osSemaphoreId_t frame_buffer_sem = NULL;
static osMessageQueueId_t vsync_queue = NULL;

// Just a dummy value to insert in the VSYNC queue.
static uint32_t dummy = 0x5a;

using namespace touchgfx;

/*
 * Initialize frame buffer semaphore and queue/mutex for VSYNC signal.
 */
void OSWrappers::initialize()
{
    // Create a queue of length 1
    frame_buffer_sem = osSemaphoreNew(1, 1, NULL); // Binary semaphore
    assert((frame_buffer_sem != NULL) && "Creation of framebuffer semaphore failed");

    // Create a queue of length 1
    vsync_queue = osMessageQueueNew(1, 4, NULL);
    assert((vsync_queue != NULL) && "Creation of vsync message queue failed");
}

/*
 * Take the frame buffer semaphore. Blocks until semaphore is available.
 */
void OSWrappers::takeFrameBufferSemaphore()
{
    osSemaphoreAcquire(frame_buffer_sem, osWaitForever);
}

/*
 * Release the frame buffer semaphore.
 */
void OSWrappers::giveFrameBufferSemaphore()
{
    osSemaphoreRelease(frame_buffer_sem);
}

/*
 * Attempt to obtain the frame buffer semaphore. If semaphore is not available, do
 * nothing.
 *
 * Note must return immediately! This function does not care who has the taken the semaphore,
 * it only serves to make sure that the semaphore is taken by someone.
 */
void OSWrappers::tryTakeFrameBufferSemaphore()
{
    osSemaphoreAcquire(frame_buffer_sem, 0);
}

/*
 * Release the frame buffer semaphore in a way that is safe in interrupt context. Called
 * from ISR.
 *
 * Release the frame buffer semaphore in a way that is safe in interrupt context.
 * Called from ISR.
 */
void OSWrappers::giveFrameBufferSemaphoreFromISR()
{
    osSemaphoreRelease(frame_buffer_sem);
}

/*
 * Signal that a VSYNC has occurred. Should make the vsync queue/mutex available.
 *
 * Note This function is called from an ISR, and should (depending on OS) trigger a
 * scheduling.
 */
void OSWrappers::signalVSync()
{
    osMessageQueuePut(vsync_queue, &dummy, 0, 0);
}

/*
 * This function blocks until a VSYNC occurs.
 *
 * Note This function must first clear the mutex/queue and then wait for the next one to
 * occur.
 */
void OSWrappers::waitForVSync()
{
    uint32_t dummyGet;
    // First make sure the queue is empty, by trying to remove an element with 0 timeout.
    osMessageQueueGet(vsync_queue, &dummyGet, 0, 0);

    // Then, wait for next VSYNC to occur.
    osMessageQueueGet(vsync_queue, &dummyGet, 0, osWaitForever);
}

/*
 * A function that causes executing task to sleep for a number of milliseconds.
 *
 * A function that causes executing task to sleep for a number of milliseconds.
 * This function is OPTIONAL. It is only used by the TouchGFX in the case of
 * a specific frame refresh strategy (REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL).
 * Due to backwards compatibility, in order for this function to be useable by the HAL
 * the function must be explicitly registered:
 * hal.registerTaskDelayFunction(&OSWrappers::taskDelay)
 *
 * see HAL::setFrameRefreshStrategy(FrameRefreshStrategy s)
 * see HAL::registerTaskDelayFunction(void (*delayF)(uint16_t))
 */
void OSWrappers::taskDelay(uint16_t ms)
{
    osDelay(static_cast<uint32_t>(ms));
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

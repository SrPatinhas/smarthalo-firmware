#include <common/TouchGFXInit.hpp>
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <touchgfx/hal/GPIO.hpp>
#include <platform/driver/lcd/LCD24bpp.hpp>

#include <STM32F7HAL.hpp>
#include <STM32F7DMA.hpp>

#include <STM32F7TouchController.hpp>
#include <STM32F7Instrumentation.hpp>

/* USER CODE BEGIN user includes */

/* USER CODE END user includes */

/***********************************************************
 ******   Single buffer in internal RAM              *******
 ***********************************************************
 * On this platform, TouchGFX is able to run using a single
 * frame buffer in internal SRAM, thereby avoiding the need
 * for external SDRAM.
 * This feature was introduced in TouchGFX 4.7.0. To enable it,
 * uncomment the define below. The function touchgfx_init()
 * later in this file will check for this define and configure
 * TouchGFX accordingly.
 * For details on the single buffer strategy, please refer to
 * the knowledge base article "Single vs double buffering in TouchGFX"
 * on our support site.
 */
//#define SINGLE_FRAME_BUFFER_INTERNAL

/***********************************************************
 ******         24 Bits Per Pixel Support            *******
 ***********************************************************
 *
 * The default bit depth of the framebuffer is 16bpp. If you want 24bpp support, define the symbol "USE_BPP" with a value
 * of "24", e.g. "USE_BPP=24". This symbol affects the following:
 *
 * 1. Type of TouchGFX LCD (16bpp vs 24bpp)
 * 2. Bit depth of the framebuffer(s)
 * 3. TFT controller configuration.
 *
 * WARNING: Remember to modify your image formats accordingly in app/config/. Please see the following knowledgebase article
 * for further details on how to choose and configure the appropriate image formats for your application:
 *
 * https://touchgfx.zendesk.com/hc/en-us/articles/206725849
 */

#include "HW_Init.hpp"
extern "C"
{
#include "stm32f7xx.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_rcc_ex.h"
#include "stm32f7xx_hal_tim.h"
}

static uint32_t frameBuf0 = (uint32_t)(0xc0000000);
extern "C" {

    uint32_t LCD_GetXSize(void)
    {
        return 480;
    }

    uint32_t LCD_GetYSize(void)
    {
        return 272;
    }
}

void GRAPHICS_HW_Init()
{


    /* Initialize the SDRAM */
    MX_FMC_Init();
    MX_SDRAM_InitEx();

    /* Configure LCD */
    MX_LCD_Init();
    GPIO::init();

    //Deactivate speculative/cache access to first FMC Bank to save FMC bandwidth
    FMC_Bank1->BTCR[0] = 0x000030D2;
}

STM32F7DMA dma;
STM32F7TouchController tc;
STM32F7Instrumentation mcuInstr;

static LCD24bpp display;
static uint16_t bitdepth = 24;

namespace touchgfx
{
void touchgfx_init()
{
    uint16_t dispWidth = 480;
    uint16_t dispHeight = 272;

    HAL& hal = touchgfx_generic_init<STM32F7HAL>(dma, display, tc, dispWidth, dispHeight, (uint16_t*) 0,
                                                 0, 0);

    hal.setFrameBufferStartAddress((uint16_t*)frameBuf0, bitdepth, true, true);

    hal.setTouchSampleRate(2);
    hal.setFingerSize(1);

    // By default frame rate compensation is off.
    // Enable frame rate compensation to smooth out animations in case there is periodic slow frame rates.
    hal.setFrameRateCompensation(false);

    // This platform can handle simultaneous DMA and TFT accesses to SDRAM, so disable lock to increase performance.
    hal.lockDMAToFrontPorch(false);

    mcuInstr.init();

    //Set MCU instrumentation and Load calculation
    hal.setMCUInstrumentation(&mcuInstr);
    hal.enableMCULoadCalculation(true);
}
}

void GRAPHICS_Init()
{
    touchgfx::touchgfx_init();
}

void GRAPHICS_MainTask(void)
{
    touchgfx::HAL::getInstance()->taskEntry();
}

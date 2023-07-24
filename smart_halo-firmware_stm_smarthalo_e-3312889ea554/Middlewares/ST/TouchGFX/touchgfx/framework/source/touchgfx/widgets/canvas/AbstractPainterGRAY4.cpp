/**
  ******************************************************************************
  * This file is part of the TouchGFX 4.10.0 distribution.
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include <touchgfx/widgets/canvas/AbstractPainterGRAY4.hpp>
#include <touchgfx/Color.hpp>
#include <platform/driver/lcd/LCD4bpp.hpp>

namespace touchgfx
{
void AbstractPainterGRAY4::render(uint8_t* ptr,
                                  int x,
                                  int xAdjust,
                                  int y,
                                  unsigned count,
                                  const uint8_t* covers)
{
    currentX = x + areaOffsetX;
    currentY = y + areaOffsetY;
    x += xAdjust;
    if (renderInit())
    {
        do
        {
            uint8_t gray, alpha;
            if (renderNext(gray, alpha))
            {
                if (widgetAlpha < 255)
                {
                    alpha = static_cast<uint8_t>((alpha * widgetAlpha) / 255);
                }
                uint32_t combinedAlpha = ((*covers) * alpha) / 255;
                covers++;

                if (combinedAlpha == 255) // max alpha=255 on "*covers" and max alpha=255 on "widgetAlpha"
                {
                    // Render a solid pixel
                    renderPixel(ptr, x, gray);
                }
                else
                {
                    uint8_t p_gray = LCD4getPixel(ptr, x);
                    uint16_t ialpha = 0x100 - combinedAlpha;
                    renderPixel(ptr, x, static_cast<uint8_t>((p_gray * ialpha + gray * combinedAlpha) >> 8));
                }
            }
            currentX++;
            x++;
        }
        while (--count != 0);
    }
}

void AbstractPainterGRAY4::renderPixel(uint8_t* p, uint16_t offset, uint8_t gray)
{
    LCD4setPixel(p, offset, gray & 0x0F);
}
} // namespace touchgfx

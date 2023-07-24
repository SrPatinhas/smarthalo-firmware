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

#include <touchgfx/widgets/canvas/AbstractPainterRGB888.hpp>
#include <touchgfx/Color.hpp>

namespace touchgfx
{
void AbstractPainterRGB888::render(uint8_t* ptr,
                                   int x,
                                   int xAdjust,
                                   int y,
                                   unsigned count,
                                   const uint8_t* covers)
{
    uint8_t* p = ptr + ((x + xAdjust) * 3);

    currentX = x + areaOffsetX;
    currentY = y + areaOffsetY;
    if (renderInit())
    {
        do
        {
            uint8_t red, green, blue, alpha;
            if (renderNext(red, green, blue, alpha))
            {
                if (widgetAlpha < 255)
                {
                    alpha = static_cast<uint8_t>((alpha * widgetAlpha) / 255);
                }
                uint32_t combinedAlpha = (*covers) * alpha;
                covers++;

                if (combinedAlpha == (255u * 255u)) // max alpha=255 on "*covers" and max alpha=255 on "widgetAlpha"
                {
                    // Render a solid pixel
                    renderPixel(reinterpret_cast<uint16_t*>(p), red, green, blue);
                }
                else
                {
                    uint8_t p_blue = p[0];
                    uint8_t p_green = p[1];
                    uint8_t p_red = p[2];
                    renderPixel(reinterpret_cast<uint16_t*>(p),
                                static_cast<uint8_t>((((red - p_red)   * combinedAlpha) + (p_red << 16)) >> 16),
                                static_cast<uint8_t>((((green - p_green) * combinedAlpha) + (p_green << 16)) >> 16),
                                static_cast<uint8_t>((((blue - p_blue)  * combinedAlpha) + (p_blue << 16)) >> 16));
                }
            }
            p += 3;
            currentX++;
        }
        while (--count != 0);
    }
}

void AbstractPainterRGB888::renderPixel(uint16_t* p, uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t* p8 = reinterpret_cast<uint8_t*>(p);
    p8[0] = blue;
    p8[1] = green;
    p8[2] = red;
}
} // namespace touchgfx

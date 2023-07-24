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

#include <touchgfx/widgets/canvas/PainterGRAY2.hpp>
#include <touchgfx/Color.hpp>
#include <platform/driver/lcd/LCD2bpp.hpp>

namespace touchgfx
{
PainterGRAY2::PainterGRAY2(colortype color, uint8_t alpha) :
    AbstractPainterGRAY2()
{
    setColor(color, alpha);
}

void PainterGRAY2::setColor(colortype color, uint8_t alpha)
{
    painterGray = (uint8_t)color;
    painterAlpha = alpha;
}

touchgfx::colortype PainterGRAY2::getColor() const
{
    return static_cast<colortype>(painterGray);
}

void PainterGRAY2::setAlpha(uint8_t alpha)
{
    painterAlpha = alpha;
}

uint8_t PainterGRAY2::getAlpha() const
{
    return painterAlpha;
}

void PainterGRAY2::render(uint8_t* ptr,
                          int x,
                          int xAdjust,
                          int y,
                          unsigned count,
                          const uint8_t* covers)
{
    currentX = x + areaOffsetX;
    currentY = y + areaOffsetY;
    x += xAdjust;
    uint8_t totalAlpha = (widgetAlpha * painterAlpha) / 255u;
    if (totalAlpha == 255)
    {
        do
        {
            uint8_t alpha = *covers;
            covers++;

            if (alpha == 255)
            {
                // Render a solid pixel
                LCD2setPixel(ptr, x, painterGray);
            }
            else
            {
                uint8_t p_gray = LCD2getPixel(ptr, x);
                LCD2setPixel(ptr, x, static_cast<uint8_t>((((painterGray - p_gray) * alpha) >> 8) + p_gray));
            }
            currentX++;
            x++;
        }
        while (--count != 0);
    }
    else if (totalAlpha != 0)
    {
        do
        {
            uint16_t alpha = (*covers) * totalAlpha;
            covers++;

            uint8_t p_gray = LCD2getPixel(ptr, x);
            LCD2setPixel(ptr, x, static_cast<uint8_t>((((painterGray - p_gray) * alpha) >> 16) + p_gray));
            currentX++;
            x++;
        }
        while (--count != 0);
    }
}

bool PainterGRAY2::renderNext(uint8_t& gray, uint8_t& alpha)
{
    gray = painterGray;
    alpha = painterAlpha;
    return true;
}
} // namespace touchgfx

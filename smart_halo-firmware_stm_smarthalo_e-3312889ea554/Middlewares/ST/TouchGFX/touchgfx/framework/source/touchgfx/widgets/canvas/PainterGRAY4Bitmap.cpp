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

#include <touchgfx/widgets/canvas/PainterGRAY4Bitmap.hpp>
#include <platform/driver/lcd/LCD4bpp.hpp>

namespace touchgfx
{
PainterGRAY4Bitmap::PainterGRAY4Bitmap(const Bitmap& bmp, uint8_t alpha) :
    AbstractPainterGRAY4(), bitmapGRAY4Pointer(0), bitmapAlphaPointer(0)
{
    setBitmap(bmp);
    setAlpha(alpha);
}

void PainterGRAY4Bitmap::setBitmap(const Bitmap& bmp)
{
    bitmap = bmp;
    bitmapRectToFrameBuffer = bitmap.getRect();
    DisplayTransformation::transformDisplayToFrameBuffer(bitmapRectToFrameBuffer);
}

void PainterGRAY4Bitmap::setAlpha(uint8_t alpha)
{
    painterAlpha = alpha;
}

uint8_t PainterGRAY4Bitmap::getAlpha() const
{
    return painterAlpha;
}

void PainterGRAY4Bitmap::render(uint8_t* ptr, int x, int xAdjust, int y, unsigned count, const uint8_t* covers)
{
    currentX = x + areaOffsetX;
    currentY = y + areaOffsetY;
    x += xAdjust;

    if (!renderInit())
    {
        return;
    }

    if (currentX + (int)count > bitmapRectToFrameBuffer.width)
    {
        count = bitmapRectToFrameBuffer.width - currentX;
    }

    uint8_t totalAlpha = (widgetAlpha * painterAlpha) / 255;
    // Get alpha data (GRAY4 format)
    if (bitmapAlphaPointer)
    {
        if (totalAlpha == 255)
        {
            do
            {
                uint8_t gray = LCD4getPixel(bitmapGRAY4Pointer, currentX);
                uint16_t alpha = (*covers) * LCD4getPixel(bitmapAlphaPointer, currentX);
                covers++;

                if (alpha == 255u * 15u)
                {
                    // Render a solid pixel
                    LCD4setPixel(ptr, x, gray);
                }
                else
                {
                    uint8_t p_gray = LCD4getPixel(ptr, x);
                    LCD4setPixel(ptr, x, static_cast<uint8_t>((((gray - p_gray) * alpha) >> 12) + p_gray));
                }
                currentX++;
                x++;
            }
            while (--count != 0);
        }
        else
        {
            do
            {
                uint8_t gray = LCD4getPixel(bitmapGRAY4Pointer, currentX);
                uint32_t alpha = (*covers) * totalAlpha * LCD4getPixel(bitmapAlphaPointer, currentX);
                covers++;

                uint8_t p_gray = LCD4getPixel(ptr, x);
                LCD4setPixel(ptr, x, static_cast<uint8_t>((((gray - p_gray) * alpha) >> 20) + p_gray));
                currentX++;
                x++;
            }
            while (--count != 0);
        }
    }
    else
    {
        if (totalAlpha == 255)
        {
            do
            {
                uint8_t gray = LCD4getPixel(bitmapGRAY4Pointer, currentX);
                uint8_t alpha = (*covers);
                covers++;

                if (alpha == 255)
                {
                    // Render a solid pixel
                    LCD4setPixel(ptr, x, gray);
                }
                else
                {
                    uint8_t p_gray = LCD4getPixel(ptr, x);
                    LCD4setPixel(ptr, x, static_cast<uint8_t>((((gray - p_gray) * alpha) >> 8) + p_gray));
                }
                currentX++;
                x++;
            }
            while (--count != 0);
        }
        else
        {
            do
            {
                uint8_t gray = LCD4getPixel(bitmapGRAY4Pointer, currentX);
                uint16_t alpha = (*covers) * totalAlpha;
                covers++;

                uint8_t p_gray = LCD4getPixel(ptr, x);
                LCD4setPixel(ptr, x, static_cast<uint8_t>((((gray - p_gray) * alpha) >> 16) + p_gray));
                currentX++;
                x++;
            }
            while (--count != 0);
        }
    }
}

bool PainterGRAY4Bitmap::renderInit()
{
    bitmapGRAY4Pointer = 0;
    bitmapAlphaPointer = 0;

    if ((currentX >= bitmapRectToFrameBuffer.width) ||
            (currentY >= bitmapRectToFrameBuffer.height))
    {
        // Outside bitmap area, do not draw anything
        return false;
    }

    bitmapGRAY4Pointer = (const uint8_t*)bitmap.getData();
    if (!bitmapGRAY4Pointer)
    {
        return false;
    }
    bitmapGRAY4Pointer += currentY * ((bitmapRectToFrameBuffer.width + 1) / 2);
    // Get alpha data (GRAY4 format)
    bitmapAlphaPointer = (const uint8_t*)bitmap.getAlphaData();
    if (bitmapAlphaPointer)
    {
        bitmapAlphaPointer += currentY * ((bitmapRectToFrameBuffer.width + 1) / 2);
    }

    return true;
}

bool PainterGRAY4Bitmap::renderNext(uint8_t& gray, uint8_t& alpha)
{
    if (currentX >= bitmapRectToFrameBuffer.width)
    {
        return false;
    }

    if (bitmapGRAY4Pointer != 0)
    {
        gray = LCD4getPixel(bitmapGRAY4Pointer, currentX);
        if (bitmapAlphaPointer)
        {
            alpha = LCD4getPixel(bitmapAlphaPointer, currentX);
            alpha |= alpha << 4; // Upscale from 0-15 to 0-255
        }
        else
        {
            alpha = 255; // No alpha per pixel in the image, it is solid
        }
    }
    if (painterAlpha < 255)
    {
        // Apply given alpha from setAlpha()
        alpha = (((uint16_t)alpha) * ((uint16_t)painterAlpha)) / 255;
    }
    return true;
}
} // namespace touchgfx

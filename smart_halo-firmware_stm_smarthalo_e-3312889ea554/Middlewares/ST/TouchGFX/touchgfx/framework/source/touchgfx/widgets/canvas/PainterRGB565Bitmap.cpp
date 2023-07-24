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

#include <touchgfx/widgets/canvas/PainterRGB565Bitmap.hpp>

namespace touchgfx
{
PainterRGB565Bitmap::PainterRGB565Bitmap(const Bitmap& bmp, uint8_t alpha) :
    AbstractPainterRGB565(), bitmapARGB8888Pointer(0), bitmapRGB565Pointer(0), bitmapAlphaPointer(0)
{
    setBitmap(bmp);
    setAlpha(alpha);
}

void PainterRGB565Bitmap::setBitmap(const Bitmap& bmp)
{
    bitmap = bmp;
    bitmapRectToFrameBuffer = bitmap.getRect();
    DisplayTransformation::transformDisplayToFrameBuffer(bitmapRectToFrameBuffer);
}

void PainterRGB565Bitmap::setAlpha(uint8_t alpha)
{
    painterAlpha = alpha;
}

uint8_t PainterRGB565Bitmap::getAlpha() const
{
    return painterAlpha;
}

void PainterRGB565Bitmap::render(uint8_t* ptr,
                                 int x,
                                 int xAdjust,
                                 int y,
                                 unsigned count,
                                 const uint8_t* covers)
{
    uint16_t* p = reinterpret_cast<uint16_t*>(ptr) + (x + xAdjust);

    currentX = x + areaOffsetX;
    currentY = y + areaOffsetY;

    if (!renderInit())
    {
        return;
    }

    if (currentX + (int)count > bitmapRectToFrameBuffer.width)
    {
        count = bitmapRectToFrameBuffer.width - currentX;
    }

    uint8_t totalAlpha = (widgetAlpha * painterAlpha) / 255u;
    if (bitmap.getFormat() == Bitmap::RGB565)
    {
        const uint16_t* src = bitmapRGB565Pointer;
        const uint8_t* srcAlpha = bitmapAlphaPointer;
        if (srcAlpha)
        {
            if (totalAlpha == 255)
            {
                do
                {
                    uint32_t alpha = (*covers) * (*srcAlpha);
                    covers++;
                    srcAlpha++;
                    if (alpha == 255u * 255u)
                    {
                        // Solid pixel
                        *p = *src;
                    }
                    else if (alpha)
                    {
                        // Non-Transparent pixel
                        *p = mixColors(*src, *p, alpha, 16);
                    }
                    p++;
                    src++;
                }
                while (--count != 0);
            }
            else
            {
                do
                {
                    uint32_t alpha = ((*covers) * (*srcAlpha) * totalAlpha) >> 8;
                    covers++;
                    srcAlpha++;
                    if (alpha) // This can never get to max=255u*255u as widgetAlpha<255
                    {
                        // Non-Transparent pixel
                        *p = mixColors(*src, *p, alpha, 16);
                    }
                    p++;
                    src++;
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
                    //use alpha from covers directly
                    uint32_t alpha = *covers++;
                    if (alpha == 255u)
                    {
                        // Solid pixel
                        *p = *src;
                    }
                    else
                    {
                        // Non-Transparent pixel
                        *p = mixColors(*src, *p, alpha, 8);
                    }
                    p++;
                    src++;
                }
                while (--count != 0);
            }
            else
            {
                do
                {
                    uint32_t alpha = (*covers) * totalAlpha;
                    covers++;

                    *p = mixColors(*src, *p, alpha, 16);

                    p++;
                    src++;
                }
                while (--count != 0);
            }
        }
    }
    else if (bitmap.getFormat() == Bitmap::ARGB8888)
    {
        const uint32_t* src = bitmapARGB8888Pointer;
        if (totalAlpha == 255)
        {
            uint32_t newpix;
            do
            {
                uint8_t srcAlpha = *src >> 24;
                uint32_t alpha = (*covers) * srcAlpha;
                covers++;
                newpix = *src;
                if (alpha == 255u * 255u)
                {
                    // Solid pixel
                    *p = ((newpix >> 8) & RMASK) | ((newpix >> 5) & GMASK) | ((newpix >> 3) & BMASK);
                }
                else if (alpha)
                {
                    // Non-Transparent pixel
                    *p = mixColors((newpix >> 8) & RMASK, (newpix >> 5) & GMASK, (newpix >> 3) & BMASK, *p, alpha, 16);
                }
                p++;
                src++;
            }
            while (--count != 0);
        }
        else
        {
            uint32_t newpix;
            do
            {
                uint8_t srcAlpha = *src >> 24;
                uint32_t alpha = ((*covers) * srcAlpha * totalAlpha) >> 8;
                covers++;
                if (alpha)
                {
                    // Non-Transparent pixel
                    newpix = *src;
                    *p = mixColors((newpix >> 8) & RMASK, (newpix >> 5) & GMASK, (newpix >> 3) & BMASK, *p, alpha, 16);
                }
                p++;
                src++;
            }
            while (--count != 0);
        }
    }
}

bool PainterRGB565Bitmap::renderInit()
{
    bitmapARGB8888Pointer = 0;
    bitmapRGB565Pointer = 0;
    bitmapAlphaPointer = 0;

    if ((currentX >= bitmapRectToFrameBuffer.width) ||
            (currentY >= bitmapRectToFrameBuffer.height))
    {
        // Outside bitmap area, do not draw anything
        // Consider the following instead of "return" to get a tiled image:
        //   currentX %= bitmapRectToFrameBuffer.width
        //   currentY %= bitmapRectToFrameBuffer.height
        return false;
    }

    if (bitmap.getFormat() == Bitmap::ARGB8888)
    {
        bitmapARGB8888Pointer = (const uint32_t*)bitmap.getData();
        if (!bitmapARGB8888Pointer)
        {
            return false;
        }
        bitmapARGB8888Pointer += currentX + currentY * bitmapRectToFrameBuffer.width;
    }
    else
    {
        bitmapRGB565Pointer = (const uint16_t*)bitmap.getData();
        if (!bitmapRGB565Pointer)
        {
            return false;
        }
        bitmapRGB565Pointer += currentX + currentY * bitmapRectToFrameBuffer.width;
        // Get alpha data (RGB565 format)
        bitmapAlphaPointer = (const uint8_t*)bitmap.getAlphaData();
        if (bitmapAlphaPointer)
        {
            bitmapAlphaPointer += currentX + currentY * bitmapRectToFrameBuffer.width;
        }
    }

    return true;
}

bool PainterRGB565Bitmap::renderNext(uint8_t& red, uint8_t& green, uint8_t& blue, uint8_t& alpha)
{
    if (currentX >= bitmapRectToFrameBuffer.width)
    {
        return false;
    }

    if (bitmapARGB8888Pointer != 0)
    {
        uint32_t argb8888 = *bitmapARGB8888Pointer++;
        alpha = (argb8888 >> 24) & 0xFF;
        red = (argb8888 >> 16) & 0xFF;
        green = (argb8888 >> 8) & 0xFF;
        blue = (argb8888) & 0xFF;
    }
    else if (bitmapRGB565Pointer != 0)
    {
        uint16_t rgb565 = *bitmapRGB565Pointer++;
        red = (rgb565 & 0xF800) >> 8;
        red |= red >> 5; // To get full range 0-0xFF, not just 0-0xF8
        green = (rgb565 & 0x07E0) >> 3;
        green |= green >> 6; // To get full range 0-0xFF, not just 0-0xFC
        blue = (rgb565 & 0x001F) << 3;
        blue |= (blue >> 5); // To get full range 0-0xFF, not just 0-0xF8
        if (bitmapAlphaPointer)
        {
            alpha = *bitmapAlphaPointer++;
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

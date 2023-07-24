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

#include <touchgfx/widgets/canvas/PainterBWBitmap.hpp>
#include <platform/driver/lcd/LCD1bpp.hpp>

namespace touchgfx
{
PainterBWBitmap::PainterBWBitmap(const Bitmap& bmp) :
    AbstractPainterBW(), bitmapBWPointer(0)
{
    setBitmap(bmp);
}

void PainterBWBitmap::setBitmap(const Bitmap& bmp)
{
    bitmap = bmp;
    bitmapRectToFrameBuffer = bitmap.getRect();
    DisplayTransformation::transformDisplayToFrameBuffer(bitmapRectToFrameBuffer);
}

// Found in LCD1bpp
void fillBits(uint8_t* fb, uint16_t startX, uint16_t startY, uint16_t stride, uint16_t count, uint8_t color);

void PainterBWBitmap::render(uint8_t* ptr, int x, int xAdjust, int y, unsigned count, const uint8_t* covers)
{
    (void)covers;
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

    if (bitmap.getFormat() == Bitmap::BW_RLE)
    {
        while (count)
        {
            uint32_t length = bw_rle.getLength();
            uint16_t bitsToDraw = (uint16_t)MIN(length, (uint32_t)count);

            fillBits(ptr, x, 0, 0 /* not used */, bitsToDraw, bw_rle.getColor());
            x += bitsToDraw;
            count -= bitsToDraw;
            bw_rle.skipNext(bitsToDraw);
        }
    }
    else
    {
        unsigned char* p = ptr + (x / 8);
        const uint8_t* src = bitmapBWPointer + currentX / 8;
        uint8_t* RESTRICT dst = p;
        uint16_t srcBitX = currentX % 8; // & 7
        uint16_t dstBitX = x % 8; // & 7

        uint16_t remainingBits = count;

        if (dstBitX > 0)
        {
            // Start by getting (dst-)aligned for faster transfer
            uint16_t neededBits = 8 - dstBitX;
            if (neededBits > remainingBits)
            {
                neededBits = remainingBits;    // Very narrow src inside same word
            }
            uint16_t availableBits = 8 - srcBitX;
            uint8_t mask = (1u << neededBits) - 1u;
            uint8_t dstShift = static_cast<uint8_t>(8u - (dstBitX + neededBits));
            mask <<= dstShift;

            uint8_t word = *src;

            if (availableBits > neededBits)
            {
                word >>= availableBits - neededBits;
            }
            else if (availableBits < neededBits)
            {
                // Get the last required bits from src[1]
                word <<= neededBits - availableBits;
                word |= src[1] >> (8u - (neededBits - availableBits));
            }

            word <<= dstShift;
            *dst = (*dst & ~mask) | (word & mask);

            srcBitX = (srcBitX + neededBits) % 8; // & 7

            if (availableBits <= neededBits)
            {
                src++;
            }
            dst++;
            remainingBits -= neededBits;
        }

        // dstX is now word aligned (or we have transferred everything of a narrow image and remainingBits==0)
        if (remainingBits >= 8)
        {
            uint16_t bytesPerLine = remainingBits / 8;
            if (srcBitX == 0)
            {
                touchgfx::HAL::getInstance()->blockCopy(dst, src, bytesPerLine);
                src += bytesPerLine;
                dst += bytesPerLine;
            }
            else
            {
                uint16_t _remainingBits = remainingBits;

                remainingBits = _remainingBits;
                while (remainingBits >= 8)
                {
                    uint8_t word = *src++;
                    word <<= srcBitX;
                    word |= (*src) >> (8 - srcBitX);
                    *dst++ = word;
                    remainingBits -= 8;
                }
            }
            remainingBits %= 8; // &= 7
        }

        // Take the last bits, again we need to mask dst
        if (remainingBits > 0)
        {
            uint8_t word = *src;
            if (srcBitX != 0)
            {
                word <<= srcBitX;
                word |= src[1] >> (8u - srcBitX);
            }
            uint8_t mask = ((1u << remainingBits) - 1u) << (8u - remainingBits);
            *dst = (*dst & ~mask) | (word & mask);
        }
    }
}

bool PainterBWBitmap::renderInit()
{
    bw_rle = 0; // Used to remember if format is BW or BW_RLE
    bitmapBWPointer = 0;

    if (currentX >= bitmapRectToFrameBuffer.width ||
            currentY >= bitmapRectToFrameBuffer.height)
    {
        // Outside bitmap area, do not draw anything
        return false;
    }

    bitmapBWPointer = (const uint8_t*)bitmap.getData();
    if (!bitmapBWPointer)
    {
        return false;
    }
    if (bitmap.getFormat() == Bitmap::BW_RLE)
    {
        bw_rle.init(bitmapBWPointer);
        uint32_t skip = (int32_t)currentY * (int32_t)bitmapRectToFrameBuffer.width + (int32_t)currentX;
        bw_rle.skipNext(skip);
    }
    else
    {
        bitmapBWPointer += currentY * ((bitmapRectToFrameBuffer.width + 7) / 8);
    }
    return true;
}

bool PainterBWBitmap::renderNext(uint8_t& color)
{
    if (currentX >= bitmapRectToFrameBuffer.width || bitmapBWPointer == 0)
    {
        return false;
    }

    if (bitmap.getFormat() == Bitmap::BW_RLE)
    {
        color = bw_rle.getColor();
        bw_rle.skipNext(1);
    }
    else
    {
        const uint8_t* src = bitmapBWPointer + currentX / 8;
        color = (*src >> (7 - (currentX % 8))) & 1;
    }
    return true;
}
} // namespace touchgfx

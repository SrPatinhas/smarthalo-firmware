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

#ifndef ABSTRACTPAINTERRGB888_HPP
#define ABSTRACTPAINTERRGB888_HPP

#include <assert.h>
#include <touchgfx/widgets/canvas/AbstractPainter.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>

namespace touchgfx
{
/**
 * @class AbstractPainterRGB888 AbstractPainterRGB888.hpp touchgfx/widgets/canvas/AbstractPainterRGB888.hpp
 *
 * @brief A Painter that will paint using a color and an alpha value.
 *
 *        The AbstractPainterRGB888 class allows a shape to be filled with a given color and
 *        alpha value. This allows transparent, anti-aliased elements to be drawn.
 *
 * @see AbstractPainter
 */
class AbstractPainterRGB888 : public AbstractPainter
{
public:
    AbstractPainterRGB888()
    {
        assert(HAL::lcd().bitDepth() == 24 && "The chosen painter only works with 24bpp displays");
    }

    virtual ~AbstractPainterRGB888() {}

    virtual void render(uint8_t* ptr, int x, int xAdjust, int y, unsigned count, const uint8_t* covers);

protected:

    /**
     * @fn virtual bool AbstractPainterRGB888::renderInit()
     *
     * @brief Initialize rendering of a single scan line of pixels for the render.
     *
     *        Initialize rendering of a single scan line of pixels for the render.
     *
     * @return true if it succeeds, false if it fails.
     */
    virtual bool renderInit()
    {
        return true;
    }

    /**
     * @fn virtual bool AbstractPainterRGB888::renderNext(uint8_t& red, uint8_t& green, uint8_t& blue, uint8_t& alpha) = 0;
     *
     * @brief Get the color of the next pixel in the scan line.
     *
     *        Get the color of the next pixel in the scan line.
     *
     * @param [out] red   The red.
     * @param [out] green The green.
     * @param [out] blue  The blue.
     * @param [out] alpha The alpha.
     *
     * @return true if the pixel should be painted, false otherwise.
     */
    virtual bool renderNext(uint8_t& red, uint8_t& green, uint8_t& blue, uint8_t& alpha) = 0;

    /**
     * @fn virtual void AbstractPainterRGB888::renderPixel(uint16_t* p, uint8_t red, uint8_t green, uint8_t blue);
     *
     * @brief Renders the pixel.
     *
     *        Renders the pixel into the frame buffer. The colors are reduced from 8,8,8 to 5,6,
     *        5.
     *
     * @param [in] p pointer into the frame buffer where the given color should be written.
     * @param red    The red color.
     * @param green  The green color.
     * @param blue   The blue color.
     */
    virtual void renderPixel(uint16_t* p, uint8_t red, uint8_t green, uint8_t blue);

    int currentX; ///< Current x coordinate relative to the widget
    int currentY; ///< Current y coordinate relative to the widget
}; // class AbstractPainterRGB888
} // namespace touchgfx

#endif // ABSTRACTPAINTERRGB888_HPP

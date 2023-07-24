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

#ifndef COLOR_HPP
#define COLOR_HPP

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>

namespace touchgfx
{
/**
 * @class Color Color.hpp touchgfx/Color.hpp
 *
 * @brief Contains functionality for color conversion.
 *
 *        Contains functionality for color conversion.
 */
class Color
{
public:

    /**
     * @fn static colortype Color::getColorFrom24BitRGB(uint8_t red, uint8_t green, uint8_t blue);
     *
     * @brief Generates a color representation to be used on the LCD, based on 24 bit RGB values.
     *        Depending on your chosen color bit depth, the color will be interpreted
     *        internally as either a 16 bit or 24 bit color value.
     *
     *        Generates a color representation to be used on the LCD, based on 24 bit RGB
     *        values. Depending on your chosen color bit depth, the color will be interpreted
     *        internally as either a 16 bit or 24 bit color value. This function can be safely
     *        used regardless of whether your application is configured for 16 or 24 bit colors.
     *
     * @param red   Value of the red part (0-255).
     * @param green Value of the green part (0-255).
     * @param blue  Value of the blue part (0-255).
     *
     * @return The color, encoded in a 16- or 24-bit representation depending on LCD color depth.
     */
    static colortype getColorFrom24BitRGB(uint8_t red, uint8_t green, uint8_t blue);

    /**
     * @fn static inline uint8_t Color::getRedColor(colortype color)
     *
     * @brief Gets the red color part of a color.
     *
     *        Gets the red color part of a color. As this function must work for all color depths, it can be somewhat slow if used in speed critical sections. Consider finding the color in another way, if possible.
     *
     * @param color The color value.
     *
     * @return The red part of the color.
     */
    static inline uint8_t getRedColor(colortype color)
    {
        uint8_t bitDepth = HAL::lcd().bitDepth();
        return bitDepth == 16 ? ((color & 0xF800) >> 8) : bitDepth == 24 ? ((color.getColor32() >> 16) & 0xFF) : bitDepth == 4 ? ((color & 0xF) * 0x11) : bitDepth == 2 ? ((color & 0x3) * 0x55) : 0;
    }

    /**
     * @fn static inline uint8_t Color::getGreenColor(colortype color)
     *
     * @brief Gets the green color part of a color.
     *
     *        Gets the green color part of a color. As this function must work for all color depths, it can be somewhat slow if used in speed critical sections. Consider finding the color in another way, if possible.
     *
     * @param color The 16 bit color value.
     *
     * @return The green part of the color.
     */
    static inline uint8_t getGreenColor(colortype color)
    {
        uint8_t bitDepth = HAL::lcd().bitDepth();
        return bitDepth == 16 ? ((color & 0x07E0) >> 3) : bitDepth == 24 ? ((color.getColor32() >> 8) & 0xFF) : bitDepth == 4 ? ((color & 0xF) * 0x11) : bitDepth == 2 ? ((color & 0x3) * 0x55) : 0;
    }

    /**
     * @fn static inline uint8_t Color::getBlueColor(colortype color)
     *
     * @brief Gets the blue color part of a color.
     *
     *        Gets the blue color part of a color. As this function must work for all color depths, it can be somewhat slow if used in speed critical sections. Consider finding the color in another way, if possible.
     *
     * @param color The 16 bit color value.
     *
     * @return The blue part of the color.
     */
    static inline uint8_t getBlueColor(colortype color)
    {
        uint8_t bitDepth = HAL::lcd().bitDepth();
        return bitDepth == 16 ? ((color & 0x001F) << 3) : bitDepth == 24 ? (color.getColor32() & 0xFF) : bitDepth == 4 ? ((color & 0xF) * 0x11) : bitDepth == 2 ? ((color & 0x3) * 0x55) : 0;
    }
};
} // namespace touchgfx

#endif // COLOR_HPP

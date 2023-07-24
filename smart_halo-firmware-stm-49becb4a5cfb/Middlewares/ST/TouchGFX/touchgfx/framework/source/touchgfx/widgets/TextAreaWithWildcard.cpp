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

#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

namespace touchgfx
{
int16_t TextAreaWithWildcardBase::calculateTextHeight(const Unicode::UnicodeChar* format, ...) const
{
    if (!typedText.hasValidId())
    {
        return 0;
    }

    va_list pArg;
    va_start(pArg, format);

    TextProvider textProvider;
    textProvider.initialize(format, pArg);

    int16_t numLines = HAL::lcd().getNumLines(textProvider, wideTextAction, typedText.getTextDirection(), typedText.getFont(), getWidth());

    const Font* fontToDraw = typedText.getFont();
    int16_t textHeight = fontToDraw->getMinimumTextHeight();

    va_end(pArg);
    return numLines * textHeight + (numLines - 1) * linespace;
}
} // namespace touchgfx

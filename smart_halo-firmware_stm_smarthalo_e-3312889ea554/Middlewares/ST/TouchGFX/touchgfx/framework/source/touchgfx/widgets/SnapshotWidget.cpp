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

#include <touchgfx/widgets/SnapshotWidget.hpp>

namespace touchgfx
{
SnapshotWidget::SnapshotWidget() : Widget(), fbCopy(0), alpha(255)
{
}

SnapshotWidget::~SnapshotWidget()
{
}

void SnapshotWidget::draw(const Rect& invalidatedArea) const
{
    if (!fbCopy)
    {
        return;
    }

    Rect absRect;
    translateRectToAbsolute(absRect);
    absRect.width = rect.width;
    absRect.height = rect.height;
    HAL::lcd().blitCopy(fbCopy, absRect, invalidatedArea, alpha, false);
}

Rect SnapshotWidget::getSolidRect() const
{
    if (alpha < 255)
    {
        return Rect(0, 0, 0, 0);
    }

    if (!fbCopy)
    {
        return Rect(0, 0, 0, 0);
    }
    else
    {
        return Rect(0, 0, getWidth(), getHeight());
    }
}

void SnapshotWidget::makeSnapshot()
{
    fbCopy = reinterpret_cast<uint16_t*>(HAL::lcd().copyFrameBufferRegionToMemory(rect));
}

void SnapshotWidget::makeSnapshot(const BitmapId bmp)
{
    fbCopy = reinterpret_cast<uint16_t*>(HAL::lcd().copyFrameBufferRegionToMemory(rect, bmp));
}
} // namespace touchgfx

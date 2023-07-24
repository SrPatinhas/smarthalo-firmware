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

#ifndef GESTURES_HPP
#define GESTURES_HPP

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/events/ClickEvent.hpp>
#include <touchgfx/events/DragEvent.hpp>
#include <touchgfx/events/GestureEvent.hpp>
#include <touchgfx/UIEventListener.hpp>

namespace touchgfx
{
/**
 * @class Gestures Gestures.hpp touchgfx/hal/Gestures.hpp
 *
 * @brief This class implements the detection of gestures.
 *
 *        This class implements the detection of gestures.
 */
class Gestures
{
    static const uint8_t MAX_TICKS_BETWEEN_MOVES_FOR_SWIPE = 7;
    static const uint8_t MIN_VELOCITY_AT_RELEASE_BEFORE_SWIPE = 3;

    /**
     * @struct DragState Gestures.hpp touchgfx/hal/Gestures.hpp
     *
     * @brief Defines the state of a drag.
     *
     *        Defines the state of a drag.
     */
    struct DragState
    {
        DragState() : startX(0),
            startY(0),
            downX(0),
            downY(0),
            tickCount(0),
            velocityX(0),
            velocityY(0),
            inProgress(false)
        {
        }

        uint16_t startX;     ///< Starting x coordinate
        uint16_t startY;     ///< Starting y coordinate
        uint16_t downX;      ///< Starting x coordinate of the drag
        uint16_t downY;      ///< Starting x coordinate of the drag
        uint16_t tickCount;  ///< Measures the timing of the drag
        int16_t  velocityX;  ///< The velocity (X orientation) of the drag
        int16_t  velocityY;  ///< The velocity (Y orientation) of the drag
        bool     inProgress; ///< Whether a drag is in progress or not
    };

public:

    /**
     * @fn Gestures::Gestures()
     *
     * @brief Default constructor.
     *
     *        Default constructor. Does nothing.
     */
    Gestures() : listener(0), dragThresholdValue(0) { }

    /**
     * @fn void Gestures::registerEventListener(UIEventListener& l);
     *
     * @brief Register the event listener.
     *
     *        Register the event listener.
     *
     * @param [in] l The EventListener to register.
     */
    void registerEventListener(UIEventListener& l);

    /**
     * @fn void Gestures::tick();
     *
     * @brief Has to be called during the timer tick.
     *
     *        Has to be called during the timer tick.
     */
    void tick();

    /**
     * @fn bool Gestures::registerDragEvent(uint16_t oldX, uint16_t oldY, uint16_t newX, uint16_t newY);
     *
     * @brief Register a drag event.
     *
     *        Register a drag event.
     *
     * @param oldX The x coordinate of the drag start position (dragged from)
     * @param oldY The y coordinate of the drag start position (dragged from)
     * @param newX The x coordinate of the new position (dragged to)
     * @param newY The y coordinate of the new position (dragged to)
     *
     * @return True if the drag exceeds threshold value (and therefore was reported as a drag),
     *         or false if the drag did not exceed threshold (and therefore was discarded).
     */
    bool registerDragEvent(uint16_t oldX, uint16_t oldY, uint16_t newX, uint16_t newY);

    /**
     * @fn void Gestures::registerClickEvent(ClickEvent::ClickEventType evt, uint16_t x, uint16_t y);
     *
     * @brief Register a click event and figure out if this is a drag event, too.
     *
     *        Register a click event and figure out if this is a drag event, too.
     *
     * @param evt The type of the click event.
     * @param x   The x coordinate of the click event.
     * @param y   The y coordinate of the click event.
     */
    void registerClickEvent(ClickEvent::ClickEventType evt, uint16_t x, uint16_t y);

    /**
     * @fn void Gestures::setDragThreshold(uint16_t val)
     *
     * @brief Configure the threshold for reporting drag events.
     *
     *        Configure the threshold for reporting drag events. A touch input movement must
     *        exceed this value in either axis in order to report a drag. Default value is 0.
     *
     * @param val New threshold value.
     */
    void setDragThreshold(uint16_t val)
    {
        dragThresholdValue = val;
    }

private:
    DragState        drag;
    UIEventListener* listener;
    uint16_t         dragThresholdValue;
};
} // namespace touchgfx
#endif // GESTURES_HPP

#ifndef RIDESCREEN_VIEW_HPP
#define RIDESCREEN_VIEW_HPP

#include <gui_generated/ridescreen_screen/rideScreenViewBase.hpp>
#include <gui/ridescreen_screen/rideScreenPresenter.hpp>

class rideScreenView : public rideScreenViewBase
{
public:
    rideScreenView();
    virtual ~rideScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void assistButtonClicked();
    virtual void handleTickEvent();
    virtual void handleGestureEvent(const GestureEvent& evt);
protected:
	int16_t hour = 0;
	int16_t minute = 0;
	int16_t second = 0;
	int16_t tickCount = 0;
	int16_t assistLevel;
};

#endif // RIDESCREEN_VIEW_HPP

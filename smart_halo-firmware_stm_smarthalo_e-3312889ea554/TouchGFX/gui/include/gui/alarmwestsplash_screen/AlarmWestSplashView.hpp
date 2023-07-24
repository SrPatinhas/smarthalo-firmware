#ifndef ALARMWESTSPLASH_VIEW_HPP
#define ALARMWESTSPLASH_VIEW_HPP

#include <gui_generated/alarmwestsplash_screen/AlarmWestSplashViewBase.hpp>
#include <gui/alarmwestsplash_screen/AlarmWestSplashPresenter.hpp>

class AlarmWestSplashView : public AlarmWestSplashViewBase
{
public:
    AlarmWestSplashView();
    virtual ~AlarmWestSplashView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
};

#endif // ALARMWESTSPLASH_VIEW_HPP

#ifndef ALARMEASTSPLASH_VIEW_HPP
#define ALARMEASTSPLASH_VIEW_HPP

#include <gui_generated/alarmeastsplash_screen/AlarmEastSplashViewBase.hpp>
#include <gui/alarmeastsplash_screen/AlarmEastSplashPresenter.hpp>

class AlarmEastSplashView : public AlarmEastSplashViewBase
{
public:
    AlarmEastSplashView();
    virtual ~AlarmEastSplashView() {}
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

#endif // ALARMEASTSPLASH_VIEW_HPP

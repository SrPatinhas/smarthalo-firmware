#ifndef HOMESPLASH_VIEW_HPP
#define HOMESPLASH_VIEW_HPP

#include <gui_generated/homesplash_screen/HomeSplashViewBase.hpp>
#include <gui/homesplash_screen/HomeSplashPresenter.hpp>

class HomeSplashView : public HomeSplashViewBase
{
public:
    HomeSplashView();
    virtual ~HomeSplashView() {}
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

#endif // HOMESPLASH_VIEW_HPP

#ifndef HOMEWESTSPLASH_VIEW_HPP
#define HOMEWESTSPLASH_VIEW_HPP

#include <gui_generated/homewestsplash_screen/HomeWestSplashViewBase.hpp>
#include <gui/homewestsplash_screen/HomeWestSplashPresenter.hpp>

class HomeWestSplashView : public HomeWestSplashViewBase
{
public:
    HomeWestSplashView();
    virtual ~HomeWestSplashView() {}
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

#endif // HOMEWESTSPLASH_VIEW_HPP

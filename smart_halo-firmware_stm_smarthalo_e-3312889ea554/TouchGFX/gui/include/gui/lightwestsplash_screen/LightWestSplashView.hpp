#ifndef LIGHTWESTSPLASH_VIEW_HPP
#define LIGHTWESTSPLASH_VIEW_HPP

#include <gui_generated/lightwestsplash_screen/LightWestSplashViewBase.hpp>
#include <gui/lightwestsplash_screen/LightWestSplashPresenter.hpp>

class LightWestSplashView : public LightWestSplashViewBase
{
public:
    LightWestSplashView();
    virtual ~LightWestSplashView() {}
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

#endif // LIGHTWESTSPLASH_VIEW_HPP

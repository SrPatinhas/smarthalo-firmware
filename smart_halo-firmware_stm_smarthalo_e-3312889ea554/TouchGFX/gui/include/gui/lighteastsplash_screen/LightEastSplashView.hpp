#ifndef LIGHTEASTSPLASH_VIEW_HPP
#define LIGHTEASTSPLASH_VIEW_HPP

#include <gui_generated/lighteastsplash_screen/LightEastSplashViewBase.hpp>
#include <gui/lighteastsplash_screen/LightEastSplashPresenter.hpp>

class LightEastSplashView : public LightEastSplashViewBase
{
public:
    LightEastSplashView();
    virtual ~LightEastSplashView() {}
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

#endif // LIGHTEASTSPLASH_VIEW_HPP

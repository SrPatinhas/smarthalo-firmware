#ifndef NAVIGATIONWESTSPLASH_VIEW_HPP
#define NAVIGATIONWESTSPLASH_VIEW_HPP

#include <gui_generated/navigationwestsplash_screen/NavigationWestSplashViewBase.hpp>
#include <gui/navigationwestsplash_screen/NavigationWestSplashPresenter.hpp>

class NavigationWestSplashView : public NavigationWestSplashViewBase
{
public:
    NavigationWestSplashView();
    virtual ~NavigationWestSplashView() {}
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

#endif // NAVIGATIONWESTSPLASH_VIEW_HPP

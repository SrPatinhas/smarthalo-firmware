#ifndef NAVIGATIONEASTSPLASH_VIEW_HPP
#define NAVIGATIONEASTSPLASH_VIEW_HPP

#include <gui_generated/navigationeastsplash_screen/NavigationEastSplashViewBase.hpp>
#include <gui/navigationeastsplash_screen/NavigationEastSplashPresenter.hpp>

class NavigationEastSplashView : public NavigationEastSplashViewBase
{
public:
    NavigationEastSplashView();
    virtual ~NavigationEastSplashView() {}
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

#endif // NAVIGATIONEASTSPLASH_VIEW_HPP

#ifndef NAVIGATIONSCREEN_VIEW_HPP
#define NAVIGATIONSCREEN_VIEW_HPP

#include <gui_generated/navigationscreen_screen/NavigationScreenViewBase.hpp>
#include <gui/navigationscreen_screen/NavigationScreenPresenter.hpp>

class NavigationScreenView : public NavigationScreenViewBase
{
public:
    NavigationScreenView();
    virtual ~NavigationScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void assistButtonClicked();
    virtual void handleClickEvent(const ClickEvent& evt);
    virtual void handleGestureEvent(const GestureEvent& evt);
protected:
    int16_t clickDownX;
    int16_t clickDownY;
    int16_t clickUpX;
    int16_t clickUpY;
	int16_t hour = 7;
	int16_t minute = 57;
	int16_t tickCount;
	int16_t percentage = 100;
	int16_t assistLevel;
};

#endif // NAVIGATIONSCREEN_VIEW_HPP

#ifndef HOMESCREEN_VIEW_HPP
#define HOMESCREEN_VIEW_HPP

#include <gui_generated/homescreen_screen/HomeScreenViewBase.hpp>
#include <gui/homescreen_screen/HomeScreenPresenter.hpp>

class HomeScreenView : public HomeScreenViewBase
{
public:
    HomeScreenView();
    virtual ~HomeScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleDragEvent(const DragEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
    virtual void scrollWheelUpdateItem(AssistIcon& item, int16_t itemIndex){
    	item.setNumber(itemIndex);
  	}
protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
	int16_t hour = 7;
	int16_t minute = 57;
	int16_t tickCount;
	int16_t percentage = 100;
};

#endif // HOMESCREEN_VIEW_HPP

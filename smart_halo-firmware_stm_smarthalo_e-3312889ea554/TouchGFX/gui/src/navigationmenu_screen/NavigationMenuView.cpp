#include <gui/navigationmenu_screen/NavigationMenuView.hpp>
#include <math.h>

NavigationMenuView::NavigationMenuView()
{

}

void NavigationMenuView::setupScreen()
{
    NavigationMenuViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(1, colortype(0x39B54A));
}

void NavigationMenuView::tearDownScreen()
{
    NavigationMenuViewBase::tearDownScreen();
}

void NavigationMenuView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
		if (evt.getType() == GestureEvent::SWIPE_VERTICAL) {
			navMenu.handleGestureEvent(evt);
		}
	}
}

void NavigationMenuView::handleDragEvent(const DragEvent& evt){
	if (evt.getOldY() > 270) {
		navMenu.handleDragEvent(evt);
	}
}

void NavigationMenuView::handleClickEvent(const ClickEvent& evt){
	if(evt.getY() > 270){
		if (evt.getType() == ClickEvent::ClickEventType::PRESSED){
			clickDownX = evt.getX();
			clickDownY = evt.getY();
		}	

		if(evt.getType() == ClickEvent::ClickEventType::RELEASED){
			clickUpX = evt.getX();
			clickUpY = evt.getY();
			if (abs(double(clickDownX - clickUpX)) < 5 && abs(double(clickDownY - clickUpY)) < 5) {
				application().gotoNavigationScreenScreenNoTransition();			
			}	
			if (abs(double(clickDownY - clickUpY)) < 60){
				if (clickDownX - clickUpX > 25) {
					application().gotoFitnessEastSplashScreenNoTransition();
				} else 	if (clickDownX - clickUpX < -25) {
					application().gotoHomeWestSplashScreenNoTransition();
				}	
			}
			navMenu.animateToItem(navMenu.getSelectedItem());
		}
	}
}		
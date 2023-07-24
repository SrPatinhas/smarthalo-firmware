#include <gui/navigationoptions_screen/NavigationOptionsView.hpp>

NavigationOptionsView::NavigationOptionsView()
{

}

void NavigationOptionsView::setupScreen()
{
    NavigationOptionsViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(1, colortype(0x39B54A));
}

void NavigationOptionsView::tearDownScreen()
{
    NavigationOptionsViewBase::tearDownScreen();
}

void NavigationOptionsView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
		if (evt.getType() == GestureEvent::SWIPE_VERTICAL) {
			navOptionMenu.handleGestureEvent(evt);
		}
	}
}

void NavigationOptionsView::handleDragEvent(const DragEvent& evt){
	if (evt.getOldY() > 270) {
		navOptionMenu.handleDragEvent(evt);
	}
}

void NavigationOptionsView::handleClickEvent(const ClickEvent& evt){
	if(evt.getY() > 270){
		if (evt.getType() == ClickEvent::ClickEventType::PRESSED){
			clickDownX = evt.getX();
			clickDownY = evt.getY();
		}	

		if(evt.getType() == ClickEvent::ClickEventType::RELEASED){
			clickUpX = evt.getX();
			clickUpY = evt.getY();
			if (abs(double(clickDownX - clickUpX)) < 5 && abs(double(clickDownY - clickUpY)) < 5) {
				if(navOptionMenu.getSelectedItem() == 0){
					application().gotoNavigationMenuScreenNoTransition();			
				}
			}	
			if (abs(double(clickDownY - clickUpY)) < 60){
				if (clickDownX - clickUpX > 25) {
					application().gotoFitnessEastSplashScreenNoTransition();
				} else 	if (clickDownX - clickUpX < -25) {
					application().gotoHomeWestSplashScreenNoTransition();
				}	
			}
			navOptionMenu.animateToItem(navOptionMenu.getSelectedItem());
		}
	}
}		
#include <gui/homewestsplash_screen/HomeWestSplashView.hpp>

HomeWestSplashView::HomeWestSplashView()
{

}

void HomeWestSplashView::setupScreen()
{
    HomeWestSplashViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(0, colortype(0xFFFFFF));
}

void HomeWestSplashView::tearDownScreen()
{
    HomeWestSplashViewBase::tearDownScreen();
}

void HomeWestSplashView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
	}
}

void HomeWestSplashView::handleClickEvent(const ClickEvent& evt){
	if(evt.getY() > 270){
		if (evt.getType() == ClickEvent::ClickEventType::PRESSED){
			clickDownX = evt.getX();
			clickDownY = evt.getY();
		}	

		if(evt.getType() == ClickEvent::ClickEventType::RELEASED){
			clickUpX = evt.getX();
			clickUpY = evt.getY();
			if (abs(double(clickDownX - clickUpX)) < 5 && abs(double(clickDownY - clickUpY)) < 5) {
				// application().gotoNavigationScreenScreenNoTransition();			
			}
			if (abs(double(clickDownY - clickUpY)) < 60){
				if (clickDownX - clickUpX > 25) {
					application().gotoNavigationEastSplashScreenNoTransition();
				} else 	if (clickDownX - clickUpX < -25) {
					//No where to go
				}	
			}
		}
	}
}		
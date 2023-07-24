#include <gui/navigationwestsplash_screen/NavigationWestSplashView.hpp>

NavigationWestSplashView::NavigationWestSplashView()
{

}

void NavigationWestSplashView::setupScreen()
{
    NavigationWestSplashViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(1, colortype(0x39B54A));
}

void NavigationWestSplashView::tearDownScreen()
{
    NavigationWestSplashViewBase::tearDownScreen();
}

void NavigationWestSplashView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
	}
}

void NavigationWestSplashView::handleClickEvent(const ClickEvent& evt){
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
					application().gotoFitnessEastSplashScreenNoTransition();
				} else 	if (clickDownX - clickUpX < -25) {
					application().gotoHomeWestSplashScreenNoTransition();
				}	
			}
		}
	}
}		
#include <gui/fitnesswestsplash_screen/FitnessWestSplashView.hpp>

FitnessWestSplashView::FitnessWestSplashView()
{

}

void FitnessWestSplashView::setupScreen()
{
    FitnessWestSplashViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(2, colortype(0xE8106B));
}

void FitnessWestSplashView::tearDownScreen()
{
    FitnessWestSplashViewBase::tearDownScreen();
}

void FitnessWestSplashView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
	}
}

void FitnessWestSplashView::handleClickEvent(const ClickEvent& evt){
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
					application().gotoLightEastSplashScreenNoTransition();
				} else 	if (clickDownX - clickUpX < -25) {
		            application().gotoNavigationWestSplashScreenNoTransition();
				}	
			}
		}
	}
}		
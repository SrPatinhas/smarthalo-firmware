#include <gui/lightwestsplash_screen/LightWestSplashView.hpp>

LightWestSplashView::LightWestSplashView()
{

}

void LightWestSplashView::setupScreen()
{
    LightWestSplashViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(3, colortype(0xF9E315));
}

void LightWestSplashView::tearDownScreen()
{
    LightWestSplashViewBase::tearDownScreen();
}
    
void LightWestSplashView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
	}
}

void LightWestSplashView::handleClickEvent(const ClickEvent& evt){
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
					application().gotoAlarmEastSplashScreenNoTransition();
				} else 	if (clickDownX - clickUpX < -25) {
					application().gotoFitnessWestSplashScreenNoTransition();
				}	
			}
		}
	}
}		
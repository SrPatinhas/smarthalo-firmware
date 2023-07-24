#include <gui/alarmwestsplash_screen/AlarmWestSplashView.hpp>

AlarmWestSplashView::AlarmWestSplashView()
{

}

void AlarmWestSplashView::setupScreen()
{
    AlarmWestSplashViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(4, colortype(0xE56B29));
}

void AlarmWestSplashView::tearDownScreen()
{
    AlarmWestSplashViewBase::tearDownScreen();
}

void AlarmWestSplashView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
	}
}

void AlarmWestSplashView::handleClickEvent(const ClickEvent& evt){
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
					application().gotoAssistantEastSplashScreenNoTransition();
				} else 	if (clickDownX - clickUpX < -25) {
					application().gotoLightWestSplashScreenNoTransition();
				}	
			}
		}
	}
}		

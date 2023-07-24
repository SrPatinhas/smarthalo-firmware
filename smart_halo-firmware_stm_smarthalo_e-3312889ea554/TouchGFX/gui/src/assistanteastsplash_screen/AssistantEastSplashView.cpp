#include <gui/assistanteastsplash_screen/AssistantEastSplashView.hpp>

AssistantEastSplashView::AssistantEastSplashView()
{

}

void AssistantEastSplashView::setupScreen()
{
    AssistantEastSplashViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(5, colortype(0x0E83CB));
}

void AssistantEastSplashView::tearDownScreen()
{
    AssistantEastSplashViewBase::tearDownScreen();
}

void AssistantEastSplashView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
	}
}

void AssistantEastSplashView::handleClickEvent(const ClickEvent& evt){
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
					//No where to go
				} else 	if (clickDownX - clickUpX < -25) {
					application().gotoAlarmWestSplashScreenNoTransition();
				}	
			}
		}
	}
}		
#include <gui/alarmscreen_screen/AlarmScreenView.hpp>

AlarmScreenView::AlarmScreenView()
{

}

void AlarmScreenView::setupScreen()
{
    AlarmScreenViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(4, colortype(0xE56B29));
}

void AlarmScreenView::tearDownScreen()
{
    AlarmScreenViewBase::tearDownScreen();
}
    
void AlarmScreenView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
		if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL) {
			if (evt.getVelocity() < -25) {
				//Swiped towards left. Go to appropriate screen
				// application().gotoAssistantEastSplashScreenNoTransition();
			} else if (evt.getVelocity() > 25){
				//Swiped towards right. Go to appropriate screen 
				// application().gotoLightWestSplashScreenNoTransition();
			}
		}
		if (evt.getType() == GestureEvent::SWIPE_VERTICAL) {
			alarmMenu.handleGestureEvent(evt);
		}
	}
}
    
void AlarmScreenView::handleClickEvent(const ClickEvent& evt){
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
		alarmMenu.animateToItem(alarmMenu.getSelectedItem());
	}
}		


void AlarmScreenView::handleDragEvent(const DragEvent& evt){
	if (evt.getOldY() > 270) {
		alarmMenu.handleDragEvent(evt);
	}
}
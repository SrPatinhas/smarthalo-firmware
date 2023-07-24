#include <gui/assistantscreen_screen/AssistantScreenView.hpp>

AssistantScreenView::AssistantScreenView()
{

}

void AssistantScreenView::setupScreen()
{
    AssistantScreenViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(5, colortype(0x0E83CB));
}

void AssistantScreenView::tearDownScreen()
{
    AssistantScreenViewBase::tearDownScreen();
}
    
void AssistantScreenView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
		if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL) {
			if (evt.getVelocity() < -30) {
				//Swiped towards left. Go to appropriate screen
			} else if (evt.getVelocity() > 30){
				//Swiped towards right. Go to appropriate screen 
				application().gotoAlarmWestSplashScreenNoTransition();
			}
		}
		if (evt.getType() == GestureEvent::SWIPE_VERTICAL) {
			assistantMenu.handleGestureEvent(evt);
		}
	}
}

void AssistantScreenView::handleDragEvent(const DragEvent& evt){
	if (evt.getOldY() > 270) {
		assistantMenu.handleDragEvent(evt);
	}
}

void AssistantScreenView::handleClickEvent(const ClickEvent& evt){
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
			assistantMenu.animateToItem(assistantMenu.getSelectedItem());
		}
	}
}		
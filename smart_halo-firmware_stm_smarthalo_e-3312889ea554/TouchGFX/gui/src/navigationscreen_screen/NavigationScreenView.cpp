#include <gui/navigationscreen_screen/NavigationScreenView.hpp>

NavigationScreenView::NavigationScreenView()
{

}

void NavigationScreenView::setupScreen()
{
    NavigationScreenViewBase::setupScreen();

    featureCarrosselDots.setColourOfDot(1, colortype(0x39B54A));
}

void NavigationScreenView::tearDownScreen()
{
    NavigationScreenViewBase::tearDownScreen();
}

void NavigationScreenView::assistButtonClicked(){
  presenter->changeAssist();

    assistLevel = presenter->getAssist();
    // Unicode::snprintf(textAssistBuffer, TEXTASSIST_SIZE, "%d", assistLevel);
}
    
void NavigationScreenView::handleGestureEvent(const GestureEvent& evt)
{
  if (evt.getY() > 270) {
		if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL) {
		}else{
			if( evt.getVelocity() < -5) {
				application().gotoNavigationOptionsScreenNoTransition();
			}
		}
	}
}

void NavigationScreenView::handleClickEvent(const ClickEvent& evt){
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
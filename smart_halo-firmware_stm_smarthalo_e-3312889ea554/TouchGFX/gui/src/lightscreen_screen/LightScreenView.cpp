#include <gui/lightscreen_screen/LightScreenView.hpp>

LightScreenView::LightScreenView()
{

}

void LightScreenView::setupScreen()
{
    LightScreenViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(3, colortype(0xF9E315));
}

void LightScreenView::tearDownScreen()
{
    LightScreenViewBase::tearDownScreen();
}
    
void LightScreenView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
		if (evt.getType() == GestureEvent::SWIPE_VERTICAL) {
			lightMenu.handleGestureEvent(evt);
		}
	}
}

void LightScreenView::handleDragEvent(const DragEvent& evt){
	if (evt.getOldY() > 270) {
		lightMenu.handleDragEvent(evt);
	}
}

void LightScreenView::handleClickEvent(const ClickEvent& evt){
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
			lightMenu.animateToItem(lightMenu.getSelectedItem());
		}
	}
}		
#include <gui/homescreen_screen/HomeScreenView.hpp>

HomeScreenView::HomeScreenView()
{

}

void HomeScreenView::setupScreen()
{
    HomeScreenViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(0, colortype(0xFFFFFF));

}

void HomeScreenView::tearDownScreen()
{
    HomeScreenViewBase::tearDownScreen();
}

void HomeScreenView::handleTickEvent()
{
    if (tickCount == 60)
    {
        minute++;
        hour = (hour + (minute / 60)) % 24;
        minute %= 60;

        Unicode::snprintf(textClockBuffer1, TEXTCLOCKBUFFER1_SIZE, "%02d", hour);
        Unicode::snprintf(textClockBuffer2, TEXTCLOCKBUFFER2_SIZE, "%02d", minute);

        textClock.invalidate();

        tickCount = 0;
        percentage--;
    }

    if(percentage >= 0){
	    Unicode::snprintf(textBatteryPercentBuffer, TEXTBATTERYPERCENT_SIZE, "%d", percentage);
	    textBatteryPercent.invalidate();
	    batteryPercentage.setValue(percentage);
	    if(percentage < 60){
	    	batteryPercentage.setColor(0xFFAC8B08);
	    }

	    if(percentage < 20){
	    	batteryPercentage.setColor(0xFFA8260E);
	    }
	}

    tickCount++;
}

void HomeScreenView::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getY() > 270) {
	 	scrollWheel.handleGestureEvent(evt);
	}
}

void HomeScreenView::handleDragEvent(const DragEvent& evt){
	if (evt.getOldY() > 270) {
		scrollWheel.handleDragEvent(evt);
	}
}

void HomeScreenView::handleClickEvent(const ClickEvent& evt){
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
			scrollWheel.animateToItem(scrollWheel.getSelectedItem());
		}
	}
}		
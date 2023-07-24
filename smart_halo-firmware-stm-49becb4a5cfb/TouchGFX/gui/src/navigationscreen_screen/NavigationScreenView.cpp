#include <gui/navigationscreen_screen/NavigationScreenView.hpp>

NavigationScreenView::NavigationScreenView()
{

}

void NavigationScreenView::setupScreen()
{
    NavigationScreenViewBase::setupScreen();

    assistLevel = presenter->getAssist();
    Unicode::snprintf(textAssistantBuffer, TEXTASSISTANT_SIZE, "%d", assistLevel);
}

void NavigationScreenView::tearDownScreen()
{
    NavigationScreenViewBase::tearDownScreen();
}

void NavigationScreenView::handleGestureEvent(const GestureEvent& evt)
{
 if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL)
 {
   if (evt.getVelocity() < 0)
   {
     //Swiped towards left. Go to appropriate screen
     // application().gotorideScreenScreenSlideTransitionEast();
     application().gotorideScreenScreenNoTransition();
   }
   else
   {
     //Swiped towards right. Go to appropriate screen 
   }
 }
}

void NavigationScreenView::handleTickEvent()
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
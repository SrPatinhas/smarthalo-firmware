#include <gui/ridescreen_screen/rideScreenView.hpp>

rideScreenView::rideScreenView()
{

}

void rideScreenView::setupScreen()
{
    rideScreenViewBase::setupScreen();

    assistLevel = presenter->getAssist();
    Unicode::snprintf(textAssistBuffer, TEXTASSIST_SIZE, "%d", assistLevel);
}

void rideScreenView::tearDownScreen()
{
    rideScreenViewBase::tearDownScreen();
}

void rideScreenView::assistButtonClicked(){
	presenter->changeAssist();

    assistLevel = presenter->getAssist();
    Unicode::snprintf(textAssistBuffer, TEXTASSIST_SIZE, "%d", assistLevel);
}
    
void rideScreenView::handleGestureEvent(const GestureEvent& evt)
{
 if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL)
 {
   if (evt.getVelocity() < 0)
   {
     //Swiped towards left. Go to appropriate screen
   }
   else
   {
     //Swiped towards right. Go to appropriate screen 
     // application().gotoNavigationScreenScreenSlideTransitionWest();
     application().gotoNavigationScreenScreenNoTransition();
   }
 }
}

void rideScreenView::handleTickEvent()
{
    if (tickCount == 60)
    {
        minute++;
        hour = (hour + (minute / 60)) % 24;
        minute %= 60;

        Unicode::snprintf(textRideTimeBuffer1, TEXTRIDETIMEBUFFER1_SIZE, "%02d", hour);
        Unicode::snprintf(textRideTimeBuffer2, TEXTRIDETIMEBUFFER2_SIZE, "%02d", minute);

        textRideTime.invalidate();

        tickCount = 0;
    }
 
    Unicode::snprintf(textRideTimeSecondsBuffer, TEXTRIDETIMESECONDS_SIZE, "%02d", tickCount);
    textRideTimeSeconds.invalidate();
    tickCount++;
}
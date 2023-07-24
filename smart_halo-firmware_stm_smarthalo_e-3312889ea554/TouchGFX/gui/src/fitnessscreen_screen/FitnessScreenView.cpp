#include <gui/fitnessscreen_screen/FitnessScreenView.hpp>

FitnessScreenView::FitnessScreenView()
{

}

void FitnessScreenView::setupScreen()
{
    FitnessScreenViewBase::setupScreen();
    featureCarrosselDots.setColourOfDot(2, colortype(0xE8106B));
}

void FitnessScreenView::tearDownScreen()
{
    FitnessScreenViewBase::tearDownScreen();
}

void FitnessScreenView::assistButtonClicked(){
	presenter->changeAssist();

    assistLevel = presenter->getAssist();
    // Unicode::snprintf(textAssistBuffer, TEXTASSIST_SIZE, "%d", assistLevel);
}
    
void FitnessScreenView::handleGestureEvent(const GestureEvent& evt)
{
    if (evt.getY() > 270) {
    }
}

void FitnessScreenView::handleTickEvent()
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

void FitnessScreenView::handleClickEvent(const ClickEvent& evt){
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
                    application().gotoLightEastSplashScreenNoTransition();
                } else  if (clickDownX - clickUpX < -25) {
                    application().gotoNavigationWestSplashScreenNoTransition();
                }   
            }
        }
    }
}       
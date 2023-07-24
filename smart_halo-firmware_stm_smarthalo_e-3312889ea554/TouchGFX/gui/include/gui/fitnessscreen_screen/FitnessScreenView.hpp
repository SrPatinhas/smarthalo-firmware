#ifndef FITNESSSCREEN_VIEW_HPP
#define FITNESSSCREEN_VIEW_HPP

#include <gui_generated/fitnessscreen_screen/FitnessScreenViewBase.hpp>
#include <gui/fitnessscreen_screen/FitnessScreenPresenter.hpp>

class FitnessScreenView : public FitnessScreenViewBase
{
public:
    FitnessScreenView();
    virtual ~FitnessScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void assistButtonClicked();
    virtual void handleTickEvent();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
protected:
    int16_t clickDownX;
    int16_t clickDownY;
    int16_t clickUpX;
    int16_t clickUpY;
	int16_t hour = 0;
	int16_t minute = 0;
	int16_t second = 0;
	int16_t tickCount = 0;
	int16_t assistLevel;
};

#endif // FITNESSSCREEN_VIEW_HPP

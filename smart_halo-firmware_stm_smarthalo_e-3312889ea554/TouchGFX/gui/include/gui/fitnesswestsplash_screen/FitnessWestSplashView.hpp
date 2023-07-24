#ifndef FITNESSWESTSPLASH_VIEW_HPP
#define FITNESSWESTSPLASH_VIEW_HPP

#include <gui_generated/fitnesswestsplash_screen/FitnessWestSplashViewBase.hpp>
#include <gui/fitnesswestsplash_screen/FitnessWestSplashPresenter.hpp>

class FitnessWestSplashView : public FitnessWestSplashViewBase
{
public:
    FitnessWestSplashView();
    virtual ~FitnessWestSplashView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
};

#endif // FITNESSWESTSPLASH_VIEW_HPP

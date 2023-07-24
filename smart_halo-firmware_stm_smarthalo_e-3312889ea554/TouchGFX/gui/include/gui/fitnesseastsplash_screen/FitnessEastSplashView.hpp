#ifndef FITNESSEASTSPLASH_VIEW_HPP
#define FITNESSEASTSPLASH_VIEW_HPP

#include <gui_generated/fitnesseastsplash_screen/FitnessEastSplashViewBase.hpp>
#include <gui/fitnesseastsplash_screen/FitnessEastSplashPresenter.hpp>

class FitnessEastSplashView : public FitnessEastSplashViewBase
{
public:
    FitnessEastSplashView();
    virtual ~FitnessEastSplashView() {}
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

#endif // FITNESSEASTSPLASH_VIEW_HPP

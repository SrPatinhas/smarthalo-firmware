#ifndef ASSISTANTEASTSPLASH_VIEW_HPP
#define ASSISTANTEASTSPLASH_VIEW_HPP

#include <gui_generated/assistanteastsplash_screen/AssistantEastSplashViewBase.hpp>
#include <gui/assistanteastsplash_screen/AssistantEastSplashPresenter.hpp>

class AssistantEastSplashView : public AssistantEastSplashViewBase
{
public:
    AssistantEastSplashView();
    virtual ~AssistantEastSplashView() {}
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

#endif // ASSISTANTEASTSPLASH_VIEW_HPP

#ifndef LIGHTSCREEN_VIEW_HPP
#define LIGHTSCREEN_VIEW_HPP

#include <gui_generated/lightscreen_screen/LightScreenViewBase.hpp>
#include <gui/lightscreen_screen/LightScreenPresenter.hpp>

class LightScreenView : public LightScreenViewBase
{
public:
    LightScreenView();
    virtual ~LightScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleDragEvent(const DragEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
    virtual void lightMenuUpdateItem(LightMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void lightMenuUpdateCenterItem(SelectedLightMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }
protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
};

#endif // LIGHTSCREEN_VIEW_HPP

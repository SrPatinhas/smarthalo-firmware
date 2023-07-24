#ifndef NAVIGATIONOPTIONS_VIEW_HPP
#define NAVIGATIONOPTIONS_VIEW_HPP

#include <gui_generated/navigationoptions_screen/NavigationOptionsViewBase.hpp>
#include <gui/navigationoptions_screen/NavigationOptionsPresenter.hpp>

class NavigationOptionsView : public NavigationOptionsViewBase
{
public:
    NavigationOptionsView();
    virtual ~NavigationOptionsView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleDragEvent(const DragEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
    virtual void navOptionMenuUpdateItem(NavOptionMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void navOptionMenuUpdateCenterItem(SelectedNavOptionMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }
protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
};

#endif // NAVIGATIONOPTIONS_VIEW_HPP

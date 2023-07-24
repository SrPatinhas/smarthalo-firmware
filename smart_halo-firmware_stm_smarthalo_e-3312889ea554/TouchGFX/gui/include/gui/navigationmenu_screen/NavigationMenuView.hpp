#ifndef NAVIGATIONMENU_VIEW_HPP
#define NAVIGATIONMENU_VIEW_HPP

#include <gui_generated/navigationmenu_screen/NavigationMenuViewBase.hpp>
#include <gui/navigationmenu_screen/NavigationMenuPresenter.hpp>

class NavigationMenuView : public NavigationMenuViewBase
{
public:
    NavigationMenuView();
    virtual ~NavigationMenuView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleDragEvent(const DragEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
    virtual void navMenuUpdateItem(NavMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void navMenuUpdateCenterItem(SelectedNavMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
};

#endif // NAVIGATIONMENU_VIEW_HPP

#ifndef SELECTEDNAVOPTIONMENUITEM_HPP
#define SELECTEDNAVOPTIONMENUITEM_HPP

#include <gui_generated/containers/SelectedNavOptionMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class SelectedNavOptionMenuItem : public SelectedNavOptionMenuItemBase
{
public:
    SelectedNavOptionMenuItem();
    virtual ~SelectedNavOptionMenuItem() {}

    virtual void initialize();
        void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Stop Nav",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON10SELECTED_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Route Type",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON11SELECTED_ID));
			    break;
    	}
    }
protected:
};

#endif // SELECTEDNAVOPTIONMENUITEM_HPP

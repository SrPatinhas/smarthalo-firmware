#ifndef NAVOPTIONMENUITEM_HPP
#define NAVOPTIONMENUITEM_HPP

#include <gui_generated/containers/NavOptionMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class NavOptionMenuItem : public NavOptionMenuItemBase
{
public:
    NavOptionMenuItem();
    virtual ~NavOptionMenuItem() {}

    virtual void initialize();
        void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Stop Nav",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON10_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Route Type",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON11_ID));
			    break;
    	}
    }
protected:
};

#endif // NAVOPTIONMENUITEM_HPP

#ifndef NAVMENUITEM_HPP
#define NAVMENUITEM_HPP

#include <gui_generated/containers/NavMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class NavMenuItem : public NavMenuItemBase
{
public:
    NavMenuItem();
    virtual ~NavMenuItem() {}

    virtual void initialize();
        void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Home",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON00_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Work",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON01_ID));
			    break;
    		case 2:
    			Unicode::strncpy(menuTitleBuffer,"Recent",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON02_ID));
			    break;
    		case 3:
    			Unicode::strncpy(menuTitleBuffer,"Favourites",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON03_ID));
			    break;
    	}
    }
protected:
};

#endif // NAVMENUITEM_HPP

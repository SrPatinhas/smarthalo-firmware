#ifndef SELECTEDNAVMENUITEM_HPP
#define SELECTEDNAVMENUITEM_HPP

#include <gui_generated/containers/SelectedNavMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class SelectedNavMenuItem : public SelectedNavMenuItemBase
{
public:
    SelectedNavMenuItem();
    virtual ~SelectedNavMenuItem() {}

    virtual void initialize();
    void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Home",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON00SELECTED_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Work",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON01SELECTED_ID));
			    break;
    		case 2:
    			Unicode::strncpy(menuTitleBuffer,"Recent",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON02SELECTED_ID));
			    break;
    		case 3:
    			Unicode::strncpy(menuTitleBuffer,"Favourites",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_NAVICON03SELECTED_ID));
			    break;
    	}
    }
protected:
};

#endif // SELECTEDNAVMENUITEM_HPP

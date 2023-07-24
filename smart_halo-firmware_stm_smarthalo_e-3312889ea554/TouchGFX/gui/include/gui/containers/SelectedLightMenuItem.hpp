#ifndef SELECTEDLIGHTMENUITEM_HPP
#define SELECTEDLIGHTMENUITEM_HPP

#include <gui_generated/containers/SelectedLightMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class SelectedLightMenuItem : public SelectedLightMenuItemBase
{
public:
    SelectedLightMenuItem();
    virtual ~SelectedLightMenuItem() {}

    virtual void initialize();
    void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Light Mode",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_LIGHTICON00SELECTED_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Brightness",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_LIGHTICON01SELECTED_ID));
			    break;
    	}
    }

protected:
};

#endif // SELECTEDLIGHTMENUITEM_HPP

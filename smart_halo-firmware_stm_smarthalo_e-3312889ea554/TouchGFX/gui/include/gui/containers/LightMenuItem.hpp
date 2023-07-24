#ifndef LIGHTMENUITEM_HPP
#define LIGHTMENUITEM_HPP

#include <gui_generated/containers/LightMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class LightMenuItem : public LightMenuItemBase
{
public:
    LightMenuItem();
    virtual ~LightMenuItem() {}

    virtual void initialize();
    void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Light Mode",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_LIGHTICON00_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Brightness",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_LIGHTICON01_ID));
			    break;
    	}
    }
protected:
};

#endif // LIGHTMENUITEM_HPP

#ifndef ASSISTANTMENUITEM_HPP
#define ASSISTANTMENUITEM_HPP

#include <gui_generated/containers/AssistantMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class AssistantMenuItem : public AssistantMenuItemBase
{
public:
    AssistantMenuItem();
    virtual ~AssistantMenuItem() {}

    virtual void initialize();
    void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Notifications",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ASSISTANTICON00_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Horn Sound",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ASSISTANTICON01_ID));
			    break;
    	}
    }
protected:
};

#endif // ASSISTANTMENUITEM_HPP

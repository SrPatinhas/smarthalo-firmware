#ifndef SELECTEDASSISTANTMENUITEM_HPP
#define SELECTEDASSISTANTMENUITEM_HPP

#include <gui_generated/containers/SelectedAssistantMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class SelectedAssistantMenuItem : public SelectedAssistantMenuItemBase
{
public:
    SelectedAssistantMenuItem();
    virtual ~SelectedAssistantMenuItem() {}

    virtual void initialize();
    void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Notifications",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ASSISTANTICON00SELECTED_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Horn Sound",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ASSISTANTICON01SELECTED_ID));
			    break;
    	}
    }
protected:
};

#endif // SELECTEDASSISTANTMENUITEM_HPP

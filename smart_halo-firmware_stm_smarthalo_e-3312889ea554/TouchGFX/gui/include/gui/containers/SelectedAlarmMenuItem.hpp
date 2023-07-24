#ifndef SELECTEDALARMMENUITEM_HPP
#define SELECTEDALARMMENUITEM_HPP

#include <gui_generated/containers/SelectedAlarmMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class SelectedAlarmMenuItem : public SelectedAlarmMenuItemBase
{
public:
    SelectedAlarmMenuItem();
    virtual ~SelectedAlarmMenuItem() {}

    virtual void initialize();
    void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Alarm Mode",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ALARMICON00SELECTED_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Sensitivity",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ALARMICON01SELECTED_ID));
			    break;
    	}
    }
protected:
};

#endif // SELECTEDALARMMENUITEM_HPP

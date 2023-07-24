#ifndef ALARMMENUITEM_HPP
#define ALARMMENUITEM_HPP

#include <gui_generated/containers/AlarmMenuItemBase.hpp>
#include <BitmapDatabase.hpp>

class AlarmMenuItem : public AlarmMenuItemBase
{
public:
    AlarmMenuItem();
    virtual ~AlarmMenuItem() {}

    virtual void initialize();
    void setNumber(int no){
    	switch (no){
    		case 0:
    			Unicode::strncpy(menuTitleBuffer,"Alarm Mode",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ALARMICON00_ID));
			    break;
    		case 1:
    			Unicode::strncpy(menuTitleBuffer,"Sensitivity",MENUTITLE_SIZE);
    			menuTitle.resizeToCurrentText();
    			menuTitle.invalidate();
			    menuIcon.setBitmap(Bitmap(BITMAP_ALARMICON01_ID));
			    break;
    	}
    }
protected:
};

#endif // ALARMMENUITEM_HPP

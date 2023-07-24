#ifndef ALARMSCREEN_VIEW_HPP
#define ALARMSCREEN_VIEW_HPP

#include <gui_generated/alarmscreen_screen/AlarmScreenViewBase.hpp>
#include <gui/alarmscreen_screen/AlarmScreenPresenter.hpp>

class AlarmScreenView : public AlarmScreenViewBase
{
public:
    AlarmScreenView();
    virtual ~AlarmScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleDragEvent(const DragEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
    virtual void alarmMenuUpdateItem(AlarmMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void alarmMenuUpdateCenterItem(SelectedAlarmMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }
protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
};

#endif // ALARMSCREEN_VIEW_HPP

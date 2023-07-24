#ifndef ASSISTANTSCREEN_VIEW_HPP
#define ASSISTANTSCREEN_VIEW_HPP

#include <gui_generated/assistantscreen_screen/AssistantScreenViewBase.hpp>
#include <gui/assistantscreen_screen/AssistantScreenPresenter.hpp>

class AssistantScreenView : public AssistantScreenViewBase
{
public:
    AssistantScreenView();
    virtual ~AssistantScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleGestureEvent(const GestureEvent& evt);
    virtual void handleDragEvent(const DragEvent& evt);
    virtual void handleClickEvent(const ClickEvent& evt);
    virtual void assistantMenuUpdateItem(AssistantMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }

    virtual void assistantMenuUpdateCenterItem(SelectedAssistantMenuItem& item, int16_t itemIndex)
    {
    	item.setNumber(itemIndex);
    }
protected:
	int16_t clickDownX;
	int16_t clickDownY;
	int16_t clickUpX;
	int16_t clickUpY;
};

#endif // ASSISTANTSCREEN_VIEW_HPP

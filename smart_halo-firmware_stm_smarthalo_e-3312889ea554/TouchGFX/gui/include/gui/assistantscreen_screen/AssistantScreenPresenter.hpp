#ifndef ASSISTANTSCREEN_PRESENTER_HPP
#define ASSISTANTSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class AssistantScreenView;

class AssistantScreenPresenter : public Presenter, public ModelListener
{
public:
    AssistantScreenPresenter(AssistantScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~AssistantScreenPresenter() {};

    void changeAssist(){
        model->changeAssist();
    }

    int16_t getAssist(){
        return model->getAssist();
    }

private:
    AssistantScreenPresenter();

    AssistantScreenView& view;
};


#endif // ASSISTANTSCREEN_PRESENTER_HPP

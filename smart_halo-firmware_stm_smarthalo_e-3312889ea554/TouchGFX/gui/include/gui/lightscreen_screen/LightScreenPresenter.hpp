#ifndef LIGHTSCREEN_PRESENTER_HPP
#define LIGHTSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class LightScreenView;

class LightScreenPresenter : public Presenter, public ModelListener
{
public:
    LightScreenPresenter(LightScreenView& v);

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

    virtual ~LightScreenPresenter() {};

    void changeAssist(){
        model->changeAssist();
    }

    int16_t getAssist(){
        return model->getAssist();
    }

private:
    LightScreenPresenter();

    LightScreenView& view;
};


#endif // LIGHTSCREEN_PRESENTER_HPP

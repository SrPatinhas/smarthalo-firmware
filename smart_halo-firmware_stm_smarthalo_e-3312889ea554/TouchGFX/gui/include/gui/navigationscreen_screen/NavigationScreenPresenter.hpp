#ifndef NAVIGATIONSCREEN_PRESENTER_HPP
#define NAVIGATIONSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class NavigationScreenView;

class NavigationScreenPresenter : public Presenter, public ModelListener
{
public:
    NavigationScreenPresenter(NavigationScreenView& v);

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

    virtual ~NavigationScreenPresenter() {};

    void changeAssist(){
        model->changeAssist();
    }

    int16_t getAssist(){
        return model->getAssist();
    }

private:
    NavigationScreenPresenter();

    NavigationScreenView& view;
};


#endif // NAVIGATIONSCREEN_PRESENTER_HPP

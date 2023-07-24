#ifndef RIDESCREEN_PRESENTER_HPP
#define RIDESCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class rideScreenView;

class rideScreenPresenter : public Presenter, public ModelListener
{
public:
    rideScreenPresenter(rideScreenView& v);

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

    virtual ~rideScreenPresenter() {};

    void changeAssist(){
        model->changeAssist();
    }

    int16_t getAssist(){
        return model->getAssist();
    }

private:
    rideScreenPresenter();

    rideScreenView& view;
};


#endif // RIDESCREEN_PRESENTER_HPP

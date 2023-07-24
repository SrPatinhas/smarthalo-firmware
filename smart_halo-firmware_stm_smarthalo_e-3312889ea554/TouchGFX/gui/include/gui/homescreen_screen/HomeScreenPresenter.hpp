#ifndef HOMESCREEN_PRESENTER_HPP
#define HOMESCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class HomeScreenView;

class HomeScreenPresenter : public Presenter, public ModelListener
{
public:
    HomeScreenPresenter(HomeScreenView& v);

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

    virtual ~HomeScreenPresenter() {};

private:
    HomeScreenPresenter();

    HomeScreenView& view;
};


#endif // HOMESCREEN_PRESENTER_HPP

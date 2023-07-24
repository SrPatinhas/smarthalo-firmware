#ifndef HOMEWESTSPLASH_PRESENTER_HPP
#define HOMEWESTSPLASH_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class HomeWestSplashView;

class HomeWestSplashPresenter : public Presenter, public ModelListener
{
public:
    HomeWestSplashPresenter(HomeWestSplashView& v);

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

    virtual ~HomeWestSplashPresenter() {};

private:
    HomeWestSplashPresenter();

    HomeWestSplashView& view;
};


#endif // HOMEWESTSPLASH_PRESENTER_HPP

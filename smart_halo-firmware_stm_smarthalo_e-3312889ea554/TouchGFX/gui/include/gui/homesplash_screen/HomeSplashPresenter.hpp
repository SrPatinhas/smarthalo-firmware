#ifndef HOMESPLASH_PRESENTER_HPP
#define HOMESPLASH_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class HomeSplashView;

class HomeSplashPresenter : public Presenter, public ModelListener
{
public:
    HomeSplashPresenter(HomeSplashView& v);

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

    virtual ~HomeSplashPresenter() {};

private:
    HomeSplashPresenter();

    HomeSplashView& view;
};


#endif // HOMESPLASH_PRESENTER_HPP

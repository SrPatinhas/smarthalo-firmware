#ifndef LIGHTWESTSPLASH_PRESENTER_HPP
#define LIGHTWESTSPLASH_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class LightWestSplashView;

class LightWestSplashPresenter : public Presenter, public ModelListener
{
public:
    LightWestSplashPresenter(LightWestSplashView& v);

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

    virtual ~LightWestSplashPresenter() {};

private:
    LightWestSplashPresenter();

    LightWestSplashView& view;
};


#endif // LIGHTWESTSPLASH_PRESENTER_HPP

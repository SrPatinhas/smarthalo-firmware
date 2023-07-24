#ifndef NAVIGATIONEASTSPLASH_PRESENTER_HPP
#define NAVIGATIONEASTSPLASH_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class NavigationEastSplashView;

class NavigationEastSplashPresenter : public Presenter, public ModelListener
{
public:
    NavigationEastSplashPresenter(NavigationEastSplashView& v);

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

    virtual ~NavigationEastSplashPresenter() {};

private:
    NavigationEastSplashPresenter();

    NavigationEastSplashView& view;
};


#endif // NAVIGATIONEASTSPLASH_PRESENTER_HPP

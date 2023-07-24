#ifndef FITNESSEASTSPLASH_PRESENTER_HPP
#define FITNESSEASTSPLASH_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class FitnessEastSplashView;

class FitnessEastSplashPresenter : public Presenter, public ModelListener
{
public:
    FitnessEastSplashPresenter(FitnessEastSplashView& v);

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

    virtual ~FitnessEastSplashPresenter() {};

private:
    FitnessEastSplashPresenter();

    FitnessEastSplashView& view;
};


#endif // FITNESSEASTSPLASH_PRESENTER_HPP

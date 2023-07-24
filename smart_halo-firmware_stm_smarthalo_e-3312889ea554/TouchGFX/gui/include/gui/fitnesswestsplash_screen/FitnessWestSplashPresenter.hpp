#ifndef FITNESSWESTSPLASH_PRESENTER_HPP
#define FITNESSWESTSPLASH_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class FitnessWestSplashView;

class FitnessWestSplashPresenter : public Presenter, public ModelListener
{
public:
    FitnessWestSplashPresenter(FitnessWestSplashView& v);

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

    virtual ~FitnessWestSplashPresenter() {};

private:
    FitnessWestSplashPresenter();

    FitnessWestSplashView& view;
};


#endif // FITNESSWESTSPLASH_PRESENTER_HPP

#ifndef ALARMEASTSPLASH_PRESENTER_HPP
#define ALARMEASTSPLASH_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class AlarmEastSplashView;

class AlarmEastSplashPresenter : public Presenter, public ModelListener
{
public:
    AlarmEastSplashPresenter(AlarmEastSplashView& v);

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

    virtual ~AlarmEastSplashPresenter() {};

private:
    AlarmEastSplashPresenter();

    AlarmEastSplashView& view;
};


#endif // ALARMEASTSPLASH_PRESENTER_HPP

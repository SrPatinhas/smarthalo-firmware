#ifndef ALARMSCREEN_PRESENTER_HPP
#define ALARMSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class AlarmScreenView;

class AlarmScreenPresenter : public Presenter, public ModelListener
{
public:
    AlarmScreenPresenter(AlarmScreenView& v);

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

    virtual ~AlarmScreenPresenter() {};

    void changeAssist(){
        model->changeAssist();
    }

    int16_t getAssist(){
        return model->getAssist();
    }

private:
    AlarmScreenPresenter();

    AlarmScreenView& view;
};


#endif // ALARMSCREEN_PRESENTER_HPP

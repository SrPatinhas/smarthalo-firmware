#ifndef FITNESSSCREEN_PRESENTER_HPP
#define FITNESSSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class FitnessScreenView;

class FitnessScreenPresenter : public Presenter, public ModelListener
{
public:
    FitnessScreenPresenter(FitnessScreenView& v);

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

    virtual ~FitnessScreenPresenter() {};

    void changeAssist(){
        model->changeAssist();
    }

    int16_t getAssist(){
        return model->getAssist();
    }

private:
    FitnessScreenPresenter();

    FitnessScreenView& view;
};


#endif // FITNESSSCREEN_PRESENTER_HPP

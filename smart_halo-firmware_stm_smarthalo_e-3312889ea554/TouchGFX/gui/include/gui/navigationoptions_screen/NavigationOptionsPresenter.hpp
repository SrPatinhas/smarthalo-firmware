#ifndef NAVIGATIONOPTIONS_PRESENTER_HPP
#define NAVIGATIONOPTIONS_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class NavigationOptionsView;

class NavigationOptionsPresenter : public Presenter, public ModelListener
{
public:
    NavigationOptionsPresenter(NavigationOptionsView& v);

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

    virtual ~NavigationOptionsPresenter() {};

private:
    NavigationOptionsPresenter();

    NavigationOptionsView& view;
};


#endif // NAVIGATIONOPTIONS_PRESENTER_HPP

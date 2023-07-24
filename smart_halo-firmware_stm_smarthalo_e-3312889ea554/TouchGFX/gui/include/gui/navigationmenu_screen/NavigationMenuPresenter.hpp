#ifndef NAVIGATIONMENU_PRESENTER_HPP
#define NAVIGATIONMENU_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class NavigationMenuView;

class NavigationMenuPresenter : public Presenter, public ModelListener
{
public:
    NavigationMenuPresenter(NavigationMenuView& v);

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

    virtual ~NavigationMenuPresenter() {};

private:
    NavigationMenuPresenter();

    NavigationMenuView& view;
};


#endif // NAVIGATIONMENU_PRESENTER_HPP

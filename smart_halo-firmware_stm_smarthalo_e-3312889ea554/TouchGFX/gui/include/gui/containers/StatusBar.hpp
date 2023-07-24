#ifndef STATUSBAR_HPP
#define STATUSBAR_HPP

#include <gui_generated/containers/StatusBarBase.hpp>

class StatusBar : public StatusBarBase
{
public:
    StatusBar();
    virtual ~StatusBar() {}

    virtual void initialize();
protected:
};

#endif // STATUSBAR_HPP

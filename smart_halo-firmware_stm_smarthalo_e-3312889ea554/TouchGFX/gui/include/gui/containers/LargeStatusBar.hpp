#ifndef LARGESTATUSBAR_HPP
#define LARGESTATUSBAR_HPP

#include <gui_generated/containers/LargeStatusBarBase.hpp>

class LargeStatusBar : public LargeStatusBarBase
{
public:
    LargeStatusBar();
    virtual ~LargeStatusBar() {}

    virtual void initialize();
protected:
};

#endif // LARGESTATUSBAR_HPP

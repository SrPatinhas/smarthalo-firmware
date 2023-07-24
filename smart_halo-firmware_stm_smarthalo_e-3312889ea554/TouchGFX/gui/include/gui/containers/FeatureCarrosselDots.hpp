#ifndef FEATURECARROSSELDOTS_HPP
#define FEATURECARROSSELDOTS_HPP

#include <gui_generated/containers/FeatureCarrosselDotsBase.hpp>

class FeatureCarrosselDots : public FeatureCarrosselDotsBase
{
public:
    FeatureCarrosselDots();
    virtual ~FeatureCarrosselDots() {}

    virtual void initialize();
    virtual void setColourOfDot(int position, colortype color);
protected:
};

#endif // FEATURECARROSSELDOTS_HPP

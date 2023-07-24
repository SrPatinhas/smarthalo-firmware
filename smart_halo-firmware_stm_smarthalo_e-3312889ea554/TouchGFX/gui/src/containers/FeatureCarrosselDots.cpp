#include <gui/containers/FeatureCarrosselDots.hpp>

FeatureCarrosselDots::FeatureCarrosselDots()
{

}

void FeatureCarrosselDots::initialize()
{
    FeatureCarrosselDotsBase::initialize();
}

void FeatureCarrosselDots::setColourOfDot(int position, colortype colour){
	switch(position){
		case 0:	homeDotPainter.setColor(colour,255); break;
		case 1: navDotPainter.setColor(colour,255); break;
		case 2: fitnessDotPainter.setColor(colour,255); break;
		case 3: lightDotPainter.setColor(colour,255); break;
		case 4: alarmDotPainter.setColor(colour,255); break;
		case 5: assistantDotPainter.setColor(colour,255); break;
	}
}
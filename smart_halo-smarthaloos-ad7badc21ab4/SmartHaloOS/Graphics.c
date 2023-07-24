#include "Graphics.h"

static Peripheral_HaloLED_API* _myHalo;
static const int DEFAULT_DURATION = 1000;

int Graphics_Register_Halo_Peripheral(Peripheral_HaloLED_API* halo) {
	_myHalo = halo;
	return 1;
}

int Graphics_Halo_flash(int _hue, int _saturation, int _lightness, int _repeat) {
	/*
		It's almost a straight pass-through here, but note that the duration isn't exposed to the application.
			This is just a way to show that these submodules will likely process their inputs before
			they actually call the peripherals.
	*/
	return _myHalo->flash(_hue, _saturation, _lightness, DEFAULT_DURATION, DEFAULT_DURATION, _repeat);
}
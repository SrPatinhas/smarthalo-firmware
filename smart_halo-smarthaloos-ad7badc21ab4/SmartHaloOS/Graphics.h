#ifndef SMARTHALO_GRAPHICS_H
#define SMARTHALO_GRAPHICS_H

/*
	I don't know what the best way to include the peripheral APIs are yet...
*/
#include "../HAL/peripheral.h"

int Graphics_Register_Halo_Peripheral(Peripheral_HaloLED_API* halo);

int Graphics_Halo_flash(int hue, int saturation, int lightness, int repeat);

#endif
#ifndef SMARTHALO_SOUNDS_H
#define SMARTHALO_SOUNDS_H

/*
	I don't know what the best way to include the peripheral APIs are yet...
*/
#include "../HAL/peripheral.h"

int Sounds_Register_Piezo_Peripheral(Peripheral_Piezo_API* halo);

int Sound_play_alarm_sound(int hue, int saturation, int lightness, int repeat);

#endif

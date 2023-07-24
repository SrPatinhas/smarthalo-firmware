/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Tone generation based on nRF HW timer
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#ifndef TONE_H_
#define TONE_H_

#include <stdbool.h>
#include "nrf.h"
#include "bsp.h"

 void Tone_setup(uint8_t tonePin);

 void Tone_play(uint16_t frequency, uint32_t duration);

 void Tone_stop();

 bool Tone_isPlaying();

#endif /* TONE_H_ */

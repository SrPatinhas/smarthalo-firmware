/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Set of functions to test SmartHalo sounds
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/05/30
 *
 */

#ifndef SOUNDTEST_H_
#define SOUNDTEST_H_

#include "bsp.h"

typedef enum {OFF = 0, CHIRPLOG, CHIRPLIN, TONE, DUALTONE} SOUNDMODE;

void SoundTest_setup(uint8_t piezoPin, uint8_t enablePin);

void SoundTest_printHelp();

bool SoundTest_parseAndExecuteCommand(char * RxBuff, int cnt);

void SoundTest_setSoundMode(SOUNDMODE sndmode);

SOUNDMODE SoundTest_getSoundMode();

void SoundTest_processSound();

int SoundTest_getTonef();

int SoundTest_getChirpMinf();

int SoundTest_getChirpMaxf();

int SoundTest_getNumPts();

int SoundTest_getDurationMs();

void SoundTest_setTonef(int tonef);

void SoundTest_setChirpMinf(int chirpmin_f);

void SoundTest_setChirpMaxf(int chirpmax_f);

void SoundTest_setNumPts(int numpts);

void SoundTest_setDurationMs(int durationms);

void SoundTest_setVolume(int percentVolume);

int SoundTest_getVolume();

#endif /* SOUNDTEST_H_ */

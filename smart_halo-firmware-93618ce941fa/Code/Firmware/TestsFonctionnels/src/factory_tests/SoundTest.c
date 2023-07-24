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

#include "SoundTest.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "pinmap.h"
#include "Tone.h"
#include "CommandLineInterface.h"
#include "nrf_delay.h"
#include "PowerSupply.h"

#define FMIN          20
#define FMAX          20000
#define NMIN          1
#define NMAX          100000
#define DURATIONMIN   0
#define DURATIONMAX   60000

#define FMIN_DEF      1400
#define FMAX_DEF      3200
#define F_DEF         2800
#define N_DEF         250
#define DURATION_DEF  0

static int tone_f           = F_DEF;
static int chirp_min_f      = FMIN_DEF;
static int chirp_max_f      = FMAX_DEF;
static int num_pts          = N_DEF;
static int duration_ms      = DURATION_DEF;

static SOUNDMODE soundmode = OFF;

void SoundTest_setup(uint8_t piezoPin)
{
	Tone_setup(piezoPin);
}

void SoundTest_printHelp()
{
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("                         SOUND");
	CommandLineInterface_printSeparator();
	CommandLineInterface_printLine("cn:\t\tPlays a linear chirp from fmin to fmax with N points");
	CommandLineInterface_printLine("cg:\t\tPlays a logarithmic chirp from fmin to fmax with N points");
	CommandLineInterface_printLine("t:\t\tPlays a single tone with frequency f indefinitely");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("dt:\t\tPlays two tones (fmin and fmax) for D ms each");
	CommandLineInterface_printLine("o:\t\tTurns off any sound");
	CommandLineInterface_printLine("fmin=<value>:\tSets the minimum frequency");
	CommandLineInterface_printLine("fmin:\t\tReturns the minimum frequency");
	CommandLineInterface_printLine("fmax=<value>:\tSets the maximum frequency");
	CommandLineInterface_printLine("fmax:\t\tReturnd the maximum frequency");
	nrf_delay_ms(50);
	CommandLineInterface_printLine("f=<value>:\tSets the frequency");
	CommandLineInterface_printLine("f:\t\tReturns the frequency");
	CommandLineInterface_printLine("N=<value>:\tSets the number of samples for chirp");
	CommandLineInterface_printLine("N:\t\tReturns the number of samples for chirp");
	CommandLineInterface_printLine("D=<value>:\tSets the duration in ms for chirp");
	CommandLineInterface_printLine("D:\t\tReturns the duration in ms for chirp");
	nrf_delay_ms(50);
}

bool SoundTest_parseAndExecuteCommand(char * RxBuff, int cnt)
{
	int value = 0;
	bool parsed = true;

	 // Word or dual letters
	if(cnt==4 && (RxBuff[0]=='C') && (RxBuff[1]=='G'))
	{
	  SoundTest_setSoundMode(CHIRPLOG);
	}
	else if(cnt==4 && (RxBuff[0]=='C') && (RxBuff[1]=='N'))
	{
	  SoundTest_setSoundMode(CHIRPLIN);
	}
	else if(cnt==4 && (RxBuff[0]=='D') && (RxBuff[1]=='T'))
	{
	  SoundTest_setSoundMode(DUALTONE);
	}
	// Single letters
	else if(cnt==3 && (RxBuff[0]=='T'))
	{
	  SoundTest_setSoundMode(TONE);
	}
	else if(cnt==3 && (RxBuff[0]=='O'))
	{
	  SoundTest_setSoundMode(OFF);
	}
	else if(sscanf(RxBuff, "FMIN=%d", &value)==1)
	{
		if(value < FMIN || value >= SoundTest_getChirpMaxf())
			CommandLineInterface_printf("Invalid FMIN: must be between %d and %d\r\n", FMIN, SoundTest_getChirpMaxf()-1);
	else
	{
		SoundTest_setChirpMinf(value);
		CommandLineInterface_printf("FMIN=%d\r\n", SoundTest_getChirpMinf());
	}
	}
	else if(sscanf(RxBuff, "FMAX=%d", &value)==1)
	{
		if(value <= SoundTest_getChirpMinf() || value > FMAX)
			CommandLineInterface_printf("Invalid FMAX: must be between %d and %d\r\n", SoundTest_getChirpMinf()+1, FMAX);
	else
	{
		SoundTest_setChirpMaxf(value);
		CommandLineInterface_printf("FMAX=%d\r\n", SoundTest_getChirpMaxf());
	}
	}
	else if(sscanf(RxBuff, "F=%d", &value)==1)
	{
		if(value < FMIN || value > FMAX)
			CommandLineInterface_printf("Invalid F: must be between %d and %d\r\n", FMIN, FMAX);
	else
	{
		SoundTest_setTonef(value);
		CommandLineInterface_printf("F=%d\r\n",SoundTest_getTonef());
	}
	}
	else if(sscanf(RxBuff, "N=%d", &value)==1)
	{
		if(value < NMIN || value > NMAX)
			CommandLineInterface_printf("Invalid N: must be between %d and %d\r\n", NMIN, NMAX);
	else
	{
		SoundTest_setNumPts(value);
		CommandLineInterface_printf("N=%d\r\n",SoundTest_getNumPts());
	}
	}
	else if(sscanf(RxBuff, "D=%d", &value)==1)
	{
		if(value < DURATIONMIN || value > DURATIONMAX)
			CommandLineInterface_printf("Invalid D: must be between %d and %d\r\n", DURATIONMIN, DURATIONMAX);
	else
	{
		SoundTest_setDurationMs(value);
		CommandLineInterface_printf("D=%d\r\n",SoundTest_getDurationMs());
	}
	}
	else if(strncmp(RxBuff, "FMIN", 4)==0)
	{
		CommandLineInterface_printf("FMIN=%d\r\n",SoundTest_getChirpMinf());
	}
	else if(strncmp(RxBuff, "FMAX", 4)==0)
	{
		CommandLineInterface_printf("FMAX=%d\r\n",SoundTest_getChirpMaxf());
	}
	else if(strncmp(RxBuff, "F", 1)==0)
	{
		CommandLineInterface_printf("F=%d\r\n",SoundTest_getTonef());
	}
	else if(strncmp(RxBuff, "N", 1)==0)
	{
		CommandLineInterface_printf("N=%d\r\n",SoundTest_getNumPts());
	}
	else if(strncmp(RxBuff, "D", 1)==0)
	{
		CommandLineInterface_printf("D=%d\r\n",SoundTest_getDurationMs());
	}
	else
	{
		parsed = false;
	}

	return parsed;
}

void SoundTest_setSoundMode(SOUNDMODE sndmode)
{
	soundmode = sndmode;

	// Update supplies
	if(sndmode == OFF)
	{
		if(PowerSupply_getState(SUPPLY_VPIEZO))
			PowerSupply_disable(SUPPLY_VPIEZO);
		if(PowerSupply_getState(SUPPLY_S2_8V))
			PowerSupply_disable(SUPPLY_S2_8V);
	}
	else
	{
		if(!PowerSupply_getState(SUPPLY_VPIEZO))
			PowerSupply_enable(SUPPLY_VPIEZO);
		if(!PowerSupply_getState(SUPPLY_S2_8V))
			PowerSupply_enable(SUPPLY_S2_8V);
	}

	switch(soundmode)
	{
		case CHIRPLOG:
			CommandLineInterface_printf("Chirp from %d to %d Hz, with %d points, for %d ms, in log scale\r\n", chirp_min_f, chirp_max_f, num_pts, duration_ms);
			break;
		case CHIRPLIN:
			CommandLineInterface_printf("Chirp from %d to %d Hz, with %d points, for %d ms, in lin scale\r\n", chirp_min_f, chirp_max_f, num_pts, duration_ms);
			break;
		case TONE:
			CommandLineInterface_printf("Tone at %d Hz\r\n", tone_f);
			break;
		case DUALTONE:
			CommandLineInterface_printf("Dual tone at %d and %d Hz, for %d ms\r\n", chirp_min_f, chirp_max_f, duration_ms);
			break;
		case OFF:
		default:
			CommandLineInterface_printLine("Turning off");
			Tone_stop();
			break;
	}
}

SOUNDMODE SoundTest_getSoundMode()
{
	return soundmode;
}

int SoundTest_getTonef()
{
	return tone_f;
}

int SoundTest_getChirpMinf()
{
	return chirp_min_f;
}

int SoundTest_getChirpMaxf()
{
	return chirp_max_f;
}

int SoundTest_getNumPts()
{
	return num_pts;
}

int SoundTest_getDurationMs()
{
	return duration_ms;
}

void SoundTest_setTonef(int tonef)
{
	tone_f = tonef;
}

void SoundTest_setChirpMinf(int chirpmin_f)
{
	chirp_min_f = chirpmin_f;
}

void SoundTest_setChirpMaxf(int chirpmax_f)
{
	chirp_max_f = chirpmax_f;
}

void SoundTest_setNumPts(int numpts)
{
	num_pts = numpts;
}

void SoundTest_setDurationMs(int durationms)
{
	duration_ms = durationms;
}


void SoundTest_Chirp(int f_min, int f_max, int numpts, int duration_ms, bool logmode)
{
	double chirp_min_f  = logmode?log10((double)f_min):f_min;
	double chirp_max_f  = logmode?log10((double)f_max):f_max;
	double chirp_range  = chirp_max_f - chirp_min_f;
	double chirp_f_inc  = chirp_range/num_pts;

	double f_log = chirp_min_f;
	double f_lin = chirp_min_f;
	int toneduration_ms = round((double)duration_ms/(double)num_pts);

	for (int i=0; i<numpts; i++)
	{
	f_log += chirp_f_inc;
	f_lin = logmode?pow(10,f_log):(f_lin+chirp_f_inc);

	while((Tone_isPlaying())&&(toneduration_ms>0));

	if(toneduration_ms > 0)
	  Tone_play(f_lin,toneduration_ms);
	else
	  Tone_play(f_lin, 0);
  }
}

void SoundTest_DualTone(int f_min, int f_max, int duration_ms)
{
	Tone_play(f_min,duration_ms);

	while(Tone_isPlaying()&&(duration_ms>0));

	Tone_play(f_max,duration_ms);

	while(Tone_isPlaying()&&(duration_ms>0));
}

void SoundTest_processSound()
{
	if(soundmode == TONE)
	{
		if(Tone_isPlaying())
		{
			Tone_stop();
		}
		Tone_play(tone_f, 0);
	}
	else if(soundmode == DUALTONE)
	{
		if(Tone_isPlaying())
		{
		  Tone_stop();
		}
		SoundTest_DualTone(chirp_min_f, chirp_max_f, duration_ms);
	}
	else if(soundmode == CHIRPLOG)
	{
		if(Tone_isPlaying())
		{
		  Tone_stop();
		}
		SoundTest_Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, true);
	}
	else if(soundmode == CHIRPLIN)
	{
		if(Tone_isPlaying())
		{
		  Tone_stop();
		}
		SoundTest_Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, false);
	}
}

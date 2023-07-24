/*
 * SH_Piezo_sounds.h
 *
 *  Created on: Apr 13, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SH_PIEZO_SOUNDS_H_
#define SH_FIRMWARE_CODE_SH_PIEZO_SOUNDS_H_

#define VOLUME_CHIP_ADDRESS				0x72
#define TIME_FOR_START_BIT 				2
#define TIME_FOR_END_BIT				2

#define TIME_FOR_LOW_FOR_LOW_BIT		12
#define TIME_FOR_HIGH_FOR_LOW_BIT		5
#define TIME_FOR_LOW_FOR_HIGH_BIT		5
#define TIME_FOR_HIGH_FOR_HIGH_BIT		12
#define NUMBER_OF_BITS_ADDRESS			8
#define NUMBER_OF_BITS_DATA_FOR_VOLUME	5


#define FMIN          20
#define FMAX          20000
#define NMIN          1
#define NMAX          100000
#define DURATIONMIN   0
#define DURATIONMAX   60000

#define FMIN_DEF      1400
#define FMAX_DEF      3200
#define F_DEF         200
#define N_DEF         250
#define DURATION_DEF  1000

//used for the piezo
typedef enum SOUNDMODE SOUNDMODE;
enum SOUNDMODE {SOUND_OFF=0, CHIRPLOG, CHIRPLIN, TONE, DUALTONE, SMS_SOUND, WARNING_SOUND, TURN_NOTIFICATION_SOUND,\
	TURN_SUCCESFUL_SOUND, PHONECALL_SOUND, ALARM_SOUND, NUMBER_OF_SOUNDS};

//generated musical notes:
//https://fr.wikipedia.org/wiki/Fr%C3%A9quences_des_touches_du_piano
//in microseconds

#define DO2_PERIOD		7644L
//#define RE_PERIOD
//#define MI_PERIOD
#define SOL2_PERIOD		5102L	//391.955 hz
//#define LA_PERIOD
//#define SI_PERIOD

#define DO3_PERIOD 		3822L  //Do3 in latin notation 261.62hz
//#define RE_PERIOD
//#define MI_PERIOD
#define SOL3_PERIOD		2551L	//391.955 hz
#define LA3_PERIOD		2273L 	//440 hz
#define SI3_PERIOD		2025L 	//493.883hz

#define DO4_PERIOD 		1911L  //Do4 in latin notation 523.251hz
//#define RE_PERIOD
//#define MI_PERIOD
//#define SOL_PERIOD
#define LA4_PERIOD		1136L
//#define SI_PERIOD

#define DO5_PERIOD 		956L  //Do5 in latin notation 1046.5hz
//#define RE_PERIOD
//#define MI_PERIOD
//#define SOL_PERIOD
#define LA5_PERIOD		568L
//#define SI_PERIOD

#define DO6_PERIOD 		478L //Do6 2093hz
//#define RE2_PERIOD
//#define MI2_PERIOD
//#define SOL2_PERIOD
//#define LA2_PERIOD
//#define SI3_PERIOD


#define HIGH_TIME_IN_MS_WITH_FREQUENCY(FREQUENCY)	((1/FREQUENCY)/2 * 1000)

#define VOLUME_0						0b00000
#define VOLUME_0_031					0b00001
#define VOLUME_0_049					0b00010
#define VOLUME_0_068					0b00011
#define VOLUME_0_086					0b00100
#define VOLUME_0_104					0b00101
#define VOLUME_0_123					0b00110
#define VOLUME_0_141					0b00111
#define VOLUME_0_160					0b01000
#define VOLUME_0_178					0b01001
#define VOLUME_0_197					0b01010
#define VOLUME_0_215					0b01011
#define VOLUME_0_234					0b01100
#define VOLUME_0_270					0b01101
#define VOLUME_0_307					0b01110
#define VOLUME_0_344					0b01111
#define VOLUME_0_381					0b10000
#define VOLUME_0_418					0b10001
#define VOLUME_0_455					0b10010
#define VOLUME_0_492					0b10011
#define VOLUME_0_528					0b10100
#define VOLUME_0_565					0b10101
#define VOLUME_0_602					0b10110
#define VOLUME_0_639					0b10111
#define VOLUME_0_713					0b11000
#define VOLUME_0_787					0b11001
#define VOLUME_0_860					0b11010
#define VOLUME_0_934					0b11011
#define VOLUME_1_008					0b11100
#define VOLUME_1_082					0b11101
#define VOLUME_1_155					0b11110
#define VOLUME_1_229					0b11111

//initiliases the timers and pin outputs for the piezo alarm
void piezo_init();

//sets the volume of the piezo using easyScale protocol
void piezo_set_volume();

//plays the specified sound
void play_sound(SOUNDMODE new_sound);

//starts the alarm sound, sound_types are defined in typedefs, piezo_init() must be called once before
bool piezo_alarm_start();

//stops the alarm sound
void piezo_alarm_stop();

//get the current sound of the piezo
SOUNDMODE piezo_get_current_sound ();

//modify the sound of the piezo (but doesn't apply it immediately, you have to call piezo_alarm_start to set the new sound
void piezo_modify_sound (SOUNDMODE new_sound);

//modify the volume of the piezo (but doesn't apply it immediately, you have to call set_volume
void piezo_modify_volume (uint8_t new_volume);

//Get the current volume of the piezo in binary
uint8_t piezo_get_current_volume();


#endif /* SH_FIRMWARE_CODE_SH_PIEZO_SOUNDS_H_ */

// ------------------------------------------------------------------------------------------------
/*!@file    soundTask.h

 */
// ------------------------------------------------------------------------------------------------


#ifndef sound_TASK_H_
#define sound_TASK_H_

#include "main.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define SOUND_MAX_SEQ 16
// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================
typedef struct {
    uint8_t  volume;
    uint8_t  repeat;
    uint16_t freq[SOUND_MAX_SEQ];
    bool     sweep[SOUND_MAX_SEQ];
    uint16_t duration[SOUND_MAX_SEQ];
    uint8_t  nbr_seq;
    bool     singleEdge;
} __attribute__((packed)) sound_cmd_t;

// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
void init_SoundTask();
void unlockSounds_SoundTask(const char * const taskName);
void lockSounds_SoundTask(const char * const taskName);
bool setSoundWithLock_SoundTask(sound_cmd_t * sound_cmd, const char * const taskName);
bool setSound_SoundTask(sound_cmd_t * sound_cmd);
void soundReceive_SoundTask(uint16_t length, uint8_t * data);
void soundStop_SoundTask();
void compileSound_SoundTask(uint16_t * frequencies, uint16_t * durations, bool * sweeps, uint8_t volume, uint8_t count, uint8_t repeats);
bool soundTest_SoundTask(bool isPlaying);
uint32_t sound_stack();

#endif  /* sound_TASK_H_ */

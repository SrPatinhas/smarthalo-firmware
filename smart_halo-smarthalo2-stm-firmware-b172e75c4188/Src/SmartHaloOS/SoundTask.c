// ------------------------------------------------------------------------------------------------
/*!@file    SoundTask.c
 */
// ------------------------------------------------------------------------------------------------
#include <CommunicationTask.h>
#include <SoundTask.h>
#include "main.h"
#include "GraphicsTask.h"
#include <stdio.h>
#include <string.h>
#include <SystemUtilitiesTask.h>
#include "PiezoDriver.h"
#include "WatchdogTask.h"
#include "SHTaskUtils.h"

// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================
#define QUEUE_LENGTH 4
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY     PRIORITY_NORMAL

// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

// ================================================================================================
// ================================================================================================
//            PRIVATE ENUM DEFINITION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE VARIABLE DECLARATION
// ================================================================================================
// ================================================================================================
static TaskHandle_t selfHandle = NULL;
static StaticTask_t xTaskBuffer;
static StackType_t SoundStack[STACK_SIZE];

static char lockingName[30] = {0};
static sound_cmd_t Sound;

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
static void prvsoundTask(void *pvParameters);
bool soundSetVolume(uint8_t volume);
// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================
/**
 * @brief Initialize the sound task
 * @details Setup the sound task, initailize the piezo driver.
 */
void init_SoundTask() {
    if (selfHandle == NULL) {
        selfHandle = xTaskCreateStatic(prvsoundTask, TASKNAME_SOUND,
        STACK_SIZE, NULL, TASK_PRIORITY, SoundStack, &xTaskBuffer);
        configASSERT(selfHandle);
    }
    init_PiezoDriver();
    setVolumeAmpState_PiezoDriver(false);
}
/**
 * @brief       sound_stack()
 * @details        Sound task stack
 * @return      uint32_t.
 */
uint32_t sound_stack() {
    return uxTaskGetStackHighWaterMark(selfHandle);
}

bool setSound(sound_cmd_t * sound_cmd)
{
    if (sound_cmd == NULL) return false;

    if (sound_cmd->volume > 100) return true;

    memcpy(&Sound, sound_cmd, sizeof(sound_cmd_t));

    setVolumeAmpState_PiezoDriver(true);
    // todo Generate and event for the task

    return true;
}

void unlockSounds_SoundTask(const char * const taskName){
    if(lockingName[0]==0 || !strcmp(lockingName,taskName))
        memset(lockingName,0,sizeof(lockingName));
}

void lockSounds_SoundTask(const char * const taskName){
    if(lockingName[0]==0 || !strcmp(lockingName,taskName))
        strcpy(lockingName, taskName);
}

bool setSoundWithLock_SoundTask(sound_cmd_t * sound_cmd, const char * const taskName){
    if(lockingName[0]==0 || !strcmp(lockingName,taskName)){
        strcpy(lockingName, taskName);
        return setSound(sound_cmd);
    }
    return false;
}

/**
 * @brief       Play a sound
 * @details        Set the new sound to play
 * @return      bool: true if success, false otherwise.
 */
bool setSound_SoundTask(sound_cmd_t * sound_cmd)
{
    if(lockingName[0]==0)
        return setSound(sound_cmd);
    return false;
}

/**
 * @brief       Play a test sound()
 * @return      bool: true if success, false otherwise.
 */
bool soundTest_SoundTask(bool isPlaying)
{
    uint32_t u32Index = 0;
    while(isPlaying)
    {
        setVolumeAmpState_PiezoDriver(true);
        setVolume_PiezoDriver((u32Index += 10 ) % 100);

        setFrequency_PiezoDriver(273);
        setToneDriverState_PiezoDriver(true);

        vTaskDelay(100);

        setFrequency_PiezoDriver(500);
        setToneDriverState_PiezoDriver(true);

        vTaskDelay(100);

        setVolumeAmpState_PiezoDriver(false);
        setToneDriverState_PiezoDriver(false);

        vTaskDelay(10);
        if (u32Index >= 100) break;
    }
    return true;
}

/**
 * @brief Create a sound track from its core components
 * @details Compiling the track from frequency, duration, sweep, volume, count and repeat data
 */
void compileSound_SoundTask(uint16_t * frequencies, uint16_t * durations, bool * sweeps, uint8_t volume, uint8_t count, uint8_t repeats){
    sound_cmd_t soundCmd;
    memset(&soundCmd, 0, sizeof(sound_cmd_t));
    memcpy(soundCmd.duration, durations, count*2);
    memcpy(soundCmd.freq, frequencies, count*2);
    memcpy(soundCmd.sweep, sweeps, count);
    soundCmd.volume = volume;
    soundCmd.nbr_seq = count;
    soundCmd.repeat = repeats;
    setSound_SoundTask(&soundCmd);
}

/**
 * @brief Create a sound from a BLE message
 */
void soundReceive_SoundTask(uint16_t length, uint8_t * data){
    uint32_t u32Ptr = 2;

    sound_cmd_t SoundCommand;
    memset(&SoundCommand, 0, sizeof(SoundCommand));

    SoundCommand.volume = data[0];
    SoundCommand.repeat = data[1];

    while (u32Ptr < length && SoundCommand.nbr_seq < SOUND_MAX_SEQ){
        SoundCommand.freq[SoundCommand.nbr_seq] = data[u32Ptr++]<<8;
        SoundCommand.freq[SoundCommand.nbr_seq] += data[u32Ptr++];
        SoundCommand.sweep[SoundCommand.nbr_seq] = ((data[u32Ptr] & 0x80) > 0) ? true : false;
        SoundCommand.duration[SoundCommand.nbr_seq] = (data[u32Ptr++] & 0x7f)<<8;
        SoundCommand.duration[SoundCommand.nbr_seq] += data[u32Ptr++];
        SoundCommand.nbr_seq++;
    }
    setSound_SoundTask(&SoundCommand);
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

void soundStop(uint16_t length, uint8_t * payload){
    Sound.nbr_seq = 0;
    genericResponse_CommunicationTask(eCOM_RETURN_STATUS_OK);
}

/**
 * @brief Halt current sound track
 */
void soundStop_SoundTask(){
    Sound.nbr_seq = 0;
}

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================

/**
 * @brief       prvsoundTask()
 * @details        Sound Task
 * @public
 * @param[IN]    pvParameters: RTOS param
 */
static void prvsoundTask(void *pvParameters) {

    float incrementCount = 10.f;
    float df = 0;
    float dt = 0.0f;

    assignFunction_CommunicationTask(COM_SOUND, SOUND_PLAY, &soundReceive_SoundTask);
    assignFunction_CommunicationTask(COM_SOUND, SOUND_STOP, &soundStop);

    setVolumeAmpState_PiezoDriver(true);

    while (1) {
        if(Sound.nbr_seq != 0){
            setVolume_PiezoDriver(Sound.volume);
            if (Sound.singleEdge){ //allows crisp clicks
                vTaskDelay(5);//allows power supplies to stabilize for consistent volume
                singleStateChange_PiezoDriver();
                setVolumeAmpState_PiezoDriver(false);
                vTaskDelay(Sound.duration[0]);
            }else{
                for (int repeat = 0; repeat <= Sound.repeat; ++repeat){
                    for (int seqNbr = 0; seqNbr < Sound.nbr_seq; ++seqNbr){
                        if (Sound.duration[seqNbr] == 0){
                            setFrequency_PiezoDriver(1);
                        }else if (Sound.freq[seqNbr] == 0){
                            setToneDriverState_PiezoDriver(false);
                            shwd_KeepAlive(eSHWD_SoundTask);
                            vTaskDelay(Sound.duration[seqNbr]);
                        }else{  
                            if (Sound.sweep[seqNbr] && (seqNbr + 1 < Sound.nbr_seq)){
                                int16_t curFreq = Sound.freq[seqNbr];
                                int16_t nextFreq = Sound.freq[seqNbr + 1];
                                df = (float)(nextFreq - curFreq) / incrementCount;
                                dt = (float) MAX(10,((Sound.duration[seqNbr]) / incrementCount));
                                for (int d = 0; d<incrementCount; d++){
                                    setFrequency_PiezoDriver((curFreq + df*d));
                                    setToneDriverState_PiezoDriver(true);
                                    shwd_KeepAlive(eSHWD_SoundTask);
                                    vTaskDelay(dt);
                                }
                            }else{
                                setFrequency_PiezoDriver((float)Sound.freq[seqNbr]);
                                setToneDriverState_PiezoDriver(true);
                                shwd_KeepAlive(eSHWD_SoundTask);
                                vTaskDelay(Sound.duration[seqNbr]);
                            }
                        }
                    }
                }
            }
            Sound.nbr_seq = 0;
            setToneDriverState_PiezoDriver(false);
        }
        vTaskDelay(50);
        if(Sound.nbr_seq == 0){
            setVolumeAmpState_PiezoDriver(false);
        }
    shwd_KeepAlive(eSHWD_SoundTask);
    }
}

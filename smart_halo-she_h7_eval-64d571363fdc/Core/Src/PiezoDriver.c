/*
 * Piezo.c
 *
 *  Created on: July 19
 *      Author: JF Lemire
 */

#include "PiezoDriver.h"
#include "main.h"
#include "tim.h"
// ================================================================================================
// ================================================================================================
//            PRIVATE DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE MACRO DEFINITION
// ================================================================================================
// ================================================================================================
#define _ENABLE_VBZ_LDO() HAL_GPIO_WritePin(LDO_BZR_EN_GPIO_Port, LDO_BZR_EN_Pin, GPIO_PIN_SET)
#define _DISABLE_VBZ_LDO() HAL_GPIO_WritePin(LDO_BZR_EN_GPIO_Port, LDO_BZR_EN_Pin, GPIO_PIN_RESET)
#define _SET_VBZ_PIN() HAL_GPIO_WritePin(BZR_GPIO_Port, BZR_Pin, GPIO_PIN_SET)
#define _RESET_VBZ_PIN() HAL_GPIO_WritePin(BZR_GPIO_Port, BZR_Pin, GPIO_PIN_RESET)
#define _TOGGLE_VBZ_PIN() HAL_GPIO_TogglePin(BZR_GPIO_Port, BZR_Pin)

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

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PUBLIC FUNCTION SECTION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            PRIVATE FUNCTION SECTION
// ================================================================================================
// ================================================================================================
static bool piezoConvertFrequency(float freq, uint32_t * toneRegister);

/**
 * @brief       Initialize the Piezo timers
 * @details     Set up the two timers to control the piezo sounds
 * @return      bool: true if success, false otherwise.
 */
bool init_PiezoDriver(void)
{
	/* Start the TIM time Base generation in interrupt mode */
	HAL_TIM_Base_Start_IT(&PIEZO_TIMER);
	HAL_TIM_SET_VALUE(&PWM_PIEZO_VOLUME, PWM_PIEZO_VOLUME_CH, 500);
	HAL_TIM_SET_STATE(&PWM_PIEZO_VOLUME, PWM_PIEZO_VOLUME_CH, false);
	setToneDriverState_PiezoDriver(false);

	return true;
}
/**
 * @brief       Set the frequency of the piezo
 * @details		PWM frequency in register value.
 *              Uses the function PiezoConvertFrequency to convert the frequency
 *              into register value.
 * @param[IN]	frequency: Frequency value
 * @return      bool: true if success, false otherwise.
 */
bool setFrequency_PiezoDriver(float frequency)
{
	uint32_t u32Tone;
	if(frequency == 0) frequency = 1;
	piezoConvertFrequency(frequency, &u32Tone);
	HAL_TIM_SET_PRESCALER(&PIEZO_TIMER,u32Tone);
	return true;
}
/**
 * @brief       Set the piezo volume
 * @details		Set the volume level in % - NOT LINEAR
 * @param[IN]	volume: Value in % of the volume.
 * @return      bool: true if success, false otherwise.
 */
bool setVolume_PiezoDriver(uint8_t volume)
{
	if (volume >= 100) volume = 100;
	HAL_TIM_SET_VALUE(&PWM_PIEZO_VOLUME, PWM_PIEZO_VOLUME_CH, volume * 10);
	HAL_TIM_SET_STATE(&PWM_PIEZO_VOLUME, PWM_PIEZO_VOLUME_CH, true);
	return true;
}
/**
 * @brief       Set the state of the piezo timer to generate a tone to be 
 *							inputted to the differential tone driver
 * @details		Set the piezo timer state.
 * @param[IN]	state: State of the buzzer timer
 * @return      bool: true if success, false otherwise.
 */
bool setToneDriverState_PiezoDriver(bool state)
{
	if (false == state)
	{
		__HAL_TIM_DISABLE(&PIEZO_TIMER);
	}
	else
	{
		__HAL_TIM_ENABLE(&PIEZO_TIMER);
	}
	return true;
}
/**
 * @brief       Set piezo regulators states, both for the 20V amplifier, 
 *							and the done driver power supply
 * @param[IN]	state: States of the power modules of the Piezo.
 * @return      bool: true if success, false otherwise.
 */
bool setVolumeAmpState_PiezoDriver(bool state)
{
	if (false == state)
	{
		_DISABLE_VBZ_LDO();
		HAL_TIM_SET_STATE(&PWM_PIEZO_VOLUME, PWM_PIEZO_VOLUME_CH, false);
	}
	else
	{
		_ENABLE_VBZ_LDO();
		HAL_TIM_SET_STATE(&PWM_PIEZO_VOLUME, PWM_PIEZO_VOLUME_CH, true);
	}
	return true;
}

/**
 * @brief       singleStateChange_PiezoDriver()
 * @details		toggles the piezo buzzer output a single time
 * @public
 * @param[IN] none
 * @param[OUT]	none
 * @return      none
 */
void singleStateChange_PiezoDriver()
{
	 _TOGGLE_VBZ_PIN();
}

/**
 * @brief       PiezoConvertFrequency()
 * @details		Convert frequency in register value.
 * @public
 * @param[IN]	u32Freq: Frequency in Hz to play
 * @param[OUT]	pu32Register: Handle on the var to return the converted value.
 * @return      bool: true if success, false otherwise.
 */
static bool piezoConvertFrequency(float freq, uint32_t * toneRegister)
{
  if (toneRegister == NULL) return false;

  /* The following formula can be found in any timer + prescaler documentation from stm, barring the /2, 
   * which is added to compensate for the fact that we need 2 timer interrupts to form a complete period 
   */
	if(freq)
	    *toneRegister = (uint32_t) (( 280000000 / ( (290+1)*freq ) ) - 1)/2;
	else
	    *toneRegister = 0;

	return true;
}

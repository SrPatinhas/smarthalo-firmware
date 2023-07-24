/*
 * FrontLEDDriver.c
 *
 *  Created on: Aug 9, 2019
 */


#include "FrontLEDDriver.h"
#include "stm32l4xx_hal.h"
#include "tim.h"
#include "gpio.h"
#include "Power.h"

/**
 * @brief Init the front LED
 * @details Set up the timer for the front LED.
 */
bool init_FrontLEDDriver(void){
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);	// Front Led Pin
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

	return true;
}

/**
 * @brief      Set the percentage of the front LED
 * @details    Adjust the PWM to the Front LED to increase the brightness
 *
 * @param[in]  percentage  brightness percentage for the LED
 */
void setFrontLEDPercentage_FrontLEDDriver(uint8_t percentage)
{
    static uint8_t lastPercentage;
    static bool    flState = false;  // to track the state of the LED

    // If nothing is changing, no need to touch the hardware
    if (lastPercentage == percentage) return;

    if (percentage > 100) {
        percentage = 100;
    }

    if (percentage > 0) {
        if (!flState) {
            flState = true;
            SLEEP_NOTALLOWED();
        }
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, percentage);
        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    } else {
        if (flState) {
            flState = false;
            SLEEP_ALLOWED();
        }
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    }

    lastPercentage = percentage;
}

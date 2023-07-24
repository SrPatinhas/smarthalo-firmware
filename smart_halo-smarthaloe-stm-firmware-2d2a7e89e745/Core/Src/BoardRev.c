/*!
    @file BoardRev.c

    Interface to PCB board revision.

    @author     Georg Nikodym
    @copyright  Copyright (c) 2020 SmartHalo Inc
 */

#include <stdint.h>
#include "BoardRev.h"
#include "stm32h7xx_hal.h"

/*! @brief      initialize the board revision GPIO lines
*/
//void init_BoardRev(void)
//{
//    HAL_PWREx_EnablePullUpPullDownConfig();
//    HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_H, REV_ID_0_Pin);
//    HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_A, REV_ID_1_Pin);
//    HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_A, REV_ID_2_Pin);
//    HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_A, REV_ID_3_Pin);
//}

/*! @brief      return the revision of the SH2 PCB
    @details    4 spare pins on the MCU are used to represent bits of a board
                version number. The pins are preset to be pull-up. Each rev of of
                the PCB will have an appropriate configuration of pull-down
                resistors populated. If there are no pull-downs then we assume
                BASE_BOARD_REV (4).

    @return     The board revision.
*/
uint8_t get_BoardRev(void)
{
    uint8_t rev = 0;
    GPIO_PinState pin;

    pin = HAL_GPIO_ReadPin(REV_ID_0_GPIO_Port, REV_ID_0_Pin);
    rev |= (pin == GPIO_PIN_RESET) ? 1 : 0;

    pin = HAL_GPIO_ReadPin(REV_ID_1_GPIO_Port, REV_ID_1_Pin);
    rev |= ((pin == GPIO_PIN_RESET) ? 1 : 0) << 1;

    pin = HAL_GPIO_ReadPin(REV_ID_2_GPIO_Port, REV_ID_2_Pin);
    rev |= ((pin == GPIO_PIN_RESET) ? 1 : 0) << 2;

    pin = HAL_GPIO_ReadPin(REV_ID_3_GPIO_Port, REV_ID_3_Pin);
    rev |= ((pin == GPIO_PIN_RESET) ? 1 : 0) << 3;

    return BASE_BOARD_REV + rev;
}

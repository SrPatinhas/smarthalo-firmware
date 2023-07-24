/*
 * Machine specific implementation of putchar.
 * Needed by open source printf() in OSS directory.
 */

#include "usart.h"

void _putchar(char c)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, 0xffff);
}

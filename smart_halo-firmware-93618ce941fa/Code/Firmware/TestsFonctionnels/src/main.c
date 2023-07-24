#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "Factory_tests.h"
#include "UART.h"
#include "nrf_delay.h"

int main(void)
{   
	UART_setup();
	enter_factory_mode();
    return 0;
}

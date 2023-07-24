#include "Bluetooth.h"
#include "CommandLineInterface.h"

int main(void)
{
	CommandLineInterface_setup();

    for (;;)
    {
        Bluetooth_power_manage();
    }
}

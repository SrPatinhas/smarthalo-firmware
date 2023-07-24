# SHe nRF code 

## Bluetooth multilink code

The shimano information profile (shiis) has been implemented to collect data from a bluetooth dongle on a shimano ebike at the same time as being connected to the Smarthalo App. 

As of the departure of Sean, this code is ready to send out information to the stm via the uart, but using the old format. Matt will need to modify the uart to the new format to create a proper demo. 

To find the Shimano Bluetooth documentation, click [here](https://app.asana.com/0/230732220528293/1160399582540528/f)

This code is very close in function to the original bleapp, except that there are now seperate event handlers for peripheral or central role events, which get dispatched based on role type (see ble_evt_dispatch() in bleapp.c). 

The shimano device will send notification every 250ms containing the information detailed in the annex of [this document](https://docs.google.com/document/d/1vkPnkMTyNhJ6i6ff6garTavYht0pS7AXKZ40Yzr5EiM/edit?usp=sharing) or in more detail in the Shimano bluetooth documentation, linked above.

Also, when doing a pull request, remember that this repo was forked from the SH2 repo so make sure you are not merging your code into smarthalo2-nrf-firmware. This is a pain in the ass, bitbucket has taken the offcial stance that this is a feature and not a bug, there is no mechanism to choose a default repo to merge in as of this writing.

## Next steps recommendations
1- Look for all //SBEITZ tags, these indicate a to do, or important information

2- the MTU event in BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST needs to be adressed. it currently works with all tested phones by disabling it. 

3- A spec should be elaborated for security settings for the shimano link, pairing mechanisms in factory and out, etc.

4- Currently the device will connect to any device with the right name. I recommend changing the filters from just name to name or uuid and whitelisted mac address. The mac address will need to be set in factory or when pairing from the user.

5- UART mods to fit the new uart format.

6- VERY important: check the data throughput when in multilink connection, make sure there are no issues with data transmission.

7- Call sean at 514 435 6090 for questions

## to debug via real time terminal (printf via segger debugger)

Console:

  In one terminal: ./gdbServer.sh
  In another Terminal: telnet localhost 19021

If no console output, in project folder:

  make reset

If still no output, stop gdbServer.sh and repeat

Can be used in conjunction with gdb debugging line by line using gdb and arm-cortex debugging plugin of vscode



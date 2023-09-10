# SmartHalo Firmware

This repository contains firmware for the SmartHalo product family. It was released publicly (except for bootloader signing keys) after SmartHalo's bankruptcy, in order that a community effort could continue to support the products.

## SmartHalo 1

This product has a single chip - an nRF52832.

Firmware (and bootloader) resides in `firmware/Code`. It uses nRF5 SDK 12.1.0.

PCB and schematic are in the `hardware` folder. They can be opened with KiCad (use pcbnew/eeschema directly, then use File -> Import -> Non KiCad).

## SmartHalo 2

This product has two chips:
* nRF52832 - for Bluetooth LE communication
* STM32(L4...?) - for display?

The firmware (and bootloader) for the nRF52 resides in `smarthalo2-nrf-firmware/Code`. It uses nRF5 SDK 16.0.0.
The firmware for the STM32 resides in `smarthalo2-stm-firmware`
The bootloader for the STM32 resides in `smarthalo2_bootloader`

The PCB and schematic are not provided????

## Tests, factory fixture etc 

Not currently understood or used.


## Security

SmartHalo implements a password mechanism in the device. Without this password it's impossible to command the device in normal mode.

It may still be possible to perform a DFU, and thereby add a reset command, to recover password-protected devices. It would be sensible to add a long delay (say 1 hour to 1 week) to prevent this from being used to bypass the anti-theft alarm.
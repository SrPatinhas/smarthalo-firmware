# SmartHalo Firmware

This repository contains firmware for the SmartHalo product family. It was released publicly (except for bootloader signing keys) after SmartHalo's bankruptcy, in order that a community effort could continue to support the products.

## SmartHalo 1

This product has a single chip - an nRF52832.

Firmware (and bootloader) resides in `firmware/Code`

## SmartHalo 2

This product has two chips:
* nRF52 - for Bluetooth LE communication
* STM32 - for display?

The firmware (and bootloader) for the nRF52 resides in `smarthalo2-nrf-firmware/Code`
The firmware for the STM32 resides in `smarthalo2-stm-firmware`
The bootloader for the STM32 resides in `smarthalo2_bootloader`

## Tests, factory fixture etc 

Not currently understood or used.
# SH2 STM FIRMWARE

## Run the docker machine

Install [docker](https://docs.docker.com/v17.09/engine/installation/), then build the docker container and tag it with `docker build . -t sh2-stm`

To log in to the container just run `docker run -v ${PWD}:/app -it --rm sh2-stm` and then `cd /app`.

## Setup .env file

Go check the CICD credentials in keepass (search for DFU - CICD - Upload endpoint), `cp scripts/.env.template scripts/.env` and fill the variables with the right values.

## Design Notes

### Console

The device's UART provides a console function, which you can connect to with
your favourite terminal program. This console provides a basic shell. Type
`help` for full (current) information.

Serial settings: 115200 baud, 8 bits, no parity, 1 stop-bit

The device will emit debug messages on the console but will automatically stop
doing so after 30 seconds.

To re-enable console debug messages, use the `debug 1` command at any time (the
command will disable the 30 second timer if it is still running). Conversely, to
disable them, you can `debug 0`.

### Watchdog

There is a hardware watchdog (a timer circuit that will reset the device on
expiry). There is a low priority `WatchdogTask()` that periodically updates the
watchdog registers to prevent this reset. The intent is to reset the device if a
higher priority task gets "stuck" in some way.

This watchdog timer, once started, cannot be stopped except by rebooting _and_
will continue running and resetting the processor even if the processor is in a
low-power mode.

At boot time, our bootloader detects the reason for a reset and on seeing a
watchdog reset can decide to reinstall the golden firmware. This decision is
controlled by the `ignoreWatchdog` bootflag.  The `WatchdogTask()` sets this
flag after the firmware has been running without a watchdog failure for 10
minutes.

### Power Management

We have a mechanism to reduce power consumption while the device is idle. This
reduction is achieved by cutting power to peripherals and by putting the STM
into a low-power sleep mode.

A wake-up of the processor can be triggered by either an accelerometer event or
by data from the BLE UART (the Nordic chip).

There is a recurring `IdleTimer` that controls entry into low power mode.
Actual entry into low-power mode is controlled by a "stay-awake" flag (if set,
stay awake, else sleep). This flag is set by `stayAwake_Power()`, a function
that UI functions call to indicate that the device should not idle.

#### Power Management and the Watchdog

As described above, in the first ten minutes, after a firmware update, a
watchdog reset will trigger a re-install of golden firmware.

Separately, sleeping the STM requires us to disable the watchdog putting
these requirements in conflict.

To mitigate, for the first ten minutes, the idle power save function will
_only_ cut the power to the peripherals but will _not_ sleep the processor.

After this interval, the watchdog is disabled. As stated above, the watchdog
circuit is independent and requires a reboot. On the next idle timeout, we set a
different bootflag, `disableWatchdog` and reboot the device. The bootloader sees
this flag and does _not_ start the watchdog timer.

After this point, there is no watchdog facility running -- we may need to
develop a different watchdog mechanism in the future but no such effort is
planned at this time.
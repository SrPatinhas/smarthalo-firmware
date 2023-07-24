# Power Notes

## Baseline Measurements

### STM firmware

idle, BLE connection, no LEDs, OLED or sound    39mA
clock animation                                 82mA
show test OLED asset                            60mA
force golden (bootloader re-writing flash)      10mA

ebike demo min/max:     39mA     729mA
speedometer demo max:            319mA
navigation demo max:            1202mA
alarm demo max:                  427mA
assistant demo max:              235mA
front light demo max:            382mA

### Bootloader / Device

during jtag flash:                 6mA
bootloader, nop loop:             13mA
halt:                              0.2mA

### Other

OLED power pins off:              38mA          OLED doesn't display, no further coding needed?
front LED pin off:                38mA          front LED still lights?
TIM 1, 2, 15 (front + bzr)        36mA          front LED stays off and buzzer is silent
LED SDB pin off                   20mA          above plus no halo leds

toggle LED SDB back on, watchdog
toggle EN_VLED_Pin, watchdog
init_HaloLedsDriver() can re-enable HaloLEDs without watchdog

Attempted no console (disable usart1 init) no visible power saving

Halo LEDs are driven by IS31FL3236 chip. It supports a software power save mode, using it gets us to 20mA.
Pulling LED SDB line down supposedly is a hardware power off on the chip and that gets us to 15mA (in conjunction with other things I've been playing with)

We can save an extra 1mA by disabling the timer interrupt clocks for TIM1, TIM2 and TIM15. If I leave them on, I can double tap and toggle the front light and hear the clicks in response to the taps. With them disabled, things are a little weird. Double tap is detected but silent. Better would be to power on when taps detected. More experimentation required.

Turning off Nordic chip appears to lower the draw by another 3mA.

Added sleep/wakeup for photo sensor -- datasheet claim is 5uA in standby / 220uA in active

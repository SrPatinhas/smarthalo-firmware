#!/bin/bash

/opt/SEGGER/JLink_V612i/JLinkGDBServer -if swd -device nRF52832_xxAA -endian little -speed 1000 -port 2331 -swoport 2332 -telnetport 2333 -vd -ir -localhostonly 1 -singlerun -strict -timeout 0 -nogui

#telnet localhost 19021

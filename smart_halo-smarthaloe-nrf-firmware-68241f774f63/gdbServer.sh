#!/bin/bash

usage() {
	echo "Usage: $PROG [<JLink serial number>]"
	echo
	echo "       You may set NRF_SERIAL in your environment OR"
	echo "       you must provide a jlink serial number if you have more"
	echo "       than one attached"
	exit 1
}

if [[ -z $NRF_SERIAL ]]
then
	NUMJLINKS=$(nrfjprog -i | wc -l)
	PROG=$(basename $0)

	if [[ -z $1 && $NUMJLINKS -gt 1 ]]
	then
		echo "$PROG: you have more than one JLink, you must specify one of:"
		echo
		nrfjprog -i
		echo
		usage
	fi

	if [[ -n $1 ]]
	then
		NRF_SERIAL=$1
	fi
fi

SERIAL_OPT="-select USB=$NRF_SERIAL"

JLinkGDBServer $SERIAL_OPT -if swd -device nRF52832_xxAA -endian little -speed 1000 -port 2331 -swoport 2332 -telnetport 2333 -vd -ir -localhostonly 1 -singlerun -strict -timeout 0 -nogui

#telnet localhost 19021

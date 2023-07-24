#!/bin/bash

PROG=$(basename $0)

# Keep circleCI happy
# If we don't have nrfjprog installed, just exit
if [[ -z $(type -t nrfjprog) ]]
then
    exit 0
fi

NUMJLINK=$(nrfjprog -i | wc -l)

if [[ $NUMJLINK -eq 0 ]]
then
    echo
    exit 0
elif [[ $NUMJLINK -eq 1 ]]
then
    echo "-s $(nrfjprog -i)"
elif [[ -z $NRF_SERIAL ]]
then
    {
        echo -e "$PROG: you have more than one segger attached, set NRF_SERIAL to one of these:\n"
        nrfjprog -i
        echo
        echo "Like so:"
        echo "    export NRF_SERIAL=123456789"
    } >&2
    exit 1
else
    echo "-s $NRF_SERIAL"
fi

#!/bin/bash
PREFIX=$1
if [ ${2} == "HEX" ]; then
        SEP=" "
else
        SEP=$2
fi
VERSION=$(git describe --long --match ${PREFIX}* | sed -e "s/^${PREFIX}_//" -e 's/-g.*$//' -e "s/[-\.]/${SEP}/g")
VERSION=${VERSION:-1}
if [ ${3} ]; then
        VERSION=${VERSION/2/0}
fi
if [ ${2} == "HEX" ]; then
        printf "0x"
        for i in $VERSION
        do
            printf "%02X" "$i"
        done
        printf "\n"
else
        echo ${VERSION}
fi
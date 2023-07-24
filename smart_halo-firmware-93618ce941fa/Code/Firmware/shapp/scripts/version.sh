#!/bin/bash
PREFIX=$1
if [ ${2} == "HEX" ]; then
	SEP=" "
else
	SEP=$2
fi
VERSION=$(git describe --long --match ${PREFIX}* | sed -e "s/-/${SEP}/g" -e "s/\./${SEP}/g" | grep -Po "(?<=${PREFIX}_)[^g]+")
VERSION=${VERSION::-1}
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

#!/bin/bash
APP=$1
NAMEVERSION=$2
HEXVERSION=$3

rm -rf _build/dev_${APP}_*.zip
TIMESTAMP="${TIMESTAMP:-`date +"%y%m%d-%H%M%S"`}"
nrfutil pkg generate --application _build/${APP}.hex --hw-version 52 --application-version ${HEXVERSION} --sd-req 0x8C --key-file ./scripts/dev_170808.pem _build/dev_${APP}_${NAMEVERSION}_${TIMESTAMP}.zip

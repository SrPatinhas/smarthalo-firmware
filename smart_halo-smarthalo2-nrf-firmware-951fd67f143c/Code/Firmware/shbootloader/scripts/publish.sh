#!/bin/bash
APP=$1
NAMEVERSION=$2
HEXVERSION=$3
#echo $APP
#echo $NAMEVERSION
#echo $HEXVERSION
rm -rf _build/${APP}_*.zip
TIMESTAMP="${TIMESTAMP:-`date +"%y%m%d-%H%M%S"`}"
nrfutil pkg generate --bootloader _build/${APP}.hex --hw-version 52 --bootloader-version ${HEXVERSION} --sd-req 0x8C --key-file /var/repo/nrfutilData/private_161028.pem _build/${APP}_${NAMEVERSION}_${TIMESTAMP}.zip
if [[ -n $(type -t gdrive) ]]
then
    gdrive upload -p 0B34CPvDx2JkhaEVENW5MYzg2QVE _build/${APP}_${NAMEVERSION}_${TIMESTAMP}.zip
else
    echo Not uploading image to google
fi

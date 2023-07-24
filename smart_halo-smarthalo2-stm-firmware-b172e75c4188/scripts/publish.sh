#!/bin/bash

# Script to upload (publish) firmware to our cloud endpoint
#
# It _requires_ two arguments:
#  - first is a gzip file containing the firmware image
#    (created by SH2.mk)
#  - second is a description

PROG=$(basename $0)

usage(){
    echo "Usage: $PROG <version> <firmware gzip> <firmware description>"
    echo
    echo "        Uploads <firmware gzip> to the cloud, attaching"
    echo "        <version>, <firmware description>, and the elf file."
    echo
    echo "        It is required that the repo be clean (no uncommitted changes)"
    exit 1
}

NAMEVERSION=$1
GZ_FILENAME=$2
DESCRIPTION="$3"
ELF_FILENAME="$(dirname $GZ_FILENAME)/$(basename $GZ_FILENAME .gz).elf"

if [[ $(git describe --dirty) =~ -dirty ]]
then
    echo "$PROG: Cannot publish from a dirty tree"
    usage
fi

if [[ ! -f $GZ_FILENAME ]]
then
    echo "$PROG: cannot open $GZ_FILENAME"
    usage
fi

if [[ -z $DESCRIPTION ]]
then
    echo "$PROG: A description must be provided"
    usage
fi

CREDFILE=scripts/.env

if [[ ! -f $CREDFILE ]]
then
    echo "$PROG: Missing credentials file, $CREDFILE, you will not be allowed to upload"
    exit 1
fi
source scripts/.env

if [[ -z $CONSUMER_TOKEN || -z $USER_EMAIL || -z $PASS ]]
then
    echo "$PROG: credentials file, $CREDFILE, did not set CONSUMER_TOKEN, USER_EMAIL and/or PASS"
    exit 1
fi

############
# API CALLS
############

# LOGIN
SERVICE_URL='https://devapi.smarthalo.bike'
FILENAME=$(basename ${GZ_FILENAME})

ACCESS_JWT=$(curl -s -S -X POST ${SERVICE_URL}/auth/local/login \
    -H 'content-type: application/json' \
    -H "x-consumer-token: ${CONSUMER_TOKEN}" \
    -d "{\"email\": \"${USER_EMAIL}\", \"password\": \"${PASS}\"}" | jq -r .access_jwt)

if [[ -z $ACCESS_JWT || $ACCESS_JWT == "null" ]]
then
    echo "$PROG: access to the cloud denied, no flying for you"
    exit 1
fi

# UPLOAD STM FIRMWARE
FILE_CONTENT=$(openssl enc -A -base64 -in ${GZ_FILENAME})

TMPJSON=/tmp/data.json.$$

cat > $TMPJSON << _EOF_
{
  "data": {
    "file_upload": {
    "base64_content": "${FILE_CONTENT}",
    "name": "${FILENAME}" },
  "description": "${DESCRIPTION}",
  "version": "${NAMEVERSION}",
  "is_default": "1",
  "device_model": "SH2_STM" }
}
_EOF_

UPLOAD_RESPONSE=$(curl -s -S -X POST ${SERVICE_URL}/v2/dfu \
    -H 'content-type: application/json' \
    -H "x-consumer-token: ${CONSUMER_TOKEN}" \
    -H "x-access-token: ${ACCESS_JWT}" \
    -d @$TMPJSON)

echo $UPLOAD_RESPONSE

# UPLOAD STM FIRMWARE ELF FILE
if [[ -f $ELF_FILENAME ]]
then

  FILE_CONTENT=$(openssl enc -A -base64 -in ${ELF_FILENAME})
  FIRMWARE_ID=$(echo ${UPLOAD_RESPONSE} | jq -r .data[0].id)

  if [[ -z $FIRMWARE_ID || $FIRMWARE_ID == "null" ]]
  then
      echo "$PROG: Could not upload the elf file, firmware id is missing"
      exit 1
  fi

cat > $TMPJSON << _EOF_
{
  "data": {
    "elf_file": {
      "base64_content": "${FILE_CONTENT}"
    }
  }
}
_EOF_

  curl -s -S -X POST ${SERVICE_URL}/v2/dfu/${FIRMWARE_ID}/log_files \
      -H 'content-type: application/json' \
      -H "x-consumer-token: ${CONSUMER_TOKEN}" \
      -H "x-access-token: ${ACCESS_JWT}" \
      -d @$TMPJSON
fi

rm -f $TMPJSON

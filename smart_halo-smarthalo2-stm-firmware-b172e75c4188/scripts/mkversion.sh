#!/bin/bash

rm -f "$1"
cat << _EOF_ > "$1"
/*
 * THIS FILE IS AUTO-GENERATED.
 * DO NOT EDIT.
 */

#include <stdint.h>

uint8_t version[4] = { $2 };
_EOF_

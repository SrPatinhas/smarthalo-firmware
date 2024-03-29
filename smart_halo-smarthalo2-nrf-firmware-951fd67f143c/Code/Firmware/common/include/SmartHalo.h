/*
 * SmartHalo.h
 *
 *  Created on: Jun 1, 2016
 *      Author: sgelinas
 */

#ifndef SRC_SMARTHALO_H_
#define SRC_SMARTHALO_H_

//hardware device type check 
#define SMARTHALO_DEV_BOARD 0x00
#define SMARTHALO_V1 		0x01 

//device revision check
#define EE_BOARD 			0x01
#define FF_BOARD			0x02


#ifdef SMARTHALO_EE
#include "SmartHaloEE.h"
#define BUILDTARGET "SmartHaloEE"
#define DEVICE_TYPE SMARTHALO_V1
#define DEVICE_REVISION_MAJOR EE_BOARD
#define DEVICE_REVISION_MINOR 0x00
#define DEVICE_REVISION ((DEVICE_REVISION_MAJOR << 8) | DEVICE_REVISION_MINOR) 

#elif defined(SMARTHALO_FF)
#define BUILDTARGET "SmartHaloFF"
#include "SmartHaloFF.h"
#define DEVICE_TYPE SMARTHALO_V1
#define DEVICE_REVISION_MAJOR FF_BOARD
#define DEVICE_REVISION_MINOR 0x00
#define DEVICE_REVISION ((DEVICE_REVISION_MAJOR << 8) | DEVICE_REVISION_MINOR) 

#elif defined(BOARD_PCA10040) && defined(TESTFIRMWARE)
#define BUILDTARGET "PCA10040"
#include "SH_PCA10040.h"
#define DEVICE_TYPE SMARTHALO_DEV_BOARD
#define DEVICE_REVISION_MAJOR 0x00 
#define DEVICE_REVISION_MINOR 0x00
#define DEVICE_REVISION ((DEVICE_REVISION_MAJOR << 8) | DEVICE_REVISION_MINOR) 

#elif defined(BOARD_PCA10040)
#define BUILDTARGET "PCA10040"
#include "SH_pinMap_SmartHaloPCA10040.h"
#define DEVICE_TYPE SMARTHALO_DEV_BOARD
#define DEVICE_REVISION_MAJOR 0x00
#define DEVICE_REVISION_MINOR 0x01
#define DEVICE_REVISION ((DEVICE_REVISION_MAJOR << 8) | DEVICE_REVISION_MINOR) 

#else
#error("You need to specify a target board")
#endif


typedef enum
{
	LITTLEENDIAN,
	BIGENDIAN
} Endianness;

#endif /* SRC_SMARTHALO_H_ */

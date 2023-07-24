/*
 * SH_pinMap.h
 *
 *  Created on: 2016-03-08
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_SH_PINMAP_H_
#define SH_FIRMWARE_CODE_SH_PINMAP_H_

#ifdef SMARTHALO_EE
#include "SmartHaloEE.h"
#define BUILDTARGET "SmartHaloEE"
#elif defined(SMARTHALO_FF)
#define BUILDTARGET "SmartHaloFF"
#include "SmartHaloFF.h"
#elif defined(BOARD_PCA10040_TEST_FIRMWARE)
#define BUILDTARGET "PCA10040_TEST_FIRMWARE"
#include "SH_PCA10040.h"
#elif defined(BOARD_PCA10040)
#define BUILDTARGET "PCA10040"
#include "SH_pinMap_SmartHaloPCA10040.h"
//#include "PCA10040.h"
#else
#error("You need to specify a target board")
#endif

//
//#ifdef BOARD_PCA10040_HALOBOX
//
//#define VUSB_SENSE_PIN		3
//#define VBAT_SENSE_PIN		4
//#define IBAT_SENSE_PIN		5
//#define BUZZER_PIN			6
//#define FRONTLED_PIN		7
//#define WAKEUP_PIN			8
//#define NFC1_PIN			9
//#define NFC2_PIN			10
//#define CENTRAL_R_PIN		11
//#define CENTRAL_G_PIN		29
//#define CENTRAL_B_PIN		12
//#define VBZ_EN				14
//#define PS_5V				15
//#define SCL_PIN				16
//#define SCA_PIN				17
//#define SCB_PIN				18
//#define INT_MAG_PIN			19
//#define INT_1_XL_PIN		20
////@@@
////#define INT_2_XL_PIN		20 //this is the real pin number on schematic
//#define INT_2_XL_PIN		22
//
//#define SLEEP_PIN			22
//#define QTOUCH_PIN			23
//#define SYNC_MODE_PIN		24
//#define CHG_PIN				25
//#define PG_PIN				26
//#define ISBT2				27
//#define RTS_PIN				28
//#define CTS_PIN				29
//#define TXD_PIN				30
//#define RXD_PIN				31
//
//
//
//#elif defined(BOARD_PCA10040)
//
//#elif defined(SMARTHALO_EE)
//
//#endif




#endif /* SH_FIRMWARE_CODE_SH_PINMAP_H_ */




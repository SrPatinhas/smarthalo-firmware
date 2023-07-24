/* Copyright (c) 2016 SmartHalo. All Rights Reserved.
 *
 * @brief	Set of functions to control the BOOST output with EasyScale protocol
 *
 *
 *
 * @author Sebastien Gelinas
 * @date 2016/10/26
 *
 */

#ifndef EASYSCALE_H_
#define EASYSCALE_H_

#include "nrf.h"

#define ES_R1					510
#define ES_R2					33
#define ES_DEVICE_ADDRESS		0x72
#define ES_MAX_DATA_RATE		160 000 // Bit/s
#define ES_MIN_DATA_RATE		1700	// Bit/s

typedef enum
{
  FV0000 = 0,
  FV0031 = 1,
  FV0049 = 2,
  FV0068 = 3,
  FV0086 = 4,
  FV0104 = 5,
  FV0123 = 6,
  FV0141 = 7,
  FV0160 = 8,
  FV0178 = 9,
  FV0197 = 10,
  FV0215 = 11,
  FV0234 = 12,
  FV0270 = 13,
  FV0307 = 14,
  FV0344 = 15,
  FV0381 = 16,
  FV0418 = 17,
  FV0455 = 18,
  FV0492 = 19,
  FV0528 = 20,
  FV0565 = 21,
  FV0602 = 22,
  FV0639 = 23,
  FV0713 = 24,
  FV0787 = 25,
  FV0860 = 26,
  FV0934 = 27,
  FV1008 = 28,
  FV1082 = 29,
  FV1155 = 30,
  FV1229 = 31
} feedbackVoltagemV;

typedef struct {
	unsigned int RFA : 1;
	unsigned int addressbits : 2;
	feedbackVoltagemV newValue : 5;		// 0 to 31
} ES_DataByte_t;

void EasyScale_setup(uint8_t enablePin);

// Must send Device address then databyte, MSB first.
// Data rate must be selected between ES_MIN_DATA_RATE AND ES_MAX_DATA_RATE
void EasyScale_setFeedbackVoltage(feedbackVoltagemV newValue);

feedbackVoltagemV EasyScale_getFeedbackVoltage();

float EasyScale_getOutputVoltage();


#endif /* EASYSCALE_H_ */

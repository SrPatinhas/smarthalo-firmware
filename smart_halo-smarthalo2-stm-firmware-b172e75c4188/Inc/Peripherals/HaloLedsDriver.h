/*!@file    RGB_HAL.h

 */
// ------------------------------------------------------------------------------------------------


#ifndef HALOLEDSDRIVER_H_
#define HALOLEDSDRIVER_H_

#include "main.h"

// ================================================================================================
// ================================================================================================
//            DEFINE DECLARATION
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
//            ENUM DECLARATION
// ================================================================================================
// ================================================================================================

typedef enum {
  IMAX = 0x01,
  IMAX_DIV_2 = 0x03,
  IMAX_DIV_3 = 0x05,
  IMAX_DIV_4 = 0x07
} haloLedDriverCurrent_t ; 

// ================================================================================================
// ================================================================================================
//            STRUCTURE DECLARATION
// ================================================================================================
// ================================================================================================



// ================================================================================================
// ================================================================================================
//            EXTERNAL FUNCTION DECLARATION
// ================================================================================================
// ================================================================================================
bool init_HaloLedsDriver(void);
bool update_HaloLedsDriver(uint8_t *ledData);
bool sleep_HaloLedsDriver(void);
bool setCurrent_HaloLedsDriver(haloLedDriverCurrent_t currentSetting);

#endif  /* HALOLEDSDRIVER_H_ */

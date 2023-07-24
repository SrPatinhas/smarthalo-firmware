/**
  ******************************************************************************
  * File Name          : gfxmmu_lut.h
  * Description        : header file for GFX MMU Configuration Table
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gfxmmu_lut_H
#define __gfxmmu_lut_H
#ifdef __cplusplus
 extern "C" {
#endif
// GFX MMU Configuration Table

  #define GFXMMU_FB_SIZE 0
  #define GFXMMU_LUT_FIRST 0
  #define GFXMMU_LUT_LAST  0
  #define GFXMMU_LUT_SIZE 1

uint32_t gfxmmu_lut_config[2*GFXMMU_LUT_SIZE] = {
  0x0
};

#ifdef __cplusplus
}
#endif
#endif /*__ gfxmmu_lut_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

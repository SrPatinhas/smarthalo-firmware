/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_it.h"
#include "tsl_time.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "BootLoaderImport.h"
#include "PiezoDriver.h"
#include "task.h"
#include "wwdg.h"
#include "rtc.h"
#include "rtcUtils.h"
#include "SHTaskUtils.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint32_t wwdgReturnAddress;
volatile extern uint32_t __update_crc__;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_i2c2_tx;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern QSPI_HandleTypeDef hqspi;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim6;
extern TSC_HandleTypeDef htsc;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern WWDG_HandleTypeDef hwwdg;
extern TIM_HandleTypeDef htim16;

/* USER CODE BEGIN EV */

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
void HardFault_Handler( void ) __attribute__( ( naked ) );

void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress,
                              uint32_t pcarg, bool wwdg)
{
    /* These are volatile to try and prevent the compiler/linker optimising them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr; /* Link register. */
    volatile uint32_t pc; /* Program counter. */
    volatile uint32_t psr;/* Program status register. */
    volatile uint32_t bfar;
    volatile uint32_t cfsr;
    volatile uint32_t hfsr;
    volatile uint32_t dfsr;
    volatile uint32_t afsr;
    volatile uint32_t scb_shcsr;

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    bfar = (*((volatile unsigned long *)(0xE000ED38)));
    cfsr = (*((volatile unsigned long *)(0xE000ED28)));
    hfsr = (*((volatile unsigned long *)(0xE000ED2C)));
    dfsr = (*((volatile unsigned long *)(0xE000ED30)));
    afsr = (*((volatile unsigned long *)(0xE000ED3C)));
    scb_shcsr = SCB->SHCSR;

    HAL_WWDG_Refresh(&hwwdg);

    // Store %lr -- it will be picked up after reboot
    // (bit of hackery, the bootloader will not accost
    // __update_crc__ if __update_len__ is zero)
    __update_crc__ = lr;

    // Try to write all the juicy register info into
    // RTC backup registers for pickup on the reboot
    HAL_WWDG_Refresh(&hwwdg);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_R0, r0);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_R1, r1);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_R2, r2);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_R3, r3);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_R12, r12);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_LR, lr);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_PC, pc);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_PSR, psr);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_BFAR, bfar);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_CFSR, cfsr);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_HFSR, hfsr);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DFSR, dfsr);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_AFSR, afsr);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_SCB_SHCSR, scb_shcsr);

    if (!wwdg) {
      NVIC_SystemReset();
    } else {
      for (;;) asm("nop");
    }
}

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  BF->crash = 1;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FAULT, eFaultHard);

  printf("hard fault\n");

  __asm volatile
  (
    // these first 4 instructions are idiomatic ARM to figure
    // out if we're in an interrupt/exception handler
    // and load %r0 with the appropriate stack pointer (there
    // are two, M and P, interrupts use the M)
    // I am _assuming_ that we want to know about the freertos
    // task that was running (I'm unconditionally using the P)
    // The rest just sets up that stack frame as an argument
    // to the call to prvGetRegistersFromStack()
    // which will handle printing.
    //
    // The reason I'm choosing to ignore the M stack is because
    // _most_ of the time it's simply the freertos scheduler and
    // not very useful for debugging (freertos schedules tasks
    // from its tick interrupt)
    //
    // " tst lr, #4                                            \n"
    // " ite eq                                                \n"
    // " mrseq r0, msp                                         \n"
    // " mrsne r0, psp                                         \n"
    " mrs r0, psp\n"        // pulFaultStackAddress
    " ldr r1, [r0, #24]\n"  // pc
    " mov r2, #0\n"         // wwdg = false
    " ldr r3, handler2_address_const\n"
    " bx r3\n"
    " handler2_address_const: .word prvGetRegistersFromStack\n"
  );

  // the above calls prvGetRegistersFromStack() and does not return

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    printf("hell froze over, trying again to reboot!\n");
		NVIC_SystemReset();
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  BF->crash = 1;
  __update_crc__ = eFaultMemManage;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FAULT, eFaultMemManage);
  printf("MemManage fault\n");

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
		NVIC_SystemReset();
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  BF->crash = 1;
  __update_crc__ = eFaultBus;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FAULT, eFaultBus);
  printf("Bus fault\n");

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
		NVIC_SystemReset();
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  BF->crash = 1;
  __update_crc__ = eFaultUsage;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FAULT, eFaultUsage);
  printf("Usage fault\n");

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
		NVIC_SystemReset();
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles Window watchdog interrupt.
  */
void WWDG_IRQHandler(void)
{
  /* USER CODE BEGIN WWDG_IRQn 0 */
  register int *r0 __asm("r0");
  extern volatile uint32_t wwdgCount;

  // Conditionally ignore the WWDG interrupt:
  //
  // If we are in a hard hang, skipping 125 WWDG interrupts
  // consumes approximately 5 seconds (experimentally derived)
  // That provides enough time for the software watchdog to
  // flag stuck tasks -- since it cycles every 2 seconds.
  if (wwdgCount++ < 125) {
    /* Check if Early Wakeup Interrupt is enable */
    if (__HAL_WWDG_GET_IT_SOURCE(&hwwdg, WWDG_IT_EWI) != RESET)
    {
      /* Check if WWDG Early Wakeup Interrupt occurred */
      if (__HAL_WWDG_GET_FLAG(&hwwdg, WWDG_FLAG_EWIF) != RESET)
      {
        /* Clear the WWDG Early Wakeup flag */
        __HAL_WWDG_CLEAR_FLAG(&hwwdg, WWDG_FLAG_EWIF);
      }
    }
    HAL_WWDG_Refresh(&hwwdg);
    return;
  }

  saveLastTasks_SHTaskUtils();

  // If we get here, we are really going to reboot -- leave
  // some forensics
  __asm(
    "TST lr, #4\n"
    "ITE EQ\n"
    "MRSEQ r0, MSP\n"
    "MRSNE r0, PSP\n" // stack pointer now in r0
    "add r0, r0, #0x14\n" // Use this line for LR -- lr now in r0
    // "add r0, r0, #0x18\n" // Use this line for PC -- pc now in r0
  );

  // stuff the extracted address into wwdgReturnAddress, it may get
  // picked up later
  wwdgReturnAddress = *r0;
  /* USER CODE END WWDG_IRQn 0 */
  HAL_WWDG_IRQHandler(&hwwdg);
  /* USER CODE BEGIN WWDG_IRQn 1 */

  /* USER CODE END WWDG_IRQn 1 */
}

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_IRQn 0 */

  /* USER CODE END EXTI2_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_IRQn 1 */

  /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line3 interrupt.
  */
void EXTI3_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI3_IRQn 0 */

  /* USER CODE END EXTI3_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
  /* USER CODE BEGIN EXTI3_IRQn 1 */

  /* USER CODE END EXTI3_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel4 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */

  /* USER CODE END DMA1_Channel4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_i2c2_tx);
  /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

  /* USER CODE END DMA1_Channel4_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel5 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel5_IRQn 0 */

  /* USER CODE END DMA1_Channel5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi2_tx);
  /* USER CODE BEGIN DMA1_Channel5_IRQn 1 */

  /* USER CODE END DMA1_Channel5_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel6 global interrupt.
  */
void DMA1_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel6_IRQn 0 */

  /* USER CODE END DMA1_Channel6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Channel6_IRQn 1 */

  /* USER CODE END DMA1_Channel6_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel7 global interrupt.
  */
void DMA1_Channel7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel7_IRQn 0 */

  /* USER CODE END DMA1_Channel7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
  /* USER CODE BEGIN DMA1_Channel7_IRQn 1 */

  /* USER CODE END DMA1_Channel7_IRQn 1 */
}

/**
  * @brief This function handles ADC1 global interrupt.
  */
void ADC1_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_IRQn 0 */

  /* USER CODE END ADC1_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC1_IRQn 1 */

  /* USER CODE END ADC1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt and TIM16 global interrupt.
  */
void TIM1_UP_TIM16_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM16_IRQn 0 */

  /* USER CODE END TIM1_UP_TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim16);
  /* USER CODE BEGIN TIM1_UP_TIM16_IRQn 1 */

  /* USER CODE END TIM1_UP_TIM16_IRQn 1 */
}

/**
  * @brief This function handles I2C1 event interrupt.
  */
void I2C1_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */

  /* USER CODE END I2C1_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c1);
  /* USER CODE BEGIN I2C1_EV_IRQn 1 */

  /* USER CODE END I2C1_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C1 error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */

  /* USER CODE END I2C1_ER_IRQn 0 */
  HAL_I2C_ER_IRQHandler(&hi2c1);
  /* USER CODE BEGIN I2C1_ER_IRQn 1 */

  /* USER CODE END I2C1_ER_IRQn 1 */
}

/**
  * @brief This function handles I2C2 event interrupt.
  */
void I2C2_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_EV_IRQn 0 */

  /* USER CODE END I2C2_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c2);
  /* USER CODE BEGIN I2C2_EV_IRQn 1 */

  /* USER CODE END I2C2_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C2 error interrupt.
  */
void I2C2_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_ER_IRQn 0 */

  /* USER CODE END I2C2_ER_IRQn 0 */
  HAL_I2C_ER_IRQHandler(&hi2c2);
  /* USER CODE BEGIN I2C2_ER_IRQn 1 */

  /* USER CODE END I2C2_ER_IRQn 1 */
}

/**
  * @brief This function handles SPI2 global interrupt.
  */
void SPI2_IRQHandler(void)
{
  /* USER CODE BEGIN SPI2_IRQn 0 */

  /* USER CODE END SPI2_IRQn 0 */
  HAL_SPI_IRQHandler(&hspi2);
  /* USER CODE BEGIN SPI2_IRQn 1 */

  /* USER CODE END SPI2_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC channel1 underrun error interrupt.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
  
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel6 global interrupt.
  */
void DMA2_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel6_IRQn 0 */

  /* USER CODE END DMA2_Channel6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
  /* USER CODE BEGIN DMA2_Channel6_IRQn 1 */

  /* USER CODE END DMA2_Channel6_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel7 global interrupt.
  */
void DMA2_Channel7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel7_IRQn 0 */

  /* USER CODE END DMA2_Channel7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA2_Channel7_IRQn 1 */

  /* USER CODE END DMA2_Channel7_IRQn 1 */
}

/**
  * @brief This function handles QUADSPI global interrupt.
  */
void QUADSPI_IRQHandler(void)
{
  /* USER CODE BEGIN QUADSPI_IRQn 0 */

  /* USER CODE END QUADSPI_IRQn 0 */
  HAL_QSPI_IRQHandler(&hqspi);
  /* USER CODE BEGIN QUADSPI_IRQn 1 */

  /* USER CODE END QUADSPI_IRQn 1 */
}

/**
  * @brief This function handles Touch sense controller interrupt.
  */
void TSC_IRQHandler(void)
{
  /* USER CODE BEGIN TSC_IRQn 0 */

  /* USER CODE END TSC_IRQn 0 */
  HAL_TSC_IRQHandler(&htsc);
  /* USER CODE BEGIN TSC_IRQn 1 */

  /* USER CODE END TSC_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

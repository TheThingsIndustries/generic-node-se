/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_LowPower/Src/stm32wlxx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "GNSE_bsp.h"
#include "stm32wlxx_it.h"

extern TIM_HandleTypeDef htim17;

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/* STM32WLxx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32wlxx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM17 Global Interrupt.
  */
void TIM17_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim17);
}

void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&GNSE_BSP_debug_usart);
}

void DMA1_Channel5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&GNSE_BSP_hdma_tx);
}

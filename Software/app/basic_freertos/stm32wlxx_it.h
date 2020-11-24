/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_LowPower/Inc/stm32wlxx_it.h
  * @author  MCD Application Team
  * @brief   This file contains the headers of the interrupt handlers.
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

#ifndef __STM32WLxx_IT_H
#define __STM32WLxx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

void HardFault_Handler(void);
void BusFault_Handler(void);
void DebugMon_Handler(void);
void TIM17_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32WLxx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

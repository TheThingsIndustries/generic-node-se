/**
  ******************************************************************************
  * @file    sys_app.h
  * @author  MCD Application Team
  * @brief   Function prototypes for sys_app.c file
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

#ifndef __SYS_APP_H__
#define __SYS_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stm32_adv_trace.h"

/**
  * @brief initialises the system (dbg pins, trace, systiemr, LPM, ...)
  * @param none
  * @return  none
  */
void SystemApp_Init(void);

/**
  * @brief  callback to get the battery level in % of full charge (254 full charge, 0 no charge)
  * @param  none
  * @return battery level
  */
uint8_t GetBatteryLevel(void);

/**
  * @brief  callback to get the current temperature in the MCU
  * @param  none
  * @return temperature level
  */
uint16_t GetTemperatureLevel(void);

/**
  * @brief  callback to get a pseudo random seed generated using the MCU Unique ID
  * @param  none
  * @return seed Generated pseudo random seed
  */
uint32_t GetRandomSeed(void);

/**
  * @brief  callback to get the board 64 bits unique ID
  * @param  unique ID
  * @return none
  */
void GetUniqueId(uint8_t *id);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_APP_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

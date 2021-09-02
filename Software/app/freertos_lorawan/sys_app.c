/**
  ******************************************************************************
  * @file    sys_app.c
  * @author  MCD Application Team
  * @brief   Initializes HW and SW system entities (not related to the radio)
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

#include <stdio.h>
#include "app.h"
#include "sys_app.h"
#include "freertos_systime.h"
#include "GNSE_lpm.h"

#define MAX_TS_SIZE (int)16

/**
  * @brief initialises the system (dbg pins, trace, mbmux, systimer, LPM, ...)
  * @param none
  * @return  none
  * TODO: Improve with system wide Init(), see https://github.com/TheThingsIndustries/generic-node-se/issues/57
  */
void SystemApp_Init(void)
{
  /* Ensure that MSI is wake-up system clock */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

  /* Initialize the Low Power Manager and Debugger */
#if defined(DEBUGGER_ON) && (DEBUGGER_ON == 1)
  GNSE_LPM_Init(GNSE_LPM_SLEEP_STOP_DEBUG_MODE);
#elif defined(DEBUGGER_ON) && (DEBUGGER_ON == 0)
  GNSE_LPM_Init(GNSE_LPM_SLEEP_STOP_MODE);
#endif
  /* Initialize Tracer/Logger */
  GNSE_TRACER_INIT();
  GNSE_app_printAppInfo();

  /* Here user can init the board peripherals and sensors */
}

/**
  * @brief This function provides delay (in ms)
  * @param Delay: specifies the delay time length, in milliseconds.
  * @return None
  */
void HAL_Delay(__IO uint32_t Delay)
{
  vTaskDelay( pdMS_TO_TICKS( Delay )); /* FreeRTOS delay */
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

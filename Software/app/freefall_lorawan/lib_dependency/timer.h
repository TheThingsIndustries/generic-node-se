/**
  ******************************************************************************
  * @file    timer.h
  * @author  MCD Application Team
  * @brief   Wrapper to timer server
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

#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_timer.h"

#define TIMER_MAX_VAL 0xFFFFFFFFU

#define TimerTime_t UTIL_TIMER_Time_t

#define TimerEvent_t UTIL_TIMER_Object_t

#define TimerInit(HANDLE, CB) do {\
                                   UTIL_TIMER_Create( HANDLE, TIMER_MAX_VAL, UTIL_TIMER_ONESHOT, CB, NULL);\
                                 } while(0)

#define TimerSetValue(HANDLE, TIMEOUT) do{ \
                                           UTIL_TIMER_SetPeriod(HANDLE, TIMEOUT);\
                                         } while(0)

#define TimerStart(HANDLE)   do {\
                                  UTIL_TIMER_Start(HANDLE);\
                                } while(0)

#define TimerStop(HANDLE)   do {\
                                 UTIL_TIMER_Stop(HANDLE);\
                               } while(0)

#define TimerGetCurrentTime  UTIL_TIMER_GetCurrentTime

#define TimerGetElapsedTime UTIL_TIMER_GetElapsedTime

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

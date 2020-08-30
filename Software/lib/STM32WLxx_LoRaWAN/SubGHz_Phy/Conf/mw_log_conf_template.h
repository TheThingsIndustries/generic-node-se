/**
  ******************************************************************************
  * @file    mw_log_conf_template.h
  * @author  MCD Application Team
  * @brief   Configure (enable/disable) traces for CM0
  *******************************************************************************
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MW_LOG_CONF_H__
#define __MW_LOG_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_adv_trace.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MW_LOG_ENABLED
/* Exported macros -----------------------------------------------------------*/

#ifdef MW_LOG_ENABLED
#define MW_LOG(TS,VL, ...)   do{ {UTIL_ADV_TRACE_COND_FSend(VL, T_REG_OFF, TS, __VA_ARGS__);} }while(0)
#else  /* MW_LOG_ENABLED */
#define VLEVEL_ABOVE_H   4    /* this level will be always filtered because too High */
#define MW_LOG(TS,VL, ...)    do{ {UTIL_ADV_TRACE_COND_FSend(VLEVEL_ABOVE_H, T_REG_OFF, TS, __VA_ARGS__);} }while(0)
#endif

/* Defines -------------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */



#ifdef __cplusplus
}
#endif

#endif /*__MW_LOG_CONF_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

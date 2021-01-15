/**
  ******************************************************************************
  * @file    radio_conf.h
  * @author  MCD Application Team
  * @brief   Header of Radio configuration
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
#ifndef __RADIO_CONF_H__
#define __RADIO_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_mem.h"
#include "mw_log_conf.h"     /* mw trace conf */
#include "GNSE_bsp.h"
#include "app_conf.h"


#define DBG_GPIO_RADIO_TX(set_rst) //DBG_GPIO_##set_rst##_LINE(DGB_LINE2_PORT, DGB_LINE2_PIN);
#define DBG_GPIO_RADIO_RX(set_rst) //DBG_GPIO_##set_rst##_LINE(DGB_LINE1_PORT, DGB_LINE1_PIN);

#define RADIO_RX_BUF_SIZE          255

#define RADIO_BAUDRATEPRESCALER    SUBGHZSPI_BAUDRATEPRESCALER_4

/* Exported macros -----------------------------------------------------------*/
#ifndef CRITICAL_SECTION_BEGIN
#define CRITICAL_SECTION_BEGIN( )      UTILS_ENTER_CRITICAL_SECTION( )
#endif /* !CRITICAL_SECTION_BEGIN */
#ifndef CRITICAL_SECTION_END
#define CRITICAL_SECTION_END( )        UTILS_EXIT_CRITICAL_SECTION( )
#endif /* !CRITICAL_SECTION_END */

/* Function mapping */
#define RADIO_DELAY_MS                          HAL_Delay
#define RADIO_MEMSET8( dest, value, size )      UTIL_MEM_set_8( dest, value, size )

#ifdef __cplusplus
}
#endif

#endif /* __RADIO_CONF_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

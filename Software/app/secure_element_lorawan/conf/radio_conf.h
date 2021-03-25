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
#include "GNSE_tracer.h"
#include "GNSE_bsp.h"
#include "app_conf.h"

/**
  * @brief Set RX pin to high or low level
  */
#define DBG_GPIO_RADIO_RX(set_rst) //DBG_GPIO_##set_rst##_LINE(DGB_LINE1_PORT, DGB_LINE1_PIN);

/**
  * @brief Set TX pin to high or low level
  */
#define DBG_GPIO_RADIO_TX(set_rst) //DBG_GPIO_##set_rst##_LINE(DGB_LINE2_PORT, DGB_LINE2_PIN);

/**
  * @brief Max payload buffer size
  */
#define RADIO_RX_BUF_SIZE          255

/**
  * @brief drive value used anytime radio is NOT in TX low power mode
  */
#define SMPS_DRIVE_SETTING_DEFAULT  SMPS_DRV_40

/**
  * @brief drive value used anytime radio is in TX low power mode
  *        TX low power mode is the worst case because the PA sinks from SMPS
  *        while in high power mode, current is sunk directly from the battery
  */
#define SMPS_DRIVE_SETTING_MAX      SMPS_DRV_60

/**
  * @brief in XO mode, set internal capacitor (from 0x00 to 0x2F starting 11.2pF with 0.47pF steps)
  */
#define XTAL_DEFAULT_CAP_VALUE      0x20

/**
  * @brief Frequency error (in Hz) can be compensated here.
  *        warning XO frequency error generates (de)modulator sampling time error which can not be compensated
  */
#define RF_FREQUENCY_ERROR          ((int32_t) 0)

/**
  * @brief voltage of vdd tcxo.
  */
#define TCXO_CTRL_VOLTAGE           TCXO_CTRL_1_7V

#ifndef CRITICAL_SECTION_BEGIN
/**
  * @brief macro used to enter the critical section
  */
#define CRITICAL_SECTION_BEGIN( )      UTILS_ENTER_CRITICAL_SECTION( )
#endif /* !CRITICAL_SECTION_BEGIN */
#ifndef CRITICAL_SECTION_END
/**
  * @brief macro used to exit the critical section
  */
#define CRITICAL_SECTION_END( )        UTILS_EXIT_CRITICAL_SECTION( )
#endif /* !CRITICAL_SECTION_END */

/* Function mapping */
/**
  * @brief SUBGHZ interface init to radio Middleware
  */
#define RADIO_INIT                              GNSE_BSP_SUBGHZ_Init

/**
  * @brief Delay interface to radio Middleware
  */
#define RADIO_DELAY_MS                          HAL_Delay

/**
  * @brief Memset utilities interface to radio Middleware
  */
#define RADIO_MEMSET8( dest, value, size )      UTIL_MEM_set_8( dest, value, size )

#ifdef __cplusplus
}
#endif

#endif /* __RADIO_CONF_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
 ******************************************************************************
 * @file    stm32_adv_tracer_conf.h
 * @author  MCD Application Team
 * @brief   Header for stm32_adv_trace.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_ADV_TRACER_CONF_H__
#define __STM32_ADV_TRACER_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_mem.h"
#include "stm32_tiny_vsnprintf.h"

/******************************************************************************
 * trace\advanced
 * the define option
 *    ADV_TRACER_CONDITIONNAL shall be defined if you want use conditional function
 *    ADV_TRACER_UNCHUNK_MODE shall be defined if you want use the unchunk mode
 *
 ******************************************************************************/
#define ADV_TRACER_SUPPORT_FLOAT /** Comment this to get smaller code size and sacrifice float printing like %f, %4.2f **/
// #define ADV_TRACER_SUPPORT_TINY_PRINTF /** Uncomment to get smaller printf code size **/
#define ADV_TRACER_CONDITIONNAL                                                      /*!< not used */
#define ADV_TRACER_UNCHUNK_MODE                                                      /*!< not used */
#define ADV_TRACER_DEBUG(...)                                                        /*!< not used */
#define ADV_TRACER_INIT_CRITICAL_SECTION( )    UTILS_INIT_CRITICAL_SECTION()         /*!< init the critical section in trace feature */
#define ADV_TRACER_ENTER_CRITICAL_SECTION( )   UTILS_ENTER_CRITICAL_SECTION()        /*!< enter the critical section in trace feature */
#define ADV_TRACER_EXIT_CRITICAL_SECTION( )    UTILS_EXIT_CRITICAL_SECTION()         /*!< exit the critical section in trace feature */
#define ADV_TRACER_TMP_BUF_SIZE                (256U)                                /*!< default trace buffer size */
#define ADV_TRACER_TMP_MAX_TIMESTMAP_SIZE      (15U)                                 /*!< default trace timestamp size */
#define ADV_TRACER_FIFO_SIZE                   (512U)                                /*!< default trace fifo size */
#define ADV_TRACER_MEMSET8( dest, value, size) UTIL_MEM_set_8((dest),(value),(size)) /*!< memset utilities interface to trace feature */
#define ADV_TRACER_VSNPRINTF(...)              tiny_vsnprintf_like(__VA_ARGS__)      /*!< vsnprintf utilities interface to trace feature */

#define ADV_TRACER_VLEVEL_OFF    0  /*!< used to set ADV_TRACER_SetVerboseLevel() (not as message param) */
#define ADV_TRACER_VLEVEL_ALWAYS 0  /*!< used as message params, if this level is given trace will be printed even when ADV_TRACER_SetVerboseLevel(OFF) */
#define ADV_TRACER_VLEVEL_L 1       /*!< just essential traces */
#define ADV_TRACER_VLEVEL_M 2       /*!< functional traces */
#define ADV_TRACER_VLEVEL_H 3       /*!< all traces */

#define ADV_TRACER_TS_OFF 0         /*!< Log without TimeStamp */
#define ADV_TRACER_TS_ON 1          /*!< Log with TimeStamp */

#define ADV_TRACER_T_REG_OFF  0     /*!< Log without bitmask */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_ADV_TRACER_CONF_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

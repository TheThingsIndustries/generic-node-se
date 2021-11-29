/**
 ******************************************************************************
 * @file    stm32_adv_tracer.h
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
#ifndef __STM32_ADV_TRACER_H__
#define __STM32_ADV_TRACER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stm32_adv_tracer_conf.h"

/** @defgroup ADV_TRACE advanced tracer
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup ADV_TRACER_exported_TypeDef ADV_TRACE exported Typedef
 *  @{
 */

/**
 *  @brief prototype of the time stamp function.
 */
typedef void cb_timestamp(uint8_t *pData, uint16_t *Size);

/**
 *  @brief prototype of the overrun function.
 */
typedef void cb_overrun(uint8_t **pData, uint16_t *size);
/**
 *  @brief  List the Advanced trace function status.
 *  list of the returned status value, any negative value is corresponding to an error.
 */
typedef enum{
  ADV_TRACER_OK              =  0,     /*!< Operation terminated successfully.*/
  ADV_TRACER_INVALID_PARAM   = -1,     /*!< Invalid Parameter.                */
  ADV_TRACER_HW_ERROR        = -2,     /*!< Hardware Error.                   */
  ADV_TRACER_MEM_FULL        = -3,     /*!< Memory fifo full.                 */
  ADV_TRACER_UNKNOWN_ERROR   = -4,     /*!< Unknown Error.                    */
#if defined(ADV_TRACER_CONDITIONNAL)
  ADV_TRACER_GIVEUP          = -5,     /*!< trace give up                     */
  ADV_TRACER_REGIONMASKED    = -6      /*!< trace region masked               */
#endif
} ADV_TRACER_Status_t;

/**
 * @brief Advanced trace driver definition
 */
typedef struct {
  ADV_TRACER_Status_t  (* Init)(void (*cb)(void *ptr));                                       /*!< Media initialization.      */
  ADV_TRACER_Status_t  (* DeInit)(void);                                                      /*!< Media Un-initialization.   */
  ADV_TRACER_Status_t  (* StartRx)(void (*cb)(uint8_t *pdata, uint16_t size, uint8_t error)); /*!< Media to start RX process. */
  ADV_TRACER_Status_t  (* Send)(uint8_t *pdata, uint16_t size);                               /*!< Media to send data.        */
}ADV_TRACER_Driver_s;

/**
 *  @}
 */

/* External variables --------------------------------------------------------*/
/** @defgroup ADV_TRACER_exported_variables ADV_TRACE exported variables
 *
 *  @{
 */
/**
 *  @brief This structure is the linked with the IF layer implementation.
 */
extern const ADV_TRACER_Driver_s UTIL_TraceDriver;

/**
 *  @}
 */

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/** @defgroup ADV_TRACER_exported_function ADV_TRACE exported function
 *  @{
 */

/**
 * @brief TraceInit Initializes Logging feature
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_Init(void);

/**
 * @brief TraceDeInit module DeInitializes.
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_DeInit(void);

/**
 * @brief start the RX process.
 * @param UserCallback ptr function used to get the RX data
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_StartRxProcess(void (*UserCallback)(uint8_t *PData, uint16_t Size, uint8_t Error));

/**
 * @brief TraceSend decode the strFormat and post it to the circular queue for printing
 * @param strFormat Trace message and format
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_FSend(const char *strFormat, ...);

/**
 * @brief post data to the circular queue
 * @param *pdata pointer to Data
 * @param length length of data buffer ro be sent
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_Send(const uint8_t *pdata, uint16_t length);

/**
 * @brief ZCSend_Allocation allocate the memory and return information to write the data
 * @param Length trase size
 * @param pData  pointer on the fifo
 * @param FifoSize size of the fifo
 * @param WritePos write position of the fifo
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_ZCSend_Allocation(uint16_t Length, uint8_t **pData, uint16_t *FifoSize, uint16_t *WritePos);

/**
 * @brief ZCSend finalize the data transfer
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_ZCSend_Finalize(void);
/**
 * @brief  Trace send started hook
 * @retval None
 */

/**
 * @brief  Trace send pre hook function
 */
void ADV_TRACER_PreSendHook(void);

/**
 * @brief  Trace send post hook function
 */
void ADV_TRACER_PostSendHook(void);

#if defined(ADV_TRACER_OVERRUN)
/**
 * @brief Register a function used to add overrun info inside the trace
 * @param cb pointer of function to return overrun information
 */
void ADV_TRACER_RegisterOverRunFunction(cb_overrun *cb);
#endif

#if defined(ADV_TRACER_CONDITIONNAL)

/**
 * @brief conditional FSend decode the strFormat and post it to the circular queue for printing
 * @param VerboseLevel verbose level of the trace
 * @param Region region of the trace
 * @param TimeStampState 0 no time stamp insertion, 1 time stamp inserted inside the trace data
 * @param strFormat formatted string
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_COND_FSend(uint32_t VerboseLevel, uint32_t Region,uint32_t TimeStampState, const char *strFormat, ...);

/**
 * @brief conditional ZCSend Write user formatted data directly in the FIFO (Z-Cpy)
 * @param VerboseLevel verbose level of the trace
 * @param Region region of the trace
 * @param TimeStampState 0 no time stamp insertion, 1 time stamp inserted inside the trace data
 * @param length  data length
 * @param pData  pointer on the fifo
 * @param FifoSize size of the fifo
 * @param WritePos write position of the fifo
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_COND_ZCSend_Allocation(uint32_t VerboseLevel, uint32_t Region, uint32_t TimeStampState, uint16_t length,uint8_t **pData, uint16_t *FifoSize, uint16_t *WritePos);

/**
 * @brief conditional ZCSend finalize the data transfer
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_COND_ZCSend_Finalize(void);

/**
 * @brief confitionnal Send post data to the circular queue
 * @param VerboseLevel verbose level of the trace
 * @param Region region of the trace
 * @param TimeStampState 0 no time stamp insertion, 1 time stamp inserted inside the trace data
 * @param *pdata pointer to Data
 * @param length length of data buffer ro be sent
 * @retval Status based on @ref ADV_TRACER_Status_t
 */
ADV_TRACER_Status_t ADV_TRACER_COND_Send(uint32_t VerboseLevel, uint32_t Region, uint32_t TimeStampState, const uint8_t *pdata, uint16_t length);

/**
 * @brief Register a function used to add timestamp inside the trace
 * @param cb pointer of function to return timestamp information
 */
void ADV_TRACER_RegisterTimeStampFunction(cb_timestamp *cb);

/**
 * @brief  Set the verbose level
 * @param  Level (0 to 256)
 * @retval None
 */
void ADV_TRACER_SetVerboseLevel(uint8_t Level);

/**
 * @brief  Get the verbose level
 * @retval verbose level
 */
uint8_t ADV_TRACER_GetVerboseLevel(void);

/**
 * @brief  add to the mask a bit field region.
 * @param  Region bit field of region to enable
 * @retval None
 */
void ADV_TRACER_SetRegion(uint32_t Region);

/**
 * @brief  add to the mask a bit field region.
 * @retval None
 */
uint32_t ADV_TRACER_GetRegion(void);

/**
 * @brief  remove from the mask a bit field region.
 * @param  Region Region bit field of region to disable
 * @retval None
 */
void ADV_TRACER_ResetRegion(uint32_t Region);

#endif

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_ADV_TRACER_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

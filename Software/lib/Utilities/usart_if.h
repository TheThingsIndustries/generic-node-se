/**
  ******************************************************************************
  * @file    usart_if.h
  * @author  MCD Application Team
  * @brief   Header for USART interface configuration
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
#include "stm32_adv_trace.h"
#include "STNODE_bsp.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_IF_H__
#define __USART_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
* @brief  Init the UART and associated DMA.
* @param  TxCpltCallback
* @return none
*/
UTIL_ADV_TRACE_Status_t vcom_Init(void (*cb)(void*));

/**
* @brief  init receiver of vcom
* @param  callback when Rx char is received
* @return none
*/
UTIL_ADV_TRACE_Status_t vcom_ReceiveInit(void (*RxCb)(uint8_t *rxChar, uint16_t size, uint8_t error));


/**
* @brief  DeInit the UART and associated DMA.
* @param  none
* @return none
*/
UTIL_ADV_TRACE_Status_t vcom_DeInit(void);

/**
* @brief  send buffer @p_data of size size to vcom in polling mode
* @param  p_data data to be sent
* @param  szie of buffer p_data to be sent
* @return none
*/
void vcom_Trace(uint8_t *p_data, uint16_t size);

/**
* @brief  send buffer @p_data of size size to vcom using DMA
* @param  p_data data to be sent
* @param  szie of buffer p_data to be sent
* @return none
*/
UTIL_ADV_TRACE_Status_t vcom_Trace_DMA(uint8_t *p_data, uint16_t size);

/**
* @brief  last byte has been sent on the uart line
* @param  none
* @return none
*/
void vcom_IRQHandler(void);

/**
* @brief  last byte has been sent from memory to uart data register
* @param  none
* @return none
*/
void vcom_DMA_TX_IRQHandler(void);

/**
  * @brief  Resume the UART and associated DMA (used by LPM)
  * @param  none
  * @return none
  */
void vcom_Resume(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_IF_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

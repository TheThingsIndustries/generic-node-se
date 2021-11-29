/**
 ******************************************************************************
 * @file    stm32_adv_usart.h
 * @author  MCD Application Team
 * @brief   Header for stm32_adv_usart.c interface configuration
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_ADV_USART_H__
#define __STM32_ADV_USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_adv_tracer.h"
#include "GNSE_bsp_serial.h"

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init the UART and associated DMA.
  * @param  cb TxCpltCallback
  * @return @ref ADV_TRACER_Status_t
  */
ADV_TRACER_Status_t vcom_Init(void (*cb)(void *));

/**
  * @brief  init receiver of vcom
  * @param  RxCb callback when Rx char is received
  * @return @ref ADV_TRACER_Status_t
  */
ADV_TRACER_Status_t vcom_ReceiveInit(void (*RxCb)(uint8_t *rxChar, uint16_t size, uint8_t error));

/**
  * @brief  DeInit the UART and associated DMA.
  * @return @ref ADV_TRACER_Status_t
  */
ADV_TRACER_Status_t vcom_DeInit(void);

/**
  * @brief  send buffer \p p_data of size \p size to vcom in polling mode
  * @param  p_data data to be sent
  * @param  size of buffer p_data to be sent
  */
void vcom_Trace(uint8_t *p_data, uint16_t size);

/**
  * @brief  send buffer \p p_data of size \p size to vcom using DMA
  * @param  p_data data to be sent
  * @param  size of buffer p_data to be sent
  * @return @ref ADV_TRACER_Status_t
  */
ADV_TRACER_Status_t vcom_Trace_DMA(uint8_t *p_data, uint16_t size);

/**
  * @brief  last byte has been sent on the uart line
  */
void vcom_IRQHandler(void);

/**
  * @brief  last byte has been sent from memory to uart data register
  */
void vcom_DMA_TX_IRQHandler(void);

/**
  * @brief  Resume the UART and associated DMA (used by LPM)
  */
void vcom_Resume(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_ADV_USART_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

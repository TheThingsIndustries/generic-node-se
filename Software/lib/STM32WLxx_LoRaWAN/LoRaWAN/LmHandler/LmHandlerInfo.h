/**
  ******************************************************************************
  * @file    LmHandlerInfo.h
  * @author  MCD Application Team
  * @brief   To give info to the application about LoraWAN configuration
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
#ifndef __LMHANDLER_INFO_H__
#define __LMHANDLER_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/*!
 * To give info to the application about LoraWAN capability 
 * it can depend how it has been compiled (e.g. compiled regions ...)
 * Params should be better uint32_t foe easier alignment with info_table concept
 */
typedef struct
{
  uint32_t ActivationMode;  /*!< 1: ABP, 2 : OTAA, 3: ABP & OTAA   */ 
  uint32_t Region;   /*!< Combination of regions compiled on MW  */ 
  uint32_t ClassB;   /*!< 0: not compiled in Mw, 1 : compiled in MW  */ 
  uint32_t Kms;      /*!< 0: not compiled in Mw, 1 : compiled in MW  */ 
}LmHandlerInfo_t;

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/**
  * @brief initialises the LoraMacInfo table
  * @param none
  * @retval  none
  */
void LmHandlerInfo_Init(void);

/**
  * @brief returns the pointer to the LoraMacInfo capabilities table
  * @param lmHandlerInfo_p pointer
  * @retval None
  */
LmHandlerInfo_t* LmHandlerInfo_GetPtr(void);

#ifdef __cplusplus
}
#endif

#endif // __LMHANDLER_INFO_H__

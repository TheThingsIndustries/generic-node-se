/**
  ******************************************************************************
  * @file    lora_info.h
  * @author  MCD Application Team
  * @brief   To give info to the application about LoRaWAN configuration
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#ifndef __LORA_INFO_H__
#define __LORA_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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
} LoraInfo_t;

/**
  * @brief initialises the LoraInfo table
  * @param none
  * @retval  none
  */
void LoraInfo_Init(void);

/**
  * @brief returns the pointer to the LoraInfo capabilities table
  * @param none
  * @retval LoraInfo pointer
  */
LoraInfo_t *LoraInfo_GetPtr(void);

/**
  * @brief  callback to get the battery level in % of full charge (254 full charge, 0 no charge)
  * @param  none
  * @return battery level
  */
uint8_t GetBatteryLevel(void);

/**
  * @brief  callback to get the current temperature in the MCU
  * @param  none
  * @return temperature level
  */
uint16_t GetTemperatureLevel(void);

/**
  * @brief  callback to get the board 64 bits unique ID
  * @param  unique ID
  * @return none
  */
void GetUniqueId(uint8_t *id);

/**
  * @brief  callback to get the board 32 bits unique ID (LSB)
  * @param  none
  * @retval devAddr Device Address
  */
uint32_t GetDevAddr(void);

#ifdef __cplusplus
}
#endif

#endif /* __LORA_INFO_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

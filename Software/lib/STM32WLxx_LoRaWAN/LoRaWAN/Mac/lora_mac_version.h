/**
  ******************************************************************************
  * @file    lora_mac_version.h
  * @author  MCD Application Team
  * @brief   Identifies the version of loramac on github
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

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __LORA_MAC_VERSION_H__
#define __LORA_MAC_VERSION_H__

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/*the 4 MSBs define the version based on Github version         */
/*https://github.com/Lora-net/LoRaMac-node/wiki/LoRaMAC-node-Wiki*/
/* version 4.4.2-rc.5 from develop branch */
/*#define LORA_MAC_VERSION   (uint32_t) 0x44250000*/
#define __LORA_MAC_VERSION_MAIN   (0x04U) /*!< [31:24] main version */
#define __LORA_MAC_VERSION_SUB1   (0x04U) /*!< [23:16] sub1 version */
#define __LORA_MAC_VERSION_SUB2   (0x03U) /*!< [15:8]  sub2 version */
#define __LORA_MAC_VERSION_RC     (0x00U) /*!< [7:0]  release candidate */ 
  
#define __LORA_MAC_MAIN_SHIFT      24
#define __LORA_MAC_SUB1_SHIFT      16
#define __LORA_MAC_SUB2_SHIFT      8
#define __LORA_MAC_RC_SHIFT        0      
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
   #define __LORA_MAC_VERSION        ((__LORA_MAC_VERSION_MAIN <<__LORA_MAC_MAIN_SHIFT)\
                                             |(__LORA_MAC_VERSION_SUB1 << __LORA_MAC_SUB1_SHIFT)\
                                             |(__LORA_MAC_VERSION_SUB2 << __LORA_MAC_SUB2_SHIFT )\
                                             |(__LORA_MAC_VERSION_RC << __LORA_MAC_RC_SHIFT)) 
/* Exported functions ------------------------------------------------------- */ 

#ifdef __cplusplus
}
#endif

#endif /*__LORA_MAC_VERSION_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

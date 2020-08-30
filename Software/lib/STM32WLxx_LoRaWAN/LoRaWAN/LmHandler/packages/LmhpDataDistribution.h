/**
  ******************************************************************************
  * @file    LmhpDataDistribution.h
  * @author  MCD Application Team
  * @brief   Function prototypes for LoRaMac Data distribution agent
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
#ifndef __LMHP_DATA_DISTRIBUTION_H__
#define __LMHP_DATA_DISTRIBUTION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "LmhPackage.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
LmHandlerErrorStatus_t LmhpDataDistributionInit(void);

LmHandlerErrorStatus_t LmhpDataDistributionPackageRegister(uint8_t id, LmhPackage_t **package);

#ifdef __cplusplus
}
#endif

#endif /* __LMHP_DATA_DISTRIBUTION_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

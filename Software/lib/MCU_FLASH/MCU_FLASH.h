/** Copyright © 2021 The Things Industries B.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file MCU_FLASH.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef MCU_FLASH_H
#define MCU_FLASH_H

#include "stm32wlxx.h"
#include "GNSE_tracer.h"
#include "GNSE_bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (DEBUG_MCU_FLASH)
#define MCU_FLASH_PPRINTF(...)  LIB_PRINTF(...)
#else
#define MCU_FLASH_PPRINTF(...)
#endif

HAL_StatusTypeDef MCU_FLASH_Init(void);
HAL_StatusTypeDef MCU_FLASH_Erase(void *pStart, uint32_t uLength);
HAL_StatusTypeDef MCU_FLASH_Write(uint32_t pDestination, uint8_t *pSource, uint32_t uLength);
HAL_StatusTypeDef MCU_FLASH_Read(void *pDestination, const void *pSource, uint32_t Length);

#ifdef __cplusplus
}
#endif

#endif /* MCU_FLASH_H */

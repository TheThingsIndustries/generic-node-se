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
 * @file GNSE_flash.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_FLASH_H
#define GNSE_FLASH_H

#include "MX25R16.h"
#include "spiffs.h"

extern MxChip GNSE_Flash;
extern spiffs GNSE_Flash_SPIFFS;

/**
 * Flash operations return type
 */
typedef enum
{
    FLASH_OP_SUCCESS = 0,
    FLASH_OP_FAIL = 1,
} FLASH_op_result_t;

FLASH_op_result_t GNSE_Flash_Init(void);
FLASH_op_result_t GNSE_Flash_DeInit(void);
FLASH_op_result_t GNSE_Flash_Read(uint32_t addr, uint32_t byteCount, uint8_t *target_buffer);
FLASH_op_result_t GNSE_Flash_Write(uint32_t addr, uint32_t byteCount, uint8_t *source_buffer);
FLASH_op_result_t GNSE_Flash_BlockErase(uint32_t addr, uint32_t block_count);
FLASH_op_result_t GNSE_Flash_ChipErase(void);
FLASH_op_result_t GNSE_Flash_mount(void);

#endif /* GNSE_FLASH_H */

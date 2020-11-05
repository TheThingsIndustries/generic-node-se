/** Copyright © 2020 The Things Industries B.V.
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
 * @file bootloader.c
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#include "bootloader.h"

typedef void (*functionPointer)(void);

/**
 * @brief  This function initializes bootloader and flash.
 * @return Bootloader_op_result_t (BL_OP_SUCCESS or BL_OP_UNKNOWN_ERROR)
 */
uint8_t Bootloader_Init(void)
{
    Bootloader_op_result_t ret = BL_OP_UNKNOWN_ERROR;
    if(MCU_FLASH_Init() == HAL_OK)
    {
        ret = BL_OP_SUCCESS;
    }
    return ret;
}

/**
 * @brief  Performs a jump to the user application in flash
 * @return None
 */
void Bootloader_JumpToApplication(void)
{
    uint32_t JumpAddress = *(__IO uint32_t *)(APP_ADDRESS + 4);
    functionPointer Jump = (functionPointer)JumpAddress;

    HAL_RCC_DeInit();
    HAL_DeInit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

#if (SET_VECTOR_TABLE)
    SCB->VTOR = APP_ADDRESS;
#endif

    __set_MSP(*(__IO uint32_t *)APP_ADDRESS);
    Jump();
}

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
 * @file bootloader.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "bootloader.h"

typedef void (*functionPointer)(void);
static volatile Bootloader_state_t bootloader_state = BOOTLOADER_STATE_APP_JMP;

/**
 * @brief  Initializes bootloader and flash
 * @return Bootloader_op_result_t (BL_OP_SUCCESS or BL_OP_UNKNOWN_ERROR)
 */
uint8_t Bootloader_Init(void)
{
    Bootloader_op_result_t ret = BL_OP_UNKNOWN_ERROR;

    /* Check system reset flags */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
    {
        /* OBL flag is active */
#if (CLEAR_RESET_FLAGS)
        /* Clear system reset flags */
        __HAL_RCC_CLEAR_RESET_FLAGS();
#endif
    }

    if (MCU_FLASH_Init() == HAL_OK)
    {
        ret = BL_OP_SUCCESS;
    }

    GNSE_BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_GPIO);

#if (BOOTLOADER_LED_FEEDBACK)
    GNSE_BSP_LED_Init(LED_RED);
    GNSE_BSP_LED_Init(LED_BLUE);
#endif

    return ret;
}

/**
 * @brief  De-Initializes bootloader
 * @return Bootloader_op_result_t
 */
uint8_t Bootloader_DeInit(void)
{

#if (BOOTLOADER_LED_FEEDBACK)
    GNSE_BSP_LED_DeInit(LED_RED);
    GNSE_BSP_LED_DeInit(LED_BLUE);
#endif

    return BL_OP_SUCCESS;
}

/**
 * @brief Set Bootloader state
 * @return None
 */
void Bootloader_SetState(Bootloader_state_t state)
{
    bootloader_state = state;
}

/**
 * @brief Get Bootloader state
 * @return None
 */
Bootloader_state_t Bootloader_GetState(void)
{
    return bootloader_state;
}

void Bootloader_HandleInput(void)
{
    uint8_t button_counter = 0;

#if (BOOTLOADER_LED_FEEDBACK)
    GNSE_BSP_LED_On(LED_RED);
    HAL_Delay(BOOTLOADER_LED_DELAY);
#endif

    while ((BOOTLOADER_BTN_PRESSED()) && (button_counter < BOOTLOADER_BTN_MAX_WAIT))
    {
        if (button_counter == BOOTLOADER_BTN_SYS_JMP_WAIT)
        {
            bootloader_state = BOOTLOADER_STATE_SYS_JMP;
        }

#if (BOOTLOADER_LED_FEEDBACK)
        if (button_counter < BOOTLOADER_BTN_SYS_JMP_WAIT)
        {
            GNSE_BSP_LED_Toggle(LED_RED);
        }
        else if (button_counter == BOOTLOADER_BTN_SYS_JMP_WAIT)
        {
            GNSE_BSP_LED_Off(LED_RED);
            GNSE_BSP_LED_On(LED_BLUE);
        }
        else
        {
            GNSE_BSP_LED_Toggle(LED_BLUE);
        }
#endif

        button_counter++;
        HAL_Delay(BOOTLOADER_BTN_SAMPLE_DELAY);
    }
#if (BOOTLOADER_LED_FEEDBACK)
    GNSE_BSP_LED_Off(LED_RED);
    GNSE_BSP_LED_Off(LED_BLUE);
#endif
}

/**
 * @brief Performs the jump to user application address or internal ST bootloader
 * @return None
 */
void Bootloader_Jump()
{
    uint32_t mem_address = 0;
    uint32_t jump_address = 0;
    switch (bootloader_state)
    {
    case BOOTLOADER_STATE_APP_JMP:
        mem_address = APP_ADDRESS;
#if (SET_VECTOR_TABLE)
        SCB->VTOR = APP_ADDRESS;
#endif
        break;
    case BOOTLOADER_STATE_SYS_JMP:
        mem_address = ST_BOOTLOADER_SYSMEM_ADDRESS;
        __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
        break;
    default:
        return;
        break;
    }
    jump_address = *(__IO uint32_t *)(mem_address + 4);
    functionPointer Jump = (functionPointer)jump_address;


    HAL_RCC_DeInit();
    HAL_DeInit();
    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    __set_MSP(*(__IO uint32_t *)mem_address);
    Jump();
}

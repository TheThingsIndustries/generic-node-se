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
 * @file GNSE_hal.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_HAL_H
#define GNSE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * HW aliases for the initialisation function
 */
typedef struct GNSE_HAL_Ctx_s
{
    bool internal_sensors_init;
    bool external_sensors_init;
    bool flash_init;
    bool leds_init;
    bool bm_init;
    bool buzzer_init;
} GNSE_HAL_Ctx_t;

/**
 * GNSE HAL return types
 */
typedef enum
{
    GNSE_HAL_OP_SUCCESS = 0,
    GNSE_HAL_OP_FAIL = 1,
} GNSE_HAL_op_result_t;

void GNSE_HAL_Init(GNSE_HAL_Ctx_t gnse_inits);
void GNSE_HAL_DeInit(GNSE_HAL_Ctx_t gnse_deinits);
void GNSE_HAL_Internal_Sensors_Init(void);
void GNSE_HAL_Internal_Sensors_DeInit(void);
GNSE_HAL_op_result_t GNSE_HAL_SysClk_Init(void);
void GNSE_HAL_Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /*GNSE_HAL_H*/

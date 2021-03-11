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
 * @file BUZZER.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "GNSE_bsp.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        BUZZER_OP_SUCCESS = 0,
        BUZZER_OP_FAIL = 1,
    } BUZZER_op_result_t;

    typedef enum
    {
        BUZZER_STATE_OFF = 0,
        BUZZER_STATE_RING = 1,
        BUZZER_STATE_TICK = 2,
        BUZZER_STATE_DODO = 3,
        BUZZER_STATE_DODODO = 4,
        BUZZER_STATE_WARNING = 5,
        BUZZER_STATE_DANGER = 6
    } BUZZER_state_t;

    BUZZER_op_result_t BUZZER_Init(void);
    BUZZER_op_result_t BUZZER_DeInit(void);
    BUZZER_op_result_t BUZZER_SetState(BUZZER_state_t state);
    BUZZER_state_t BUZZER_GetState(void);

#ifdef __cplusplus
}
#endif

#endif /* BUZZER_H */

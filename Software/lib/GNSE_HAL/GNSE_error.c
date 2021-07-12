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
 * @file GNSE_error.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_error.h"
#include "GNSE_bsp.h"
#include "GNSE_lpm.h"

__weak void GNSE_Error_Handler(void)
{
    GNSE_BSP_LED_Init(LED_RED);
    GNSE_BSP_LED_On(LED_RED);
    GNSE_LPM_EnterLowPower();
    while (1)
    {
    }
}

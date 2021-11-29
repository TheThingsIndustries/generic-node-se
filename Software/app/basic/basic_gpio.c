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
 * @file basic_gpio.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"

void led_toggle(uint8_t n_toggles, uint32_t toggle_delay)
{
    GNSE_BSP_LED_Init(LED_BLUE);
    GNSE_BSP_LED_Init(LED_RED);
    GNSE_BSP_LED_Init(LED_GREEN);
    APP_PPRINTF("\r\n Toggling LED lights \r\n");
    for (size_t counter = 0; counter < n_toggles; counter++)
    {
        GNSE_BSP_LED_Toggle(LED_BLUE);
        HAL_Delay(toggle_delay);
        GNSE_BSP_LED_Toggle(LED_RED);
        HAL_Delay(toggle_delay);
        GNSE_BSP_LED_Toggle(LED_GREEN);
        HAL_Delay(toggle_delay);
    }
    GNSE_BSP_LED_Off(LED_BLUE);
    GNSE_BSP_LED_Off(LED_RED);
    GNSE_BSP_LED_Off(LED_GREEN);
}

void buzzer_play(uint8_t n_plays, uint32_t play_delay)
{
    if (BUZZER_Init() == BUZZER_OP_SUCCESS)
    {
        APP_PPRINTF("\r\n Playing Buzzer tones \r\n");
        for (size_t counter = 0; counter < n_plays; counter++)
        {
            BUZZER_SetState(BUZZER_STATE_TICK);
            HAL_Delay(play_delay);
            BUZZER_SetState(BUZZER_STATE_DODO);
            HAL_Delay(play_delay);
            BUZZER_SetState(BUZZER_STATE_DODODO);
            HAL_Delay(play_delay);
            BUZZER_SetState(BUZZER_STATE_WARNING);
            HAL_Delay(play_delay);
            BUZZER_SetState(BUZZER_STATE_DANGER);
            HAL_Delay(play_delay);
            BUZZER_SetState(BUZZER_STATE_RING);
            HAL_Delay(play_delay);
            BUZZER_SetState(BUZZER_STATE_OFF);
        }
    }
    else
    {
        APP_PPRINTF("\r\n Failed to play Buzzer tones \r\n");
    }
}

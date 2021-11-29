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
 * @file BUZZER.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "BUZZER.h"
#include "stdbool.h"

static volatile uint8_t buzzerCount = 0;
static volatile BUZZER_state_t buzzer_state = BUZZER_STATE_OFF;

static void BUZZER_Ring(void);
static void BUZZER_Tick(void);
static void BUZZER_DODO(void);
static void BUZZER_DODODO(void);
static void BUZZER_Warning(void);
static void BUZZER_Danger(void);
static void BUZZER_TIM_IRQHandler(TIM_HandleTypeDef *htim);

static bool init_flag = false;

BUZZER_op_result_t BUZZER_Init(void)
{

  if (init_flag == false)
  {
    if (GNSE_BSP_BUZZER_TIM_Init(BUZZER_TIM_IRQHandler) == GNSE_BSP_ERROR_NONE)
    {
      init_flag = true;
      buzzer_state = BUZZER_STATE_OFF;
      return BUZZER_OP_SUCCESS;
    }
    else
    {
      return BUZZER_OP_FAIL;
    }
  }
  return BUZZER_OP_SUCCESS;
}

BUZZER_op_result_t BUZZER_DeInit(void)
{
  if (GNSE_BSP_BUZZER_TIM_DeInit() != GNSE_BSP_ERROR_NONE)
  {
    return BUZZER_OP_FAIL;
  }
  init_flag = false;
  return BUZZER_OP_SUCCESS;
}

static void BUZZER_Off(void)
{
  BUZZER_Init();

  __HAL_TIM_SET_COMPARE(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL, 0);
  HAL_GPIO_WritePin(BUZZER_TIMER_PWM_PORT, BUZZER_TIMER_PWM_PIN, GPIO_PIN_RESET);

  buzzer_state = BUZZER_STATE_OFF;
}

static void BUZZER_Ring(void)
{
  BUZZER_Init();

  __HAL_TIM_SET_AUTORELOAD(&GNSE_BSP_buzzer_timer, BUZZER_PERIOD);
  __HAL_TIM_SET_COMPARE(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL, (BUZZER_PERIOD + 1) / 2);
  buzzer_state = BUZZER_STATE_RING;
}

static void BUZZER_Tick(void)
{
  BUZZER_Init();

  __HAL_TIM_SET_AUTORELOAD(&GNSE_BSP_buzzer_timer, (BUZZER_PERIOD + 1) / 2 - 1);
  __HAL_TIM_SET_COMPARE(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL, (BUZZER_PERIOD + 1) / 4);

  __HAL_TIM_CLEAR_FLAG(&GNSE_BSP_buzzer_timer, TIM_FLAG_UPDATE);
  __HAL_TIM_ENABLE_IT(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_IT);

  buzzerCount = 0;
  buzzer_state = BUZZER_STATE_TICK;
}

static void BUZZER_DODO(void)
{
  BUZZER_Init();

  __HAL_TIM_SET_AUTORELOAD(&GNSE_BSP_buzzer_timer, (BUZZER_PERIOD + 1) / 4 - 1);
  __HAL_TIM_SET_COMPARE(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL, (BUZZER_PERIOD + 1) / 8);

  __HAL_TIM_CLEAR_FLAG(&GNSE_BSP_buzzer_timer, TIM_FLAG_UPDATE);
  __HAL_TIM_ENABLE_IT(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_IT);

  buzzerCount = 1;
  buzzer_state = BUZZER_STATE_DODO;
}

static void BUZZER_DODODO(void)
{
  BUZZER_Init();

  __HAL_TIM_SET_AUTORELOAD(&GNSE_BSP_buzzer_timer, (BUZZER_PERIOD + 1) / 8 - 1);
  __HAL_TIM_SET_COMPARE(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL, (BUZZER_PERIOD + 1) / 16);

  __HAL_TIM_CLEAR_FLAG(&GNSE_BSP_buzzer_timer, TIM_FLAG_UPDATE);
  __HAL_TIM_ENABLE_IT(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_IT);

  buzzerCount = 3;
  buzzer_state = BUZZER_STATE_DODODO;
}

static void BUZZER_Warning(void)
{
  BUZZER_Init();

  __HAL_TIM_SET_AUTORELOAD(&GNSE_BSP_buzzer_timer, (BUZZER_PERIOD + 1) / 2 - 1);
  __HAL_TIM_SET_COMPARE(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL, (BUZZER_PERIOD + 1) / 4);

  buzzer_state = BUZZER_STATE_WARNING;
}

static void BUZZER_Danger(void)
{
  BUZZER_Init();

  __HAL_TIM_SET_AUTORELOAD(&GNSE_BSP_buzzer_timer, (BUZZER_PERIOD + 1) / 4 - 1);
  __HAL_TIM_SET_COMPARE(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL, (BUZZER_PERIOD + 1) / 8);

  buzzer_state = BUZZER_STATE_DANGER;
}

BUZZER_op_result_t BUZZER_SetState(BUZZER_state_t state)
{
  if (state != buzzer_state)
  {
    switch (state)
    {
    case BUZZER_STATE_OFF:
      BUZZER_Off();
      break;
    case BUZZER_STATE_RING:
      BUZZER_Ring();
      break;
    case BUZZER_STATE_TICK:
      BUZZER_Tick();
      break;
    case BUZZER_STATE_DODO:
      BUZZER_DODO();
      break;
    case BUZZER_STATE_DODODO:
      BUZZER_DODODO();
      break;
    case BUZZER_STATE_WARNING:
      BUZZER_Warning();
      break;
    case BUZZER_STATE_DANGER:
      BUZZER_Danger();
      break;

    default:
      // BUZZER_state_t is not implemented
      return BUZZER_OP_FAIL;
      break;
    }

    return BUZZER_OP_SUCCESS;
  }
  else
  {
    return BUZZER_OP_FAIL;
  }
}

BUZZER_state_t BUZZER_GetState(void)
{
  return buzzer_state;
}

static void BUZZER_TIM_IRQHandler(TIM_HandleTypeDef *htim)
{
  if (__HAL_TIM_GET_IT_SOURCE(htim, BUZZER_TIMER_IT) != RESET)
  {
    __HAL_TIM_CLEAR_IT(htim, BUZZER_TIMER_IT);

    if ((buzzer_state == BUZZER_STATE_TICK) || (buzzer_state == BUZZER_STATE_DODO) || (buzzer_state == BUZZER_STATE_DODODO))
    {
      if (buzzerCount > 0)
      {
        buzzerCount--;
      }
      else
      {
        __HAL_TIM_DISABLE_IT(htim, BUZZER_TIMER_IT);
        BUZZER_Off();
      }
    }
    else
    {
      __HAL_TIM_DISABLE_IT(htim, BUZZER_TIMER_IT);
    }
  }
}

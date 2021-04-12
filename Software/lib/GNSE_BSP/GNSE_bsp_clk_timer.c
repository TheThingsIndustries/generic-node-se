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
 * @file GNSE_bsp_clk_timer.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */
#include "GNSE_bsp_clk_timer.h"

TIM_HandleTypeDef GNSE_BSP_buzzer_timer;
RTC_HandleTypeDef GNSE_BSP_rtc;
IWDG_HandleTypeDef GNSE_BSP_iwdg;

int32_t GNSE_BSP_RTC_Init(void)
{
  RTC_AlarmTypeDef sAlarm = {0};

  GNSE_BSP_rtc.Instance = RTC;
  GNSE_BSP_rtc.Init.AsynchPrediv = RTC_PREDIV_A;
  GNSE_BSP_rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  GNSE_BSP_rtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  GNSE_BSP_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  GNSE_BSP_rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  GNSE_BSP_rtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  GNSE_BSP_rtc.Init.BinMode = RTC_BINARY_ONLY;
  if (HAL_RTC_Init(&GNSE_BSP_rtc) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }

  // Initialize RTC and set the Time and Date
  if (HAL_RTCEx_SetSSRU_IT(&GNSE_BSP_rtc) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }
  // Enable the Alarm A
  sAlarm.BinaryAutoClr = RTC_ALARMSUBSECONDBIN_AUTOCLR_NO;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDBINMASK_NONE;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&GNSE_BSP_rtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }
  return GNSE_BSP_ERROR_NONE;
}

/**
  * @brief  Configures independent watchdog timer
  * @note   The GNSE_BSP_IWDG_Refresh should be called every time before the timer expires
  * @arg    iwdg_reload: Timeout (in ticks of the IWDG timer) for the IWDG (max is 0xFFF)
  * @return GNSE_BSP status
  */
int32_t GNSE_BSP_IWDG_Init(uint32_t iwdg_reload)
{
  /* Set IWDG settings to maximum reload value */
  GNSE_BSP_iwdg.Instance = IWDG;
  GNSE_BSP_iwdg.Init.Prescaler = IWDG_PRESCALER_256;
  GNSE_BSP_iwdg.Init.Window = IWDG_WINR_WIN;
  GNSE_BSP_iwdg.Init.Reload = iwdg_reload;

  if (HAL_IWDG_Init(&GNSE_BSP_iwdg) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }

  return GNSE_BSP_ERROR_NONE;
}

/**
  * @brief  Refreshes the watchdog timer counter
  * @return None
  */
void GNSE_BSP_IWDG_Refresh(void)
{
  HAL_IWDG_Refresh(&GNSE_BSP_iwdg);
}

int32_t GNSE_BSP_BUZZER_TIM_Init(pTIM_CallbackTypeDef cb)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  GNSE_BSP_buzzer_timer.Instance = BUZZER_TIMER;
  GNSE_BSP_buzzer_timer.Init.Prescaler = BUZZER_PRESCALER;
  GNSE_BSP_buzzer_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  GNSE_BSP_buzzer_timer.Init.Period = BUZZER_PERIOD;
  GNSE_BSP_buzzer_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  GNSE_BSP_buzzer_timer.Init.RepetitionCounter = 0;
  GNSE_BSP_buzzer_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&GNSE_BSP_buzzer_timer) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&GNSE_BSP_buzzer_timer, &sMasterConfig) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&GNSE_BSP_buzzer_timer, &sConfigOC, BUZZER_TIMER_CHANNEL) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&GNSE_BSP_buzzer_timer, &sBreakDeadTimeConfig) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }
  HAL_TIM_RegisterCallback(&GNSE_BSP_buzzer_timer, HAL_TIM_PERIOD_ELAPSED_CB_ID, cb);
  if (HAL_TIM_PWM_Start_IT(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }
  return GNSE_BSP_ERROR_NONE;
}

int32_t GNSE_BSP_BUZZER_TIM_DeInit()
{
  if (HAL_TIM_PWM_DeInit(&GNSE_BSP_buzzer_timer) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }
  if (HAL_TIM_PWM_Stop_IT(&GNSE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL) != HAL_OK)
  {
    return GNSE_BSP_ERROR_NO_INIT;
  }
  return GNSE_BSP_ERROR_NONE;
}

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
 * @file GNSE_bsp_clk_timer.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_BSP_CLK_TIMER_H
#define GNSE_BSP_CLK_TIMER_H

#include "stm32wlxx_hal.h"
#include "GNSE_bsp_error.h"
#include "GNSE_bsp_conf.h"

extern TIM_HandleTypeDef GNSE_BSP_buzzer_timer;
extern RTC_HandleTypeDef GNSE_BSP_rtc;
extern IWDG_HandleTypeDef GNSE_BSP_iwdg;

#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1 << RTC_N_PREDIV_S) - 1)
#define RTC_PREDIV_A ((1 << (15 - RTC_N_PREDIV_S)) - 1)

#define IWDG_MAX_RELOAD 0xFFFU

#define BUZZER_TIMER TIM2
#define BUZZER_TIMER_CHANNEL TIM_CHANNEL_1
/**
 * In this example TIM2 input clock (TIM2CLK) is set to APB1 clock (PCLK1),
 * since APB1 prescaler is equal to 1.
 * TIM1CLK = PCLK1
 * PCLK1 = HCLK
 * => TIM1CLK = HCLK = SystemCoreClock
 * To get TIM1 counter clock at 1 MHz, the prescaler is computed as follows:
 * Prescaler = (TIM2CLK / TIM2 counter clock) - 1
 * Prescaler = ((SystemCoreClock) /1 MHz) - 1
 */
#define BUZZER_PRESCALER (uint32_t)(((SystemCoreClock) / 100000) - 1)

/**
 * To get TIM2 output clock at 2.73 KHz, the period (BUZZER_PERIOD)) is computed as follows:
 * BUZZER_PERIOD = (TIM2 counter clock / TIM2 output clock) - 1 ~= 365
 *
 */
#define BUZZER_PERIOD (365U)
#define BUZZER_TIMER_PWM_PIN GPIO_PIN_15
#define BUZZER_TIMER_PWM_PORT GPIOA
#define BUZZER_TIMER_CLK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#define BUZZER_TIMER_CLK_DISABLE() __HAL_RCC_TIM2_CLK_DISABLE()
#define BUZZER_TIMER_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUZZER_TIMER_AF GPIO_AF1_TIM2

#define BUZZER_TIMER_IRQn TIM2_IRQn
#define BUZZER_TIMER_IT TIM_IT_UPDATE

int32_t GNSE_BSP_BUZZER_TIM_Init(pTIM_CallbackTypeDef cb);
int32_t GNSE_BSP_BUZZER_TIM_DeInit();

int32_t GNSE_BSP_RTC_Init(void);

int32_t GNSE_BSP_IWDG_Init(uint32_t iwdg_reload);
void GNSE_BSP_IWDG_Refresh(void);

#endif /* GNSE_BSP_CLK_TIMER_H */

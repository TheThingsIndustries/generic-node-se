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
 * @file GNSE_rtc.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_RTC_H
#define GNSE_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_timer.h"
#include "stm32_systime.h"

/* Exported functions prototypes ---------------------------------------------*/
/*!
 * @brief Init RTC hardware
 * @param none
 * @retval none
 */
UTIL_TIMER_Status_t GNSE_RTC_Init(void);

/*!
 * @brief Set the alarm
 * @note The alarm is set at timeout from timer Reference (TimerContext)
 * @param timeout Duration of the Timer in ticks
 */
UTIL_TIMER_Status_t GNSE_RTC_StartTimer(uint32_t timeout);

/*!
 * @brief Stop the Alarm
 * @param none
 * @retval none
 */
UTIL_TIMER_Status_t GNSE_RTC_StopTimer(void);

/*!
 * @brief set timer Reference (TimerContext)
 * @param none
 * @retval  Timer Reference Value in  Ticks
 */
uint32_t GNSE_RTC_SetTimerContext(void);

/*!
 * @brief Get the RTC timer Reference
 * @param none
 * @retval Timer Value in  Ticks
 */
uint32_t GNSE_RTC_GetTimerContext(void);

/*!
 * @brief Get the timer elapsed time since timer Reference (TimerContext) was set
 * @param none
 * @retval RTC Elapsed time in ticks
 */
uint32_t GNSE_RTC_GetTimerElapsedTime(void);

/*!
 * @brief Get the timer value
 * @param none
 * @retval RTC Timer value in ticks
 */
uint32_t GNSE_RTC_GetTimerValue(void);

/*!
 * @brief Return the minimum timeout in ticks the RTC is able to handle
 * @param none
 * @retval minimum value for a timeout in ticks
 */
uint32_t GNSE_RTC_GetMinimumTimeout(void);

/*!
 * @brief a delay of delay ms by polling RTC
 * @param delay in ms
 * @retval none
 */
void GNSE_RTC_DelayMs(uint32_t delay);

/*!
 * @brief converts time in ms to time in ticks
 * @param [IN] time in milliseconds
 * @retval returns time in timer ticks
 */
uint32_t GNSE_RTC_Convert_ms2Tick(uint32_t timeMilliSec);

/*!
 * @brief converts time in ticks to time in ms
 * @param [IN] time in timer ticks
 * @retval returns time in timer milliseconds
 */
uint32_t GNSE_RTC_Convert_Tick2ms(uint32_t tick);

/*!
 * @brief Get rtc time
 * @param [OUT] subSeconds in ticks
 * @retval returns time seconds
 */
uint32_t GNSE_RTC_GetTime(uint16_t *subSeconds);

/*!
 * @brief write seconds in backUp register
 * @Note Used to store seconds difference between RTC time and Unix time
 * @param [IN] time in seconds
 * @retval None
 */
void GNSE_RTC_BkUp_Write_Seconds(uint32_t Seconds);

/*!
 * @brief reads seconds from backUp register
 * @Note Used to store seconds difference between RTC time and Unix time
 * @param [IN] None
 * @retval Time in seconds
 */
uint32_t GNSE_RTC_BkUp_Read_Seconds(void);

/*!
 * @brief writes SubSeconds in backUp register
 * @Note Used to store SubSeconds difference between RTC time and Unix time
 * @param [IN] time in SubSeconds
 * @retval None
 */
void GNSE_RTC_BkUp_Write_SubSeconds(uint32_t SubSeconds);

/*!
 * @brief reads SubSeconds from backUp register
 * @Note Used to store SubSeconds difference between RTC time and Unix time
 * @param [IN] None
 * @retval Time in SubSeconds
 */
uint32_t GNSE_RTC_BkUp_Read_SubSeconds(void);

#ifdef __cplusplus
}
#endif

#endif /*GNSE_RTC_H*/

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
 * @file GNSE_rtc.c
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#include <math.h>
#include "GNSE_bsp.h"
#include "GNSE_rtc.h"
#include "stm32wlxx_ll_rtc.h"

#define RtcHandle GNSE_BSP_rtc

#define MIN_ALARM_DELAY               3 /* in ticks */
/* RTC Ticks/ms conversion */
#define RTC_BKP_SECONDS    RTC_BKP_DR0
#define RTC_BKP_SUBSECONDS RTC_BKP_DR1
#define RTC_BKP_MSBTICKS   RTC_BKP_DR2

/* #define RTIF_DEBUG */

#ifdef RTIF_DEBUG
#include "sys_app.h" /*for app_log*/
#define GNSE_RTC_DBG_PRINTF(...) do{ {UTIL_ADV_TRACE_COND_FSend(VLEVEL_ALWAYS, T_REG_OFF, TS_OFF, __VA_ARGS__);} }while(0);
#else
#define GNSE_RTC_DBG_PRINTF(...)
#endif /* RTIF_DEBUG */

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RTC_Initialized = false;

/*!
 * \brief RtcTimerContext
 */
static uint32_t RtcTimerContext = 0;

/* Private function prototypes -----------------------------------------------*/
/*!
 * \brief Get rtc timer Value in rtc tick
 * \param [IN] None
 * \return val the rtc timer value (upcounting)
 */
static inline uint32_t GetTimerTicks(void);

/*!
 * @brief Writes MSBticks to backup register
 * Absolute RTC time in tick is (MSBticks)<<32 + (32bits binary counter)
 * @Note MSBticks incremented every time the 32bits RTC timer wraps around (~44days)
 * @param [IN] MSBticks
 * @return None
 */
static void GNSE_RTC_BkUp_Write_MSBticks(uint32_t MSBticks);

/*!
 * @brief Reads MSBticks from backup register
 * Absolute RTC time in tick is (MSBticks)<<32 + (32bits binary counter)
 * @Note MSBticks incremented every time the 32bits RTC timer wraps around (~44days)
 * @param [IN] None
 * @return MSBticks
 */
static uint32_t GNSE_RTC_BkUp_Read_MSBticks(void);

/*Timer driver*/
const UTIL_TIMER_Driver_s UTIL_TimerDriver =
{
  GNSE_RTC_Init,
  NULL,

  GNSE_RTC_StartTimer,
  GNSE_RTC_StopTimer,

  GNSE_RTC_SetTimerContext,
  GNSE_RTC_GetTimerContext,

  GNSE_RTC_GetTimerElapsedTime,
  GNSE_RTC_GetTimerValue,
  GNSE_RTC_GetMinimumTimeout,

  GNSE_RTC_Convert_ms2Tick,
  GNSE_RTC_Convert_Tick2ms,
};

/*System Time driver*/
const UTIL_SYSTIM_Driver_s UTIL_SYSTIMDriver =
{
  GNSE_RTC_BkUp_Write_Seconds,
  GNSE_RTC_BkUp_Read_Seconds,
  GNSE_RTC_BkUp_Write_SubSeconds,
  GNSE_RTC_BkUp_Read_SubSeconds,
  GNSE_RTC_GetTime,
};

UTIL_TIMER_Status_t GNSE_RTC_Init(void)
{
  if (RTC_Initialized == false)
  {
    GNSE_BSP_rtc.IsEnabled.RtcFeatures = UINT32_MAX;

    GNSE_BSP_RTC_Init();
    /** Stop Timer */
    GNSE_RTC_StopTimer();
    /** Configure the Alarm A */
    HAL_RTC_DeactivateAlarm(&GNSE_BSP_rtc, RTC_ALARM_A);
    /*Enable Direct Read of the calendar registers (not through Shadow) */
    HAL_RTCEx_EnableBypassShadow(&GNSE_BSP_rtc);
    /*initialise MSB ticks*/
    GNSE_RTC_BkUp_Write_MSBticks(0);

    GNSE_RTC_SetTimerContext();

    RTC_Initialized = true;
  }
  return UTIL_TIMER_OK;
}

UTIL_TIMER_Status_t GNSE_RTC_StartTimer(uint32_t timeout)
{
  RTC_AlarmTypeDef sAlarm = {0};
  /*Stop timer if one is already started*/
  GNSE_RTC_StopTimer();
  timeout += RtcTimerContext;

  GNSE_RTC_DBG_PRINTF("Start timer: time=%d, alarm=%d\n\r",  GetTimerTicks(), timeout);
  /* starts timer*/
  sAlarm.BinaryAutoClr = RTC_ALARMSUBSECONDBIN_AUTOCLR_NO;
  sAlarm.AlarmTime.SubSeconds = UINT32_MAX - timeout;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDBINMASK_NONE;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&GNSE_BSP_rtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    return UTIL_TIMER_HW_ERROR;
  }
  return UTIL_TIMER_OK;
}

UTIL_TIMER_Status_t GNSE_RTC_StopTimer(void)
{
  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_CLEAR_FLAG(&GNSE_BSP_rtc, RTC_FLAG_ALRAF);
  /* Disable the Alarm A interrupt */
  HAL_RTC_DeactivateAlarm(&GNSE_BSP_rtc, RTC_ALARM_A);
  /*overload RTC feature enable*/
  GNSE_BSP_rtc.IsEnabled.RtcFeatures = UINT32_MAX;
  return UTIL_TIMER_OK;
}

uint32_t GNSE_RTC_SetTimerContext(void)
{
  /*store time context*/
  RtcTimerContext = GetTimerTicks();
  GNSE_RTC_DBG_PRINTF("GNSE_RTC_SetTimerContext=%d\n\r", RtcTimerContext);
  /*return time context*/
  return RtcTimerContext;
}

uint32_t GNSE_RTC_GetTimerContext(void)
{
  GNSE_RTC_DBG_PRINTF("GNSE_RTC_GetTimerContext=%d\n\r", RtcTimerContext);
  /*return time context*/
  return RtcTimerContext;
}

uint32_t GNSE_RTC_GetTimerElapsedTime(void)
{
  return ((uint32_t)(GetTimerTicks() - RtcTimerContext));
}

uint32_t GNSE_RTC_GetTimerValue(void)
{
  if (RTC_Initialized == true)
  {
    return GetTimerTicks();
  }
  else
  {
    return 0;
  }
}

uint32_t GNSE_RTC_GetMinimumTimeout(void)
{
  return (MIN_ALARM_DELAY);
}

uint32_t GNSE_RTC_Convert_ms2Tick(uint32_t timeMilliSec)
{
  return ((uint32_t)((((uint64_t) timeMilliSec) << RTC_N_PREDIV_S) / 1000));
}

uint32_t GNSE_RTC_Convert_Tick2ms(uint32_t tick)
{
  return ((uint32_t)((((uint64_t)(tick)) * 1000) >> RTC_N_PREDIV_S));
}

void GNSE_RTC_DelayMs(uint32_t delay)
{
  uint32_t delayTicks = GNSE_RTC_Convert_ms2Tick(delay);
  uint32_t timeout = GetTimerTicks();

  /* Wait delay ms */
  while (((GetTimerTicks() - timeout)) < delayTicks)
  {
    __NOP();
  }
}

/**
  * @brief  Alarm A callback.
  * @param  RtcHandle: RTC handle
  * @return None
  */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *RtcHandle)
{
  UTIL_TIMER_IRQ_Handler();
}

/**
  * @brief SSRU subsecond register underflow callback. Indicates that the SSRU reaches 0 and wraps around
  * @note this happens every ~44 days when 1024ticks per seconds
  * @param  RtcHandle: RTC handle
  * @return None
  */
void HAL_RTCEx_SSRUEventCallback(RTC_HandleTypeDef *hrtc)
{
  /*called every 48 days with 1024 ticks per seconds*/
  GNSE_RTC_DBG_PRINTF(">>Handler SSRUnderflow at %d\n\r", GetTimerTicks());
  /*Increment MSBticks*/
  uint32_t MSB_ticks = GNSE_RTC_BkUp_Read_MSBticks();
  GNSE_RTC_BkUp_Write_MSBticks(MSB_ticks + 1);
}

uint32_t GNSE_RTC_GetTime(uint16_t *mSeconds)
{
  uint64_t ticks;
  uint32_t timerValueLsb = GetTimerTicks();
  uint32_t timerValueMSB = GNSE_RTC_BkUp_Read_MSBticks();

  ticks = (((uint64_t) timerValueMSB) << 32) + timerValueLsb;

  uint32_t seconds = (uint32_t)(ticks >> RTC_N_PREDIV_S);

  ticks = (uint32_t) ticks & RTC_PREDIV_S;

  *mSeconds = GNSE_RTC_Convert_Tick2ms(ticks);

  return seconds;
}

void GNSE_RTC_BkUp_Write_Seconds(uint32_t Seconds)
{
  HAL_RTCEx_BKUPWrite(&GNSE_BSP_rtc, RTC_BKP_SECONDS, Seconds);
}

void GNSE_RTC_BkUp_Write_SubSeconds(uint32_t SubSeconds)
{
  HAL_RTCEx_BKUPWrite(&GNSE_BSP_rtc, RTC_BKP_SUBSECONDS, SubSeconds);
}

uint32_t GNSE_RTC_BkUp_Read_Seconds(void)
{
  return HAL_RTCEx_BKUPRead(&GNSE_BSP_rtc, RTC_BKP_SECONDS);
}

uint32_t GNSE_RTC_BkUp_Read_SubSeconds(void)
{
  return HAL_RTCEx_BKUPRead(&GNSE_BSP_rtc, RTC_BKP_SUBSECONDS);
}

static void GNSE_RTC_BkUp_Write_MSBticks(uint32_t MSBticks)
{
  HAL_RTCEx_BKUPWrite(&GNSE_BSP_rtc, RTC_BKP_MSBTICKS, MSBticks);
}

static uint32_t GNSE_RTC_BkUp_Read_MSBticks(void)
{
  uint32_t MSBticks;
  MSBticks = HAL_RTCEx_BKUPRead(&GNSE_BSP_rtc, RTC_BKP_MSBTICKS);
  return MSBticks;
}

static inline uint32_t GetTimerTicks(void)
{
  return (UINT32_MAX - LL_RTC_TIME_GetSubSecond(RTC));
}

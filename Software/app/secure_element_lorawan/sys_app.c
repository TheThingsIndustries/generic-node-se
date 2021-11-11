/**
  ******************************************************************************
  * @file    sys_app.c
  * @author  MCD Application Team
  * @brief   Initializes HW and SW system entities (not related to the radio)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include <stdio.h>
#include "app.h"
#include "sys_app.h"
#include "stm32_seq.h"
#include "stm32_systime.h"
#include "GNSE_lpm.h"
#include "GNSE_rtc.h"
#include "GNSE_bm.h"
#include "sensors.h"

#define MAX_TS_SIZE (int)16

#if defined(IWDG_TIMER_ON) && (IWDG_TIMER_ON == 1)
/**
  * @brief Timer to handle refreshing of the IWDG
  */
static UTIL_TIMER_Object_t iwdg_refresh_timer;

/**
 * @brief Handler for periodically refreshing the IWDG
 */
static void GNSE_On_IWDG_Event(void *context);
#endif /* IWDG_TIMER_ON */

/**
  * @brief Returns sec and msec based on the systime in use
  * @param none
  * @return  none
  */
static void TimestampNow(uint8_t *buff, uint16_t *size);

/**
  * @brief  it calls ADV_TRACER_VSNPRINTF
  */
static void tiny_snprintf_like(char *buf, uint32_t maxsize, const char *strFormat, ...);

/**
  * @brief initialises the system (dbg pins, trace, mbmux, systimer, LPM, ...)
  * @param none
  * @return  none
  * TODO: Improve with system wide Init(), see https://github.com/TheThingsIndustries/generic-node-se/issues/57
  */
void SystemApp_Init(void)
{
  /* Ensure that MSI is wake-up system clock */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  /*Initialises timer and RTC*/
  UTIL_TIMER_Init();

  /* Initialize the Low Power Manager and Debugger */
#if defined(DEBUGGER_ON) && (DEBUGGER_ON == 1)
  GNSE_LPM_Init(GNSE_LPM_SLEEP_STOP_DEBUG_MODE);
#elif defined(DEBUGGER_ON) && (DEBUGGER_ON == 0)
  GNSE_LPM_Init(GNSE_LPM_SLEEP_STOP_MODE);
#endif

  /* Initialize Tracer/Logger */
  GNSE_TRACER_INIT();
  GNSE_TRACER_TIMESTAMP(TimestampNow);
  GNSE_app_printAppInfo();

  GNSE_BSP_LED_Init(LED_BLUE);
  GNSE_BSP_LED_Init(LED_RED);
  GNSE_BSP_LED_Init(LED_GREEN);

  GNSE_BSP_LED_On(LED_RED); // To indicated if something went wrong

  APP_PPRINTF("\r\n Powering and using HW secure element (ATECC608A-TNGLORA) \r\n");
  GNSE_BSP_LS_Init(LOAD_SWITCH_SENSORS);
  GNSE_BSP_LS_On(LOAD_SWITCH_SENSORS);
  HAL_Delay(LOAD_SWITCH_SENSORS_DELAY_MS);
  APP_PPRINTF("\r\n Initializing on-board sensors bus I2C1 \r\n");
  GNSE_BSP_Sensor_I2C1_Init();
  APP_PPRINTF("\r\n Initializing on-board sensors and LEDs \r\n");
  sensors_init();

#if defined(IWDG_TIMER_ON) && (IWDG_TIMER_ON == 1)
  uint32_t iwdg_reload_value = IWDG_MAX_RELOAD;
  /* 256 signifies the default prescaler set in GNSE_BSP_IWDG_Init, change if necessary */
  uint32_t iwdg_seq_timeout_ms = (iwdg_reload_value / (LSI_VALUE / 256U) * 1000U); //Every 32 Seconds

  GNSE_BSP_IWDG_Init(iwdg_reload_value);

  /* Create timer to refresh the IWDG timer before it triggers */
  UTIL_TIMER_Create(&iwdg_refresh_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, GNSE_On_IWDG_Event, NULL);
  UTIL_TIMER_SetPeriod(&iwdg_refresh_timer, iwdg_seq_timeout_ms);
  UTIL_TIMER_Start(&iwdg_refresh_timer);
#endif /* IWDG_TIMER_ON */

  GNSE_BSP_LED_Off(LED_RED);

  for (size_t counter = 0; counter < LED_STARTUP_TOGGEL; counter++)
  {
    GNSE_BSP_LED_Toggle(LED_BLUE);
    HAL_Delay(LED_STARTUP_DELAY);
    GNSE_BSP_LED_Toggle(LED_RED);
    HAL_Delay(LED_STARTUP_DELAY);
    GNSE_BSP_LED_Toggle(LED_GREEN);
    HAL_Delay(LED_STARTUP_DELAY);
  }
}

#if defined(IWDG_TIMER_ON) && (IWDG_TIMER_ON == 1)
/**
  * @brief  On refreshing timer event, refreshes watchdog and reinitialized the timer for next refresh
  * @return None
  */
static void GNSE_On_IWDG_Event(void *context)
{
  GNSE_BSP_IWDG_Refresh();
  UTIL_TIMER_Start(&iwdg_refresh_timer);
}
#endif /* IWDG_TIMER_ON */

void GNSE_LPM_PreStopModeHook(void)
{
  GNSE_LPM_SensorBus_Off();
}

void GNSE_LPM_PostStopModeHook(void)
{
  GNSE_TRACER_RESUME();
  GNSE_LPM_SensorBus_Resume();
  GNSE_LPM_BatteryADC_Resume();
}

/**
  * @brief redefines __weak function in stm32_seq.c such to enter low power
  * @param none
  * @return  none
  */
void UTIL_SEQ_Idle(void)
{
  GNSE_LPM_EnterLowPower();
}

static void TimestampNow(uint8_t *buff, uint16_t *size)
{
  SysTime_t curtime = SysTimeGet();
  tiny_snprintf_like((char *)buff, MAX_TS_SIZE, "%ds%03d:", curtime.Seconds, curtime.SubSeconds);
  *size = strlen((char *)buff);
}

/* Disable StopMode when traces need to be printed */
void ADV_TRACER_PreSendHook(void)
{
  GNSE_LPM_SetStopMode((1 << GNSE_LPM_UART_TRACER), GNSE_LPM_DISABLE);
}

/* Re-enable StopMode when traces have been printed */
void ADV_TRACER_PostSendHook(void)
{
  GNSE_LPM_SetStopMode((1 << GNSE_LPM_UART_TRACER), GNSE_LPM_ENABLE);
}

static void tiny_snprintf_like(char *buf, uint32_t maxsize, const char *strFormat, ...)
{
  va_list vaArgs;
  va_start(vaArgs, strFormat);
  ADV_TRACER_VSNPRINTF(buf, maxsize, strFormat, vaArgs);
  va_end(vaArgs);
}

/**
  * @brief This function configures the source of the time base.
  * @brief  don't enable systick
  * @param TickPriority: Tick interrupt priority.
  * @return HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /*Initialize the RTC services */
  return HAL_OK;
}

/**
  * @brief Provide a tick value in millisecond measured using RTC
  * @note This function overwrites the __weak one from HAL
  * @return tick value
  */
uint32_t HAL_GetTick(void)
{
  return GNSE_RTC_GetTimerValue();
}

/**
  * @brief This function provides delay (in ms)
  * @param Delay: specifies the delay time length, in milliseconds.
  * @return None
  */
void HAL_Delay(__IO uint32_t Delay)
{
  GNSE_RTC_DelayMs(Delay); /* based on RTC */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

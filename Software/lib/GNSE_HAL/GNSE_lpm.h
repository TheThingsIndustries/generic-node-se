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
 * @file GNSE_lpm.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_LPM_H
#define GNSE_LPM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/**
 * @brief macro used to enter the critical section
 */
#define GNSE_LPM_ENTER_CRITICAL_SECTION( )    uint32_t primask_bit= __get_PRIMASK();\
  __disable_irq()

/**
 * @brief macro used to exit the critical section
 */
#define GNSE_LPM_EXIT_CRITICAL_SECTION( )     __set_PRIMASK(primask_bit)

/**
 * @brief value used to reset the LPM mode
 */
#define GNSE_LPM_NO_BIT_SET   (0UL)

/**
 * @brief type definition to represent the bit mask of an LPM mode
 */
typedef uint32_t GNSE_LPM_bm_t;

/**
 * Supported requester to the GNSE Low Power Manager - can be increased up to 32
 * It lists a bit mapping of all user of the Low Power Manager
 */
typedef enum
{
  GNSE_LPM_UART_TRACER,
  GNSE_LPM_I2C_SENSORS,
  GNSE_LPM_TIM_BUZZER,
  GNSE_LPM_ADC_BM,
  GNSE_LPM_LIB,
  GNSE_LPM_APP,
} GNSE_LPM_Id_t;

/**
 * @brief type definition to represent value of an LPM mode
 */
typedef enum
{
  GNSE_LPM_DISABLE=0,
  GNSE_LPM_ENABLE,
} GNSE_LPM_State_t;

/**
 * @brief type definition to represent the different type of LPM mode
 */

typedef enum
{
  GNSE_LPM_SLEEP_ONLY_MODE,
  GNSE_LPM_SLEEP_DEBUG_MODE,
  GNSE_LPM_SLEEP_STOP_MODE,
  GNSE_LPM_SLEEP_STOP_DEBUG_MODE,
} GNSE_LPM_Mode_t;

void GNSE_LPM_Init(GNSE_LPM_Mode_t init_mode);
void GNSE_LPM_DeInit(void);

void GNSE_LPM_Debugger_Enable(void);
void GNSE_LPM_Debugger_Disable(void);

void GNSE_LPM_SensorBus_Resume(void);
void GNSE_LPM_SensorBus_Off(void);

void GNSE_LPM_BatteryADC_Resume(void);

void GNSE_LPM_PreStopModeHook(void);
void GNSE_LPM_PostStopModeHook(void);
void GNSE_LPM_PreSleepModeHook(void);
void GNSE_LPM_PostSleepModeHook(void);

void GNSE_LPM_SetStopMode(GNSE_LPM_bm_t lpm_id_bm, GNSE_LPM_State_t state);
GNSE_LPM_bm_t GNSE_LPM_GetStopMode(void);

void GNSE_LPM_EnterLowPower(void);

#ifdef __cplusplus
}
#endif

#endif /*GNSE_LPM_H*/

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
 * @file app.h
 * @brief Common file for GNSE applications
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef APP_H
#define APP_H

/**
 * Utilities
 */
#include <stdio.h>
#include <stdint.h>
#include "stm32_adv_trace.h"

/**
 * GNSE Libraries
 */
#include "GNSE_bsp.h"
#include "SHTC3.h"
#include "MX25R16.h"
#include "LIS2DH12.h"
#include "BUZZER.h"

/**
 * Application helpers
 */
#include "app_conf.h"

#if defined (APP_LOG_ENABLED) && (APP_LOG_ENABLED == 1)
#define APP_PPRINTF(...)  do{ } while( UTIL_ADV_TRACE_OK != UTIL_ADV_TRACE_COND_FSend(VLEVEL_OFF, T_REG_OFF, TS_OFF, __VA_ARGS__) ) //Polling Mode
#define APP_TPRINTF(...)   do{ {UTIL_ADV_TRACE_COND_FSend(VLEVEL_OFF, T_REG_OFF, TS_ON, __VA_ARGS__);} }while(0); //with timestamp
#define APP_PRINTF(...)   do{ {UTIL_ADV_TRACE_COND_FSend(VLEVEL_OFF, T_REG_OFF, TS_OFF, __VA_ARGS__);} }while(0);
#else
#define APP_PPRINTF(...)
#define APP_TPRINTF(...)
#define APP_PRINTF(...)
#endif


#if defined (APP_LOG_ENABLED) && (APP_LOG_ENABLED == 1)
#define APP_LOG(TS,VL,...)   do{ {UTIL_ADV_TRACE_COND_FSend(VL, T_REG_OFF, TS, __VA_ARGS__);} }while(0);
#elif defined (APP_LOG_ENABLED) && (APP_LOG_ENABLED == 0) /* APP_LOG disabled */
#define APP_LOG(TS,VL,...)
#else
#error "APP_LOG_ENABLED not defined or out of range <0,1>"
#endif

#endif /* APP_H */

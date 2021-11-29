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
 * @file GNSE_tracer.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_TRACER_H
#define GNSE_TRACER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_adv_tracer.h"
#include "stm32_adv_usart.h"
#include "tiny_printf.h"
#include "app_conf.h"

#if defined (GNSE_ADVANCED_TRACER_ENABLE) && (GNSE_ADVANCED_TRACER_ENABLE == 1)

#define GNSE_TRACER_INIT()                     do{ {ADV_TRACER_Init(); \
                                                    ADV_TRACER_SetVerboseLevel(ADV_TRACER_VLEVEL_H);}} while(0);
#define GNSE_TRACER_VERBOSE(V_LEVEL)           do{ {ADV_TRACER_SetVerboseLevel(V_LEVEL);}} while(0);
#define GNSE_TRACER_START_RX(RX_CB)            do{ {ADV_TRACER_StartRxProcess(RX_CB);}} while(0);
#define GNSE_TRACER_TIMESTAMP(TS_CB)           do{ {ADV_TRACER_RegisterTimeStampFunction(TS_CB);}} while(0);
#define GNSE_TRACER_RESUME()                   do{ {vcom_Resume();}} while(0);
#define GNSE_TRACER_DEINIT()                   do{ {ADV_TRACER_DeInit();}} while(0);

#define APP_PPRINTF(...)                do{ } while( ADV_TRACER_OK != ADV_TRACER_COND_FSend(ADV_TRACER_VLEVEL_ALWAYS, ADV_TRACER_T_REG_OFF, ADV_TRACER_TS_OFF, __VA_ARGS__) ) //Polling Mode
#define APP_TPRINTF(...)                do{ {ADV_TRACER_COND_FSend(ADV_TRACER_VLEVEL_ALWAYS, ADV_TRACER_T_REG_OFF, ADV_TRACER_TS_ON, __VA_ARGS__);}} while(0); //with timestamp
#define APP_PRINTF(...)                 do{ {ADV_TRACER_COND_FSend(ADV_TRACER_VLEVEL_ALWAYS, ADV_TRACER_T_REG_OFF, ADV_TRACER_TS_OFF, __VA_ARGS__);}} while(0);
#define LIB_PRINTF(...)                 do{ } while( ADV_TRACER_OK != ADV_TRACER_COND_FSend(ADV_TRACER_VLEVEL_ALWAYS, ADV_TRACER_T_REG_OFF, ADV_TRACER_TS_OFF, __VA_ARGS__) ) //Polling Mode
#define APP_LOG(TS,VL,...)              do{ {ADV_TRACER_COND_FSend(VL, ADV_TRACER_T_REG_OFF, TS, __VA_ARGS__);}} while(0);
#define LIB_LOG(TS,VL,...)              do{ {ADV_TRACER_COND_FSend(VL, ADV_TRACER_T_REG_OFF, TS, __VA_ARGS__);}} while(0);
;

#elif defined (GNSE_TINY_TRACER_ENABLE) && (GNSE_TINY_TRACER_ENABLE == 1)

#define GNSE_TRACER_INIT()              do{ {GNSE_BSP_USART_Init();}} while(0);
#define GNSE_TRACER_VERBOSE(V_LEVEL)
#define GNSE_TRACER_START_RX(RX_CB)
#define GNSE_TRACER_TIMESTAMP(TS_CB)
#define GNSE_TRACER_RESUME()            do{ {GNSE_BSP_USART_Init();}} while(0);
#define GNSE_TRACER_DEINIT()

#define APP_PPRINTF(...)                do{ {tiny_printf(__VA_ARGS__);}} while(0);
#define APP_TPRINTF(...)                do{ {tiny_printf(__VA_ARGS__);}} while(0);
#define APP_PRINTF(...)                 do{ {tiny_printf(__VA_ARGS__);}} while(0);
#define LIB_PRINTF(...)                 do{ {tiny_printf(__VA_ARGS__);}} while(0);
#define APP_LOG(TS,VL,...)              do{ {tiny_printf(__VA_ARGS__);}} while(0);
#define LIB_LOG(TS,VL,...)              do{ {tiny_printf(__VA_ARGS__);}} while(0);


#else
#define GNSE_TRACER_INIT()
#define GNSE_TRACER_VERBOSE(V_LEVEL)
#define GNSE_TRACER_START_RX(RX_CB)
#define GNSE_TRACER_TIMESTAMP(TS_CB)
#define GNSE_TRACER_RESUME()
#define GNSE_TRACER_DEINIT()

#define APP_PPRINTF(...)
#define APP_TPRINTF(...)
#define APP_PRINTF(...)
#define LIB_PRINTF(...)
#define APP_LOG(TS,VL,...)
#define LIB_LOG(TS,VL,...)

#endif

#endif  /** GNSE_TRACER_H **/

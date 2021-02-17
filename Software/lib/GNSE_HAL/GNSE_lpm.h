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

#include "stm32_lpm.h"

void GNSE_LPM_EnterOffMode(void);
void GNSE_LPM_ExitOffMode(void);
void GNSE_LPM_EnterStopMode(void);
void GNSE_LPM_ExitStopMode(void);
void GNSE_LPM_EnterSleepMode(void);
void GNSE_LPM_ExitSleepMode(void);

#ifdef __cplusplus
}
#endif

#endif /*GNSE_LPM_H*/

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
 * @file STNODE_lpm.h
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#ifndef STNODE_LPM_H
#define STNODE_LPM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_lpm.h"

/**
  * @brief Enters Low Power Off Mode
  * @param none
  * @return none
  */
void STNODE_LPM_EnterOffMode(void);
/**
  * @brief Exits Low Power Off Mode
  * @param none
  * @return none
  */
void STNODE_LPM_ExitOffMode(void);

/**
  * @brief Enters Low Power Stop Mode
  * @note ARM exists the function when waking up
  * @param none
  * @return none
  */
void STNODE_LPM_EnterStopMode(void);
/**
  * @brief Exits Low Power Stop Mode
  * @note Enable the pll at 32MHz
  * @param none
  * @return none
  */
void STNODE_LPM_ExitStopMode(void);

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @return none
  */
void STNODE_LPM_EnterSleepMode(void);

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @return none
  */
void STNODE_LPM_ExitSleepMode(void);

#ifdef __cplusplus
}
#endif

#endif /*STNODE_LPM_H*/

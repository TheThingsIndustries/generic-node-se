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
 * @file freefall.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef FREEFALL_H
#define FREEFALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GNSE_acc.h"
#include "LmHandler.h"

/**
  * @brief  Sets the accelerometer registers to detect free fall events
  * @param  none
  * @return ACC_op_result_t
  */
ACC_op_result_t ACC_FreeFall_Enable(void);

/**
  * @brief  Resets free fall event detection
  * @note   This function only turns off interrupt detection, the accelerometer will still generate interrupts
  * @param  none
  * @return ACC_op_result_t
  */
ACC_op_result_t ACC_FreeFall_Disable(void);

/**
  * @brief  Handle free fall interrupts
  * @param  none
  * @return none
  */
void ACC_FreeFall_IT_Handler(void);

/**
  * @brief  Handle free fall downlink events
  * @param  rx_data: Data pointer
  * @return none
  */
void ACC_FreeFall_Downlink_Handler(LmHandlerAppData_t *rx_data);


/**
  * @brief  Function run after pressing the button, turns off all peripherals set after downlink
  * @return none
  */
void ACC_Disable_FreeFall_Notification(void);

#ifdef __cplusplus
}
#endif

#endif /* FREEFALL_H */

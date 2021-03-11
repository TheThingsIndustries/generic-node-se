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
 * @file GNSE_acc.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_ACC_H
#define GNSE_ACC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GNSE_bsp_error.h"
#include "GNSE_bsp_gpio.h"

/**
 * Accelerometer return types
 */
typedef enum
{
    ACC_OP_SUCCESS = 0,
    ACC_OP_FAIL = 1,
} ACC_op_result_t;

/**
  * @brief  Initialises the accelerometer hardware
  * @param  none
  * @return ACC_op_result_t
  */
ACC_op_result_t GNSE_ACC_Init(void);

/**
  * @brief  Deinitialises the accelerometer hardware
  * @param  none
  * @return ACC_op_result_t
  */
ACC_op_result_t GNSE_ACC_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /*GNSE_ACC_H*/

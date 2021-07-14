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
 * @file GNSE_error.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_ERROR_H
#define GNSE_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  Generic error handler
  * @note   Weak implementation that can be overridden by the application layer
  * @param  none
  * @return none
  */
void GNSE_Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /*GNSE_ACC_H*/

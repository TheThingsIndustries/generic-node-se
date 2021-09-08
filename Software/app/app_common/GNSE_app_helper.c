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
 * @file GNSE_app_helper.c
 * @brief application helper for GNSE applications
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_app_helper.h"

void GNSE_app_printAppInfo(void)
{
    APP_PPRINTF("\r\n *********> Running GNSE %s app <******** \r\n", GNSE_APP_NAME);
    APP_PPRINTF("\r\n *********> Compiled on: %s , %s <******** \r\n", __TIME__, __DATE__);
}

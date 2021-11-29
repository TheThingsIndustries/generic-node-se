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
 * Standard libraries
 */
#include <stdio.h>
#include <stdint.h>

/**
 * GNSE libraries
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
#include "GNSE_tracer.h"
#include "GNSE_app_helper.h"

#endif /* APP_H */

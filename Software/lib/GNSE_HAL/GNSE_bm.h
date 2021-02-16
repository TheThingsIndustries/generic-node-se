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
 * @file GNSE_bm.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_BM_H
#define GNSE_BM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GNSE_bsp.h"

/*
 * Reference calibration value if we don't use VREFINT_CAL_ADDR production calibration value
 * Calibration value reference temperature is 30'c
 */
#define GNSE_BM_VREFINT_CAL (VREFINT_CAL_VREF * 1510UL)

/*
 * RP605Z283B Battery Monitor division ratio of the battery voltage
 * The converted ADC value represents the battery voltage divided by the ratio
 */
#define GNSE_BM_OUTPUT_DIVISION_RATIO 3U

uint16_t GNSE_BM_GetInternalRefVoltage();
uint16_t GNSE_BM_GetBatteryVoltage();

#ifdef __cplusplus
}
#endif

#endif /*GNSE_BM_H*/

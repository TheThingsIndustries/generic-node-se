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
 * @file GNSE_bm.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_bm.h"

/**
 * @brief Gets the MCU internal refernce voltage
 *
 * @return uint16_t of the calibrated reference voltage in millivolt (mv)
 */
uint16_t GNSE_BM_GetInternalRefVoltage(void)
{
    uint16_t internal_ref_mV = 0;
    uint32_t adc_measurement = 0;

    GNSE_BSP_BM_ConfChannel(VREF_ADC_CHANNEL);

    adc_measurement = GNSE_BSP_BM_ReadChannel();
    if (adc_measurement != 0)
    {
        /**
             * We can use multiple methonds if the SOC is calibrated in production
             * (uint32_t)*VREFINT_CAL_ADDR
             * __LL_ADC_CALC_VREFANALOG_VOLTAGE(measuredLevel, ADC_RESOLUTION_12B)
             */
        internal_ref_mV = GNSE_BM_VREFINT_CAL / adc_measurement;
    }
    return internal_ref_mV;
}

/**
 * @brief Gets the battery voltage level
 *
 * @return uint16_t of the battery voltage in millivolt (mv)
 */
uint16_t GNSE_BM_GetBatteryVoltage(void)
{
    uint16_t battery_mV = 0;
    uint16_t internal_ref_mV = 0;
    uint32_t adc_measurement = 0;
    internal_ref_mV = GNSE_BM_GetInternalRefVoltage();
    GNSE_BSP_BM_ConfChannel(VBAT_ADC_CHANNEL);
    adc_measurement = GNSE_BSP_BM_ReadChannel();
    if (adc_measurement != 0)
    {
        battery_mV = __LL_ADC_CALC_DATA_TO_VOLTAGE(internal_ref_mV, adc_measurement, VBAT_ADC_RES);
        battery_mV = battery_mV * GNSE_BM_OUTPUT_DIVISION_RATIO;
    }
    return battery_mV;
}

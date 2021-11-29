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
 * @file basic_battery.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "GNSE_bm.h"

void battery_report(void)
{
    uint16_t mcu_internal_voltage = 0;
    uint16_t battery_voltage = 0;

    GNSE_BSP_BM_Init();
    GNSE_BSP_BM_Enable();
    HAL_Delay(100);
    mcu_internal_voltage = GNSE_BM_GetInternalRefVoltage();
    battery_voltage = GNSE_BM_GetBatteryVoltage();
    APP_PPRINTF("\r\n MCU internal reference voltage %d mv \r\n", mcu_internal_voltage);
    APP_PPRINTF("\r\n Battery voltage %d mv \r\n", battery_voltage);
}

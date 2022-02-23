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
 * @file sensors.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "GNSE_bm.h"
#include "sensors.h"

sensors_op_result_t sensors_init(void)
{
    if (GNSE_BSP_BM_Init() != GNSE_BSP_ERROR_NONE)
    {
        APP_PPRINTF("\r\n Failed to initialize battery monitor ADC \r\n");
        return SENSORS_OP_FAIL;
    }
    GNSE_BSP_BM_Enable();
    if (SHTC3_probe() != SHTC3_STATUS_OK)
    {
        APP_PPRINTF("\r\n Failed to initialize SHTC3 sensor \r\n");
        return SENSORS_OP_FAIL;
    }
    APP_PPRINTF("\r\n Successfully intialized all sensors \r\n");
    return SENSORS_OP_SUCCESS;
}

sensors_op_result_t sensors_sample(sensors_t *sensor_data)
{
    int16_t status = 0;
    sensor_data->battery_voltage = GNSE_BM_GetBatteryVoltage();
    status = SHTC3_measure_blocking_read(&sensor_data->temperature, &sensor_data->humidity);
    if (status != SHTC3_STATUS_OK)
    {
        APP_PPRINTF("\r\n Failed to read data from SHTC3 sensor, Error status: %d \r\n", status);
        return SENSORS_OP_FAIL;
    }
    APP_PPRINTF("\r\n Successfully sampled sensors \r\n");
    return SENSORS_OP_SUCCESS;
}

uint32_t sensors_downlink_conf_check(LmHandlerAppData_t *appData)
{
    uint32_t rxbuffer = 0;
    if (appData->Port == SENSORS_DOWNLINK_CONF_PORT)
    {
      if (appData->BufferSize <= sizeof(rxbuffer))
      {
        for (int i = appData->BufferSize - 1; i >= 0; i--)
        {
            rxbuffer += appData->Buffer[i] << (8 * (appData->BufferSize - 1 - i));
        }
        if (rxbuffer <= SENSORS_DUTYCYCLE_CONF_MAX_S && rxbuffer >= SENSORS_DUTYCYCLE_CONF_MIN_S)
        {
          APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n RX time changed to %ld seconds \r\n", rxbuffer);
          rxbuffer *= 1000; /* Time data has to be converted from s to ms */
          return rxbuffer;
        }
      }
    }
    return 0;
}

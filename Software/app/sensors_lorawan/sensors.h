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
 * @file sensors.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef __SENSORS_H__
#define __SENSORS_H__

typedef enum
{
    SENSORS_OP_SUCCESS = 0,
    SENSORS_OP_FAIL = 1,
} sensors_op_result_t;

typedef struct
{
    uint16_t battery_voltage;
    int32_t temperature;
    int32_t humidity;
} sensors_t;

/**
 * @brief Initializes the sensors that will be sampled
 *
 * @return sensors_op_result_t
 */
sensors_op_result_t sensors_init(void);

/**
 * @brief Retrieve and sample sensor data
 *
 * @param sensor_data passed reference sensor data
 * @return sensors_op_result_t
 */
sensors_op_result_t sensors_sample(sensors_t *sensor_data);

#endif /* __SENSORS_H__ */

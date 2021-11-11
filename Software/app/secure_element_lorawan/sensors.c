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
#include "LIS2DH12.h"
#include "sensors.h"

static lis2dh12_int1_src_t acc_int_src;
static stmdev_ctx_t dev_ctx;

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
    if (GNSE_ACC_Init() != ACC_OP_SUCCESS)
    {
        APP_PPRINTF("\r\n Accelerometer failed to initialize properly \r\n");
    }
    APP_PPRINTF("\r\n Successfully intialized all sensors \r\n");

    return SENSORS_OP_SUCCESS;
}

sensors_op_result_t temperature_sample(sensors_t *sensor_data)
{
    int16_t status = 0;
    status = SHTC3_measure_blocking_read(&sensor_data->temperature, &sensor_data->humidity);
    if (status != SHTC3_STATUS_OK)
    {
        APP_PPRINTF("\r\n Failed to read data from SHTC3 sensor, Error status: %d \r\n", status);
        return SENSORS_OP_FAIL;
    }
    APP_PPRINTF("\r\n Successfully sampled sensors \r\n");
    return SENSORS_OP_SUCCESS;
}

ACC_op_result_t ACC_FreeFall_Enable(void)
{
    int8_t acc_check;

    acc_check = LIS2DH12_init(&dev_ctx);
    /* Set Output Data rate */
    acc_check += (int8_t)lis2dh12_data_rate_set(&dev_ctx, ACC_FF_ODR);

    /* Set full scale */
    acc_check += (int8_t)lis2dh12_full_scale_set(&dev_ctx, ACC_FF_SCALE);

    /* Map interrupt 1 on INT2 pin */
    lis2dh12_ctrl_reg6_t ctrl6_set = {
        .not_used_01 = 0,
        .int_polarity = 0,
        .not_used_02 = 0,
        .i2_act = 0,
        .i2_boot = 0,
        .i2_ia2 = 0,
        .i2_ia1 = 1,
        .i2_click = 0};
    acc_check += (int8_t)lis2dh12_pin_int2_config_set(&dev_ctx, &ctrl6_set);

    /* Set interrupt threshold */
    acc_check += (int8_t)lis2dh12_int1_gen_threshold_set(&dev_ctx, ACC_FF_THRESHOLD);

    /* Set interrupt threshold duration */
    acc_check += (int8_t)lis2dh12_int1_gen_duration_set(&dev_ctx, ACC_FF_DURATION);

    /* Set all axes with low event detection and AND operator */
    lis2dh12_int1_cfg_t accel_cfg = {
        .xlie = 1,
        .xhie = 0,
        .ylie = 1,
        .yhie = 0,
        .zlie = 1,
        .zhie = 0,
        ._6d = 0,
        .aoi = 1};
    acc_check += (int8_t)lis2dh12_int1_gen_conf_set(&dev_ctx, &accel_cfg);

    /* Set low power, 8 bit data output mode */
    acc_check += (int8_t)lis2dh12_operating_mode_set(&dev_ctx, LIS2DH12_LP_8bit);

    /* See if all checks were passed */
    if (acc_check != 0)
    {
        return ACC_OP_FAIL;
    }

    /* Set interrupt pin */
    if (GNSE_BSP_Acc_Int_Init() != GNSE_BSP_ERROR_NONE)
    {
        return ACC_OP_FAIL;
    }

    return ACC_OP_SUCCESS;
}

ACC_op_result_t ACC_Shake_Enable(void)
{
    int8_t acc_check;

    acc_check = LIS2DH12_init(&dev_ctx);
    /* Set Output Data rate */
    acc_check += (int8_t)lis2dh12_data_rate_set(&dev_ctx, ACC_SHAKE_ODR);

    /* Set full scale */
    acc_check += (int8_t)lis2dh12_full_scale_set(&dev_ctx, ACC_SHAKE_SCALE);

    /* Map interrupt 1 on INT2 pin */
    lis2dh12_ctrl_reg6_t ctrl6_set = {
        .not_used_01 = 0,
        .int_polarity = 0,
        .not_used_02 = 0,
        .i2_act = 0,
        .i2_boot = 0,
        .i2_ia2 = 0,
        .i2_ia1 = 1,
        .i2_click = 0};
    acc_check += (int8_t)lis2dh12_pin_int2_config_set(&dev_ctx, &ctrl6_set);

    /* Set interrupt threshold */
    acc_check += (int8_t)lis2dh12_int1_gen_threshold_set(&dev_ctx, ACC_SHAKE_THRESHOLD);

    /* Set interrupt threshold duration */
    acc_check += (int8_t)lis2dh12_int1_gen_duration_set(&dev_ctx, ACC_SHAKE_DURATION);

    /* Set all axes with low event detection and AND operator */
    lis2dh12_int1_cfg_t accel_cfg = {
        .xlie = 0,
        .xhie = 1,
        .ylie = 0,
        .yhie = 1,
        .zlie = 0,
        .zhie = 0,
        ._6d = 0,
        .aoi = 0};
    acc_check += (int8_t)lis2dh12_int1_gen_conf_set(&dev_ctx, &accel_cfg);

    /* Set low power, 8 bit data output mode */
    acc_check += (int8_t)lis2dh12_operating_mode_set(&dev_ctx, LIS2DH12_LP_8bit);

    /* See if all checks were passed */
    if (acc_check != 0)
    {
        return ACC_OP_FAIL;
    }

    /* Set interrupt pin */
    if (GNSE_BSP_Acc_Int_Init() != GNSE_BSP_ERROR_NONE)
    {
        return ACC_OP_FAIL;
    }

    acc_check += (int8_t)lis2dh12_int1_gen_conf_set(&dev_ctx, &accel_cfg);

    return ACC_OP_SUCCESS;
}

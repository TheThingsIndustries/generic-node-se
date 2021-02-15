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
 * @file freefall.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "freefall.h"

static int8_t freefall_registers_init(void);

stmdev_ctx_t dev_ctx;

int8_t freefall_init(void) 
{
    /* Set load switch */
    GNSE_BSP_LS_Init(LOAD_SWITCH_SENSORS);
    GNSE_BSP_LS_On(LOAD_SWITCH_SENSORS);
    HAL_Delay(100);

    GNSE_BSP_Sensor_I2C1_Init();
    HAL_Delay(100);

    /* Set interrupt pin */
    if (GNSE_BSP_Acc_Int_Init())
    {
        return 1;
    }
    /* Set registers */
    if (freefall_registers_init())
    {
        return 1;
    }
    
    return 0;
}

static int8_t freefall_registers_init(void)
{
    int8_t acc_check;
    uint8_t whoami;
    
    acc_check = LIS2DH12_init(&dev_ctx);

    /* Check device ID */
    acc_check += (int8_t)lis2dh12_device_id_get(&dev_ctx, &whoami);
    if (whoami != LIS2DH12_ID)
    {
        return 1;
    }
    /* Set Output Data rate */
    acc_check += (int8_t)lis2dh12_data_rate_set(&dev_ctx, FF_ODR);

    /* Set full scale */
    acc_check += (int8_t)lis2dh12_full_scale_set(&dev_ctx, FF_SCALE);


    /* Map interrupt 1 on INT2 pin */
    lis2dh12_ctrl_reg6_t ctrl6_set = {
        .not_used_01 = 0,
        .int_polarity = 0,
        .not_used_02 = 0,
        .i2_act = 0,
        .i2_boot = 0,
        .i2_ia2 = 0,
        .i2_ia1 = 1,
        .i2_click = 0
    };
    acc_check += (int8_t)lis2dh12_pin_int2_config_set(&dev_ctx, &ctrl6_set); 

    /* Set interrupt threshold */
    acc_check += (int8_t)lis2dh12_int1_gen_threshold_set(&dev_ctx, FF_THRESHOLD); 

    /* Set interrupt threshold duration */
    acc_check += (int8_t)lis2dh12_int1_gen_duration_set(&dev_ctx, FF_DURATION); 

    /* Set all axes with low event detection and AND operator */
    lis2dh12_int1_cfg_t accel_cfg = {
        .xlie = 1,
        .xhie = 0,
        .ylie = 1,
        .yhie = 0,
        .zlie = 1,
        .zhie = 0,
        ._6d = 0,
        .aoi = 1
    };
    acc_check += (int8_t)lis2dh12_int1_gen_conf_set(&dev_ctx, &accel_cfg); 

    /* See if all checks were passed */
    if (acc_check != 0)
    {
        return 1;
    }
    
    return 0;
}

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
#include "LIS2DH12.h"
#include "app_conf.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "GNSE_lpm.h"
#include "BUZZER.h"

static void ACC_Downlink_Callback(void *context);

static UTIL_TIMER_Object_t DownlinkTimer;

ACC_op_result_t ACC_FreeFall_Disable(void)
{
    if (GNSE_BSP_Acc_Int_DeInit() != GNSE_BSP_ERROR_NONE)
    {
        return ACC_OP_FAIL;
    }
    return ACC_OP_SUCCESS;
}

ACC_op_result_t ACC_FreeFall_Enable(void)
{
    int8_t acc_check;
    stmdev_ctx_t dev_ctx;

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
        .i2_click = 0
    };
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
        .aoi = 1
    };
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

void ACC_FreeFall_IT_Handler(void)
{
    static uint8_t freefall_log_amount;
    LoRaWAN_Send_Payload(&freefall_log_amount, sizeof(freefall_log_amount));
    freefall_log_amount++;
}

void ACC_FreeFall_Downlink_Handler(LmHandlerAppData_t *rx_data)
{
/**
 * User can replace this function with their own implementation
 * Default will start a timer for a buzzer if any data is send over ACC_FF_DOWNLINK_PORT
*/
    if (rx_data->Port == ACC_FF_DOWNLINK_PORT)
    {
        UTIL_TIMER_Create(&DownlinkTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, ACC_Downlink_Callback, NULL);
        UTIL_TIMER_SetPeriod(&DownlinkTimer, ACC_FF_DOWNLINK_TIME_MS);
        UTIL_TIMER_Start(&DownlinkTimer);
        BUZZER_SetState(BUZZER_STATE_OFF);
    }
}

static void ACC_Downlink_Callback(void *context)
{
    /*
     * User can change this function with any indication
     * Default is a buzzer beeping
     * Frequency of this callback can be set in ACC_FF_DOWNLINK_TIME_MS
     */
    static bool buzzer_state = true;
    if (buzzer_state)
    {
        /* Buzzer requires the LPM to be switched off */
        GNSE_LPM_SetStopMode((1 << GNSE_LPM_TIM_BUZZER), GNSE_LPM_DISABLE);
        BUZZER_SetState(BUZZER_STATE_DANGER);
    }
    else
    {
        BUZZER_SetState(BUZZER_STATE_OFF);
        BUZZER_DeInit();
        GNSE_LPM_SetStopMode((1 << GNSE_LPM_TIM_BUZZER), GNSE_LPM_ENABLE);
    }
    buzzer_state = !buzzer_state;

    /* Set next timer event */
    UTIL_TIMER_Start(&DownlinkTimer);
}

void ACC_Disable_FreeFall_Notification(void)
{
    BUZZER_SetState(BUZZER_STATE_OFF);
    BUZZER_DeInit();
    UTIL_TIMER_Stop(&DownlinkTimer);
    GNSE_LPM_SetStopMode((1 << GNSE_LPM_TIM_BUZZER), GNSE_LPM_ENABLE);
}

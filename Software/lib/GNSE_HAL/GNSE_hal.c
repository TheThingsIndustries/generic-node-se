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
 * @file GNSE_hal.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_hal.h"
#include "GNSE_acc.h"
#include "GNSE_bm.h"
#include "GNSE_flash.h"
#include "SHTC3.h"
#include "BUZZER.h"

/**
  * @brief Initialises the internal sensors for GNSE, the accelerometer and humidity/temperature sensor
  * @note I2C interface should be initialised before this function
  * @param none
  * @return none
  */
void GNSE_HAL_Internal_Sensors_Init(void)
{
    GNSE_BSP_LS_Init(LOAD_SWITCH_SENSORS);
    GNSE_BSP_LS_On(LOAD_SWITCH_SENSORS);
    HAL_Delay(LOAD_SWITCH_SENSORS_DELAY_MS);

    GNSE_ACC_Init();
    SHTC3_probe();
}

/**
  * @brief Deinitialises the internal sensors for GNSE, the accelerometer and humidity/temperature sensor
  * @note The I2C interface is not turned off by this function
  * @param none
  * @return none
  */
void GNSE_HAL_Internal_Sensors_DeInit(void)
{
    GNSE_ACC_DeInit();
    GNSE_BSP_LS_Off(LOAD_SWITCH_SENSORS);
    GNSE_BSP_LS_DeInit(LOAD_SWITCH_SENSORS);
}

/**
  * @brief Initialises all common GNSE hardware
  * @param GNSE_HAL_Ctx_t gnse_inits: Interfaces to configure
  * @return none
  */
void GNSE_HAL_Init(GNSE_HAL_Ctx_t gnse_inits)
{
    if (gnse_inits.internal_sensors_init)
    {
        GNSE_BSP_Sensor_I2C1_Init();
        GNSE_HAL_Internal_Sensors_Init();
    }
    if (gnse_inits.external_sensors_init)
    {
        GNSE_BSP_Ext_Sensor_I2C2_Init();
    }
    if (gnse_inits.flash_init)
    {
        GNSE_Flash_Init();
    }
    if (gnse_inits.bm_init)
    {
        GNSE_BM_Init();
    }
    if (gnse_inits.leds_init)
    {
        GNSE_BSP_LED_Init(LED_BLUE);
        GNSE_BSP_LED_Init(LED_RED);
        GNSE_BSP_LED_Init(LED_GREEN);
    }
    if (gnse_inits.buzzer_init)
    {
        BUZZER_Init();
    }
}

/**
  * @brief Deinitialises all common GNSE hardware
  * @param GNSE_HAL_Ctx_t gnse_deinits: Interfaces to deconfigure
  * @return none
  */
void GNSE_HAL_DeInit(GNSE_HAL_Ctx_t gnse_deinits)
{

    if (gnse_deinits.internal_sensors_init)
    {
        GNSE_BSP_Sensor_I2C1_DeInit();
        GNSE_HAL_Internal_Sensors_DeInit();
    }
    if (gnse_deinits.external_sensors_init)
    {
        GNSE_BSP_Ext_Sensor_I2C2_DeInit();
    }
    if (gnse_deinits.flash_init)
    {
        GNSE_Flash_DeInit();
    }
    if (gnse_deinits.bm_init)
    {
        GNSE_BM_DeInit();
    }
    if (gnse_deinits.leds_init)
    {
        GNSE_BSP_LED_DeInit(LED_BLUE);
        GNSE_BSP_LED_DeInit(LED_RED);
        GNSE_BSP_LED_DeInit(LED_GREEN);
    }
    if (gnse_deinits.buzzer_init)
    {
        BUZZER_DeInit();
    }
}

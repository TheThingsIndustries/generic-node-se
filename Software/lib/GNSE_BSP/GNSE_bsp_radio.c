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
 * @file GNSE_radio.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_bsp_radio.h"

SUBGHZ_HandleTypeDef hsubghz;

int32_t GNSE_BSP_SUBGHZ_Init(void)
{
    hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
    if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK)
    {
        return GNSE_BSP_ERROR_NO_INIT;
    }
    return GNSE_BSP_ERROR_NONE;
}

int32_t RBI_Init(void)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    /* Enable the Radio Switch Clock */
    RF_SW_CTRL3_GPIO_CLK_ENABLE();

    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin = RF_SW_CTRL1_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(RF_SW_CTRL1_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = RF_SW_CTRL2_PIN;
    HAL_GPIO_Init(RF_SW_CTRL2_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = RF_SW_CTRL3_PIN;
    HAL_GPIO_Init(RF_SW_CTRL3_GPIO_PORT, &gpio_init_structure);

    HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET);

    return 0;
}

int32_t RBI_DeInit(void)
{
    RF_SW_CTRL3_GPIO_CLK_ENABLE();

    /* Turn off switch */
    HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET);

    /* DeInit the Radio Switch pin */
    HAL_GPIO_DeInit(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN);
    HAL_GPIO_DeInit(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN);
    HAL_GPIO_DeInit(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN);

    return 0;
}

int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config)
{
    switch (Config)
    {
    case RBI_SWITCH_OFF:
    {
        /* Turn off switch */
        HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
        break;
    }
    case RBI_SWITCH_RX:
    {
        /*Turns On in Rx Mode the RF Switch */
        HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
        break;
    }
    case RBI_SWITCH_RFO_LP:
    {
        /*Turns On in Tx Low Power the RF Switch */
        HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_SET);
        break;
    }
    case RBI_SWITCH_RFO_HP:
    {
        /*Turns On in Tx High Power the RF Switch */
        HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_SET);
        break;
    }
    default:
        break;
    }

    return 0;
}

int32_t RBI_GetTxConfig(void)
{
    return RBI_CONF_RFO;
}

int32_t RBI_GetWakeUpTime(void)
{
    return RF_WAKEUP_TIME;
}

int32_t RBI_IsTCXO(void)
{
    return IS_TCXO_SUPPORTED;
}

int32_t RBI_IsDCDC(void)
{
    return IS_DCDC_SUPPORTED;
}

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
 * @file st_hw_i2c_implementation.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "LIS2DH12.h"
#include "GNSE_bsp.h"

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t st_platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                                 uint16_t len)
{
    HAL_StatusTypeDef status = HAL_OK;
    reg |= LIS2DE12_WRITE_MULTIPLE;
    return HAL_I2C_Mem_Write(handle, LIS2DH12_I2C_ADD_H, reg,
                             I2C_MEMADD_SIZE_8BIT, bufp, len, SENSOR_I2C1_TIMOUT);
    return status;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t st_platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                                uint16_t len)
{
    HAL_StatusTypeDef status = HAL_OK;
    reg |= LIS2DE12_READ_MULTIPLE;
    status = HAL_I2C_Mem_Read(handle, LIS2DH12_I2C_ADD_H, reg,
                              I2C_MEMADD_SIZE_8BIT, bufp, len, SENSOR_I2C1_TIMOUT);
    return status;
}

/**
 * @brief Initilize and map the LIS2DH12 aplication layer I2C read and write functions
 *
 * @param app_ctx
 * @return LIS2DE12_op_result_t See enum for more details
 */
LIS2DE12_op_result_t LIS2DH12_init(stmdev_ctx_t *app_ctx)
{
    if (app_ctx == NULL)
    {
        return LIS2DE12_OP_CONFIG_ERROR;
    }
    else
    {
        app_ctx->write_reg = st_platform_write;
        app_ctx->read_reg = st_platform_read;
        app_ctx->handle = &GNSE_BSP_sensor_i2c1;
        return LIS2DE12_OP_SUCCESS;
    }
    // TODO: add an improved init, see https://github.com/TheThingsIndustries/generic-node-se/issues/33
}

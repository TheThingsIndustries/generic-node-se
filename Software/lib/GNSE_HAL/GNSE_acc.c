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
 * @file GNSE_acc.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_acc.h"
#include <stdint.h>
#include "LIS2DH12.h"

ACC_op_result_t GNSE_ACC_Init(void)
{
    /* Check device ID */
    uint8_t whoami;
    stmdev_ctx_t dev_ctx;
    LIS2DH12_init(&dev_ctx);
    lis2dh12_device_id_get(&dev_ctx, &whoami);
    if (whoami != LIS2DH12_ID)
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


ACC_op_result_t GNSE_ACC_DeInit(void)
{
    /* Unset interrupt pin */
    if (GNSE_BSP_Acc_Int_DeInit() != GNSE_BSP_ERROR_NONE)
    {
        return ACC_OP_FAIL;
    }

    return ACC_OP_SUCCESS;
}

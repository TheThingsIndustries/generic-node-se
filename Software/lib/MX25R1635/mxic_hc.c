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
 * @file mxic_hc.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "mxic_hc.h"

/*
 * Function:      MxHardwareInit
 * Arguments:      Spi,       pointer to an MxSpi structure of transfer.
 * Return Value:  MXST_SUCCESS.
 * Description:   This function is used for initializing the controller by set the registers of controller to proper value.
 */
int MxHardwareInit(MxSpi *Spi)
{
    int32_t bsp_status = GNSE_BSP_ERROR_NONE;
    bsp_status = GNSE_BSP_Flash_SPI_Init();
    if (bsp_status != GNSE_BSP_ERROR_NONE)
    {
        return MXST_FAILURE;
    }
    return MXST_SUCCESS;
}

/*
 * Function:      MxPolledTransfer
 * Arguments:      Spi,       pointer to an MxSpi structure of transfer.
 *                   WrBuf,     pointer to a data buffer where the write data will be stored.
 *                   RdBuf,     pointer to a data buffer where the read data will be stored.
 *                   ByteCount, the byte count of the data will be transferred.
 * Return Value:  MXST_SUCCESS.
 * Description:   This function is used for transferring specified data on the bus in polled mode.
 */
int MxPolledTransfer(MxSpi *Spi, u8 *WrBuf, u8 *RdBuf, u32 ByteCount)
{
    HAL_StatusTypeDef status = HAL_OK;
    if (RdBuf == NULL)
    {
        HAL_GPIO_WritePin(FLASH_SPI_GPIO_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_RESET);
        status = HAL_SPI_Transmit(&GNSE_BSP_flash_spi, (uint8_t *)WrBuf, ByteCount, Flash_SPI_TIMOUT);
        HAL_GPIO_WritePin(FLASH_SPI_GPIO_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(FLASH_SPI_GPIO_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_RESET);
        status = HAL_SPI_TransmitReceive(&GNSE_BSP_flash_spi, (uint8_t *)WrBuf, (uint8_t *)RdBuf, ByteCount, Flash_SPI_TIMOUT);
        HAL_GPIO_WritePin(FLASH_SPI_GPIO_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_SET);
    }

    if (status != HAL_OK)
    {
        if (status == HAL_TIMEOUT)
        {
            return MXST_TIMEOUT;
        }
        else
        {
            //Either HAL_BUSY, HAL_ERROR or UNKNOWN error
            return MXST_FAILURE;
        }
    }
    return MXST_SUCCESS;
}

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
 * @file tnglora_read_helper.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "tnglora_read_helper.h"

static ATCA_STATUS convert_ascii_devEUI(uint8_t *devEUI_ascii, uint8_t *devEUI);

ATCA_STATUS atcab_read_joinEUI(uint8_t *joinEUI)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t read_buf[ATCA_BLOCK_SIZE];

    if (!joinEUI)
    {
        return ATCA_BAD_PARAM;
    }

    do
    {
        if ((status = atcab_read_zone(ATCA_ZONE_DATA, JOIN_EUI_SLOT, 0, 0, read_buf, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS)
        {
            break;
        }
        memcpy(&joinEUI[0], &read_buf[0], JOIN_EUI_SIZE_BYTE);
    } while (0);

    return status;
}

ATCA_STATUS atcab_read_ascii_devEUI(uint8_t *devEUI_ascii)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t read_buf[ATCA_BLOCK_SIZE];

    if (!devEUI_ascii)
    {
        return ATCA_BAD_PARAM;
    }

    do
    {
        if ((status = atcab_read_zone(ATCA_ZONE_DATA, DEV_EUI_SLOT, 0, 0, read_buf, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS)
        {
            break;
        }
        memcpy(&devEUI_ascii[0], &read_buf[0], DEV_EUI_ASCII_SIZE_BYTE);
    } while (0);

    return status;
}

static ATCA_STATUS convert_ascii_devEUI(uint8_t *devEUI_ascii, uint8_t *devEUI)
{
    for (size_t pos = 0; pos < DEV_EUI_ASCII_SIZE_BYTE; pos += 2)
    {
        uint8_t temp = 0;
        if (devEUI_ascii[pos] >= '0' && devEUI_ascii[pos] <= '9')
        {
            temp = (devEUI_ascii[pos] - '0') << 4;
        }
        else if (devEUI_ascii[pos] >= 'A' && devEUI_ascii[pos] <= 'F')
        {
            temp = ((devEUI_ascii[pos] - 'A') + 10) << 4;
        }
        else
        {
            return ATCA_BAD_PARAM;
        }
        if (devEUI_ascii[pos + 1] >= '0' && devEUI_ascii[pos + 1] <= '9')
        {
            temp |= devEUI_ascii[pos + 1] - '0';
        }
        else if (devEUI_ascii[pos + 1] >= 'A' && devEUI_ascii[pos + 1] <= 'F')
        {
            temp |= (devEUI_ascii[pos + 1] - 'A') + 10;
        }
        else
        {
            return ATCA_BAD_PARAM;
        }
        devEUI[pos / 2] = temp;
    }
    return ATCA_SUCCESS;
}

ATCA_STATUS atcab_read_devEUI(uint8_t *devEUI)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t devEUI_ascii[DEV_EUI_ASCII_SIZE_BYTE];
    status = atcab_read_ascii_devEUI(devEUI_ascii);
    if (status != ATCA_SUCCESS)
    {
        return status;
    }
    status = convert_ascii_devEUI(devEUI_ascii, devEUI);
    return status;
}

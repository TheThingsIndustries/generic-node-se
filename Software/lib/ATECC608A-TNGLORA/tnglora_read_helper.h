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
 * @file tnglora_read_helper.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef TNGLORA_READ_HELPER_H
#define TNGLORA_READ_HELPER_H

#include "cryptoauthlib.h"
#include "atca_devtypes.h"

#define JOIN_EUI_SLOT 9U
#define DEV_EUI_SLOT 10U

#define JOIN_EUI_SIZE_BYTE 8U
#define DEV_EUI_SIZE_BYTE 8U
#define DEV_EUI_ASCII_SIZE_BYTE 16U

/**
 * @brief Read JoinEUI (Application EUI) from JOIN_EUI_SLOT (ATCA_ZONE_DATA)
 *
 * @param joinEUI Pointer to array that will be populated if the read is successful
 * @return ATCA_STATUS
 */
ATCA_STATUS atcab_read_joinEUI(uint8_t *joinEUI);

/**
 * @brief Read Device EUI in ASCII (original form) without any conversion
 *
 * @param devEUI_ascii Pointer to array that will be populated if the read is successful
 * @return ATCA_STATUS
 */
ATCA_STATUS atcab_read_ascii_devEUI(uint8_t *devEUI_ascii);

/**
 * @brief Read Device EUI and convert it from ASCII to Binary
 *
 * @param devEUI Pointer to array that will be populated if the read is successful
 * @return ATCA_STATUS
 */
ATCA_STATUS atcab_read_devEUI(uint8_t *devEUI);

#endif /* TNGLORA_READ_HELPER_H */

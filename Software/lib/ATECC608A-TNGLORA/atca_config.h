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
 * @file atca_config.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

/* Cryptoauthlib Configuration File */
#ifndef ATCA_CONFIG_H
#define ATCA_CONFIG_H

/* Include HALS */
#define ATCA_HAL_I2C

/* Included device support */
#define ATCA_ATECC608A_SUPPORT

/* ATECC608A I2C bus configuration */
#define ATCA_HAL_ATECC608A_I2C_FREQUENCY 100000U
#define ATCA_HAL_ATECC608A_I2C_BUS_PINS 2U
#define ATCA_HAL_ATECC608A_I2C_ADDRESS 0xB2U         //or 0x 59 based on the I2C drver read write bit handling
#define ATCA_HAL_ATECC608A_I2C_RX_RETRIES 2
#define ATCA_HAL_ATECC608A_I2C_WAKEUP_DELAY 3500U
#define ATCA_HAL_ATECC608A_LONG_TIMEOUT        20    /* Long Timeout 1s */

/* \brief How long to wait after an initial wake failure for the POST to
 *         complete.
 * If Power-on self test (POST) is enabled, the self test will run on waking
 * from sleep or during power-on, which delays the wake reply.
 */
#ifndef ATCA_POST_DELAY_MSEC
#define ATCA_POST_DELAY_MSEC 25
#endif

#endif

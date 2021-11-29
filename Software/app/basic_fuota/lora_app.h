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
 * @file lora_app.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef __LORA_APP_H__
#define __LORA_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* LoraWAN application configuration (Mw is configured by lorawan_conf.h) */
#define ACTIVE_REGION        LORAMAC_REGION_EU868

/*!
 * Defines the application data transmission duty cycle. 10s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            10000

/*!
 * LoRaWAN application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_APP_PORT                            2

/*!
 * LoRaWAN default endNode class port
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_A

/*!
 * LoRaWAN default confirm state
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE                           LORAMAC_HANDLER_ADR_ON

/*!
 * LoRaWAN default activation type
 */
#define LORAWAN_DEFAULT_ACTIVATION_TYPE             ACTIVATION_TYPE_OTAA

/*!
 * LoRaWAN Default data Rate Data Rate
 * @note Please note that LORAWAN_DEFAULT_DATA_RATE is used only when LORAWAN_ADR_STATE is disabled
 */
#define LORAWAN_DEFAULT_DATA_RATE                   DR_0

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

/*!
 * Default Unicast ping slots periodicity
 *
 * \remark periodicity is equal to 2^LORAWAN_DEFAULT_PING_SLOT_PERIODICITY seconds
 *         example: 2^3 = 8 seconds. The end-device will open an Rx slot every 8 seconds.
 */
#define LORAWAN_DEFAULT_PING_SLOT_PERIODICITY       4

/**
  * @brief  Init Lora Application
  * @param None
  * @return None
  */
void LoRaWAN_Init(void);

#ifdef __cplusplus
}
#endif

#endif /*__LORA_APP_H__*/

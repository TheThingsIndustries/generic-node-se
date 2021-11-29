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
 * @file lorawan_conf.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef __LORAWAN_CONF_H__
#define __LORAWAN_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos_systime.h"
#include "lora_info.h"

/* Region configuration */
/* 1- Configuration Region listed below will be linked in the MW code */
/* 2- Must also configure ACTIVE_REGION in the application */

#define REGION_EU868
// #define REGION_US915

#define LORAMAC_CLASSB_ENABLED  0

#ifndef CRITICAL_SECTION_BEGIN
#define CRITICAL_SECTION_BEGIN( )      UTILS_ENTER_CRITICAL_SECTION( )
#endif /* !CRITICAL_SECTION_BEGIN */
#ifndef CRITICAL_SECTION_END
#define CRITICAL_SECTION_END( )        UTILS_EXIT_CRITICAL_SECTION( )
#endif /* !CRITICAL_SECTION_END */

/**
 * @brief Maximum time to wait to receive a downlink packet or event after sending an uplink packet.
 *
 * As per LoRaWAN spec, class A end device uses two receive windows slots after sending an uplink packet. For US915the max window duration is
 * 3000 ms and the second RX window max delay is 2 seconds. So setting the receive timeout to higher than the receive window slots.
 */
#define CLASSA_RECEIVE_WINDOW_DURATION_MS    ( 6000 )

/*
 * @brief The version of LoRaWAN stack on Network Server, to be configured beforehand, only required for ABP activation.
 * Version is set by default to 1.0.3.0.
 */
#define lorawanConfigABP_LORAWAN_VERSION        0x01000300

/*
 * @brief LoRaWAN network ID, only required for ABP activation.
 */
#define lorawanConfigNETWORK_ID                 ( ( uint32_t ) ( 0 ) )

/**
 * @brief Flag to indicate if application is using a public network such
 * as The Things Network.
 */
#define lorawanConfigPUBLIC_NETWORK             ( 1 )

/**
 * @brief Maximum join attempts before giving up.
 *
 * Retry attempts tries to send join requests in different channels thereby finding a suitable gateway which
 * is tuned to that channel.
 */
#define lorawanConfigMAX_JOIN_ATTEMPTS    ( 1000 )

/**
 * @brief Interval between retry attempts for OTAA join.
 * It waits for a retry interval +- random jitter ( to avoid dos ) before attempting to
 * join again with LoRaWAN network.
 */
#define lorawanConfigJOIN_RETRY_INTERVAL_MS    ( 2000 )

/**
 * @brief Defines a random jitter bound in milliseconds for application data transmission duty cycle.
 *
 * This allows devices to space their transmissions slightly between each other in cases like all devices reboots and tries to
 * join server at same time.
 */
#define lorawanConfigMAX_JITTER_MS    ( 500 )

/**
 * @brief Default config to enable or disable adaptive data rate.
 *
 * Enabling adaptive data rate allows the network to set optimized data rates for end devices
 * thereby optimizing on air time and power consumption. Its recommended to enable adaptive
 * data rate for static devices and devices with stable RF conditions.
 * Adaptive data rate can be toggled runtime using API.
 *
 */
#define lorawanConfigADR_ON    ( 1 )

/**
 * @brief Default config to set the number of retries of a failed send attempt.
 *
 */
#define lorawanConfigMAX_SEND_RETRIES    ( 8 )

/**
 * @brief Overall timing error threshold for the system.
 */
#define lorawanConfigRX_MAX_TIMING_ERROR    ( 50 )

/**
 * @brief Maximum payload length defined by LoRaWAN spec
 *
 * This can be used to cap the maximum packet size that can be transferred anytime by the application.
 * LoRaWAN payload can vary up to 222 bytes. However applications should take care of duty cycle restrictions and
 * fair access policies for each region while determining the size of a message to be transmitted.
 * Larger messages leads to longer air-time and increased power consumption for the
 * radio as well as using up all of the duty cycle for a channel.
 */
#define lorawanConfigMAX_MESSAGE_SIZE    ( 222 )

/**
 * @brief Size of response queue used to receive responses to requests.
 * Queue is used to separate out events from responses so application can do a synchronous call to
 * join to a network or send a confirmed message. Since there is at most 1 LoRaWAN operation at a time, queue size
 * is set to 1.
 */
#define lorawanConfigRESPONSE_QUEUE_SIZE    ( 1 )

/**
 * @brief Queue size for downlink data.
 *
 * Class A application sends an uplink and then polls for downlink messages, the next two receive windows. Only one message is sent
 * by downlink server for each uplink. Hence setting the queue size to 1.
 */
#define lorawanConfigDOWNLINK_QUEUE_SIZE    ( 1 )

/**
 * @brief Queue size for downlink events.
 *
 * For class A application at most 4 events can be received downlink per uplink at any time (SRV_MAC_LINK_CHECK_ANS, SRV_MAC_DEVICE_TIME_ANS, FRAME LOSS, DOWNLINK DATA)
 * Queue size can be adjusted based on application needs.
 */
#define lorawanConfigEVENT_QUEUE_SIZE       ( 4 )

/**
 * @brief Stack size for LoRaMAC task.
 * Set to a reasonable size as required for LoRaMAC layer functions.
 */
#define lorawanConfigLORAMAC_TASK_STACK_SIZE    ( 512 )

/**
 * @brief Priority for LoRaMAC task.
 * LoRaMAC task is set to wake up on interrupts from radio layer and needs to process
 * radio interrupts as soon as possible. Hence setting to the max possible priority.
 */
#define lorawanConfigLORAMAC_TASK_PRIORITY      ( configMAX_PRIORITIES - 1 )

/**
 * @brief Device EUI is a globally Unique identifier used to identify the devices across LoRaWAN networks.
 * Device EUI is a 64 bit value and returned as an array of 8 hex byte values in big endian form.
 * Example: { 0x11, 0x22, 0x33, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE }
 *
 * Note: If the device EUI is pre-provisioned using a secure element, remove this config parameter to use the pre-provisioned value.
 */
extern void getDeviceEUI( uint8_t * deviceEUI );
#define lorawanConfigGET_DEV_EUI    getDeviceEUI

/**
 * @brief IN EUI or APP EUI is a globally Unique identifier used to identify the application this device is associated with..
 * Join EUI is a 64 bit value and returned as an array of 8 hex values in big endian form.
 * Example: { 0x11, 0x22, 0x33, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE }
 *
 * Note: If the join EUI is pre-provisioned using a secure element, remove this config parameter to use the pre-provisioned value.
 */
extern void getJoinEUI( uint8_t * joinEUI );
#define lorawanConfigGET_JOIN_EUI    getJoinEUI

/**
 * @brief App key is used to derive session keys used for OTAA join session.
 * App key is a 128 bit value and returned as an array of 16 hex values in big endian form.
 *
 * Note: If the App key is pre-provisioned using a secure element, remove this config parameter to use the pre-provisioned value.
 */
extern void getAppKey( uint8_t * appKey );
#define lorawanConfigGET_APP_KEY    getAppKey

/**
 * @brief End-device address which is only used for ABP join .
 *
 */
extern uint32_t getDeviceAddress( void );
#define lorawanConfigGET_DEV_ADDR    getDeviceAddress

/**
 * @brief Application Session key to be configured beforehand, only required for ABP join.
 * Application session key is a 128 bit value and returned as an array of 16 hex values in big endian form.
 *
 *  Note: If the application session key is pre-provisioned using a secure element, remove this config parameter to use the pre-provisioned value.
 */
extern void getGetAppSessionKey( uint8_t * appSessionKey );
#define lorawanConfigGET_APP_SESSION_KEY    getGetAppSessionKey

/**
 * @brief Network session key to be configured beforehand, only required for ABP join.
 * Network session key is a 128 bit value and returned as an array of 16 hex values in big endian form.
 *
 *  Note: If the network session key is pre-provisioned using a secure element, remove this config parameter to use the pre-provisioned value.
 */
extern void getGetNwkSessionKey( uint8_t * nwkSessionKey );
#define lorawanConfigGET_NETWORK_SESSION_KEY    getGetNwkSessionKey


#ifdef __cplusplus
}
#endif

#endif /* __LORAWAN_CONF_H__ */

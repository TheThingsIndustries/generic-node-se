/*
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */

#ifndef LORAWAN_H
#define LORAWAN_H

#include "FreeRTOS.h"

#include "lorawan_conf.h"
#include "LoRaMac.h"

/**
 * @brief Structure which holds the LoRaWAN payload information.
 * The same structure is used for both payload send and received.
 */
typedef struct LoRaWANMessage
{
    uint16_t port;                                 /**< @brief Application port for the payload. */
    uint8_t data[ lorawanConfigMAX_MESSAGE_SIZE ]; /**< @brief The buffer of fixed maximum size used to hold the payload. */
    size_t length;                                 /**< @brief Length of the payload. */
    uint8_t dataRate;                              /**< @brief the data rate used to transfer the payload. */
} LoRaWANMessage_t;


/**
 * @brief Network parameters for LoRaWAN.
 */
typedef struct LoRaWANNetworkParams
{
    int8_t txPower;               /**< @brief TX output power. */
    uint8_t dataRate;             /**< @brief Data rate used for sending uplink. */
    RxChannelParams_t rx2Channel; /**< @brief Second RX window channel parameters, data rate and frequeuncy.*/
} LoRaWANNetworkParams_t;

/**
 * @brief Information sent as part of link check reply event.
 */
typedef struct LoRaWANLinkCheckInfo
{
    uint8_t DemodMargin; /**< @brief Demodulation margin. Contains the link margin [dB] of the last successfully received LinkCheckReq. */
    uint8_t NbGateways;  /**< @brief Number of gateways which received the last LinkCheckReq. */
} LoRaWANLinkCheckInfo_t;

/**
 * @brief Event types received from LoRaWAN network.
 */
typedef enum LoRaWANEventType
{
    LORAWAN_EVENT_UNKNOWN = 0,         /**< @brief Type to denote an unexpected event type. */
    LORAWAN_EVENT_DOWNLINK_PENDING,    /**< @brief Indicates that server has to send more downlink data or waiting for a mac command uplink. */
    LORAWAN_EVENT_TOO_MANY_FRAME_LOSS, /**< @brief Indicates too many frames are missed between end device and LoRa network server. */
    LORAWAN_EVENT_DEVICE_TIME_UPDATED, /**< @brief Indicates the device time has been synchronized with LoRa network server. */
    LORAWAN_EVENT_LINK_CHECK_REPLY     /**< @brief Reply for a link check request from end device. */
} LoRaWANEventType_t;

/**
 * @brief Structure to hold event information.
 */
typedef struct LoRaWANEventInfo
{
    LoRaWANEventType_t type;         /**< @brief Type of event. */
    LoRaMacEventInfoStatus_t status; /**< @breif Status associated with the event. */

    union
    {
        LoRaWANLinkCheckInfo_t linkCheck; /**< @brief Link check information associated with LORAWAN_EVENT_LINK_CHECK_REPLY. */
        bool ackReceived;                 /**< @brief Acknoweldgement flag for a confirmed uplink. */
    } info;
} LoRaWANEventInfo_t;

/**
 * @brief Initializes LoRaWAN stack for the specified region.
 * Configures and starts the underlying LoRaMAC stack. Creates a high priority task to process LoRaMAC events from Radio.
 *
 * @param[in] region The region for the LoRaWAN network.
 * @return LORAMAC_STATUS_OK if the initialization was successful. Appropriate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_Init( LoRaMacRegion_t region );

/**
 * @breif Retrieves the network parameters for LoRaWAN connectivity.
 *
 * @param[out] pNetworkParams Values for all network parameters.
 * return LORAMAC_STATUS_OK If query network params was successful. Appropriate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_GetNetworkParams( LoRaWANNetworkParams_t * pNetworkParams );

/**
 * @breif Sets the network parameters to be used for LoRaWAN connectivity.
 * This sets the default network parameters to be used for LoRaWAN connectivity. API should
 * be invoked before a JOIN request.
 *
 * @param[in] pNetworkParams Values to be set for network parameters.
 * return LORAMAC_STATUS_OK If set network params was successful. Appropriate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_SetNetworkParams( LoRaWANNetworkParams_t * pNetworkParams );

/**
 * @brief Performs a join operation using OTAA handshake with the LoRa Network Server.
 * API is blocking untill the handshake is complete. It performs JOIN retries at
 * specified interval with a random jitter, for a configured number of tries.
 *
 * @return LORAMAC_STATUS_OK if the join was successful. Appropriate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_Join( void );

/**
 * @brief Activates the device by personalization without doing a JOIN handshake.
 * For ABP join, end-device does not exchange any message with LoRa Network Server.
 *
 * @return LORAMAC_STATUS_OK if the join was successful. Appropriate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_ActivateByPersonalization( void );


/**
 * @brief Enables or disables adaptive data rate.
 * Adaptive data rate mechanism is used by LoRa Network Server to find the right data rate for the device by observing the
 * uplink traffic from end-device. Its recommended to be always turned on for devices with fixed location.
 *
 * @param[in] enable Enable or disable flag
 * @return LORAMAC_STATUS_OK if the operation was successful. Appropriate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_SetAdaptiveDataRate( bool enable );

/**
 * @brief Request for device time synchronization with LoRa Network Server.
 * Piggy backs a MAC command along with the next uplink payload to request for time sync from LoRa network server. LoRaWAN stack gets the response from
 * LoRa network server to correct the clock drift for the device. An event is generated for a successful device time update.
 *
 * @return LORAMAC_STATUS_OK if the request operation was successful. Appropirate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_RequestDeviceTimeSync( void );

/**
 * @brief Request for link check with LoRa Network Server.
 * Piggy backs a MAC command along with next uplink  payload to perform link connectivity check with LoRa Network Server. Gets back the response from LoRa Network
 * Server and sends an event witht the link check infromation to the user.
 *
 * @return LORAMAC_STATUS_OK if the request operation was successful. Appropirate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_RequestLinkCheck( void );

/**
 * @brief Sends a payload to LoRa Network server.
 * This is blocking call untill the payload is send out of radio for an unconfirmed message, or an acknoweledgement is received or the retries
 * are exhausted for a confirmed payload. Number of retries for a confirmed payload is configurable. The retries uses different
 * frequencies uplink so as to find the right overlapping frequency with the gateway.
 *
 * @param[in] pMessage Pointer to the payload along with other information.
 * @param[in] confirmed Should send a confirmed payload or not.
 * @return LORAMAC_STATUS_OK if the request operation was successful. Appropirate error code otherwise.
 */
LoRaMacStatus_t LoRaWAN_Send( LoRaWANMessage_t * pMessage,
                              bool confirmed );

/**
 * @brief Receives a downlink message from LoRa Network server.
 * Blocks for the specified timeout provided.
 *
 * @param[out] pEventInfo Pointer to structure containing event type and other information.
 * @param[in] timeoutMS Timeout in milliseconds to block for an event.  Set to 0 to not block for an event.
 * @return pdFALSE if there is no data.
 */
BaseType_t LoRaWAN_Receive( LoRaWANMessage_t * pMessage,
                            uint32_t timeoutMS );


/**
 * @brief Poll for a downlink event from LoRa Network server.
 * Blocks for the specified timeout provided.
 *
 * @param[out] pEventInfo Pointer to structure containing event type and other information.
 * @param[in] timeoutMS Timeout in milliseconds to block for an event. Set to 0 to not block for an event.
 * @return pdFALSE if there are no events to be processed.
 */
BaseType_t LoRaWAN_PollEvent( LoRaWANEventInfo_t * pEventInfo,
                              uint32_t timeoutMS );

/**
 * @brief Cleans up LoRaWAN stack.
 * Stops and deinits the LoRaMAC stack. Deletes the LoRaMAC task and associated resources.
 */
void LoRaWAN_Cleanup( void );

#endif /* LORAWAN_H */

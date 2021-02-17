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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "freertos_lorawan.h"
#include "utilities.h"
#include "app_conf.h"
#include "lorawan_conf.h"

/*!
 * Prints the provided buffer in HEX
 *
 * \param buffer Buffer to be printed
 * \param size   Buffer size to be printed
 */
static void prvPrintHexBuffer( uint8_t * buffer,
                               uint8_t size )
{
    uint8_t newline = 0;

    for( uint8_t i = 0; i < size; i++ )
    {
        if( newline != 0 )
        {
            configPRINTF( ( "\r\n" ) );
            newline = 0;
        }

        configPRINTF( ( "%02X\r\n", buffer[ i ] ) );

        if( ( ( i + 1 ) % 16 ) == 0 )
        {
            newline = 1;
        }
    }

    configPRINTF( ( "\r\n" ) );
}

static LoRaMacStatus_t prvFetchDownlinkPacket( void )
{
    LoRaMacStatus_t status;
    LoRaWANMessage_t uplink = { 0 };
    LoRaWANMessage_t downlink = { 0 };

    /* Send an empty uplink message in confirmed mode. */
    uplink.length = 0;
    uplink.port = LORAWAN_APP_PORT;

    status = LoRaWAN_Send( &uplink, true );

    if( status == LORAMAC_STATUS_OK )
    {
        configPRINTF( ( "\r\n Successfully sent an uplink packet, confirmed = true.\r\n" ) );

        if( LoRaWAN_Receive( &downlink, CLASSA_RECEIVE_WINDOW_DURATION_MS ) == pdTRUE )
        {
            configPRINTF( ( "\r\n Received downlink data on port %d:\r\n", downlink.port ) );
            prvPrintHexBuffer( downlink.data, downlink.length );
        }
    }

    return status;
}

void vLorawanClassATask( void * params )
{
    LoRaMacStatus_t status;
    uint32_t ulTxIntervalMs;
    LoRaWANMessage_t uplink;
    LoRaWANMessage_t downlink;
    LoRaWANEventInfo_t event;


    configPRINTF( ( "\r\n ###### ===== FreeRTOS Class A LoRaWAN application ==== ###### \r\n" ) );

    status = LoRaWAN_Init( LORAWAN_APP_REGION );

    if( status != LORAMAC_STATUS_OK )
    {
        configPRINTF( ( "\r\n Failed to initialize lorawan error = %d\r\n", status ) );
    }

    if( status == LORAMAC_STATUS_OK )
    {
        configPRINTF( ( "\r\n Initiating OTAA join procedure.\r\n" ) );

        status = LoRaWAN_Join();
    }

    if( status != LORAMAC_STATUS_OK )
    {
        configPRINTF( ( "\r\n Failed to join to a lorawan network, error = %d\r\n", status ) );
    }
    else
    {
        /*
         * Adaptive data rate is set to ON by default but this can be changed runtime if needed
         * for mobile devices with no fixed locations.
         */

        LoRaWAN_SetAdaptiveDataRate( true );

        /**
         * Successfully joined a LoRaWAN network. Now the  task runs in an infinite loop,
         * sends periodic uplink message of 1 byte by obeying fair access policy for the LoRaWAN network.
         * If the MAC has indicated to schedule an uplink message as soon as possible, then it sends
         * an uplink message immediately after the duty cycle wait time. After each uplink it also waits
         * on downlink queue for any messages from the network server.
         */

        configPRINTF( ( "\r\n Successfully joined a LoRaWAN network. Sending data in loop.\r\n" ) );

        uplink.port = LORAWAN_APP_PORT;
        uplink.length = 1;
        uplink.data[ 0 ] = 0xFF;
        uplink.dataRate = 0;

        for( ; ; )
        {
            status = LoRaWAN_Send( &uplink, LORAWAN_CONFIRMED_SEND );

            if( status == LORAMAC_STATUS_OK )
            {
                configPRINTF( ( "\r\n Successfully sent an uplink packet, confirmed = %d \r\n", LORAWAN_CONFIRMED_SEND ) );


                configPRINTF( ( "\r\n Waiting for downlink data.\r\n" ) );

                if( LoRaWAN_Receive( &downlink, CLASSA_RECEIVE_WINDOW_DURATION_MS ) == pdTRUE )
                {
                    configPRINTF( ( "\r\n Received downlink data on port %d:\r\n", downlink.port ) );
                    prvPrintHexBuffer( downlink.data, downlink.length );
                }
                else
                {
                    configPRINTF( ( "\r\n No downlink data.\r\n" ) );
                }

                /**
                 * Poll for events from LoRa network server.
                 */
                for( ; ; )
                {
                    if( LoRaWAN_PollEvent( &event, 0 ) == pdTRUE )
                    {
                        switch( event.type )
                        {
                            case LORAWAN_EVENT_DOWNLINK_PENDING:

                                /**
                                 * MAC layer indicated there are pending acknowledgments to be sent
                                 * uplink as soon as possible. Wait for duty cycle time and send an uplink.
                                 */
                                configPRINTF( ( "\r\n Received a downlink pending event. Send an empty uplink to fetch downlink packets.\r\n" ) );
                                status = prvFetchDownlinkPacket();
                                break;

                            case LORAWAN_EVENT_TOO_MANY_FRAME_LOSS:

                                /**
                                 *  If LoRaMAC stack reports a too many frame loss event, it indicates that gateway and device frame counter
                                 *  values are not in sync. The only way to recover from this is to initiate a rejoin procedure to reset
                                 *  the frame counter at both sides.
                                 */
                                configPRINTF( ( "\r\n Too many frame loss detected. Rejoining to LoRaWAN network.\r\n" ) );
                                status = LoRaWAN_Join();

                                if( status != LORAMAC_STATUS_OK )
                                {
                                    configPRINTF( ( "\r\n Cannot rejoin to the LoRAWAN network.\r\n" ) );
                                }

                                break;

                            case LORAWAN_EVENT_DEVICE_TIME_UPDATED:
                                configPRINTF( ( "\r\n Device time synchronized. \r\n" ) );
                                break;


                            default:
                                configPRINTF( ( "\r\n Unhandled event type %d received.\r\n", event.type ) );
                                break;
                        }
                    }
                    else
                    {
                        configPRINTF( ( "\r\n No more downlink events.\r\n" ) );
                        break;
                    }
                }

                if( status == LORAMAC_STATUS_OK )
                {
                    /**
                     * Frame was sent successfully. Wait for next TX schedule to send uplink thereby obeying fair
                     * access policy.
                     */

                    ulTxIntervalMs = ( LORAWAN_APPLICATION_TX_INTERVAL_SEC * 1000 ) + randr( -LORAWAN_APPLICATION_JITTER_MS, LORAWAN_APPLICATION_JITTER_MS );

                    configPRINTF( ( "\r\n TX-RX cycle complete. Waiting for %u seconds, before starting next cycle.\r\n", ( ulTxIntervalMs / 1000 ) ) );

                    vTaskDelay( pdMS_TO_TICKS( ulTxIntervalMs ) );
                }
                else
                {
                    configPRINTF( ( "\r\n Failed to recover from an error. Exiting the demo.\r\n" ) );
                    break;
                }
            }
            else
            {
                configPRINTF( ( "\r\n Failed to send an uplink packet with error = %d\r\n", status ) );
                configPRINTF( ( "\r\n Waiting for %u seconds, before sending next uplink.\r\n", LORAWAN_APPLICATION_TX_INTERVAL_SEC ) );
                vTaskDelay( pdMS_TO_TICKS( LORAWAN_APPLICATION_TX_INTERVAL_SEC * 1000 ) );
            }
        }
    }

    LoRaWAN_Cleanup();

    vTaskDelete( NULL );
}

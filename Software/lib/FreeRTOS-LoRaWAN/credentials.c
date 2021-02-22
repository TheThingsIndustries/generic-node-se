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

/**
 * This file is provided for demonstration purpose of how to read a pre-provisioned credentials
 * from flash or memory to activate by OTAA or ABP. For production use case, its strongly
 * recommended to use a secure element to preprovision the credentials. For which you can use
 * the LoRaMAC secure element interface and remove these functions from lorawan_conf.h
 */
#include <stdint.h>
#include <string.h>
#include "Commissioning.h"


// TODO: Improve the readability and configurability of EUI and key handling, see https://github.com/TheThingsIndustries/generic-node-se/issues/56

/**
 * @brief Device EUI needed for both OTAA and ABP activation.
 */
static const uint8_t devEUI[ 8 ] = LORAWAN_DEVICE_EUI;

/**
 * @brief JOIN EUI needed for both OTAA and ABP activation.
 */
static const uint8_t joinEUI[ 8 ] = LORAWAN_JOIN_EUI;

/**
 * @brief App key required for OTAA activation.
 */
static const uint8_t appKey[ 16 ] = LORAWAN_APP_KEY;

/**
 * @brief App session key required for ABP activation.
 */
static const uint8_t appSessionKey[ 16 ] = LORAWAN_APP_S_KEY;

/**
 * @brief Network Session key required for ABP activation.
 */
static const uint8_t nwkSessionKey[ 16 ] = LORAWAN_NWK_S_KEY;

/**
 * @brief End device address required for ABP activation.
 */
#define END_DEVICE_ADDR    ( ( uint32_t ) ( 0x0 ) )

void getDeviceEUI( uint8_t * param )
{
    memcpy( param, devEUI, sizeof( devEUI ) );
}

void getJoinEUI( uint8_t * param )
{
    memcpy( param, joinEUI, sizeof( joinEUI ) );
}

void getAppKey( uint8_t * param )
{
    memcpy( param, appKey, sizeof( appKey ) );
}

uint32_t getDeviceAddress( void )
{
    return END_DEVICE_ADDR;
}

void getGetAppSessionKey( uint8_t * param )
{
    memcpy( param, appSessionKey, sizeof( appSessionKey ) );
}


void getGetNwkSessionKey( uint8_t * param )
{
    memcpy( param, nwkSessionKey, sizeof( nwkSessionKey ) );
}

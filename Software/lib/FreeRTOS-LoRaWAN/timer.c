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
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#include <string.h>

#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "timer.h"

#if configUSE_16_BIT_TICKS == 1
#error "16 bit ticks is not supported for LoRaWAN timer implementation."
#endif


struct TimerEvent_s {
    TimerHandle_t handle;
    void ( *callback )( void *context );
    void *context;
    TickType_t timerTicks;
};


static void prvCallbackExecutor( TimerHandle_t xTimer  )
{
    struct TimerEvent_s * pEvent  = ( struct TimerEvent_s * ) pvTimerGetTimerID( xTimer );

    if( pEvent != NULL && ( pEvent->callback != NULL ) )
    {
        pEvent->callback( pEvent->context );
    }
}

void TimerInit( TimerEvent_t * obj, void ( *callback )( void *context ) )
{
    TickType_t initialPeriod = ( TickType_t )( 1UL );
    TimerHandle_t timerHandle;
    struct TimerEvent_s * pEvent = pvPortMalloc( sizeof( struct TimerEvent_s ) );

    configASSERT( pEvent != NULL );
    memset( pEvent, 0x00, sizeof( struct TimerEvent_s ) );
    pEvent->callback = callback;
    timerHandle = xTimerCreate( "LoraWANTimer",
            initialPeriod,
            pdFALSE,
            pEvent,
            prvCallbackExecutor );

    configASSERT( timerHandle != NULL );
    pEvent->handle = timerHandle;
    *obj = pEvent;
}

void TimerSetContext( TimerEvent_t *obj, void* context )
{
    struct TimerEvent_s * pEvent = ( struct TimerEvent_s * ) ( *obj );
    configASSERT( pEvent != NULL );
    pEvent->context = context;
}

void TimerStart( TimerEvent_t *obj )
{
    struct TimerEvent_s * pEvent = ( struct TimerEvent_s * ) ( *obj );

    configASSERT( pEvent != NULL );
    vTimerSetReloadMode( pEvent->handle, pdTRUE );
    if( xPortIsInsideInterrupt() == pdTRUE )
    {
        xTimerChangePeriodFromISR( pEvent->handle, pEvent->timerTicks, NULL );
    }
    else
    {
        xTimerChangePeriod( pEvent->handle, pEvent->timerTicks, portMAX_DELAY );
    }

}

bool TimerIsStarted( TimerEvent_t *obj )
{
    struct TimerEvent_s * pEvent = ( struct TimerEvent_s * ) ( *obj );
    configASSERT( pEvent != NULL );

    return ( bool ) ( xTimerIsTimerActive( pEvent->handle ) );
}

void TimerStop( TimerEvent_t *obj )
{
    struct TimerEvent_s * pEvent = ( struct TimerEvent_s * ) ( *obj );
    configASSERT( pEvent != NULL );

    if( xPortIsInsideInterrupt() == pdTRUE )
    {
        xTimerStopFromISR( pEvent->handle, NULL );
    }
    else
    {
        xTimerStop( pEvent->handle, portMAX_DELAY );
    }
}

void TimerReset( TimerEvent_t *obj )
{
    struct TimerEvent_s * pEvent = ( struct TimerEvent_s * ) ( *obj );
    configASSERT( pEvent != NULL );

    if( xPortIsInsideInterrupt() == pdTRUE )
    {
        xTimerResetFromISR( pEvent->handle, NULL );
    }
    else
    {
        xTimerReset( pEvent->handle, portMAX_DELAY );
    }

}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    TickType_t ticks = pdMS_TO_TICKS( value );
    struct TimerEvent_s * pEvent = ( struct TimerEvent_s * ) ( *obj );

    configASSERT( pEvent != NULL );

    if( ticks == 0 )
    {
        ticks++;
    }

    xTimerStop( pEvent->handle, portMAX_DELAY );
    pEvent->timerTicks = ticks;
}

TimerTime_t TimerGetCurrentTime( void )
{
    TickType_t ticks = 0;

    if( xPortIsInsideInterrupt() == pdTRUE )
    {
        ticks = xTaskGetTickCountFromISR();
    }
    else
    {
        ticks = xTaskGetTickCount();
    }

    return  ( TimerTime_t ) ( ticks / ( ( TickType_t ) configTICK_RATE_HZ ) * 1000 );
}

TimerTime_t TimerGetElapsedTime( TimerTime_t past )
{
    TickType_t nowTicks = xTaskGetTickCount();
    TickType_t pastTicks;
    TimerTime_t elapsed = 0;
    if ( past > 0 )

    {
        pastTicks = pdMS_TO_TICKS( past );
        elapsed = ( TimerTime_t ) ( pdMS_TO_TICKS( nowTicks - pastTicks ));
    }

    return elapsed;
}

TimerTime_t TimerTempCompensation( TimerTime_t period, float temperature )
{
    //TODO: Integrate RTC, see https://github.com/TheThingsIndustries/generic-node-se/issues/100
    // return RtcTempCompensation( period, temperature );
    return 0;
}

void TimerProcess( void )
{
}

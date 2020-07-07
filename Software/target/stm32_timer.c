/*!
 * \file      timer.c
 *
 * \brief     Timer objects and scheduling management implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */

/******************************************************************************
 * @file    stm32_timer.c
 * @author  MCD Application Team
 * @brief   Time server utility
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32_timer.h"

/** @addtogroup TIMER_SERVER
  * @{
  */

/* Private defines -----------------------------------------------------------*/
/**
 * @defgroup TIMER_SERVER_private_defines TIMER_SERVER private defines
 *  @{
 */
#ifndef UTIL_TIMER_INIT_CRITICAL_SECTION
  #define UTIL_TIMER_INIT_CRITICAL_SECTION( )
#endif

#ifndef UTIL_TIMER_ENTER_CRITICAL_SECTION
  #define UTIL_TIMER_ENTER_CRITICAL_SECTION( )   UTILS_ENTER_CRITICAL_SECTION( )
#endif

#ifndef UTIL_TIMER_EXIT_CRITICAL_SECTION
  #define UTIL_TIMER_EXIT_CRITICAL_SECTION( )    UTILS_EXIT_CRITICAL_SECTION( )
#endif
/**
  *  @}
  */
 
/* Private variables -----------------------------------------------------------*/
/**
 * @defgroup TIMER_SERVER_private_varaible TIMER_SERVER private variable
 *  @{
 */

/**
  * @brief Timers list head pointer
  *
  */
UTIL_TIMER_Object_t *TimerListHead = NULL;

/**
  *  @}
  */

/**
 * @defgroup TIMER_SERVER_private_function TIMER_SERVER private function
 *  @{
 */

void TimerInsertNewHeadTimer( UTIL_TIMER_Object_t *TimerObject );
void TimerInsertTimer( UTIL_TIMER_Object_t *TimerObject );
void TimerSetTimeout( UTIL_TIMER_Object_t *TimerObject );
bool TimerExists( UTIL_TIMER_Object_t *TimerObject );

/**
  *  @}
  */

/* Functions Definition ------------------------------------------------------*/
/**
  * @addtogroup TIMER_SERVER_exported_function
  *  @{
  */

UTIL_TIMER_Status_t UTIL_TIMER_Init(void)
{
  UTIL_TIMER_INIT_CRITICAL_SECTION();
  TimerListHead = NULL;
  return UTIL_TimerDriver.InitTimer();
}

UTIL_TIMER_Status_t UTIL_TIMER_DeInit(void)
{
  return UTIL_TimerDriver.DeInitTimer();
}

UTIL_TIMER_Status_t UTIL_TIMER_Create( UTIL_TIMER_Object_t *TimerObject, uint32_t PeriodValue, UTIL_TIMER_Mode_t Mode, void ( *Callback )( void *), void *Argument)
{
  if((TimerObject != NULL) && (Callback != NULL))
  {
    TimerObject->Timestamp = 0;
    TimerObject->ReloadValue = UTIL_TimerDriver.ms2Tick(PeriodValue);
    TimerObject->IsPending = 0;
    TimerObject->IsRunning = 0;
    TimerObject->Callback = Callback;
    TimerObject->argument = Argument;
    TimerObject->Mode = Mode;
    TimerObject->Next = NULL;
    return UTIL_TIMER_OK;
  }
  else
  {
    return UTIL_TIMER_INVALID_PARAM;
  }
}

UTIL_TIMER_Status_t UTIL_TIMER_Start( UTIL_TIMER_Object_t *TimerObject)
{
  uint32_t elapsedTime;
  uint32_t minValue;
  uint32_t ticks;
  UTIL_TIMER_Status_t  ret;
    
  UTIL_TIMER_ENTER_CRITICAL_SECTION();
  
  if( ( TimerObject != NULL ) && ( TimerExists( TimerObject ) == false ) && (TimerObject->IsRunning == 0U))
  {
    ticks = TimerObject->ReloadValue;
    
    minValue = UTIL_TimerDriver.GetMinimumTimeout( );
    
    if( ticks < minValue )
    {
      ticks = minValue;
    }
    
    TimerObject->Timestamp = ticks;
    TimerObject->IsPending = 0;
    TimerObject->IsRunning = 1;
    TimerObject->IsReloadStopped = 0;
    if( TimerListHead == NULL )
    {
      UTIL_TimerDriver.SetTimerContext();
      TimerInsertNewHeadTimer( TimerObject ); /* insert a timeout at now+obj->Timestamp */
    }
    else 
    {
      elapsedTime = UTIL_TimerDriver.GetTimerElapsedTime( );
      TimerObject->Timestamp += elapsedTime;
      
      if( TimerObject->Timestamp < TimerListHead->Timestamp )
      {
        TimerInsertNewHeadTimer( TimerObject);
      }
      else
      {
        TimerInsertTimer( TimerObject);
      }
    }
    ret = UTIL_TIMER_OK;
  }
  else
  {
    ret =  UTIL_TIMER_INVALID_PARAM;
  }
  
  UTIL_TIMER_EXIT_CRITICAL_SECTION();
  return ret;
}

UTIL_TIMER_Status_t UTIL_TIMER_Stop( UTIL_TIMER_Object_t *TimerObject )
{
  UTIL_TIMER_Status_t  ret;
  UTIL_TIMER_Object_t* prev = TimerListHead;
  UTIL_TIMER_Object_t* cur = TimerListHead;
  

  UTIL_TIMER_ENTER_CRITICAL_SECTION();
  
  if( TimerObject != NULL )
  {
	  TimerObject->IsReloadStopped = 1;

  /* List is empty or the Obj to stop does not exist  */
  if( TimerListHead != NULL )
  {
    TimerObject->IsRunning = 0;


    if( TimerListHead == TimerObject ) /* Stop the Head */
    {
      if( TimerListHead->IsPending == 1U ) /* The head is already pending */
      {  
        
        TimerListHead->IsPending = 0;
        if( TimerListHead->Next != NULL )
        {
          TimerListHead = TimerListHead->Next;
          TimerSetTimeout( TimerListHead );
        }
        else
        {
          UTIL_TimerDriver.StopTimerEvt( );
          TimerListHead = NULL;
        }
      }
      else /* Stop the head before it is started */
      {   
        if( TimerListHead->Next != NULL )   
        {
          TimerListHead = TimerListHead->Next;
        }
        else
        {
          TimerListHead = NULL;
        }
      }
    }
    else /* Stop an object within the list */
    {      
      while( cur != NULL )
      {
        if( cur == TimerObject )
        {
          if( cur->Next != NULL )
          {
            cur = cur->Next;
            prev->Next = cur;
          }
          else
          {
            cur = NULL;
            prev->Next = cur;
          }
          break;
        }
        else
        {
          prev = cur;
          cur = cur->Next;
        }
      }   
    }
    ret = UTIL_TIMER_OK;
  }
  }
  else
  {
    ret = UTIL_TIMER_INVALID_PARAM;
  }
  
  UTIL_TIMER_EXIT_CRITICAL_SECTION();
  
  return ret;
}  

UTIL_TIMER_Status_t UTIL_TIMER_SetPeriod(UTIL_TIMER_Object_t *TimerObject, uint32_t NewPeriodValue)
{
  UTIL_TIMER_Status_t  ret = UTIL_TIMER_OK;

  if(NULL == TimerObject)
  {
	  ret = UTIL_TIMER_INVALID_PARAM;
  }
  else
  {
	TimerObject->ReloadValue = UTIL_TimerDriver.ms2Tick(NewPeriodValue);
	if(TimerExists(TimerObject))
	{
	  UTIL_TIMER_Stop(TimerObject);
	  ret = UTIL_TIMER_Start(TimerObject);
	}
  }
  return ret;
}

UTIL_TIMER_Status_t UTIL_TIMER_SetReloadMode(UTIL_TIMER_Object_t *TimerObject, UTIL_TIMER_Mode_t ReloadMode)
{
  UTIL_TIMER_Status_t  ret;

  if(NULL == TimerObject)
  {
	ret = UTIL_TIMER_INVALID_PARAM;
  }
  else
  {
	TimerObject->Mode = ReloadMode;
  }
  return ret;
}

UTIL_TIMER_Status_t UTIL_TIMER_GetRemainingTime(UTIL_TIMER_Object_t *TimerObject, uint32_t *ElapsedTime)
{
	UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;
	if(TimerExists(TimerObject))
	{
		uint32_t time = UTIL_TimerDriver.GetTimerElapsedTime();
		if (TimerObject->Timestamp < time )
		{
			*ElapsedTime = 0;
		}
		else
		{
			*ElapsedTime = TimerObject->Timestamp - time;
		}
	}
	else
		ret = UTIL_TIMER_INVALID_PARAM;

	return ret;
}

uint32_t UTIL_TIMER_IsRunning( UTIL_TIMER_Object_t *TimerObject )
{
  if( TimerObject != NULL )
  {
    return TimerObject->IsRunning;
  }
  else
  {
    return 0;
  }
}

uint32_t UTIL_TIMER_GetFirstRemainingTime(void)
{
	uint32_t NextTimer = 0xFFFFFFFF;

	if(TimerListHead != NULL)
	{
		UTIL_TIMER_GetRemainingTime(TimerListHead, &NextTimer);
	}
	return NextTimer;
}

void UTIL_TIMER_IRQ_Handler( void )
{
  UTIL_TIMER_Object_t* cur;
  uint32_t old, now, DeltaContext;

  UTIL_TIMER_ENTER_CRITICAL_SECTION();

  old  =  UTIL_TimerDriver.GetTimerContext( );
  now  =  UTIL_TimerDriver.SetTimerContext( );

  DeltaContext = now  - old; /*intentional wrap around */
  
  /* update timeStamp based upon new Time Reference*/
  /* because delta context should never exceed 2^32*/
  if ( TimerListHead != NULL )
  {
    cur = TimerListHead;
	do {
      if (cur->Timestamp > DeltaContext)
      {
        cur->Timestamp -= DeltaContext;
      }
      else
      {
        cur->Timestamp = 0;
      }
      cur = cur->Next;
    } while(cur != NULL);
  }

  /* Execute expired timer and update the list */
  while ((TimerListHead != NULL) && ((TimerListHead->Timestamp == 0) || (TimerListHead->Timestamp < UTIL_TimerDriver.GetTimerElapsedTime(  ))))
  {
      cur = TimerListHead;
      TimerListHead = TimerListHead->Next;
      cur->IsPending = 0;
      cur->IsRunning = 0;
      cur->Callback(cur->argument);
      if(( cur->Mode == UTIL_TIMER_PERIODIC) && (cur->IsReloadStopped == 0))
      {
        UTIL_TIMER_Start(cur);
      }
      else
      {
        cur->IsRunning = 0;
      }
  }

  /* start the next TimerListHead if it exists and it is not pending*/
  if(( TimerListHead != NULL ) && (TimerListHead->IsPending == 0U))
  {
    TimerSetTimeout( TimerListHead );
  }
  UTIL_TIMER_EXIT_CRITICAL_SECTION();
}

UTIL_TIMER_Time_t UTIL_TIMER_GetCurrentTime(void)
{
  uint32_t now = UTIL_TimerDriver.GetTimerValue( );
  return  UTIL_TimerDriver.Tick2ms(now);
}

UTIL_TIMER_Time_t UTIL_TIMER_GetElapsedTime(UTIL_TIMER_Time_t past )
{
  uint32_t nowInTicks = UTIL_TimerDriver.GetTimerValue( );
  uint32_t pastInTicks = UTIL_TimerDriver.ms2Tick( past );
  /* intentional wrap around. Works Ok if tick duation below 1ms */
  return UTIL_TimerDriver.Tick2ms( nowInTicks- pastInTicks );
}

/**
  *  @}
  */

/**************************** Private functions *******************************/

/**
  *  @addtogroup TIMER_SERVER_private_function
  *
  *  @{
  */
/**
 * @brief Check if the Object to be added is not already in the list
 *
 * @param TimerObject Structure containing the timer object parameters
 * @retval 1 (the object is already in the list) or 0
 */
bool TimerExists( UTIL_TIMER_Object_t *TimerObject )
{
  UTIL_TIMER_Object_t* cur = TimerListHead;

  while( cur != NULL )
  {
    if( cur == TimerObject )
    {
      return true;
    }
    cur = cur->Next;
  }
  return false;
}

/**
 * @brief Sets a timeout with the duration "timestamp"
 *
 * @param TimerObject Structure containing the timer object parameters
 */
void TimerSetTimeout( UTIL_TIMER_Object_t *TimerObject )
{
  uint32_t minTicks= UTIL_TimerDriver.GetMinimumTimeout( );
  TimerObject->IsPending = 1;

  /* In case deadline too soon */
  if(TimerObject->Timestamp  < (UTIL_TimerDriver.GetTimerElapsedTime(  ) + minTicks) )
  {
	  TimerObject->Timestamp = UTIL_TimerDriver.GetTimerElapsedTime(  ) + minTicks;
  }
  UTIL_TimerDriver.StartTimerEvt( TimerObject->Timestamp );
}

/**
 * @brief Adds a timer to the list.
 *
 * @remark The list is automatically sorted. The list head always contains the
 *     next timer to expire.
 *
 * @param TimerObject Structure containing the timer object parameters
 */
void TimerInsertTimer( UTIL_TIMER_Object_t *TimerObject)
{
  UTIL_TIMER_Object_t* cur = TimerListHead;
  UTIL_TIMER_Object_t* next = TimerListHead->Next;

  while (cur->Next != NULL )
  {  
    if( TimerObject->Timestamp  > next->Timestamp )
    {
        cur = next;
        next = next->Next;
    }
    else
    {
        cur->Next = TimerObject;
        TimerObject->Next = next;
        return;

    }
  }
  cur->Next = TimerObject;
  TimerObject->Next = NULL;
}

/**
 * @brief Adds or replace the head timer of the list.
 *
 * @param TimerObject Structure containing the timer object parameters
 *
 * @remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 */
void TimerInsertNewHeadTimer( UTIL_TIMER_Object_t *TimerObject )
{
  UTIL_TIMER_Object_t* cur = TimerListHead;

  if( cur != NULL )
  {
    cur->IsPending = 0;
  }

  TimerObject->Next = cur;
  TimerListHead = TimerObject;
  TimerSetTimeout( TimerListHead );
}

/**
  *  @}
  */

/**
  *  @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

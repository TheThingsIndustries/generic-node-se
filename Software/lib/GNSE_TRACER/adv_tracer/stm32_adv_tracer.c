/**
 ******************************************************************************
 * @file    stm32_adv_tracer.c
 * @author  MCD Application Team
 * @brief   This file contains the advanced trace functions.
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
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "GNSE_tracer.h"
#include "stm32_adv_tracer_conf.h"

#include "stdarg.h"
#include "stdio.h"

/** @addtogroup ADV_TRACER
  * @{
  */

/* Private defines -----------------------------------------------------------*/

/** @defgroup ADV_TRACER_Private_defines ADV_TRACE Privates defines
 *  @{
 */

/**
 *  @brief  memory address of the trace buffer location.
 *  This define can be used, to change the buffer location.
 *
 */
#if !defined(ADV_TRACER_MEMLOCATION)
#define ADV_TRACER_MEMLOCATION
#endif

#if defined(ADV_TRACER_OVERRUN)
/**
 *  @brief  List the overrun status.
 *  list of the overrun status used to handle the overrun trace evacuation.
 *
 *  @note only valid if ADV_TRACER_OVERRUN has been enabled inside utilities conf
 */
typedef enum {
  TRACE_OVERRUN_NONE = 0,     /*!<overrun status none.                        */
  TRACE_OVERRUN_INDICATION,   /*!<overrun status an indication shall be sent. */
  TRACE_OVERRUN_TRANSFERT,    /*!<overrun status data transfer ongoing.       */
  TRACE_OVERRUN_EXECUTED,     /*!<overrun status data transfer complete.      */
} TRACE_OVERRUN_STATUS;
#endif

#if defined(ADV_TRACER_UNCHUNK_MODE)
/**
 *  @brief  List the unchunk status.
 *  list of the unchunk status used to handle the unchunk case.
 *
 *  @note only valid if ADV_TRACER_UNCHUNK_MODE has been enabled inside utilities conf
 */
typedef enum {
  TRACE_UNCHUNK_NONE = 0,     /*!<unchunk status none.                            */
  TRACE_UNCHUNK_DETECTED,     /*!<unchunk status an unchunk has been detected.    */
  TRACE_UNCHUNK_TRANSFER      /*!<unchunk status an unchunk transfer is ongoing. */
} TRACE_UNCHUNK_STATUS;
#endif
/**
  * @}
 */

/**
 *  @brief  advanced macro to override to enable debug mode
 */
#ifndef ADV_TRACER_DEBUG
#define ADV_TRACER_DEBUG(...)
#endif
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/** @defgroup ADV_TRACER_private_typedef ADV_TRACE private typedef
 *  @{
 */

/**
 *  @brief  ADV_TRACER_Context.
 *  this structure contains all the data to handle the trace context.
 *
 *  @note some part of the context are depend with the selected switch inside the configuration file
 *  ADV_TRACER_UNCHUNK_MODE, ADV_TRACER_OVERRUN, ADV_TRACER_CONDITIONNAL
 */
typedef struct {
#if defined(ADV_TRACER_UNCHUNK_MODE)
  uint16_t unchunk_enabled;                              /*!<unchunck enable.                           */
  TRACE_UNCHUNK_STATUS unchunk_status;                   /*!<unchunk transfer status.                  */
#endif
#if defined(ADV_TRACER_OVERRUN)
  TRACE_OVERRUN_STATUS OverRunStatus;                    /*!<overrun status.                             */
  cb_overrun *overrun_func;                               /*!<overrun function                            */
#endif
#if defined(ADV_TRACER_CONDITIONNAL)
  cb_timestamp *timestamp_func;                           /*!<ptr of function used to insert time stamp. */
  uint8_t CurrentVerboseLevel;                           /*!<verbose level used.                        */
  uint32_t RegionMask;                                   /*!<mask of the enabled region.                */
#endif
  uint16_t TraceRdPtr;                                   /*!<read pointer the trace system.             */
  uint16_t TraceWrPtr;                                   /*!<write pointer the trace system.            */
  uint16_t TraceSentSize;                                /*!<size of the latest transfer.               */
  uint16_t TraceLock;                                    /*!<lock counter of the trace system.          */
} ADV_TRACER_Context;

/**
 *  @}
 */

/* Private variables ---------------------------------------------------------*/
/** @defgroup ADV_TRACER_private_variable ADV_TRACE private variable
 * private variable of the advanced trace system.
 *  @{
 */

/**
 * @brief trace context
 * this variable contains all the internal data of the advanced trace system.
 */
static ADV_TRACER_Context ADV_TRACER_Ctx;
static ADV_TRACER_MEMLOCATION uint8_t ADV_TRACER_Buffer[ADV_TRACER_FIFO_SIZE];

#if defined(ADV_TRACER_CONDITIONNAL) && defined(ADV_TRACER_UNCHUNK_MODE)
/**
 * @brief temporary buffer used by ADV_TRACER_COND_FSend
 * a temporary buffers variable used to evaluate a formatted string size.
 */
static uint8_t sztmp[ADV_TRACER_TMP_BUF_SIZE];
#endif
/**
 * @}
 */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup ADV_TRACER_private_function ADV_TRACE private function
 *
 *  @{
 */
static void TRACE_TxCpltCallback(void *Ptr);
static int16_t TRACE_AllocateBufer(uint16_t Size, uint16_t *Pos);
static ADV_TRACER_Status_t TRACE_Send(void);

static void TRACE_Lock(void);
static void TRACE_UnLock(void);
static uint32_t TRACE_IsLocked(void);

/**
  * @}
  */

/* Functions Definition ------------------------------------------------------*/

/** @addtogroup ADV_TRACER_exported_function
 *  @{
 */
ADV_TRACER_Status_t ADV_TRACER_Init(void)
{
  /* initialize the Ptr for Read/Write */
  (void)ADV_TRACER_MEMSET8(&ADV_TRACER_Ctx, 0x0, sizeof(ADV_TRACER_Context));
  (void)ADV_TRACER_MEMSET8(&ADV_TRACER_Buffer, 0x0, sizeof(ADV_TRACER_Buffer));

#if defined(ADV_TRACER_UNCHUNK_MODE)
  ADV_TRACER_DEBUG("\nUNCHUNK_MODE\n");
#endif
  /* Allocate Lock resource */
  ADV_TRACER_INIT_CRITICAL_SECTION();

  /* Initialize the Low Level interface */
  return UTIL_TraceDriver.Init(TRACE_TxCpltCallback);
}

ADV_TRACER_Status_t ADV_TRACER_DeInit(void)
{
  /* Un-initialize the Low Level interface */
  return UTIL_TraceDriver.DeInit();
}

ADV_TRACER_Status_t ADV_TRACER_StartRxProcess(void (*UserCallback)(uint8_t *PData, uint16_t Size, uint8_t Error))
{
  /* start the RX process */
  return UTIL_TraceDriver.StartRx(UserCallback);
}

#if defined(ADV_TRACER_CONDITIONNAL)
ADV_TRACER_Status_t ADV_TRACER_COND_FSend(uint32_t VerboseLevel, uint32_t Region, uint32_t TimeStampState, const char *strFormat, ...)
{
  va_list vaArgs;
#if defined(ADV_TRACER_UNCHUNK_MODE)
  uint8_t buf[ADV_TRACER_TMP_MAX_TIMESTMAP_SIZE];
  uint16_t timestamp_size = 0u;
  uint16_t writepos;
  uint16_t idx;
#else
  uint8_t buf[ADV_TRACER_TMP_BUF_SIZE+ADV_TRACER_TMP_MAX_TIMESTMAP_SIZE];
#endif
  uint16_t buff_size = 0u;

  /* check verbose level */
  if (!( ADV_TRACER_Ctx.CurrentVerboseLevel >= VerboseLevel))
  {
    return ADV_TRACER_GIVEUP;
  }

  if(( Region & ADV_TRACER_Ctx.RegionMask) != Region)
  {
    return ADV_TRACER_REGIONMASKED;
  }

#if defined(ADV_TRACER_UNCHUNK_MODE)
  if((ADV_TRACER_Ctx.timestamp_func != NULL) && (TimeStampState != 0u))
  {
    ADV_TRACER_Ctx.timestamp_func(buf,&timestamp_size);
  }

  va_start( vaArgs, strFormat);
  buff_size =(uint16_t)ADV_TRACER_VSNPRINTF((char *)sztmp,ADV_TRACER_TMP_BUF_SIZE, strFormat, vaArgs);

  TRACE_Lock();

  /* if allocation is ok, write data into the buffer */
  if (TRACE_AllocateBufer((buff_size+timestamp_size),&writepos) != -1)
  {
#if defined(ADV_TRACER_OVERRUN)
    ADV_TRACER_ENTER_CRITICAL_SECTION();
    if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_EXECUTED)
    {
      /* clear the over run */
      ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_NONE;
    }
    ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif

    /* copy the timestamp */
    for (idx = 0u; idx < timestamp_size; idx++)
    {
      ADV_TRACER_Buffer[writepos] = buf[idx];
      writepos = writepos + 1u;
    }

    /* copy the data */
    (void)ADV_TRACER_VSNPRINTF((char *)(&ADV_TRACER_Buffer[writepos]), ADV_TRACER_TMP_BUF_SIZE, strFormat, vaArgs);
    va_end(vaArgs);

    TRACE_UnLock();

    return TRACE_Send();
  }

  va_end(vaArgs);
  TRACE_UnLock();
#if defined(ADV_TRACER_OVERRUN)
  ADV_TRACER_ENTER_CRITICAL_SECTION();
  if((ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_NONE ) && (NULL != ADV_TRACER_Ctx.overrun_func))
  {
    ADV_TRACER_DEBUG("ADV_TRACER_Send:TRACE_OVERRUN_INDICATION");
    ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_INDICATION;
  }
  ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif

  return ADV_TRACER_MEM_FULL;

#else
  if((ADV_TRACER_Ctx.timestamp_func != NULL) && (TimeStampState != 0u))
  {
    ADV_TRACER_Ctx.timestamp_func(buf,&buff_size);
  }

  va_start( vaArgs, strFormat);
  buff_size+=(uint16_t)ADV_TRACER_VSNPRINTF((char *)(buf + buff_size), ADV_TRACER_TMP_BUF_SIZE, strFormat, vaArgs);
  va_end(vaArgs);

  return ADV_TRACER_Send(buf, buff_size);
#endif
}
#endif

ADV_TRACER_Status_t ADV_TRACER_FSend(const char *strFormat, ...)
{
  uint8_t buf[ADV_TRACER_TMP_BUF_SIZE];
  va_list vaArgs;

  va_start( vaArgs, strFormat);
  uint16_t bufSize=(uint16_t)ADV_TRACER_VSNPRINTF((char *)buf ,ADV_TRACER_TMP_BUF_SIZE, strFormat, vaArgs);
  va_end(vaArgs);

  return ADV_TRACER_Send(buf, bufSize);
}

#if defined(ADV_TRACER_CONDITIONNAL)
ADV_TRACER_Status_t ADV_TRACER_COND_ZCSend_Allocation(uint32_t VerboseLevel, uint32_t Region, uint32_t TimeStampState, uint16_t length, uint8_t **pData, uint16_t *FifoSize, uint16_t *WritePos)
{
  ADV_TRACER_Status_t ret = ADV_TRACER_OK;
  uint16_t writepos;
  uint8_t timestamp_ptr[ADV_TRACER_TMP_MAX_TIMESTMAP_SIZE];
  uint16_t timestamp_size = 0u;

  /* check verbose level */
  if (!( ADV_TRACER_Ctx.CurrentVerboseLevel >= VerboseLevel))
  {
    return ADV_TRACER_GIVEUP;
  }

  if(( Region & ADV_TRACER_Ctx.RegionMask) != Region)
  {
	  return ADV_TRACER_REGIONMASKED;
  }

  if((ADV_TRACER_Ctx.timestamp_func != NULL) && (TimeStampState != 0u))
  {
	  ADV_TRACER_Ctx.timestamp_func(timestamp_ptr,&timestamp_size);
  }

  TRACE_Lock();

  /* if allocation is ok, write data into the buffer */
  if (TRACE_AllocateBufer(length+timestamp_size, &writepos) != -1)
  {
#if defined(ADV_TRACER_OVERRUN)
    ADV_TRACER_ENTER_CRITICAL_SECTION();
    if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_EXECUTED)
    {
      /* clear the over run */
	    ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_NONE;
    }
    ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif

    /* fill time stamp information */
    for(uint16_t index = 0u; index < timestamp_size; index++)
    {
      ADV_TRACER_Buffer[writepos] = timestamp_ptr[index];
      writepos = (uint16_t)((writepos + 1u) % ADV_TRACER_FIFO_SIZE);
    }

    /*user fill */
    *pData = ADV_TRACER_Buffer;
    *FifoSize = (uint16_t)ADV_TRACER_FIFO_SIZE;
    *WritePos = writepos;
  }
  else
  {
#if defined(ADV_TRACER_OVERRUN)
    ADV_TRACER_ENTER_CRITICAL_SECTION();
    if((ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_NONE ) && (NULL != ADV_TRACER_Ctx.overrun_func))
    {
      ADV_TRACER_DEBUG(":TRACE_OVERRUN_INDICATION");
      ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_INDICATION;
    }
    ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif
    TRACE_UnLock();
    ret = ADV_TRACER_MEM_FULL;
  }
  return ret;
}

ADV_TRACER_Status_t ADV_TRACER_COND_ZCSend_Finalize(void)
{
	return ADV_TRACER_ZCSend_Finalize();
}
#endif

ADV_TRACER_Status_t ADV_TRACER_ZCSend_Allocation(uint16_t Length, uint8_t **pData, uint16_t *FifoSize, uint16_t *WritePos)
{
  ADV_TRACER_Status_t ret = ADV_TRACER_OK;
  uint16_t writepos;

  TRACE_Lock();

  /* if allocation is ok, write data into the buffer */
  if (TRACE_AllocateBufer(Length,&writepos)  != -1)
  {
#if defined(ADV_TRACER_OVERRUN)
	ADV_TRACER_ENTER_CRITICAL_SECTION();
	if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_EXECUTED)
	{
		/* clear the over run */
	    ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_NONE;
	}
	ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif

	/*user fill */
	*pData = ADV_TRACER_Buffer;
	*FifoSize = ADV_TRACER_FIFO_SIZE;
	*WritePos = (uint16_t)writepos;
  }
  else
  {
#if defined(ADV_TRACER_OVERRUN)
	ADV_TRACER_ENTER_CRITICAL_SECTION();
	if((ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_NONE ) && (NULL != ADV_TRACER_Ctx.overrun_func))
	{
		ADV_TRACER_DEBUG("ADV_TRACER_ZCSend:TRACE_OVERRUN_INDICATION");
		ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_INDICATION;
	}
	ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif
    TRACE_UnLock();
    ret = ADV_TRACER_MEM_FULL;
  }

  return ret;
}

ADV_TRACER_Status_t ADV_TRACER_ZCSend_Finalize(void)
{
    TRACE_UnLock();
    return TRACE_Send();
}

#if defined(ADV_TRACER_CONDITIONNAL)
ADV_TRACER_Status_t ADV_TRACER_COND_Send(uint32_t VerboseLevel, uint32_t Region, uint32_t TimeStampState, const uint8_t *pData, uint16_t Length)
{
  ADV_TRACER_Status_t ret;
  uint16_t writepos;
  uint32_t  idx;
  uint8_t timestamp_ptr[ADV_TRACER_TMP_MAX_TIMESTMAP_SIZE];
  uint16_t timestamp_size = 0u;

  /* check verbose level */
  if (!( ADV_TRACER_Ctx.CurrentVerboseLevel >= VerboseLevel))
  {
	return ADV_TRACER_GIVEUP;
  }

  if(( Region & ADV_TRACER_Ctx.RegionMask) != Region)
  {
	  return ADV_TRACER_REGIONMASKED;
  }

  if((ADV_TRACER_Ctx.timestamp_func != NULL) && (TimeStampState != 0u))
  {
	  ADV_TRACER_Ctx.timestamp_func(timestamp_ptr,&timestamp_size);
  }

  TRACE_Lock();

  /* if allocation is ok, write data into the buffer */
  if (TRACE_AllocateBufer(Length + timestamp_size, &writepos) != -1)
  {
#if defined(ADV_TRACER_OVERRUN)
    ADV_TRACER_ENTER_CRITICAL_SECTION();
    if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_EXECUTED)
    {
      /* clear the over run */
	    ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_NONE;
    }
    ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif

    /* fill time stamp information */
    for( idx = 0; idx < timestamp_size; idx++)
    {
      ADV_TRACER_Buffer[writepos] = timestamp_ptr[idx];
      writepos = (uint16_t)((writepos + 1u) % ADV_TRACER_FIFO_SIZE);
    }

    for (idx = 0u; idx < Length; idx++)
    {
      ADV_TRACER_Buffer[writepos] = pData[idx];
      writepos = (uint16_t)((writepos + 1u) % ADV_TRACER_FIFO_SIZE);
    }

    TRACE_UnLock();
    ret = TRACE_Send();
  }
  else
  {
	TRACE_UnLock();
#if defined(ADV_TRACER_OVERRUN)
	ADV_TRACER_ENTER_CRITICAL_SECTION();
	if((ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_NONE ) && (NULL != ADV_TRACER_Ctx.overrun_func))
	{
		ADV_TRACER_DEBUG("ADV_TRACER_Send:TRACE_OVERRUN_INDICATION");
		ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_INDICATION;
	}
	ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif
    ret = ADV_TRACER_MEM_FULL;
  }

  return ret;
}
#endif

ADV_TRACER_Status_t ADV_TRACER_Send(const uint8_t *pData, uint16_t Length)
{
  ADV_TRACER_Status_t ret;
  uint16_t writepos;
  uint32_t  idx;

  TRACE_Lock();

  /* if allocation is ok, write data into the buffer */
  if (TRACE_AllocateBufer(Length,&writepos) != -1)
  {

#if defined(ADV_TRACER_OVERRUN)
	ADV_TRACER_ENTER_CRITICAL_SECTION();
    if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_EXECUTED)
    {
      /* clear the over run */
	    ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_NONE;
    }
    ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif

    /* initialize the Ptr for Read/Write */
    for (idx = 0u; idx < Length; idx++)
    {
      ADV_TRACER_Buffer[writepos] = pData[idx];
      writepos = (uint16_t)((writepos + 1u) % ADV_TRACER_FIFO_SIZE);
    }
    TRACE_UnLock();

    ret = TRACE_Send();
  }
  else
  {
	TRACE_UnLock();
#if defined(ADV_TRACER_OVERRUN)
	ADV_TRACER_ENTER_CRITICAL_SECTION();
	if((ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_NONE ) && (NULL != ADV_TRACER_Ctx.overrun_func))
	{
		ADV_TRACER_DEBUG("ADV_TRACER_Send:TRACE_OVERRUN_INDICATION");
		ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_INDICATION;
	}
	ADV_TRACER_EXIT_CRITICAL_SECTION();
#endif

    ret = ADV_TRACER_MEM_FULL;
  }

  return ret;
}

#if defined(ADV_TRACER_OVERRUN)
void ADV_TRACER_RegisterOverRunFunction(cb_overrun *cb)
{
	ADV_TRACER_Ctx.overrun_func = *cb;
}
#endif

#if defined(ADV_TRACER_CONDITIONNAL)
void ADV_TRACER_RegisterTimeStampFunction(cb_timestamp *cb)
{
	ADV_TRACER_Ctx.timestamp_func = *cb;
}

void ADV_TRACER_SetVerboseLevel(uint8_t Level)
{
	ADV_TRACER_Ctx.CurrentVerboseLevel = Level;
}

uint8_t ADV_TRACER_GetVerboseLevel(void)
{
	return ADV_TRACER_Ctx.CurrentVerboseLevel;
}

void ADV_TRACER_SetRegion(uint32_t Region)
{
	ADV_TRACER_Ctx.RegionMask|= Region;
}

uint32_t ADV_TRACER_GetRegion(void)
{
	return ADV_TRACER_Ctx.RegionMask;
}

void ADV_TRACER_ResetRegion(uint32_t Region)
{
	ADV_TRACER_Ctx.RegionMask&= ~Region;
}
#endif

__WEAK void ADV_TRACER_PreSendHook (void)
{
}

__WEAK void ADV_TRACER_PostSendHook (void)
{
}

/**
 * @}
 */

/** @addtogroup ADV_TRACER_private_function
 *  @{
 */

/**
  * @brief send the data of the trace to low layer
  * @retval Status based on @ref ADV_TRACER_Status_t
  */
static ADV_TRACER_Status_t TRACE_Send(void)
{
  ADV_TRACER_Status_t ret = ADV_TRACER_OK;
  uint8_t *ptr = NULL;

  ADV_TRACER_ENTER_CRITICAL_SECTION();

  if(TRACE_IsLocked() == 0u)
  {
    TRACE_Lock();

#if defined(ADV_TRACER_OVERRUN)
	if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_INDICATION )
	{
		ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_TRANSFERT;
	    ADV_TRACER_EXIT_CRITICAL_SECTION();
	    ADV_TRACER_PreSendHook();

	    ADV_TRACER_Ctx.overrun_func(&ptr, &ADV_TRACER_Ctx.TraceSentSize);
	    ADV_TRACER_DEBUG("\n--TRACE_Send overrun(%d)--\n", ADV_TRACER_Ctx.TraceSentSize);
	    return UTIL_TraceDriver.Send(ptr, ADV_TRACER_Ctx.TraceSentSize);
	}
#endif

    if (ADV_TRACER_Ctx.TraceRdPtr != ADV_TRACER_Ctx.TraceWrPtr)
    {
#ifdef ADV_TRACER_UNCHUNK_MODE
   	  if(TRACE_UNCHUNK_DETECTED == ADV_TRACER_Ctx.unchunk_status)
   	  {
        ADV_TRACER_Ctx.TraceSentSize = (uint16_t)(ADV_TRACER_Ctx.unchunk_enabled - ADV_TRACER_Ctx.TraceRdPtr);
        ADV_TRACER_Ctx.unchunk_status = TRACE_UNCHUNK_TRANSFER;
        ADV_TRACER_Ctx.unchunk_enabled = 0;

        ADV_TRACER_DEBUG("\nTRACE_TxCpltCallback::unchunk start(%d,%d)\n",ADV_TRACER_Ctx.unchunk_enabled, ADV_TRACER_Ctx.TraceRdPtr);

        if (0u == ADV_TRACER_Ctx.TraceSentSize)
        {
          ADV_TRACER_Ctx.unchunk_status = TRACE_UNCHUNK_NONE;
          ADV_TRACER_Ctx.TraceRdPtr = 0;
        }
   	  }

   	  if(TRACE_UNCHUNK_NONE == ADV_TRACER_Ctx.unchunk_status)
   	  {
#endif
   	    if (ADV_TRACER_Ctx.TraceWrPtr > ADV_TRACER_Ctx.TraceRdPtr)
        {
   	    	ADV_TRACER_Ctx.TraceSentSize = ADV_TRACER_Ctx.TraceWrPtr - ADV_TRACER_Ctx.TraceRdPtr;
        }
        else  /* TraceRdPtr > TraceWrPtr */
        {
        	ADV_TRACER_Ctx.TraceSentSize = ADV_TRACER_FIFO_SIZE - ADV_TRACER_Ctx.TraceRdPtr;

        }
#ifdef ADV_TRACER_UNCHUNK_MODE
      }
#endif
      ptr = &ADV_TRACER_Buffer[ADV_TRACER_Ctx.TraceRdPtr];

      ADV_TRACER_EXIT_CRITICAL_SECTION();
      ADV_TRACER_PreSendHook();

      ADV_TRACER_DEBUG("\n--TRACE_Send(%d-%d)--\n",ADV_TRACER_Ctx.TraceRdPtr, ADV_TRACER_Ctx.TraceSentSize);
      ret = UTIL_TraceDriver.Send(ptr, ADV_TRACER_Ctx.TraceSentSize);
    }
    else
    {
      TRACE_UnLock();
      ADV_TRACER_EXIT_CRITICAL_SECTION();
    }
  }
  else
  {
    ADV_TRACER_EXIT_CRITICAL_SECTION();
  }

  return ret;
}

/**
  * @brief Tx callback called by the low layer level to inform a transfer complete
  * @param Ptr pointer not used only for HAL compatibility
  * @retval none
  */
static void TRACE_TxCpltCallback(void *Ptr)
{
  ADV_TRACER_ENTER_CRITICAL_SECTION();

#if defined(ADV_TRACER_OVERRUN)
  if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_TRANSFERT )
  {
    ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_EXECUTED;
    ADV_TRACER_DEBUG("\n--TRACE_Send overrun complete--\n");
    ADV_TRACER_Ctx.TraceSentSize = 0u;
  }
#endif

#if defined(ADV_TRACER_UNCHUNK_MODE)
  if(TRACE_UNCHUNK_TRANSFER == ADV_TRACER_Ctx.unchunk_status)
  {
	  ADV_TRACER_Ctx.unchunk_status = TRACE_UNCHUNK_NONE;
	  ADV_TRACER_Ctx.TraceRdPtr = 0;
	  ADV_TRACER_DEBUG("\nTRACE_TxCpltCallback::unchunk complete\n");
  }
  else
  {
	  ADV_TRACER_Ctx.TraceRdPtr = (ADV_TRACER_Ctx.TraceRdPtr + ADV_TRACER_Ctx.TraceSentSize) % ADV_TRACER_FIFO_SIZE;
  }
#else
  ADV_TRACER_Ctx.TraceRdPtr = (ADV_TRACER_Ctx.TraceRdPtr + ADV_TRACER_Ctx.TraceSentSize) % ADV_TRACER_FIFO_SIZE;
#endif

#if defined(ADV_TRACER_OVERRUN)
	if(ADV_TRACER_Ctx.OverRunStatus == TRACE_OVERRUN_INDICATION )
	{
		uint8_t *ptr = NULL;

		ADV_TRACER_Ctx.OverRunStatus = TRACE_OVERRUN_TRANSFERT;
		ADV_TRACER_EXIT_CRITICAL_SECTION();
		ADV_TRACER_Ctx.overrun_func(&ptr, &ADV_TRACER_Ctx.TraceSentSize);
		ADV_TRACER_DEBUG("\n--Driver_Send overrun(%d)--\n", ADV_TRACER_Ctx.TraceSentSize);
		UTIL_TraceDriver.Send(ptr, ADV_TRACER_Ctx.TraceSentSize);
    return;
	}
#endif

  if((ADV_TRACER_Ctx.TraceRdPtr != ADV_TRACER_Ctx.TraceWrPtr) && (1u == ADV_TRACER_Ctx.TraceLock))
  {
#ifdef ADV_TRACER_UNCHUNK_MODE
    if(TRACE_UNCHUNK_DETECTED == ADV_TRACER_Ctx.unchunk_status)
    {
   		ADV_TRACER_Ctx.TraceSentSize = ADV_TRACER_Ctx.unchunk_enabled - ADV_TRACER_Ctx.TraceRdPtr;
   		ADV_TRACER_Ctx.unchunk_status = TRACE_UNCHUNK_TRANSFER;
   		ADV_TRACER_Ctx.unchunk_enabled = 0;

    	ADV_TRACER_DEBUG("\nTRACE_TxCpltCallback::unchunk start(%d,%d)\n",ADV_TRACER_Ctx.unchunk_enabled, ADV_TRACER_Ctx.TraceRdPtr);

    	if (0u == ADV_TRACER_Ctx.TraceSentSize)
      {
        ADV_TRACER_Ctx.unchunk_status = TRACE_UNCHUNK_NONE;
        ADV_TRACER_Ctx.TraceRdPtr = 0;
      }
    }

    if(TRACE_UNCHUNK_NONE == ADV_TRACER_Ctx.unchunk_status)
    {
#endif
      if (ADV_TRACER_Ctx.TraceWrPtr > ADV_TRACER_Ctx.TraceRdPtr)
      {
        ADV_TRACER_Ctx.TraceSentSize = ADV_TRACER_Ctx.TraceWrPtr - ADV_TRACER_Ctx.TraceRdPtr;
      }
      else  /* TraceRdPtr > TraceWrPtr */
      {
        ADV_TRACER_Ctx.TraceSentSize = ADV_TRACER_FIFO_SIZE - ADV_TRACER_Ctx.TraceRdPtr;
      }
#ifdef ADV_TRACER_UNCHUNK_MODE
    }
#endif
    ADV_TRACER_EXIT_CRITICAL_SECTION();
    ADV_TRACER_DEBUG("\n--TRACE_Send(%d-%d)--\n", ADV_TRACER_Ctx.TraceRdPtr, ADV_TRACER_Ctx.TraceSentSize);
    UTIL_TraceDriver.Send(&ADV_TRACER_Buffer[ADV_TRACER_Ctx.TraceRdPtr], ADV_TRACER_Ctx.TraceSentSize);
  }
  else
  {
    ADV_TRACER_PostSendHook();
    ADV_TRACER_EXIT_CRITICAL_SECTION();
    TRACE_UnLock();
  }
}

/**
  * @brief  allocate space inside the buffer to push data
  * @param  Size to allocate within fifo
  * @param  Pos position within the fifo
  * @retval write position inside the buffer is -1 no space available.
  */
static int16_t TRACE_AllocateBufer(uint16_t Size, uint16_t *Pos)
{
  uint16_t freesize;
  int16_t ret = -1;

  ADV_TRACER_ENTER_CRITICAL_SECTION();

  if (ADV_TRACER_Ctx.TraceWrPtr == ADV_TRACER_Ctx.TraceRdPtr)
  {
#ifdef ADV_TRACER_UNCHUNK_MODE
    freesize = (uint16_t)(ADV_TRACER_FIFO_SIZE - ADV_TRACER_Ctx.TraceWrPtr);
    if((Size >= freesize) && (ADV_TRACER_Ctx.TraceRdPtr > Size))
    {
      ADV_TRACER_Ctx.unchunk_status = TRACE_UNCHUNK_DETECTED;
      ADV_TRACER_Ctx.unchunk_enabled = ADV_TRACER_Ctx.TraceWrPtr;
      freesize = ADV_TRACER_Ctx.TraceRdPtr;
      ADV_TRACER_Ctx.TraceWrPtr = 0;
    }
#else
    /* need to add buffer full management*/
    freesize = (int16_t)ADV_TRACER_FIFO_SIZE;
#endif
  }
  else
  {
#ifdef ADV_TRACER_UNCHUNK_MODE
    if (ADV_TRACER_Ctx.TraceWrPtr > ADV_TRACER_Ctx.TraceRdPtr)
    {
      freesize = (uint16_t)(ADV_TRACER_FIFO_SIZE - ADV_TRACER_Ctx.TraceWrPtr);
      if((Size >= freesize) && (ADV_TRACER_Ctx.TraceRdPtr > Size))
      {
        ADV_TRACER_Ctx.unchunk_status = TRACE_UNCHUNK_DETECTED;
        ADV_TRACER_Ctx.unchunk_enabled = ADV_TRACER_Ctx.TraceWrPtr;
        freesize = ADV_TRACER_Ctx.TraceRdPtr;
        ADV_TRACER_Ctx.TraceWrPtr = 0;
      }
    }
    else
    {
      freesize = (uint16_t)(ADV_TRACER_Ctx.TraceRdPtr - ADV_TRACER_Ctx.TraceWrPtr);
    }
#else
    if (ADV_TRACER_Ctx.TraceWrPtr > ADV_TRACER_Ctx.TraceRdPtr)
    {
      freesize = ADV_TRACER_FIFO_SIZE - ADV_TRACER_Ctx.TraceWrPtr + ADV_TRACER_Ctx.TraceRdPtr;
    }
    else
    {
      freesize = ADV_TRACER_Ctx.TraceRdPtr - ADV_TRACER_Ctx.TraceWrPtr;
    }
#endif
  }

  if (freesize > Size)
  {
    *Pos = ADV_TRACER_Ctx.TraceWrPtr;
    ADV_TRACER_Ctx.TraceWrPtr = (ADV_TRACER_Ctx.TraceWrPtr + Size) % ADV_TRACER_FIFO_SIZE;
    ret = 0;

#ifdef ADV_TRACER_UNCHUNK_MODE
    ADV_TRACER_DEBUG("\n--TRACE_AllocateBufer(%d-%d-%d::%d-%d)--\n",freesize - Size, Size, ADV_TRACER_Ctx.unchunk_enabled, ADV_TRACER_Ctx.TraceRdPtr, ADV_TRACER_Ctx.TraceWrPtr);
#else
    ADV_TRACER_DEBUG("\n--TRACE_AllocateBufer(%d-%d::%d-%d)--\n",freesize - Size, Size, ADV_TRACER_Ctx.TraceRdPtr, ADV_TRACER_Ctx.TraceWrPtr);
#endif
  }

  ADV_TRACER_EXIT_CRITICAL_SECTION();
  return ret;
}

/**
  * @brief  Lock the trace buffer.
  * @retval None.
  */
static void TRACE_Lock(void)
{
  ADV_TRACER_ENTER_CRITICAL_SECTION();
  ADV_TRACER_Ctx.TraceLock++;
  ADV_TRACER_EXIT_CRITICAL_SECTION();
}

/**
  * @brief  UnLock the trace buffer.
  * @retval None.
  */
static void TRACE_UnLock(void)
{
  ADV_TRACER_ENTER_CRITICAL_SECTION();
  ADV_TRACER_Ctx.TraceLock--;
  ADV_TRACER_EXIT_CRITICAL_SECTION();
}

/**
  * @brief  UnLock the trace buffer.
  * @retval None.
  */
static uint32_t TRACE_IsLocked(void)
{
  return (ADV_TRACER_Ctx.TraceLock == 0u? 0u: 1u);
}

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

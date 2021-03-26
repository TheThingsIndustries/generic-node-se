/**
  ******************************************************************************
  * @file    lora_app.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "app.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "stm32_timer.h"
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_lpm.h"
#include "LmHandler.h"
#include "lora_info.h"

/**
  * @brief  LoRa endNode send request
  * @param  none
  * @return none
  */
static void SendTxData(void);

/**
  * @brief  TX timer callback function
  * @param  timer context
  * @return none
  */
static void OnTxTimerEvent(void *context);

/**
  * @brief  join event callback function
  * @param  params
  * @return none
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief  tx event callback function
  * @param  params
  * @return none
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRa endNode has received a frame
  * @param appData
  * @param params
  * @return None
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/*!
 * Will be called each time a Radio IRQ is handled by the MAC layer
 *
 */
static void OnMacProcessNotify(void);

/**
  * @brief User application buffer
  */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
  * @brief User application data structure
  */
static LmHandlerAppData_t AppData = {0, 0, AppDataBuffer};

static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;

/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
    {
        .GetBatteryLevel = GetBatteryLevel,
        .GetTemperature = GetTemperatureLevel,
        .OnMacProcess = OnMacProcessNotify,
        .OnJoinRequest = OnJoinRequest,
        .OnTxData = OnTxData,
        .OnRxData = OnRxData
        };

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
    {
        .ActiveRegion = ACTIVE_REGION,
        .DefaultClass = LORAWAN_DEFAULT_CLASS,
        .AdrEnable = LORAWAN_ADR_STATE,
        .TxDatarate = LORAWAN_DEFAULT_DATA_RATE,
        .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY
        };

/**
  * @brief Timer to handle the application Tx
  */
static UTIL_TIMER_Object_t TxTimer;

void LoRaWAN_Init(void)
{
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimer), UTIL_SEQ_RFU, SendTxData);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);

  LmHandlerJoin(ActivationType);

  /* send every time timer elapses */
  UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
  UTIL_TIMER_SetPeriod(&TxTimer, APP_TX_DUTYCYCLE);
  UTIL_TIMER_Start(&TxTimer);
}

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  if ((appData != NULL) && (params != NULL))
  {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Received Unhandled Downlink on F_PORT:%d \r\n", appData->Port);
  }
}

static void SendTxData(void)
{
  UTIL_TIMER_Time_t nextTxIn = 0;

  AppData.Port = LORAWAN_APP_PORT;
  AppData.BufferSize = 3;
  AppData.Buffer[0] = 0xAA;
  AppData.Buffer[1] = 0xBB;
  AppData.Buffer[2] = 0xCC;

  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "SEND REQUEST\r\n");
  }
  else if (nextTxIn > 0)
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
  }
}

static void OnTxTimerEvent(void *context)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimer), CFG_SEQ_Prio_0);

  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
}

static void OnTxData(LmHandlerTxParams_t *params)
{
  if ((params != NULL) && (params->IsMcpsConfirm != 0))
  {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### ========== MCPS-Confirm =============\r\n");
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "###### U/L FRAME:%04d | PORT:%d | DR:%d | PWR:%d", params->UplinkCounter,
            params->AppData.Port, params->Datarate, params->TxPower);

    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, " | MSG TYPE:");
    if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG)
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "CONFIRMED [%s]\r\n", (params->AckReceived != 0) ? "ACK" : "NACK");
    }
    else
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "UNCONFIRMED\r\n");
    }
  }
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### = JOINED = ");
      if (joinParams->Mode == ACTIVATION_TYPE_ABP)
      {
        APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "ABP ======================\r\n");
      }
      else
      {
        APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "OTAA =====================\r\n");
      }
    }
    else
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### = JOIN FAILED\r\n");
    }
  }
}

static void OnMacProcessNotify(void)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

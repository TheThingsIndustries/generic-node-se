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
#include "freefall.h"

/*!
 * LoRa State Machine states
 */
typedef enum TxEventType_e
{
  /*!
   * @brief Applicaiton data transmission issue based on timer with APP_TX_DUTYCYCLE
   */
  TX_ON_TIMER,
  /*!
   * @brief Application data transmission on external event (button press)
   */
  TX_ON_EVENT
} TxEventType_t;

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
  * @brief  LED timer callback function
  * @param  LED context
  * @return none
  */
static void OnTimerLedEvent(void *context);

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

/*!
 * User application buffer
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * User application data structure
 */
static LmHandlerAppData_t AppData = {0, 0, AppDataBuffer};

static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;

/*!
 * LoRaWAN handler Callbacks
 */
static LmHandlerCallbacks_t LmHandlerCallbacks =
    {
        .GetBatteryLevel = GetBatteryLevel,
        .GetTemperature = GetTemperatureLevel,
        .GetUniqueId = GetUniqueId,
        .GetRandomSeed = GetRandomSeed,
        .OnJoinRequest = OnJoinRequest,
        .OnTxData = OnTxData,
        .OnRxData = OnRxData,
        .OnMacProcess = OnMacProcessNotify};

/*!
 * LoRaWAN handler parameters
 */
static LmHandlerParams_t LmHandlerParams =
    {
        .ActiveRegion = ACTIVE_REGION,
        .DefaultClass = LORAWAN_DEFAULT_CLASS,
        .AdrEnable = LORAWAN_ADR_STATE,
        .TxDatarate = LORAWAN_DEFAULT_DATA_RATE,
        .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY};

/*!
 * Type of Event to generate application Tx
 */
static TxEventType_t EventType = TX_ON_EVENT;

/*!
 * Timer to handle the application Tx
 */
static UTIL_TIMER_Object_t TxTimer;

/*!
 * Timer to handle the application Tx Led to toggle
 */
static UTIL_TIMER_Object_t TxLedTimer;

void LoRaWAN_Init(void)
{
  // User can add any indication here (LED manipulation or Buzzer)

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerPackageProcess), UTIL_SEQ_RFU, LmHandlerPackagesProcess);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, SendTxData);

  LmHandlerConfigure(&LmHandlerParams);

  LmHandlerJoin(ActivationType);

  if (EventType == TX_ON_TIMER)
  {
    /* send every time timer elapses */
    UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxTimer, APP_TX_DUTYCYCLE);
    UTIL_TIMER_Start(&TxTimer);
  }
  else
  {
    if (freefall_init())
    {
      APP_LOG(TS_ON, VLEVEL_H, "\r\nAccelerometer failed to initialize properly \r\n");
    }
    else
    {
      APP_LOG(TS_ON, VLEVEL_H, "\r\nAccelerometer initialized \r\n");
    }
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BUTTON_SW1)
  {
    /* Note: when "EventType == TX_ON_TIMER" this GPIO is not initialised */
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
  }
  if (GPIO_Pin == ACC_INT_PIN)
  {
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
  }
}

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  if ((appData != NULL) && (params != NULL))
  {
    static const char *slotStrings[] = {"1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot"};

    APP_LOG(TS_OFF, VLEVEL_M, "\r\n ###### ========== MCPS-Indication ==========\r\n");
    APP_LOG(TS_OFF, VLEVEL_M, "\r\n ###### D/L FRAME:%04d | SLOT:%s | PORT:%d | DR:%d | RSSI:%d | SNR:%d\r\n",
            params->DownlinkCounter, slotStrings[params->RxSlot], appData->Port, params->Datarate, params->Rssi, params->Snr);
    switch (appData->Port)
    {
    case LRAWAN_APP_SWITCH_CLASS_PORT:
      /*this port switches the class*/
      if (appData->BufferSize == 1)
      {
        switch (appData->Buffer[0])
        {
        case LRAWAN_APP_SWITCH_CLASS_A:
        {
          LmHandlerRequestClass(CLASS_A);
          break;
        }
        case LRAWAN_APP_SWITCH_CLASS_B:
        {
#if defined(LORAMAC_CLASSB_ENABLED) && (LORAMAC_CLASSB_ENABLED == 1)
          LmHandlerRequestClass(CLASS_B);
#else
          APP_LOG(TS_OFF, VLEVEL_M, "\r\n Configure LORAMAC_CLASSB_ENABLED to be able to switch to this class \r\n");
#endif
          break;
        }
        case LRAWAN_APP_SWITCH_CLASS_C:
        {
          LmHandlerRequestClass(CLASS_C);
          break;
        }
        default:
          break;
        }
      }
      break;
    case LORAWAN_APP_PORT:
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n Recieved %d bytes on LORAWAN_APP_PORT: %d \r\n", appData->BufferSize, LORAWAN_APP_PORT);
      break;
    default:
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n Recieved %d bytes on undefined port: %d \r\n", appData->BufferSize, LORAWAN_APP_PORT);
      break;
    }
  }
}

static void SendTxData(void)
{
  UTIL_TIMER_Time_t nextTxIn = 0;

  UTIL_TIMER_Create(&TxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerLedEvent, NULL);
  UTIL_TIMER_SetPeriod(&TxLedTimer, 200);

  // User can add any indication here (LED manipulation or Buzzer)

  UTIL_TIMER_Start(&TxLedTimer);

  AppData.Port = LORAWAN_APP_PORT;
  AppData.BufferSize = 3;
  AppData.Buffer[0] = 0xAA;
  AppData.Buffer[1] = 0xBB;
  AppData.Buffer[2] = 0xCC;

  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
    APP_LOG(TS_ON, VLEVEL_L, "SEND REQUEST\r\n");
  }
  else if (nextTxIn > 0)
  {
    APP_LOG(TS_ON, VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
  }
}

static void OnTxTimerEvent(void *context)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);

  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
}

static void OnTimerLedEvent(void *context)
{
  // User can add any indication here (LED manipulation or Buzzer)
}

static void OnTxData(LmHandlerTxParams_t *params)
{
  if ((params != NULL) && (params->IsMcpsConfirm != 0))
  {
    APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ========== MCPS-Confirm =============\r\n");
    APP_LOG(TS_OFF, VLEVEL_H, "###### U/L FRAME:%04d | PORT:%d | DR:%d | PWR:%d", params->UplinkCounter,
            params->AppData.Port, params->Datarate, params->TxPower);

    APP_LOG(TS_OFF, VLEVEL_H, " | MSG TYPE:");
    if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG)
    {
      APP_LOG(TS_OFF, VLEVEL_H, "CONFIRMED [%s]\r\n", (params->AckReceived != 0) ? "ACK" : "NACK");
    }
    else
    {
      APP_LOG(TS_OFF, VLEVEL_H, "UNCONFIRMED\r\n");
    }
  }
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOINED = ");
      if (joinParams->Mode == ACTIVATION_TYPE_ABP)
      {
        APP_LOG(TS_OFF, VLEVEL_M, "ABP ======================\r\n");
      }
      else
      {
        APP_LOG(TS_OFF, VLEVEL_M, "OTAA =====================\r\n");
      }
    }
    else
    {
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOIN FAILED\r\n");
    }
  }
}

static void OnMacProcessNotify(void)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerPackageProcess), CFG_SEQ_Prio_0);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

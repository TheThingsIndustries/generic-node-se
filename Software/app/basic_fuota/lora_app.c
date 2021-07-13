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
 * @file lora_app.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "stm32_timer.h"
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "LmHandler.h"
#include "lora_info.h"

/* include files for Application packages*/
#include "LmhpClockSync.h"
#include "LmhpRemoteMcastSetup.h"
#include "LmhpFragmentation.h"
#include "FragDecoder.h"

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
  * @brief callback when end node does moving class
  * @param deviceClass
  * @retval None
  */
static void OnClassChange(DeviceClass_t deviceClass);

/**
  * @brief callback when end node does synchronization
  * @param None
  * @retval None
  */
static void OnSysTimeUpdate(void);

/**
  * @brief  Init `data` buffer of `size` starting at address `addr`
  * @param  addr Address start index to erase.
  * @param  size number of bytes.
  * @retval status Init operation status [0: Success, -1 Fail]
  */
static uint8_t FragDecoderErase(uint32_t addr, uint32_t size);

/**
  * @brief callback to store the data fragment received from network
  * @param [in] addr Address start index to write to.
  * @param [in] data Data buffer to be written.
  * @param [in] size Size of data buffer to be written.
  * @retval status [0: OK, -1 KO]
  */
static uint8_t FragDecoderWrite(uint32_t addr, uint8_t *data, uint32_t size);

/**
  * @brief callback to read data fragment which has been stored in memory
  * @param [in] addr Address start index to read from.
  * @param [in] data Data buffer to be read.
  * @param [in] size Size of data buffer to be read.
  * @retval status [0: OK, -1 KO]
  */
static uint8_t FragDecoderRead(uint32_t addr, uint8_t *data, uint32_t size);

/**
  * @brief callback to follow the data fragment downloading
  * @param deviceClass
  * @retval None
  */
static void OnFragProgress(uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost);

/**
  * @brief callback to notify that the compete data block has been received
  * @param deviceClass
  * @retval None
  */
static void OnFragDone(int32_t status, uint32_t size);

static uint32_t Crc32(uint8_t *buffer, uint16_t length);

/**
  * @brief User application buffer
  */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

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
        .OnRxData = OnRxData,
        .OnClassChange = OnClassChange,
        .OnSysTimeUpdate = OnSysTimeUpdate
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
        .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY};

#define UNFRAGMENTED_DATA_SIZE                     ( FRAG_MAX_NB * FRAG_MAX_SIZE )

/*
 * Un-fragmented data storage.
 */
static uint8_t UnfragmentedData[UNFRAGMENTED_DATA_SIZE];

LmhpFragmentationParams_t FragmentationParams =
{
  .DecoderCallbacks =
  {
    .FragDecoderErase = FragDecoderErase,
    .FragDecoderWrite = FragDecoderWrite,
    .FragDecoderRead = FragDecoderRead,
  },

  .OnProgress = OnFragProgress,
  .OnDone = OnFragDone
};

/*
 * Indicates if LoRaMacProcess call is pending.
 *
 * warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

/*!
 * Indicates if a Tx frame is pending.
 *
 * \warning Set to 1 when OnTxTimerEvent raised
 */
static volatile uint8_t IsTxFramePending = 0;

/*
 * Indicates if the system time has been synchronized
 */
static volatile bool IsClockSynched = false;

/*
 * MC Session Started
 */
static volatile bool IsMcSessionStarted = false;

/*
 * Indicates if the file transfer is done
 */
static volatile bool IsFileTransferDone = false;

/*
 *  Received file computed CRC32
 */
static volatile uint32_t FileRxCrc = 0;

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

  LmHandlerPackageRegister(PACKAGE_ID_CLOCK_SYNC, NULL);

  LmHandlerPackageRegister(PACKAGE_ID_REMOTE_MCAST_SETUP, NULL);

  LmHandlerPackageRegister(PACKAGE_ID_FRAGMENTATION, &FragmentationParams);

  LmHandlerConfigure(&LmHandlerParams);

  /* state variable to indicate synchronization done*/
  IsClockSynched = false;

  /* state variable to indicate data block transfer done*/
  IsFileTransferDone = false;

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
    static const char *slotStrings[] = {"1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot"};

    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n ###### ========== MCPS-Indication ==========\r\n");
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n ###### D/L FRAME:%04d | SLOT:%s | PORT:%d | DR:%d | RSSI:%d | SNR:%d\r\n",
            params->DownlinkCounter, slotStrings[params->RxSlot], appData->Port, params->Datarate, params->Rssi, params->Snr);
  }
}

static void SendTxData(void)
{
  LmHandlerErrorStatus_t status = LORAMAC_HANDLER_ERROR;

  uint8_t isPending = 0;
  CRITICAL_SECTION_BEGIN();
  isPending = IsTxFramePending;
  IsTxFramePending = 0;
  CRITICAL_SECTION_END();
  if (isPending == 1)
  {
    if (LmHandlerIsBusy() == true)
    {
      return;
    }

    if (IsMcSessionStarted == false)
    {
      /*
       * Currently in Class A
       * Request AppTimeReq to initiate FUOTA
       */
      if (IsClockSynched == false)
      {
        APP_PPRINTF("\r\n Clock sync in progress, requesting AppTimeReq to initiate FUOTA \r\n");
        status = LmhpClockSyncAppTimeReq();
      }
      else
      {
        APP_PPRINTF("\r\n Clock sync successful, sending random uplink to continue with FUOTA \r\n");
        AppDataBuffer[0] = randr(0, 255);
        /* Send random packet */
        LmHandlerAppData_t appData =
        {
          .Buffer = AppDataBuffer,
          .BufferSize = 1,
          .Port = 1
        };
        status = LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true);
      }
    }
    else
    {
      /*
       * Currently in Class C
       * FUOTA process will be activated
       */
      if (IsFileTransferDone == false)
      {
        /* do nothing up until the transfer is done */
      }
      else
      {
        APP_PPRINTF("\r\n File transfer successful, sending the CRC32 value of the received file\r\n");
        AppDataBuffer[0] = 0x05; // FragDataBlockAuthReq
        AppDataBuffer[1] = FileRxCrc & 0x000000FF;
        AppDataBuffer[2] = (FileRxCrc >> 8) & 0x000000FF;
        AppDataBuffer[3] = (FileRxCrc >> 16) & 0x000000FF;
        AppDataBuffer[4] = (FileRxCrc >> 24) & 0x000000FF;

        /* Send FragAuthReq */
        LmHandlerAppData_t appData =
        {
          .Buffer = AppDataBuffer,
          .BufferSize = 5,
          .Port = 201
        };
        status = LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true);
      }

      if (status == LORAMAC_HANDLER_SUCCESS)
      {
        /* CRC32 is returned to the server */
        APP_PPRINTF("\r\n CRC sent to server \n\r");
      }
    }
  }
}

static void OnTxTimerEvent(void *context)
{
  IsTxFramePending = 1;
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimer), CFG_SEQ_Prio_0);

  /* Wait for next tx slot */
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

static void OnClassChange(DeviceClass_t deviceClass)
{
  APP_PPRINTF("\r\n...... Switch to Class %c done. .......\r\n", "ABC"[deviceClass]);

  switch (deviceClass)
  {
    default:
    case CLASS_A:
    {
      IsMcSessionStarted = false;
      break;
    }
    case CLASS_B:
    {
      /* Inform the server as soon as possible that the end-device has switched to ClassB */
      LmHandlerAppData_t appData =
      {
        .Buffer = NULL,
        .BufferSize = 0,
        .Port = 0
      };
      LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true);
      IsMcSessionStarted = true;
      break;
    }
    case CLASS_C:
    {
      IsMcSessionStarted = true;
      break;
    }
  }
}

static void OnSysTimeUpdate(void)
{
  IsClockSynched = true;
}

static void OnMacProcessNotify(void)
{
  IsMacProcessPending = 1;
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);
}

static void OnFragProgress(uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost)
{
  APP_PPRINTF("\r\n....... FRAG_DECODER in Progress .......\r\n");
  APP_PPRINTF("RECEIVED    : %5d / %5d Fragments\r\n", fragCounter, fragNb);
  APP_PPRINTF("              %5d / %5d Bytes\r\n", fragCounter * fragSize, fragNb * fragSize);
  APP_PPRINTF("LOST        :       %7d Fragments\r\n\r\n", fragNbLost);
}


static void OnFragDone(int32_t status, uint32_t size)
{
  IsFileTransferDone = true;
  APP_PPRINTF("\r\n....... FRAG_DECODER Finished .......\r\n");
  APP_PPRINTF("STATUS      : %d\r\n", status);

  FileRxCrc = Crc32(UnfragmentedData, size);
  APP_PPRINTF("Size      : %d\r\n", size);
  APP_PPRINTF("CRC         : %08X\r\n\r\n", FileRxCrc);
}

static uint32_t Crc32(uint8_t *buffer, uint16_t length)
{
  // The CRC calculation follows CCITT - 0x04C11DB7
  const uint32_t reversedPolynom = 0xEDB88320;

  // CRC initial value
  uint32_t crc = 0xFFFFFFFF;

  if (buffer == NULL)
  {
    return 0;
  }

  for (uint16_t i = 0; i < length; ++i)
  {
    crc ^= (uint32_t)buffer[i];
    for (uint16_t i = 0; i < 8; i++)
    {
      crc = (crc >> 1) ^ (reversedPolynom & ~((crc & 0x01) - 1));
    }
  }

  return ~crc;
}

static uint8_t FragDecoderErase(uint32_t addr, uint32_t size)
{
  if (size >= UNFRAGMENTED_DATA_SIZE)
  {
    return (uint8_t) - 1; /* Fail */
  }

  for (uint32_t i = 0; i < size; i++)
  {
    UnfragmentedData[addr + i] = 0xFF;
  }
  return 0; /* Success */
}

static uint8_t FragDecoderWrite(uint32_t addr, uint8_t *data, uint32_t size)
{
  if (size >= UNFRAGMENTED_DATA_SIZE)
  {
    return (uint8_t) - 1; /* Fail */
  }

  for (uint32_t i = 0; i < size; i++)
  {
    UnfragmentedData[addr + i] = data[i];
  }

  return 0; // Success
}

static uint8_t FragDecoderRead(uint32_t addr, uint8_t *data, uint32_t size)
{
  if (size >= UNFRAGMENTED_DATA_SIZE)
  {
    return (uint8_t) - 1; /* Fail */
  }

  for (uint32_t i = 0; i < size; i++)
  {
    data[i] = UnfragmentedData[addr + i];
  }
  return 0; // Success
}

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
#include "LmHandler.h"
#include "lora_info.h"
#include "sensors.h"

static uint32_t sensors_tx_dutycycle = SENSORS_TX_DUTYCYCLE_DEFAULT_M * 60000;

volatile uint8_t button_press = 0; 

static int32_t temp_fallback = 0;
static int32_t humid_fallback = 0;

// static uint8_t uplink_count_test = 0;

/**
  * @brief LoRa State Machine states
  */
typedef enum TxEventType_e
{
  /**
    * @brief Application data transmission issue based on timer every TxDutyCycleTime
    */
  TX_ON_TIMER,
  /**
    * @brief AppdataTransmition external event plugged on OnSendEvent( )
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
  * @brief  RX LED timer callback function
  * @param  LED context
  * @return none
  */
static void OnRxTimerLedEvent(void *context);

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
        .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY};

/**
  * @brief Type of Event to generate application Tx
  */
static TxEventType_t EventType = TX_ON_TIMER;

/**
  * @brief Timer to handle the application Tx
  */
static UTIL_TIMER_Object_t TxTimer;

/**
  * @brief Timer to handle rx led events
  */
static UTIL_TIMER_Object_t RxLedTimer;

void LoRaWAN_Init(void)
{
  // User can add any indication here (LED manipulation or Buzzer)
  GNSE_BSP_LED_Init(LED_BLUE);
  GNSE_BSP_LED_Init(LED_RED);
  GNSE_BSP_LED_Init(LED_GREEN);

  GNSE_BSP_LED_On(LED_GREEN);

  GNSE_BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI); // each button press will call the HAL_GPIO_EXTI_Callback

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, SendTxData);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);

  LmHandlerJoin(ActivationType);

  if (EventType == TX_ON_TIMER)
  {
    /* send every time timer elapses */
    UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxTimer, sensors_tx_dutycycle);
    UTIL_TIMER_Start(&TxTimer);
  }
  else
  {
    GNSE_BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
  }

  HAL_Delay(1000);
  GNSE_BSP_LED_Off(LED_GREEN);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BUTTON_SW1_PIN)
  {
    /* Note: when "EventType == TX_ON_TIMER" this GPIO is not initialised */
    button_press = 1;
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
  }
}

/*
Downlink interval settings
0x0A - 10 seconds
0x3C - 60 seconds
0x258 - 10 minutes
0xE10 - 1 hour
*/

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  uint32_t rxbuffer = 0;
  if ((appData != NULL) && (params != NULL))
  {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Received Downlink on F_PORT:%d \r\n", appData->Port);
    rxbuffer = sensors_downlink_conf_check(appData);
    if (rxbuffer)
    {
        UTIL_TIMER_Create(&RxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnRxTimerLedEvent, NULL);
        UTIL_TIMER_SetPeriod(&RxLedTimer, SENSORS_LED_RX_PERIOD_MS);
        UTIL_TIMER_Start(&RxLedTimer);
        UTIL_TIMER_SetPeriod(&TxTimer, rxbuffer);
    }
    else /* Function returns 0 on fail */
    {
        UTIL_TIMER_Create(&RxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnRxTimerLedEvent, NULL);
        UTIL_TIMER_SetPeriod(&RxLedTimer, SENSORS_LED_UNHANDLED_RX_PERIOD_MS);
        UTIL_TIMER_Start(&RxLedTimer);
    }
  }
}

static void SendTxData(void)
{
  sensors_t sensor_data;
  UTIL_TIMER_Time_t nextTxIn = 0;

  extern int32_t temp_fallback;
  extern int32_t humid_fallback;
//   extern uint8_t uplink_count_test;

  if(button_press == 1){
      GNSE_BSP_LED_On(LED_RED);
  }else{
      GNSE_BSP_LED_On(LED_BLUE);
  }
  
  sensors_op_result_t rc = sensors_sample(&sensor_data);

//   // Simulate sensor failure every 4th uplink to check if fallback values work
//   if(uplink_count_test % 4 == 0)
//   {
//     uplink_count_test = 0;
//     rc = SENSORS_OP_FAIL;
//   }
//   uplink_count_test ++;

  // Use the last reading of temperature and humidity in case sensor reading failed
  if(rc != SENSORS_OP_SUCCESS)
  {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H,"Sensors returned SENSORS_OP_FAIL \r\n");
    sensor_data.temperature = temp_fallback;
    sensor_data.humidity = humid_fallback;
  }

  temp_fallback = sensor_data.temperature;
  humid_fallback = sensor_data.humidity;
  APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "Temp fallback now: %d, Humid fallback now: %d \r\n", temp_fallback, humid_fallback);
  APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "Sensor Readings (raw): VBAT = %d,  T:%d,  H:%d \r\n",
          sensor_data.battery_voltage, sensor_data.temperature, sensor_data.humidity );

  uint8_t vbat_uint = (uint8_t)(sensor_data.battery_voltage / 100);
  uint16_t temp_uint = (uint16_t)((sensor_data.temperature / 100) + 500); // negative temp offset
  uint16_t humidity_uint = (uint16_t)(sensor_data.humidity / 100);

  APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "Sensor Readings (fmt): VBAT = %d,  T:%d,  H:%d \r\n", vbat_uint, temp_uint-500, humidity_uint );

  AppData.Port = SENSORS_PAYLOAD_APP_PORT;
  AppData.BufferSize = 6;
  AppData.Buffer[0] = vbat_uint;
  AppData.Buffer[1] = (uint8_t)(temp_uint >> 8);
  AppData.Buffer[2] = (uint8_t)(temp_uint & 0xFF);
  AppData.Buffer[3] = (uint8_t)(humidity_uint >> 8);
  AppData.Buffer[4] = (uint8_t)(humidity_uint & 0xFF);
  AppData.Buffer[5] = button_press; 

  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "SEND REQUEST\r\n");
  }
  else if (nextTxIn > 0)
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
  }
  
  if(button_press == 1){
      GNSE_BSP_LED_Off(LED_RED);
  }else{
      GNSE_BSP_LED_Off(LED_BLUE);
  }
  button_press = 0; // clear the button press state
}

static void OnTxTimerEvent(void *context)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);

  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
}

static void OnRxTimerLedEvent(void *context)
{
  static uint8_t led_counter = 0;

  if (led_counter == 0)
  {
    GNSE_BSP_LED_Init(LED_GREEN);
  }
  if (led_counter < SENSORS_LED_RX_TOGGLES)
  {
    GNSE_BSP_LED_Toggle(LED_GREEN);
    led_counter++;
  }
  else
  {
    led_counter = 0;
    GNSE_BSP_LED_Analog(LED_GREEN);
    UTIL_TIMER_Stop(&RxLedTimer);

  }
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

#include "app.h"
#include "Region.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "LmHandler.h"
#include "lora_info.h"
#include "sensors.h"
#include "GNSE_bm.h"
#include "GNSE_lpm.h"

static uint32_t heartbeat_tx_dutycycle = HEARTBEAT_TX_DUTYCYCLE_DEFAULT_S * 1000;
static uint32_t temperature_tx_dutycycle = TEMPERATURE_TX_DUTYCYCLE_DEFAULT_S * 1000;

static void SendHeartBeatData(void);
static void SendTemperatureData(void);
static void SendAccelerometerData(void);

static void heartbeat_rx_handle(LmHandlerAppData_t *appData);
static void HeartBeatTimerEvent(void *context);
static void TemperatureTimerEvent(void *context);
static void AccelerometerShakeEvent(void *context);
static void AccelerometerFreeallEvent(void *context);
static void BuzzerTimerEvent(void *context);

static void OnTxTimerLedEvent(void *context);
static void OnRxTimerLedEvent(void *context);
static void OnJoinTimerLedEvent(void *context);
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);
static void OnTxData(LmHandlerTxParams_t *params);
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

static void heartbeat_rx_handle(LmHandlerAppData_t *appData);
static void temperature_sensor_rx_handle(LmHandlerAppData_t *appData);
static void accelerometer_sensor_rx_handle(LmHandlerAppData_t *appData);
static void button_rx_handle(LmHandlerAppData_t *appData);
static void buzzer_rx_handle(LmHandlerAppData_t *appData);
static void led_rx_handle(LmHandlerAppData_t *appData);

static void OnMacProcessNotify(void);
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];
static LmHandlerAppData_t AppData = {0, 0, AppDataBuffer};
static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;
static LmHandlerCallbacks_t LmHandlerCallbacks =
    {
        .GetBatteryLevel = GetBatteryLevel,
        .GetTemperature = GetTemperatureLevel,
        .OnMacProcess = OnMacProcessNotify,
        .OnJoinRequest = OnJoinRequest,
        .OnTxData = OnTxData,
        .OnRxData = OnRxData};

static LmHandlerParams_t LmHandlerParams =
    {
        .ActiveRegion = ACTIVE_REGION,
        .DefaultClass = LORAWAN_DEFAULT_CLASS,
        .AdrEnable = LORAWAN_ADR_STATE,
        .TxDatarate = LORAWAN_DEFAULT_DATA_RATE,
        .JoinDatarate = LORAWAN_DEFAULT_JOIN_DATA_RATE,
        .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY};

static UTIL_TIMER_Object_t HearBeatTxTimer;
static UTIL_TIMER_Object_t TemperatureTxTimer;
static UTIL_TIMER_Object_t BuzzerTimer;

static UTIL_TIMER_Object_t TxLedTimer;
static UTIL_TIMER_Object_t RxLedTimer;
static UTIL_TIMER_Object_t JoinLedTimer;

void LoRaWAN_Init(void)
{
  UTIL_TIMER_Create(&TxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerLedEvent, NULL);
  UTIL_TIMER_Create(&RxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnRxTimerLedEvent, NULL);
  UTIL_TIMER_Create(&JoinLedTimer, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnJoinTimerLedEvent, NULL);
  UTIL_TIMER_SetPeriod(&TxLedTimer, 500);
  UTIL_TIMER_SetPeriod(&RxLedTimer, 500);
  UTIL_TIMER_SetPeriod(&JoinLedTimer, 500);

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendHeartBeatOnTxTimer), UTIL_SEQ_RFU, SendHeartBeatData);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendTemperatureOnTxTimer), UTIL_SEQ_RFU, SendTemperatureData);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnAccelerometerEvent), UTIL_SEQ_RFU, SendAccelerometerData);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnButtonEvent), UTIL_SEQ_RFU, SendTemperatureData);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);

  UTIL_TIMER_Start(&JoinLedTimer);
  LmHandlerJoin(ActivationType);

  UTIL_TIMER_Create(&HearBeatTxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, HeartBeatTimerEvent, NULL);
  UTIL_TIMER_SetPeriod(&HearBeatTxTimer, heartbeat_tx_dutycycle);
  UTIL_TIMER_Start(&HearBeatTxTimer);

  UTIL_TIMER_Create(&TemperatureTxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, TemperatureTimerEvent, NULL);
  UTIL_TIMER_SetPeriod(&TemperatureTxTimer, temperature_tx_dutycycle);

  UTIL_TIMER_Create(&BuzzerTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, BuzzerTimerEvent, NULL);
  UTIL_TIMER_SetPeriod(&BuzzerTimer, 1000);

  GNSE_BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == ACC_INT_PIN)
  {
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnAccelerometerEvent), CFG_SEQ_Prio_0);
  }
  if (GPIO_Pin == BUTTON_SW1_PIN)
  {
    /* Note: when "EventType == TX_ON_TIMER" this GPIO is not initialised */
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnButtonEvent), CFG_SEQ_Prio_0);
  }
}

static void SendTemperatureData(void)
{
  sensors_t sensor_data;
  UTIL_TIMER_Time_t nextTxIn = 0;

  temperature_sample(&sensor_data);
  AppData.Port = TX_TEMPERATURE_SENSOR_PORT;
  AppData.BufferSize = TX_TEMPERATURE_SENSOR_BUFFER_SIZE;
  AppData.Buffer[0] = (uint8_t)((sensor_data.temperature / 100) >> 8);
  AppData.Buffer[1] = (uint8_t)((sensor_data.temperature / 100) & 0xFF);
  AppData.Buffer[2] = (uint8_t)((sensor_data.humidity / 100) >> 8);
  AppData.Buffer[3] = (uint8_t)((sensor_data.humidity / 100) & 0xFF);

  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "SEND REQUEST\r\n");
  }
  else if (nextTxIn > 0)
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
  }
}

static void SendHeartBeatData(void)
{
  UTIL_TIMER_Time_t nextTxIn = 0;
  uint16_t battery_voltage = GNSE_BM_GetBatteryVoltage();

  AppData.Port = TX_HEARTBEAT_PORT;
  AppData.BufferSize = TX_HEARTBEAT_BUFFER_SIZE;
  AppData.Buffer[0] = (uint8_t)(battery_voltage / 100);
  AppData.Buffer[1] = (uint8_t)(GNSE_FW_VERSION_MAIN);
  AppData.Buffer[2] = (uint8_t)(GNSE_FW_VERSION_SUB1);
  AppData.Buffer[3] = (uint8_t)(GNSE_HW_VERSION_MAIN);
  AppData.Buffer[4] = (uint8_t)(GNSE_HW_VERSION_SUB1);

  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "SEND REQUEST\r\n");
  }
  else if (nextTxIn > 0)
  {
    APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
  }
}

void SendAccelerometerData(void)
{
  ACC_IT_Handler();
}

static void HeartBeatTimerEvent(void *context)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendHeartBeatOnTxTimer), CFG_SEQ_Prio_0);
  UTIL_TIMER_Start(&HearBeatTxTimer);
}

static void TemperatureTimerEvent(void *context)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendTemperatureOnTxTimer), CFG_SEQ_Prio_0);
  UTIL_TIMER_Start(&TemperatureTxTimer);
}

static void BuzzerTimerEvent(void *context)
{
  BUZZER_SetState(BUZZER_STATE_OFF);
  BUZZER_DeInit();
  GNSE_LPM_SetStopMode((1 << GNSE_LPM_TIM_BUZZER), GNSE_LPM_ENABLE);
}

static void OnTxTimerLedEvent(void *context)
{
  GNSE_BSP_LED_Off(LED_GREEN);
}

static void OnRxTimerLedEvent(void *context)
{
  GNSE_BSP_LED_Off(LED_RED);
}

static void OnJoinTimerLedEvent(void *context)
{
  GNSE_BSP_LED_Toggle(LED_RED);
}

static void OnTxData(LmHandlerTxParams_t *params)
{
  if ((params != NULL) && (params->IsMcpsConfirm != 0))
  {
    GNSE_BSP_LED_On(LED_GREEN);
    UTIL_TIMER_Start(&TxLedTimer);

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

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  if ((appData != NULL) && (params != NULL))
  {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Received Downlink on F_PORT:%d \r\n", appData->Port);
    switch (appData->Port)
    {
    case RX_HEARTBEAT_PORT:
      heartbeat_rx_handle(appData);
      break;
    case RX_TEMPERATURE_SENSOR_PORT:
      temperature_sensor_rx_handle(appData);
      break;
    case RX_ACCELEROMETER_SENSOR_PORT:
      accelerometer_sensor_rx_handle(appData);
      break;
    case RX_BUTTON_PORT:
      button_rx_handle(appData);
      break;
    case RX_BUZZER_PORT:
      buzzer_rx_handle(appData);
      break;
    case RX_LED_PORT:
      led_rx_handle(appData);
      break;
    default:
      break;
    }
  }
  GNSE_BSP_LED_Off(LED_GREEN);
  GNSE_BSP_LED_On(LED_RED);
  UTIL_TIMER_Start(&RxLedTimer);
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### = JOINED OTAA = ");
      UTIL_TIMER_Stop(&JoinLedTimer);
      GNSE_BSP_LED_Off(LED_RED);
      UTIL_TIMER_Start(&TemperatureTxTimer);
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

static void heartbeat_rx_handle(LmHandlerAppData_t *appData)
{
  if (appData->BufferSize == RX_HEARTBEAT_BUFFER_SIZE)
  {
    if (appData->Buffer[0] == 0)
    {
      UTIL_TIMER_Stop(&HearBeatTxTimer);
    }
    else if (appData->Buffer[0] >= HEARTBEAT_DUTYCYCLE_MIN_MINUTES && appData->Buffer[0] <= HEARTBEAT_DUTYCYCLE_MAX_MINUTES)
    {
      heartbeat_tx_dutycycle = ((appData->Buffer[0]) * MINUTES_TO_MS);
      UTIL_TIMER_SetPeriod(&HearBeatTxTimer, heartbeat_tx_dutycycle);
      UTIL_TIMER_Start(&HearBeatTxTimer);
    }
  }
}

static void temperature_sensor_rx_handle(LmHandlerAppData_t *appData)
{
  if (appData->BufferSize == RX_TEMPERATURE_SENSOR_BUFFER_SIZE)
  {
    if (appData->Buffer[0] == 0)
    {
      UTIL_TIMER_Stop(&TemperatureTxTimer);
    }
    else if (appData->Buffer[0] >= TEMPERATURE_DUTYCYCLE_MIN_MINUTES && appData->Buffer[0] <= TEMPERATURE_DUTYCYCLE_MAX_MINUTES)
    {
      temperature_tx_dutycycle = ((appData->Buffer[0]) * MINUTES_TO_MS);
      UTIL_TIMER_SetPeriod(&TemperatureTxTimer, temperature_tx_dutycycle);
      UTIL_TIMER_Start(&TemperatureTxTimer);
    }
  }
}

static void accelerometer_sensor_rx_handle(LmHandlerAppData_t *appData)
{
  if (appData->BufferSize == RX_ACCELEROMETER_SENSOR_BUFFER_SIZE)
  {
    if (appData->Buffer[0] == 0)
    {
      //TODO: Disable interrupts and allow the system to kill accelerometer
      GNSE_BSP_Acc_Int_DeInit();
    }
    else if (appData->Buffer[0] == 0x01)
    {
      //TODO: Prevent the system from killing the accelerometer
      ACC_Shake_Enable();
    }
    else if (appData->Buffer[0] == 0x02)
    {
      //TODO: Prevent the system from killing the accelerometer
      ACC_FreeFall_Enable();
    }
  }
}

static void button_rx_handle(LmHandlerAppData_t *appData)
{
  if (appData->BufferSize == RX_BUTTON_PORT)
  {
    if (appData->Buffer[0] == 0)
    {
      //TODO: Allow the system to fully sleep?
      GNSE_BSP_PB_DeInit(BUTTON_SW1);
    }
    else
    {
      GNSE_BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
    }
  }
}

static void buzzer_rx_handle(LmHandlerAppData_t *appData)
{
  if (appData->BufferSize == RX_BUZZER_BUFFER_SIZE)
  {
    if (appData->Buffer[0] == 0x01)
    {
      BUZZER_SetState(BUZZER_STATE_OFF);
      GNSE_LPM_SetStopMode((1 << GNSE_LPM_TIM_BUZZER), GNSE_LPM_DISABLE);
      UTIL_TIMER_Start(&BuzzerTimer);
      BUZZER_SetState(BUZZER_STATE_WARNING);
    }
    else if (appData->Buffer[0] == 0x02)
    {
      BUZZER_SetState(BUZZER_STATE_OFF);
      GNSE_LPM_SetStopMode((1 << GNSE_LPM_TIM_BUZZER), GNSE_LPM_DISABLE);
      UTIL_TIMER_Start(&BuzzerTimer);
      BUZZER_SetState(BUZZER_STATE_DANGER);
    }
  }
}

static void led_rx_handle(LmHandlerAppData_t *appData)
{
  if (appData->BufferSize == RX_LED_BUFFER_SIZE)
  {
    if (appData->Buffer[0] == 0)
    {
      GNSE_BSP_LED_Off(LED_BLUE);
    }
    else
    {
      GNSE_BSP_LED_On(LED_BLUE);
    }
  }
}

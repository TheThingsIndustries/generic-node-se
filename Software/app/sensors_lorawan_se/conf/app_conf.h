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
 * @file app_conf.h
 * @brief Common configuration file for GNSE applications
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef APP_CONF_H
#define APP_CONF_H

#define GNSE_ADVANCED_TRACER_ENABLE 1

/* if ON (=1) it enables the debugger plus 4 dbg pins */
/* if OFF (=0) the debugger is OFF (lower consumption) */
#define DEBUGGER_ON       0

/*!
 * LoRaWAN application port where sensors information can be retrieved by the application server
 * @note do not use 224. It is reserved for certification
 */
#define SENSORS_PAYLOAD_APP_PORT        2

/*! LoRaWAN application port where sensors transmission/uplink interval can be changed (via a downlink)
 *  @note Payload signifies the amount of minutes
 * */
#define SENSORS_DOWNLINK_CONF_PORT        1

/* Default, max, and min time interval of the TX dutycycle window in minutes */
#define SENSORS_TX_DUTYCYCLE_DEFAULT_M 10
#define SENSORS_DUTYCYCLE_CONF_MAX_M 8640
#define SENSORS_DUTYCYCLE_CONF_MIN_M 1

/**
  * RX LED definitions
  */
#define SENSORS_LED_RX_PERIOD_MS 200
#define SENSORS_LED_UNHANDLED_RX_PERIOD_MS 20
#define SENSORS_LED_RX_TOGGLES 5

#if !WITH_SECURE_ELEMENT
/**
  * Define keys here if not using the secure element
  */
/* LoRaWAN v1.0.2 software based OTAA activation information */
#define APPEUI                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define DEVEUI                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define APPKEY                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

#endif

/**
  * sequencer definitions
  */

/**
  * This is the list of priority required by the application
  * Each Id shall be in the range 0..31
  */
typedef enum
{
  CFG_SEQ_Prio_0,
  CFG_SEQ_Prio_NBR,
} CFG_SEQ_Prio_Id_t;

/**
  * This is the list of task id required by the application
  * Each Id shall be in the range 0..31
  */
typedef enum
{
  CFG_SEQ_Task_LmHandlerProcess,
  CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent,
  CFG_SEQ_Task_NBR
} CFG_SEQ_Task_Id_t;

#endif /* APP_CONF_H */

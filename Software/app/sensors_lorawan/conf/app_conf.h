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

#define GNSE_ADVANCED_TRACER_ENABLE 0

/* if ON (=1) it enables the debugger plus 4 dbg pins */
/* if OFF (=0) the debugger is OFF (lower consumption) */
#define DEBUGGER_ON       1

/*!
 * LoRaWAN application port where sensors information can be retrieved by the application server
 * @note do not use 224. It is reserved for certification
 */
#define SENSORS_PAYLOAD_APP_PORT        2

/*! LoRaWAN application port where sensors transmission/uplink interval can be changed (via a downlink)
 *  @note Payload signifies the amount of seconds
 * */
#define SENSORS_DOWNLINK_CONF_PORT        1

/* Default, max, and min time interval of the TX dutycycle window in seconds */
#define SENSORS_TX_DUTYCYCLE_DEFAULT_S 60
#define SENSORS_DUTYCYCLE_CONF_MAX_S 8640
#define SENSORS_DUTYCYCLE_CONF_MIN_S 5

/**
  * RX LED definitions
  */
#define SENSORS_LED_RX_PERIOD_MS 200
#define SENSORS_LED_UNHANDLED_RX_PERIOD_MS 20
#define SENSORS_LED_RX_TOGGLES 5

/* LoRaWAN v1.0.2 software based OTAA activation information 
#define APPEUI                 0x71, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0x00, 0x00
#define DEVEUI                 0x00, 0x04, 0xA3, 0x10, 0x00, 0x1A, 0xAE, 0x83
#define APPKEY                0x9D, 0x79, 0x85, 0x76, 0xAF, 0xF9, 0xE5, 0xF7, 0x63, 0xCA, 0x03, 0xF9, 0x70, 0xF6, 0x38, 0x04
*/

// eui-70b3d57ed004b88e
#define APPEUI                 0x07, 0x10, 0xB3, 0x0D, 0x50, 0x7E, 0x0D, 0x00
#define DEVEUI                 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xB8, 0x8E
#define APPKEY                0x58, 0xA6, 0x98, 0xB2, 0x62, 0xEF, 0xBD, 0xD8, 0x1B, 0x55, 0x20, 0x75, 0xA4, 0x7B, 0x24, 0x52


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

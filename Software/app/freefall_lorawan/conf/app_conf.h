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

/* if ON (=1) the independent watchdog timer is used (reset MCU after inactivity) */
/* if OFF (=0) the independent watchdog timer is not used */
#define IWDG_TIMER_ON     0

/* LOW_POWER_DISABLE = 0 : LowPowerMode enabled : MCU enters stop2 mode*/
/* LOW_POWER_DISABLE = 1 : LowPowerMode disabled : MCU enters sleep mode only */
#define LOW_POWER_DISABLE 0

/* LoRaWAN v1.0.2 software based OTAA activation information */
#define APPEUI                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define DEVEUI                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define APPKEY                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

/**
  * FF_THRESHOLD is defined as the value all axes are +/-0g, depending on the scale
  * @2g scale 1 LSb = 16 mg
  * @4g scale 1 LSb = 32 mg
  * @8g scale 1 LSb = 62 mg
  * @16g scale 1 LSb = 186 mg
  * FF_DURATION is defined as the time below the threshold times the Output Data Rate
  * @100Hz ODR a value for FREEFALL_DURATION = 3 means a duration of 30 ms below the threshold.
  */
#define ACC_FF_THRESHOLD 0x16
#define ACC_FF_DURATION 0x03
#define ACC_FF_SCALE LIS2DH12_2g
#define ACC_FF_ODR LIS2DH12_ODR_100Hz

/**
 * This variable sets the LoRaWAN transmission port of free fall events
 */
#define ACC_FF_LORA_PORT 2

/**
 * Downlink defs
 */
/* This variable sets the LoRaWAN downlink port for locally indicating free fall events */
#define ACC_FF_DOWNLINK_PORT 1
/* Time in milliseconds the downlink callback (controlling the buzzer) will be initiated */
#define ACC_FF_DOWNLINK_TIME_MS 2000

/**
  * Supported requester to the MCU Low Power Manager - can be increased up  to 32
  * It lists a bit mapping of all user of the Low Power Manager
  */
typedef enum
{
  CFG_LPM_APPLI_Id,
  CFG_LPM_UART_TX_Id,
  CFG_LPM_TCXO_WA_Id,
  CFG_LPM_FF_ACC_Id,
} CFG_LPM_Id_t;

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
  CFG_SEQ_Task_NBR
} CFG_SEQ_Task_Id_t;

#endif /* APP_CONF_H */

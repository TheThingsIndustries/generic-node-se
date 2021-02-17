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

#include "lorawan_conf.h"

#define APP_LOG_ENABLED 1

/* if ON (=1) it enables the debugger plus 4 dgb pins */
/* if OFF (=0) the debugger is OFF (lower consumption) */
#define DEBUGGER_ON       1


/* LOW_POWER_DISABLE = 0 : LowPowerMode enabled : MCU enters stop2 mode*/
/* LOW_POWER_DISABLE = 1 : LowPowerMode disabled : MCU enters sleep mode only */
#define LOW_POWER_DISABLE 1

/**
 * @brief Stack size for LoRaWAN Class A task.
 */
#define LORAWAN_CLASSA_TASK_STACK_SIZE    ( 512 )


/**
 * @brief Prirority for LoRaWAN Class A task.
 * Priority is set to lowest task priority which is above the idle task priority.
 */
#define LORAWAN_CLASSA_TASK_PRIORITY    ( tskIDLE_PRIORITY + 1 )


/* The SPI driver polls at a high priority. The logging task's priority must also
 * be high to be not be starved of CPU time. */
#define mainLOGGING_TASK_PRIORITY                         ( configMAX_PRIORITIES - 1 )
#define mainLOGGING_TASK_STACK_SIZE                       ( 512 )
#define mainLOGGING_MESSAGE_QUEUE_LENGTH                  ( 15 )

/**
 * @brief Default region is set to EU868. Application can choose to configure a different region
 * by defining the region in lorawan_conf.h
 * region.
 */
#ifdef REGION_EU868
#define LORAWAN_APP_REGION    LORAMAC_REGION_EU868
#elif REGION_US915
#define LORAWAN_APP_REGION    LORAMAC_REGION_US915
#endif

/**
 * @brief LoRa MAC layer port used by the application.
 * Downlink unicast messages should be send to this port number.
 */
#define LORAWAN_APP_PORT                       ( 2 )

/**
 * @brief Should send confirmed messages (with an acknowledgment) or not.
 */
#define LORAWAN_CONFIRMED_SEND                 ( 0 )

/**
 * @brief Defines the application data transmission duty cycle time in milliseconds.
 *
 * Although there is a duty cycle restriction which allows to transmit for only a portion of time on a channel,
 * there are also additional policies enforced by networks to prevent interference and quality degradation for transmission.
 *
 * For example TTN enforces uplink of 36sec airtime/day and 10 downlink messages/day. Demo sends 1 byte of unconfirmed uplink,
 * using a default data rate of DR_0 (SF = 10, BW=125Khz) for US915, so airtime for each uplink ~289ms, hence setting the interval
 * to 700 seconds to achieve maximum air time of 36sec per day.
 *
 */
#define LORAWAN_APPLICATION_TX_INTERVAL_SEC    ( 10U )

/**
 * @brief Defines a random jitter bound in milliseconds for application data transmission duty cycle.
 *
 * This allows devices to space their transmissions slightly between each other in cases like all devices reboots and tries to
 * join server at same time.
 */
#define LORAWAN_APPLICATION_JITTER_MS          ( 500 )

/**
  * Supported requester to the MCU Low Power Manager - can be increased up  to 32
  * It lists a bit mapping of all user of the Low Power Manager
  */
typedef enum
{
  CFG_LPM_APPLI_Id,
  CFG_LPM_UART_TX_Id,
  CFG_LPM_TCXO_WA_Id,
} CFG_LPM_Id_t;

#endif /* APP_CONF_H */

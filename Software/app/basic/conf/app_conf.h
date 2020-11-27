/** Copyright © 2020 The Things Industries B.V.
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
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#ifndef APP_CONF_H
#define APP_CONF_H

#define APP_LOG_ENABLED 1

/* if ON (=1) it enables the debbugger plus 4 dgb pins */
/* if OFF (=0) the debugger is OFF (lower consumption) */
#define DEBUGGER_ON       0


/* LOW_POWER_DISABLE = 0 : LowPowerMode enabled : MCU enters stop2 mode*/
/* LOW_POWER_DISABLE = 1 : LowPowerMode disabled : MCU enters sleep mode only */
#define LOW_POWER_DISABLE 0

#define NUMBER_LED_TOGGLE 20
#define LED_TOGGLE_INTERVAL 100

#define NUMBER_BUZZER_PLAY 2
#define BUZZER_PLAY_INTERVAL 1000

#define NUMBER_TEMPRETURE_SENSOR_READ 5
#define TEMPRETURE_SENSOR_READ_INTERVAL 1000

#define NUMBER_ACCLEROMETER_READ 5
#define ACCELEROMETER_READ_INTERVAL 1000


#endif /* APP_CONF_H */

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

#define GNSE_TINY_TRACER_ENABLE 1

#define mainLOGGING_TASK_PRIORITY 24
#define mainLOGGING_TASK_STACK_SIZE (512)
#define mainLOGGING_MESSAGE_QUEUE_LENGTH (15)

/* The rate at which the Tx task sends to the queue. */
#define TX_DELAY                 (5000)

/* The length of time the LED will remain on for.  It is on just long enough
to be able to see with the human eye so as not to distort the power readings too
much. */
#define LED_TOGGLE_DELAY         (20)


#endif /* APP_CONF_H */

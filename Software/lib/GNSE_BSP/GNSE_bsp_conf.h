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
 * @file GNSE_bsp_conf.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef GNSE_BSP_CONF_H
#define GNSE_BSP_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* IRQ priorities
* Can be from 0 to 15 with 0 as the highest
* With OS, the lowest value should be the scheduler timer IRQ
* In FreeRTOS this is configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
*/
#define GNSE_BSP_SUBGHZ_RADIO_IT_PRIORITY       5U
#define GNSE_BSP_RTC_IT_PRIORITY                5U
#define DEBUG_USART_IT_PRIORITY                 7U
#define DEBUG_USART_DMA_IT_PRIORITY             7U
#define GNSE_BSP_BUZZER_TIMER_IT_PRIORITY       8U
#define GNSE_BSP_BUTTON_SWx_IT_PRIORITY         15U
#define ACC_INT_PRIORITY                        15U

#ifdef __cplusplus
}
#endif

#endif /* GNSE_BSP_CONF_H */

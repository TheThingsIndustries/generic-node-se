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
 * @file STNODE_bsp_conf.h
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#ifndef STNODE_BSP_CONF_H
#define STNODE_BSP_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

#define STNODE_BSP_V_0_1 01
#define STNODE_BSP_V_0_2 02

#define STNODE_BSP_VERSION STNODE_BSP_V_0_1

/* IRQ priorities */
#define STNODE_BSP_BUTTON_SWx_IT_PRIORITY         15U

#ifdef __cplusplus
}
#endif

#endif /* STNODE_BSP_CONF_H */

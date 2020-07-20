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
 * @file STNODE_bsp.h
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#ifndef STNODE_BSP_H
#define STNODE_BSP_H

#include "STNODE_bsp_error.h"
#include "STNODE_bsp_conf.h"
#include "stm32wlxx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * HW aliases for the board components
 */

typedef enum
{
  LED1 = 0,
  LED2 = 1,
  /* Color LED aliases */
  LED_RED = LED1,
  LED_BLUE = LED2
} Led_TypeDef;

typedef enum
{
  BUTTON_SW1 = 0,
} Button_TypeDef;

typedef enum
{
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

typedef enum
{
  LOAD_SWITCH1 = 0,
  LOAD_SWITCH2 = 1,
  LOAD_SWITCH3 = 2,
  /* Load Switch aliases */
  LOAD_SWITCH_SENSORS = LOAD_SWITCH1,
  LOAD_SWITCH_FLASH = LOAD_SWITCH2,
  LOAD_SWITCH_SECURE_ELEMENT = LOAD_SWITCH3
} Load_Switch_TypeDef;

/**
 * HAL defines
 * Configure below for any pin change
 */

#define LEDn 2

#define LED1_PIN GPIO_PIN_13
#define LED1_GPIO_PORT GPIOC
#define LED1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOC_CLK_DISABLE()

#define LED2_PIN GPIO_PIN_5
#define LED2_GPIO_PORT GPIOB
#define LED2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define LEDx_GPIO_CLK_ENABLE(__INDEX__) \
  do                                    \
  {                                     \
    if ((__INDEX__) == LED1)            \
      LED1_GPIO_CLK_ENABLE();           \
    else if ((__INDEX__) == LED2)       \
      LED2_GPIO_CLK_ENABLE();           \
  } while (0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__) \
  do                                     \
  {                                      \
    if ((__INDEX__) == LED1)             \
      LED1_GPIO_CLK_DISABLE();           \
    else if ((__INDEX__) == LED2)        \
      LED2_GPIO_CLK_DISABLE();           \
  } while (0)

#define BUTTONn 1

#define BUTTON_SW1_PIN GPIO_PIN_3
#define BUTTON_SW1_GPIO_PORT GPIOB
#define BUTTON_SW1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define BUTTON_SW1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()
#define BUTTON_SW1_EXTI_LINE EXTI_LINE_3
#ifdef CORE_CM0PLUS
#define BUTTON_SW1_EXTI_IRQn EXTI3_0_IRQn
#else
#define BUTTON_SW1_EXTI_IRQn EXTI3_IRQn
#endif

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__) BUTTON_SW1_GPIO_CLK_ENABLE();

#define LOAD_SWITCHn 3

#define LOAD_SWITCHx_GPIO_CLK_ENABLE(__INDEX__) __HAL_RCC_GPIOB_CLK_ENABLE()   /* All load switches on same port */
#define LOAD_SWITCHx_GPIO_CLK_DISABLE(__INDEX__) __HAL_RCC_GPIOB_CLK_DISABLE() /* All load switches on same port */

#define LOAD_SWITCH1_PIN GPIO_PIN_12
#define LOAD_SWITCH1_GPIO_PORT GPIOB
#define LOAD_SWITCH1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LOAD_SWITCH1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define LOAD_SWITCH2_PIN GPIO_PIN_7
#define LOAD_SWITCH2_GPIO_PORT GPIOB
#define LOAD_SWITCH2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LOAD_SWITCH2_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define LOAD_SWITCH3_PIN GPIO_PIN_6
#define LOAD_SWITCH3_GPIO_PORT GPIOB
#define LOAD_SWITCH3_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LOAD_SWITCH3_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define VBAT_PIN GPIO_PIN_4
#define VBAT_PORT GPIOB
#define VBAT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define VBAT_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

// TODO: Add Pin mapping for UART, SPI and I2C, see https://github.com/TheThingsIndustries/st-node/issues/30

/**
 * BSP APIs
 */

int32_t STNODE_BSP_LED_Init(Led_TypeDef Led);
int32_t STNODE_BSP_LED_DeInit(Led_TypeDef Led);
int32_t STNODE_BSP_LED_On(Led_TypeDef Led);
int32_t STNODE_BSP_LED_Off(Led_TypeDef Led);
int32_t STNODE_BSP_LED_Toggle(Led_TypeDef Led);
int32_t STNODE_BSP_LED_GetState(Led_TypeDef Led);

int32_t STNODE_BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
int32_t STNODE_BSP_PB_DeInit(Button_TypeDef Button);
int32_t STNODE_BSP_PB_GetState(Button_TypeDef Button);
void STNODE_BSP_PB_Callback(Button_TypeDef Button);
void STNODE_BSP_PB_IRQHandler(Button_TypeDef Button);

int32_t STNODE_BSP_LS_Init(Load_Switch_TypeDef loadSwitch);
int32_t STNODE_BSP_LS_SWITCH_DeInit(Load_Switch_TypeDef loadSwitch);
int32_t STNODE_BSP_LS_On(Load_Switch_TypeDef loadSwitch);
int32_t STNODE_BSP_LS_Off(Load_Switch_TypeDef loadSwitch);
int32_t STNODE_BSP_LS_Toggle(Load_Switch_TypeDef loadSwitch);
int32_t STNODE_BSP_LS_GetState(Load_Switch_TypeDef loadSwitch);

int32_t STNODE_BSP_BM_Init();
int32_t STNODE_BSP_BM_DeInit();
int32_t STNODE_BSP_BM_Enable();
int32_t STNODE_BSP_BM_Disable();
int32_t STNODE_BSP_BM_GetState();

#ifdef __cplusplus
}
#endif

#endif /* STNODE_BSP_H */
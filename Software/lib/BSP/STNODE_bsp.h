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

#ifndef NODE_BSP_H
#define NODE_BSP_H
// TODO: Define conf and error files

/* #ifdef __cplusplus
extern "C" {
#endif */

/**
 * HW aliases for the board comnponents
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

#define LED1_PIN GPIO_PIN_15
#define LED1_GPIO_PORT GPIOB
#define LED1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define LED2_PIN GPIO_PIN_9
#define LED2_GPIO_PORT GPIOB
#define LED2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define LEDx_GPIO_CLK_ENABLE(__INDEX__) __HAL_RCC_GPIOB_CLK_ENABLE()  /* All Led on same port */
#define LEDx_GPIO_CLK_DISABLE(__INDEX__) __HAL_RCC_GPIOB_CLK_ENABLE() /* All Led on same port */

#define BUTTONn 1

#define BUTTON_SW1_PIN GPIO_PIN_0
#define BUTTON_SW1_GPIO_PORT GPIOA
#define BUTTON_SW1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUTTON_SW1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOA_CLK_DISABLE()
#define BUTTON_SW1_EXTI_LINE EXTI_LINE_0
#ifdef CORE_CM0PLUS
#define BUTTON_SW1_EXTI_IRQn EXTI1_0_IRQn
#else
#define BUTTON_SW1_EXTI_IRQn EXTI0_IRQn
#endif

#define LOAD_SWITCHn 3

#define LOAD_SWITCHx_GPIO_CLK_ENABLE(__INDEX__) __HAL_RCC_GPIOB_CLK_ENABLE()  /* All load switches on same port */
#define LOAD_SWITCHx_GPIO_CLK_DISABLE(__INDEX__) __HAL_RCC_GPIOB_CLK_ENABLE() /* All load switches on same port */

#define LOAD_SWITCH1_PIN GPIO_PIN_15
#define LOAD_SWITCH1_GPIO_PORT GPIOB
#define LOAD_SWITCH1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LOAD_SWITCH1_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define LOAD_SWITCH2_PIN GPIO_PIN_9
#define LOAD_SWITCH2_GPIO_PORT GPIOB
#define LOAD_SWITCH2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LOAD_SWITCH2_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define LOAD_SWITCH3_PIN GPIO_PIN_9
#define LOAD_SWITCH3_GPIO_PORT GPIOB
#define LOAD_SWITCH3_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define LOAD_SWITCH3_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define VBAT_PIN GPIO_PIN_15
#define VBAT_PORT GPIOB
#define VBAT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define VBAT_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

// TODO: Add commuincation defines here like I2C and SPI and UART

/**
 * BSP APIs
 */

int32_t BSP_LED_Init(Led_TypeDef Led);
int32_t BSP_LED_DeInit(Led_TypeDef Led);
int32_t BSP_LED_On(Led_TypeDef Led);
int32_t BSP_LED_Off(Led_TypeDef Led);
int32_t BSP_LED_Toggle(Led_TypeDef Led);
int32_t BSP_LED_GetState(Led_TypeDef Led);

int32_t BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
int32_t BSP_PB_DeInit(Button_TypeDef Button);
int32_t BSP_PB_GetState(Button_TypeDef Button);
void BSP_PB_Callback(Button_TypeDef Button);
void BSP_PB_IRQHandler(Button_TypeDef Button);

int32_t BSP_LS_Init(Load_Switch_TypeDef loadSwitch);
int32_t BSP_LS_SWITCH_DeInit(Load_Switch_TypeDef loadSwitch);
int32_t BSP_LS_On(Load_Switch_TypeDef loadSwitch);
int32_t BSP_LS_Off(Load_Switch_TypeDef loadSwitch);
int32_t BSP_LS_Toggle(Load_Switch_TypeDef loadSwitch);
int32_t BSP_LS_GetState(Load_Switch_TypeDef loadSwitch);

int32_t BSP_BM_Init();
int32_t BSP_BM_DeInit();
int32_t BSP_BM_Enable();
int32_t BSP_BM_Disable();
int32_t BSP_BM_GetState();

/* #ifdef __cplusplus
}
#endif */

#endif // NODE_BSP_H
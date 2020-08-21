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

extern UART_HandleTypeDef STNODE_BSP_debug_usart;
extern I2C_HandleTypeDef STNODE_BSP_sensor_i2c1;
extern SPI_HandleTypeDef STNODE_BSP_flash_spi;
extern TIM_HandleTypeDef STNODE_BSP_buzzer_timer;

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

#define DEBUG_USART USART2
#define DEBUG_USART_BAUDRATE 115200
#define DEBUG_USART_PERIPH_CLK RCC_PERIPHCLK_USART2
#define DEBUG_USART_SOURCE_CLK RCC_USART2CLKSOURCE_SYSCLK //If adjusted, Update Usart2ClockSelection in STNODE_msp.c

#define DEBUG_USART_CLK_ENABLE()        __HAL_RCC_USART2_CLK_ENABLE()
#define DEBUG_USART_CLK_DISABLE()       __HAL_RCC_USART2_CLK_DISABLE()

#define DEBUG_USART_TX_PIN GPIO_PIN_2
#define DEBUG_USART_TX_GPIO_PORT GPIOA
#define DEBUG_USART_RX_PIN GPIO_PIN_3
#define DEBUG_USART_RX_GPIO_PORT GPIOA

#define DEBUG_USART_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define DEBUG_USART_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define DEBUG_USART_TX_AF                     GPIO_AF7_USART2
#define DEBUG_USART_RX_AF                     GPIO_AF7_USART2

#define DEBUG_USART_EXTI_WAKEUP               LL_EXTI_LINE_27

#define DEBUG_USART_DMA_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define DEBUG_USART_DMAMUX_CLK_ENABLE()              __HAL_RCC_DMAMUX1_CLK_ENABLE()

#define DEBUG_USART_TX_DMA_REQUEST             DMA_REQUEST_USART2_TX
#define DEBUG_USART_TX_DMA_CHANNEL             DMA1_Channel5

#define DEBUG_USART_DMA_TX_IRQn                DMA1_Channel5_IRQn
#define DEBUG_USART_DMA_TX_IRQHandler          DMA1_Channel5_IRQHandler

#define DEBUG_USART_IRQn                      USART2_IRQn

#define DEBUG_USART_PRIORITY 0
#define DEBUG_USART_DMA_PRIORITY 0

#define SENSOR_I2C1                  I2C1
#define SENSOR_I2C1_TIMING           0x00707CBBU  //Evaluates to 10 KHz bus frequency
#define SENSOR_I2C1_TIMOUT           100U //Read and write operattions timeout in ms
#define SENSOR_I2C1_PERIPH_CLK      RCC_PERIPHCLK_I2C1
#define SENSOR_I2C1_SOURCE_CLK      RCC_I2C1CLKSOURCE_SYSCLK

#define SENSOR_I2C1_CLK_ENABLE()    __HAL_RCC_I2C1_CLK_ENABLE()
#define SENSOR_I2C1_CLK_DISABLE()   __HAL_RCC_I2C1_CLK_DISABLE()

#define SENSOR_I2C1_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SENSOR_I2C1_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define SENSOR_I2C1_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define SENSOR_I2C1_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

#define SENSOR_I2C1_SCL_PIN                    GPIO_PIN_9
#define SENSOR_I2C1_SCL_GPIO_PORT              GPIOA
#define SENSOR_I2C1_SDA_PIN                    GPIO_PIN_10
#define SENSOR_I2C1_SDA_GPIO_PORT              GPIOA
#define SENSOR_I2C1_SCL_SDA_AF                 GPIO_AF4_I2C1

#define FLASH_SPI SPI1
#define FLASH_SPI_BAUDRATE            SPI_BAUDRATEPRESCALER_256
#define Flash_SPI_TIMOUT 100U        //Read and write operattions timeout in ms

#define FLASH_SPI_CLK_ENABLE() __HAL_RCC_SPI1_CLK_ENABLE()
#define FLASH_SPI_CLK_DISABLE() __HAL_RCC_SPI1_CLK_DISABLE()

#define FLASH_SPI_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define FLASH_SPI_FORCE_RESET() __HAL_RCC_SPI1_FORCE_RESET()
#define FLASH_SPI_RELEASE_RESET() __HAL_RCC_SPI1_RELEASE_RESET()

#define FLASH_SPI_GPIO_PORT GPIOA
#define FLASH_SPI_CS_PIN GPIO_PIN_4
#define FLASH_SPI_SCK_PIN GPIO_PIN_5
#define FLASH_SPI_MISO_PIN GPIO_PIN_6
#define FLASH_SPI_MOSI_PIN GPIO_PIN_7

#define FLASH_SPI_AF GPIO_AF5_SPI1
// TODO: Add Pin mapping for UART, SPI and I2C, see https://github.com/TheThingsIndustries/st-node/issues/30

#define BUZZER_TIMER TIM2
#define BUZZER_TIMER_CHANNEL TIM_CHANNEL_1
/**
 * In this example TIM2 input clock (TIM2CLK) is set to APB1 clock (PCLK1),
 * since APB1 prescaler is equal to 1.
 * TIM1CLK = PCLK1
 * PCLK1 = HCLK
 * => TIM1CLK = HCLK = SystemCoreClock
 * To get TIM1 counter clock at 1 MHz, the prescaler is computed as follows:
 * Prescaler = (TIM2CLK / TIM2 counter clock) - 1
 * Prescaler = ((SystemCoreClock) /1 MHz) - 1
 */
#define BUZZER_PRESCALER (uint32_t)(((SystemCoreClock) / 100000) - 1)
/**
 * To get TIM2 output clock at 2.73 KHz, the period (BUZZER_PERIOD)) is computed as follows:
 * BUZZER_PERIOD = (TIM2 counter clock / TIM2 output clock) - 1 ~= 365
 *
 */
#define BUZZER_PERIOD (365U)
#define BUZZER_TIMER_PWM_PIN GPIO_PIN_15
#define BUZZER_TIMER_PWM_PORT GPIOA
#define BUZZER_TIMER_CLK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#define BUZZER_TIMER_CLK_DISABLE() __HAL_RCC_TIM2_CLK_DISABLE()
#define BUZZER_TIMER_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUZZER_TIMER_AF GPIO_AF1_TIM2

#define BUZZER_TIMER_IRQn TIM2_IRQn
#define BUZZER_TIMER_IT TIM_IT_UPDATE

#define BUZZER_TIMER_PRIORITY 0

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

int32_t STNODE_BSP_USART_Init(void);
int32_t STNODE_BSP_UART_DMA_Init(void);

int32_t STNODE_BSP_Sensor_I2C1_Init(void);

int32_t STNODE_BSP_Flash_SPI_Init(void);

int32_t STNODE_BSP_BUZZER_TIM_Init(pTIM_CallbackTypeDef cb);

#ifdef __cplusplus
}
#endif

#endif /* STNODE_BSP_H */
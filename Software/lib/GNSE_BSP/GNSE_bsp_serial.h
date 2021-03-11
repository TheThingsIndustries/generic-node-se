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
 * @file GNSE_bsp_serial.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */


#ifndef GNSE_BSP_SERIAL_H
#define GNSE_BSP_SERIAL_H

#include "stm32wlxx_hal.h"
#include "GNSE_bsp_error.h"
#include "GNSE_bsp_conf.h"

extern UART_HandleTypeDef GNSE_BSP_debug_usart;
extern I2C_HandleTypeDef GNSE_BSP_sensor_i2c1;
extern I2C_HandleTypeDef GNSE_BSP_ext_sensor_i2c2;
extern SPI_HandleTypeDef GNSE_BSP_flash_spi;
extern DMA_HandleTypeDef GNSE_BSP_hdma_tx;

/**
 * HAL defines
 * Configure below for any pin change
 */

#define DEBUG_USART USART2
#define DEBUG_USART_BAUDRATE 115200
#define DEBUG_USART_PERIPH_CLK RCC_PERIPHCLK_USART2
#define DEBUG_USART_SOURCE_CLK RCC_USART2CLKSOURCE_SYSCLK //If adjusted, Update Usart2ClockSelection in GNSE_msp.c

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

#define SENSOR_I2C1                  I2C1
/* SENSOR_I2C1_TIMING is set at 300 kHz
 * This setting is altered by both the clock speed and I2C setting
 * Clock speed is set in SystemClock_Config, default is 48 MHz
 * I2C setting is set in GNSE_BSP_Sensor_I2C1_Init, default is Fast mode plus
 * Changing any of these variables without reconfiguring this define could break the I2C communication
 */
#define SENSOR_I2C1_TIMING           0x0020088DU
#define SENSOR_I2C1_TIMOUT           100U //Read and write operations timeout in ms
#define SENSOR_I2C1_PERIPH_CLK      RCC_PERIPHCLK_I2C1
#define SENSOR_I2C1_SOURCE_CLK      RCC_I2C1CLKSOURCE_SYSCLK
#define SENSOR_I2C1_FASTMODEPLUS    I2C_FASTMODEPLUS_I2C1

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

#define EXT_SENSOR_I2C2                  I2C2
/* EXT_SENSOR_I2C2_TIMING is set at 100 kHz
 * This setting is altered by both the clock speed and I2C setting
 * Clock speed is set in SystemClock_Config, default is 48 MHz
 * I2C setting is set in GNSE_BSP_Sensor_I2C1_Init, default is Fast mode plus
 * Changing any of these variables without reconfiguring this define could break the I2C communication
 */
#define EXT_SENSOR_I2C2_TIMING           0x20003D5EU
#define EXT_SENSOR_I2C2_TIMOUT           100U //Read and write operations timeout in ms
#define EXT_SENSOR_I2C2_PERIPH_CLK      RCC_PERIPHCLK_I2C2
#define EXT_SENSOR_I2C2_SOURCE_CLK      RCC_I2C2CLKSOURCE_SYSCLK
#define EXT_SENSOR_I2C2_FASTMODEPLUS    I2C_FASTMODEPLUS_I2C2

#define EXT_SENSOR_I2C2_CLK_ENABLE()    __HAL_RCC_I2C2_CLK_ENABLE()
#define EXT_SENSOR_I2C2_CLK_DISABLE()   __HAL_RCC_I2C2_CLK_DISABLE()

#define EXT_SENSOR_I2C2_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define EXT_SENSOR_I2C2_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define EXT_SENSOR_I2C2_FORCE_RESET()              __HAL_RCC_I2C2_FORCE_RESET()
#define EXT_SENSOR_I2C2_RELEASE_RESET()            __HAL_RCC_I2C2_RELEASE_RESET()

#define EXT_SENSOR_I2C2_SCL_PIN                    GPIO_PIN_12
#define EXT_SENSOR_I2C2_SCL_GPIO_PORT              GPIOA
#define EXT_SENSOR_I2C2_SDA_PIN                    GPIO_PIN_11
#define EXT_SENSOR_I2C2_SDA_GPIO_PORT              GPIOA
#define EXT_SENSOR_I2C2_SCL_SDA_AF                 GPIO_AF4_I2C2

#define FLASH_SPI SPI1
/* Speed of the SPI interface depends on both the system clock speed (48 MHz is default) and a prescaler */
#define FLASH_SPI_BAUDRATE            SPI_BAUDRATEPRESCALER_16
#define Flash_SPI_TIMOUT 100U        //Read and write operations timeout in ms

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

/**
 * BSP Serial APIs
 */

int32_t GNSE_BSP_USART_Init(void);
int32_t GNSE_BSP_UART_DMA_Init(void);

int32_t GNSE_BSP_Sensor_I2C1_Init(void);
int32_t GNSE_BSP_Sensor_I2C1_DeInit(void);
int32_t GNSE_BSP_Ext_Sensor_I2C2_Init(void);
int32_t GNSE_BSP_Ext_Sensor_I2C2_DeInit(void);

int32_t GNSE_BSP_Flash_SPI_Init(void);
int32_t GNSE_BSP_Flash_SPI_DeInit(void);

#endif /* GNSE_BSP_SERIAL_H */

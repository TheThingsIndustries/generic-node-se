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
 * @file STNODE_msp.c
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */
#include "STNODE_bsp.h"

static DMA_HandleTypeDef hdma_tx;
static void msp_error_handler();

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    if (uartHandle->Instance == DEBUG_USART)
    {
        DEBUG_USART_TX_GPIO_CLK_ENABLE();
        DEBUG_USART_RX_GPIO_CLK_ENABLE();

        DEBUG_USART_DMA_CLK_ENABLE();
        DEBUG_USART_DMAMUX_CLK_ENABLE();
        DEBUG_USART_CLK_ENABLE();

        PeriphClkInit.PeriphClockSelection = DEBUG_USART_PERIPH_CLK;
        PeriphClkInit.Usart2ClockSelection = DEBUG_USART_SOURCE_CLK; // Configures USART2
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            msp_error_handler();
        }

        GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
        HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;
        HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct);

        /* Configure the DMA handler for Transmission process */
        hdma_tx.Instance = DEBUG_USART_TX_DMA_CHANNEL;
        hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_tx.Init.Mode = DMA_NORMAL;
        hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_tx.Init.Request = DEBUG_USART_TX_DMA_REQUEST;

        if (HAL_DMA_Init(&hdma_tx) != HAL_OK)
        {
            msp_error_handler();
        }

        /* Associate the initialized DMA handle to the UART handle */
        __HAL_LINKDMA(uartHandle, hdmatx, hdma_tx);

        /* NVIC configuration for DMA transfer complete interrupt */
        HAL_NVIC_SetPriority(DEBUG_USART_DMA_TX_IRQn, DEBUG_USART_PRIORITY, 1);
        HAL_NVIC_EnableIRQ(DEBUG_USART_DMA_TX_IRQn);

        /* NVIC for DEBUG_USART, to catch the TX complete */
        HAL_NVIC_SetPriority(DEBUG_USART_IRQn, DEBUG_USART_DMA_PRIORITY, 1);
        HAL_NVIC_EnableIRQ(DEBUG_USART_IRQn);

        /* Enable DEBUG_USART wakeup interrupt */
        LL_EXTI_EnableIT_0_31(DEBUG_USART_EXTI_WAKEUP);
    }
    else
    {
        msp_error_handler();
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
    if (uartHandle->Instance == DEBUG_USART)
    {
        DEBUG_USART_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, DEBUG_USART_TX_PIN | DEBUG_USART_RX_PIN);
        HAL_DMA_DeInit(uartHandle->hdmatx);
        HAL_NVIC_DisableIRQ(DEBUG_USART_IRQn);
    }
    else
    {
        msp_error_handler();
    }
}

static void msp_error_handler()
{
    while (1)
    {
    }
}

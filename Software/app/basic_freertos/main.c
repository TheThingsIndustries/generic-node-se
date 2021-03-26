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
 * @file main.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "cmsis_os.h"
#include "FreeRTOS_iot_log_task.h"

uint32_t osQueueMsg;
uint32_t Queue_value = 100;

/* Definitions for TxThread */
osThreadId_t TxThreadHandle;
const osThreadAttr_t TxThread_attributes = {
    .name = "TxThread",
    .priority = (osPriority_t)osPriorityBelowNormal,
    .stack_size = 512 * 4};

/* Definitions for RxThread */
osThreadId_t RxThreadHandle;
const osThreadAttr_t RxThread_attributes = {
    .name = "RxThread",
    .priority = (osPriority_t)osPriorityNormal,
    .stack_size = 512 * 4};

/* Definitions for osqueue */
osMessageQueueId_t osqueueHandle;
const osMessageQueueAttr_t osqueue_attributes = {
    .name = "osqueue"};

static void SystemClock_Config(void);
static void Error_Handler(void);

void QueueSendThread(void *argument);
void QueueReceiveThread(void *argument);

static void MX_GPIO_Init(void);

/**
  * @brief  Function implementing the TxThread thread.
  * @param  argument: Not used
  * @retval None
  */
void QueueSendThread(void *argument)
{
  for (;;)
  {
#if (GNSE_TINY_TRACER_ENABLE)
    configPRINTF(("\r\n Send a Tx msg \r\n"));
#endif
    /* Place this thread into the blocked state until it is time to run again.
       The kernel will place the MCU into the Retention low power sleep state
       when the idle thread next runs. */
    osDelay(TX_DELAY);

    /* Send to the queue - causing the queue receive thread to flash its LED.
       It should not be necessary to block on the queue send because the Rx
       thread will already have removed the last queued item. */
    osMessageQueuePut(osqueueHandle, &Queue_value, 100, 0U);
  }
}

/**
* @brief Function implementing the RxThread thread.
* @param argument: Not used
* @retval None
*/
void QueueReceiveThread(void *argument)
{
  osStatus_t status;

  for (;;)
  {
    status = osMessageQueueGet(osqueueHandle, &osQueueMsg, NULL, 100);

    if (status == osOK)
    {
#if (GNSE_TINY_TRACER_ENABLE)
      configPRINTF(("\r\n Received Rx msg \r\n"));
#endif
      if (osQueueMsg == Queue_value)
      {
        GNSE_BSP_LED_On(LED_BLUE);
        osDelay(LED_TOGGLE_DELAY);
        GNSE_BSP_LED_Off(LED_BLUE);
      }
    }
  }
}

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  MX_GPIO_Init();

  /* Initialize LED */
  GNSE_BSP_LED_Init(LED_BLUE);
  GNSE_BSP_LED_Init(LED_RED);

  /* Init scheduler */
  osKernelInitialize();

#if (GNSE_TINY_TRACER_ENABLE)
  /* Initialize Tracer/Logger */
  GNSE_TRACER_INIT();

  xLoggingTaskInitialize(mainLOGGING_TASK_STACK_SIZE,
                         mainLOGGING_TASK_PRIORITY,
                         mainLOGGING_MESSAGE_QUEUE_LENGTH);
#endif
  /* Create the queue(s) */
  /* creation of osqueue */
  osqueueHandle = osMessageQueueNew(1, sizeof(uint16_t), &osqueue_attributes);

  /* Create the thread(s) */
  /* creation of TxThread */
  TxThreadHandle = osThreadNew(QueueSendThread, NULL, &TxThread_attributes);

  /* creation of RxThread */
  RxThreadHandle = osThreadNew(QueueReceiveThread, NULL, &RxThread_attributes);

  /* Start scheduler */
  osKernelStart();

  while (1)
  {
    //Should never reach here
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pins : PA14 PA12 PA15 PA13
                           PA11 PA10 PA0 PA9
                           PA6 PA1 PA3 PA2
                           PA7 PA4 PA5 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_13
                          |GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_0|GPIO_PIN_9
                          |GPIO_PIN_6|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_2
                          |GPIO_PIN_7|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB15 PB3 PB4 PB7
                           PB9 PB14 PB5 PB8
                           PB13 PB2 PB6 PB12
                           PB1 PB0 PB11 PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_7
                          |GPIO_PIN_9|GPIO_PIN_14|GPIO_PIN_5|GPIO_PIN_8
                          |GPIO_PIN_13|GPIO_PIN_2|GPIO_PIN_6|GPIO_PIN_12
                          |GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_11|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC14 PC15 PC13 PC2
                           PC3 PC5 PC1 PC0
                           PC4 PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_13|GPIO_PIN_2
                          |GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_1|GPIO_PIN_0
                          |GPIO_PIN_4|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PH3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @return None
  */
static void Error_Handler(void)
{
  while (1)
  {
  }
}

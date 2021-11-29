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

#include "tx_api.h"
#include "app.h"

static void SystemClock_Config(void);
static void Error_Handler(void);

static int32_t Queue_value = 100;

/* Define the ThreadX object control blocks */
TX_THREAD thread_1;
TX_THREAD thread_2;
TX_QUEUE queue_0;
TX_BYTE_POOL byte_pool_0;

/* Define thread prototypes */
void thread_1_entry(ULONG thread_input);
void thread_2_entry(ULONG thread_input);

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

#if (GNSE_TINY_TRACER_ENABLE)
    /* Initialize Tracer/Logger */
    GNSE_TRACER_INIT();
    GNSE_app_printAppInfo();
#endif

    /* Initialize LED */
    GNSE_BSP_LED_Init(LED_BLUE);

    /* Enter the ThreadX kernel */
    tx_kernel_enter();
}

/* Define what the initial system looks like */
void tx_application_define(void *first_unused_memory)
{

    CHAR *pointer;

    /* Create a byte memory pool from which to allocate the thread stacks */
    tx_byte_pool_create(&byte_pool_0, "byte pool 0", first_unused_memory,
                        MEM_BYTE_POOL_SIZE);

    /* Allocate the stack for thread 1 */
    tx_byte_allocate(&byte_pool_0, (VOID **)&pointer, THREAD_STACK_SIZE,
                     TX_NO_WAIT);

    /* Create threads 1 and 2. These threads pass information through a ThreadX
     message queue */
    tx_thread_create(&thread_1, "thread 1", thread_1_entry, 1, pointer,
                     THREAD_STACK_SIZE, 2, 2, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for thread 2 */
    tx_byte_allocate(&byte_pool_0, (VOID **)&pointer, THREAD_STACK_SIZE,
                     TX_NO_WAIT);

    tx_thread_create(&thread_2, "thread 2", thread_2_entry, 2, pointer,
                     THREAD_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the message queue */
    tx_byte_allocate(&byte_pool_0, (VOID **)&pointer,
                     QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);

    /* Create the message queue shared by threads 1 and 2 */
    tx_queue_create(&queue_0, "queue 0", TX_1_ULONG, pointer,
                    QUEUE_SIZE * sizeof(ULONG));
}

void thread_1_entry(ULONG thread_input)
{
    UINT status;

    /* This thread simply sends messages to a queue shared by thread 2 */
    while (1)
    {
        /* Send message to queue 0 */
        APP_PPRINTF(("\r\n Send Tx msg \r\n"));
        status = tx_queue_send(&queue_0, &Queue_value,
                               TX_WAIT_FOREVER);

        /* Check completion status */
        if (status == TX_SUCCESS)
        {
            tx_thread_sleep(TX_DELAY);
        }
        else
        {
            // Should stop the application if we fail to send one message
            break;
        }
    }
}

void thread_2_entry(ULONG thread_input)
{
    ULONG received_message;
    UINT status;

    /* This thread retrieves messages placed on the queue by thread 1 */
    while (1)
    {
        /* Retrieve a message from the queue */
        status = tx_queue_receive(&queue_0, &received_message, TX_WAIT_FOREVER);

        /* Check completion status and make sure the message is what we
         expected */
        if ((status == TX_SUCCESS) && (received_message == Queue_value))
        {
            APP_PPRINTF(("\r\n Received Rx msg \r\n"));
            GNSE_BSP_LED_On(LED_BLUE);
            tx_thread_sleep(LED_TOGGLE_DELAY);
            GNSE_BSP_LED_Off(LED_BLUE);
        }
        else
        {
            // Should stop the application if we fail to receive one message
            break;
        }
    }
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

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
#include "bootloader.h"

static void SystemClock_Config(void);
static void Error_Handler(void);

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  SystemClock_Config();

  /* Initialize Tracer/Logger */
  GNSE_TRACER_INIT();

  APP_PPRINTF("\r\n -------------- Starting GNSE basic bootloader -------------- \r\n");
  Bootloader_Init();
  Bootloader_HandleInput();

#if (GNSE_TINY_TRACER_ENABLE)
  Bootloader_state_t state = Bootloader_GetState();
  switch (state)
  {
  case BOOTLOADER_STATE_APP_JMP:
    APP_PPRINTF("\r\n Jumping to application at APP_ADDRESS: 0x%08x \r\n", APP_ADDRESS);
    break;
  case BOOTLOADER_STATE_SYS_JMP:
    APP_PPRINTF("\r\n Jumping to ST bootloader at SYSMEM_ADDRESS : 0x%08x \r\n", ST_BOOTLOADER_SYSMEM_ADDRESS);
    break;
  default:
    APP_PPRINTF("\r\n Un-handled bootloader state \r\n");
    break;
  }
  HAL_Delay(APP_PRINT_DELAY);
#endif

#if (GNSE_TINY_TRACER_ENABLE)
  GNSE_TRACER_DEINIT();
#endif

  Bootloader_DeInit();
  Bootloader_Jump();

  Error_Handler();
}

/**
  * @brief System Clock Configuration
  * @return None
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
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

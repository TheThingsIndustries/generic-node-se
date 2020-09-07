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
 * @file main.c
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#include "app.h"

static void SystemClock_Config(void);
static void Error_Handler(void);

void tempreture_sensor_read_data_polling(uint8_t n_reads, uint32_t read_delay);
void accelerometer_read_data_polling(uint8_t n_reads, uint32_t read_delay);

void uart_rxcallback(uint8_t *rxChar, uint16_t size, uint8_t error)
{
  APP_PPRINTF("\r\n DEBUG_USART data received \r\n");
}

int main(void)
{
  int16_t status;
  MxChip mxic;
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  SystemClock_Config();

  STNODE_BSP_LED_Init(LED_BLUE);
  STNODE_BSP_LED_Init(LED_RED);

  UTIL_ADV_TRACE_Init();
  UTIL_ADV_TRACE_StartRxProcess(uart_rxcallback);
  UTIL_ADV_TRACE_SetVerboseLevel(VLEVEL_H);

  APP_PPRINTF("\r\n Starting STNODE basic app \r\n");

  STNODE_BSP_LS_Init(LOAD_SWITCH_SENSORS);
  STNODE_BSP_LS_On(LOAD_SWITCH_SENSORS);
  HAL_Delay(100);

  STNODE_BSP_Sensor_I2C1_Init();
  HAL_Delay(100);

  tempreture_sensor_read_data_polling(NUMBER_TEMPRETURE_SENSOR_READ, TEMPRETURE_SENSOR_READ_INTERVAL);
  accelerometer_read_data_polling(NUMBER_ACCLEROMETER_READ, ACCELEROMETER_READ_INTERVAL);

  STNODE_BSP_LS_Init(LOAD_SWITCH_FLASH);
  STNODE_BSP_LS_On(LOAD_SWITCH_FLASH);
  HAL_Delay(100);

  status = MX25R16_Init(&mxic);
  if (status != MXST_SUCCESS)
  {
    APP_PPRINTF("\r\n Failed to init external SPI flash (MX25R1635F)\r\n");
  }

  status = MxSimpleTest(&mxic);
  if (status == MXST_SUCCESS)
  {
    APP_PPRINTF("\r\n Simple external SPI flash (MX25R1635F) test passed!\r\n");
  }
  else
  {
    APP_PPRINTF("\r\n Simple external SPI flash test (MX25R1635F) failed, check UART logs for more details\r\n");
  }

  if (BUZZER_Init() == BUZZER_OP_SUCCESS)
  {
    APP_PPRINTF("\r\n Testing Buzzer!\r\n");
    BUZZER_SetState(BUZZER_STATE_TICK);
    HAL_Delay(1000);
    BUZZER_SetState(BUZZER_STATE_DODO);
    HAL_Delay(1000);
    BUZZER_SetState(BUZZER_STATE_DODODO);
    HAL_Delay(1000);
    BUZZER_SetState(BUZZER_STATE_WARNING);
    HAL_Delay(1000);
    BUZZER_SetState(BUZZER_STATE_DANGER);
    HAL_Delay(1000);
    BUZZER_SetState(BUZZER_STATE_RING);
    HAL_Delay(1000);
    BUZZER_SetState(BUZZER_STATE_OFF);
  }
  else
  {
    APP_PPRINTF("\r\n Failed to test the Buzzer!\r\n");
  }

  while (1)
  {
    STNODE_BSP_LED_Toggle(LED_BLUE);
    /* Insert delay 100 ms */
    HAL_Delay(100);
    STNODE_BSP_LED_Toggle(LED_RED);
    /* Insert delay 100 ms */
    HAL_Delay(100);
  }
  return 0;
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

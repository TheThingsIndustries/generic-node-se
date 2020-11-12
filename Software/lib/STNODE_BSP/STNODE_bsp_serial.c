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
 * @file STNODE_bsp_serial.c
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */

#include "STNODE_bsp_serial.h"

UART_HandleTypeDef STNODE_BSP_debug_usart;
I2C_HandleTypeDef STNODE_BSP_sensor_i2c1;
I2C_HandleTypeDef STNODE_BSP_sec_elm_i2c2;
SPI_HandleTypeDef STNODE_BSP_flash_spi;
DMA_HandleTypeDef STNODE_BSP_hdma_tx;


/**
 * @brief Init the UART interface for debugging.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_USART_Init(void)
{
  STNODE_BSP_debug_usart.Instance = DEBUG_USART;
  STNODE_BSP_debug_usart.Init.BaudRate = DEBUG_USART_BAUDRATE;
  STNODE_BSP_debug_usart.Init.WordLength = UART_WORDLENGTH_8B;
  STNODE_BSP_debug_usart.Init.StopBits = UART_STOPBITS_1;
  STNODE_BSP_debug_usart.Init.Parity = UART_PARITY_NONE;
  STNODE_BSP_debug_usart.Init.Mode = UART_MODE_TX_RX;
  STNODE_BSP_debug_usart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  STNODE_BSP_debug_usart.Init.OverSampling = UART_OVERSAMPLING_16;
  STNODE_BSP_debug_usart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  STNODE_BSP_debug_usart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  STNODE_BSP_debug_usart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&STNODE_BSP_debug_usart) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  if (HAL_UARTEx_SetTxFifoThreshold(&STNODE_BSP_debug_usart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&STNODE_BSP_debug_usart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }
  if (HAL_UARTEx_EnableFifoMode(&STNODE_BSP_debug_usart) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  return STNODE_BSP_ERROR_NONE;
}

/**
 * @brief Enable DMA controller clock.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_UART_DMA_Init(void)
{
  /* DMA controller clock enable */
  DEBUG_USART_DMAMUX_CLK_ENABLE();
  DEBUG_USART_DMA_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, DEBUG_USART_DMA_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

  return STNODE_BSP_ERROR_NONE;
}

/**
 * @brief Init the Sensors I2C1 bus.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_Sensor_I2C1_Init(void)
{
  STNODE_BSP_sensor_i2c1.Instance = SENSOR_I2C1;
  STNODE_BSP_sensor_i2c1.Init.Timing = SENSOR_I2C1_TIMING; // I2C1 bus frequency config
  STNODE_BSP_sensor_i2c1.Init.OwnAddress1 = 0x00;
  STNODE_BSP_sensor_i2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  STNODE_BSP_sensor_i2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  STNODE_BSP_sensor_i2c1.Init.OwnAddress2 = 0x00;
  STNODE_BSP_sensor_i2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  STNODE_BSP_sensor_i2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&STNODE_BSP_sensor_i2c1) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  /* Enable the Analog I2C Filter */
  if (HAL_I2CEx_ConfigAnalogFilter(&STNODE_BSP_sensor_i2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&STNODE_BSP_sensor_i2c1, 0) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  HAL_I2CEx_EnableFastModePlus(SENSOR_I2C1_FASTMODEPLUS);

  return STNODE_BSP_ERROR_NONE;
}

/**
 * @brief Init the Sensors I2C2 bus.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_SEC_ELM_I2C2_Init(void)
{
  STNODE_BSP_sec_elm_i2c2.Instance = SEC_ELM_I2C2;
  STNODE_BSP_sec_elm_i2c2.Init.Timing = SEC_ELM_I2C2_TIMING; // I2C2 bus frequency config
  STNODE_BSP_sec_elm_i2c2.Init.OwnAddress1 = 0x00;
  STNODE_BSP_sec_elm_i2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  STNODE_BSP_sec_elm_i2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  STNODE_BSP_sec_elm_i2c2.Init.OwnAddress2 = 0x00;
  STNODE_BSP_sec_elm_i2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  STNODE_BSP_sec_elm_i2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&STNODE_BSP_sec_elm_i2c2) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  /* Enable the Analog I2C Filter */
  if (HAL_I2CEx_ConfigAnalogFilter(&STNODE_BSP_sec_elm_i2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&STNODE_BSP_sec_elm_i2c2, 0) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  HAL_I2CEx_EnableFastModePlus(SEC_ELM_I2C2_FASTMODEPLUS);

  return STNODE_BSP_ERROR_NONE;
}

int32_t STNODE_BSP_Flash_SPI_Init(void)
{
  /* FLASH_SPI parameter configuration*/
  STNODE_BSP_flash_spi.Instance = FLASH_SPI;
  STNODE_BSP_flash_spi.Init.Mode = SPI_MODE_MASTER;
  STNODE_BSP_flash_spi.Init.Direction = SPI_DIRECTION_2LINES;
  STNODE_BSP_flash_spi.Init.DataSize = SPI_DATASIZE_8BIT;
  STNODE_BSP_flash_spi.Init.CLKPolarity = SPI_POLARITY_LOW;
  STNODE_BSP_flash_spi.Init.CLKPhase = SPI_PHASE_1EDGE;
  STNODE_BSP_flash_spi.Init.NSS = SPI_NSS_SOFT; //TODO: USE HW NSS, see https://github.com/TheThingsIndustries/st-node/issues/40
  STNODE_BSP_flash_spi.Init.BaudRatePrescaler = FLASH_SPI_BAUDRATE;
  STNODE_BSP_flash_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  STNODE_BSP_flash_spi.Init.TIMode = SPI_TIMODE_DISABLE;
  STNODE_BSP_flash_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  STNODE_BSP_flash_spi.Init.CRCPolynomial = 7;
  STNODE_BSP_flash_spi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  STNODE_BSP_flash_spi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&STNODE_BSP_flash_spi) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }
  return STNODE_BSP_ERROR_NONE;
}

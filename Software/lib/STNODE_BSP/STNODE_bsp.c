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
 * @file STNODE_bsp.c
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */
#include "STNODE_bsp.h"

typedef void (*STNODE_BSP_EXTI_LineCallback)(void);
EXTI_HandleTypeDef hpb_exti[BUTTONn];

static GPIO_TypeDef *LED_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT};
static const uint16_t LED_PIN[LEDn] = {LED1_PIN, LED2_PIN};
static GPIO_TypeDef *BUTTON_PORT[BUTTONn] = {BUTTON_SW1_GPIO_PORT};
static const uint16_t BUTTON_PIN[BUTTONn] = {BUTTON_SW1_PIN};
static const IRQn_Type BUTTON_IRQn[BUTTONn] = {BUTTON_SW1_EXTI_IRQn};
static GPIO_TypeDef *LOAD_SWITCH_PORT[LOAD_SWITCHn] = {LOAD_SWITCH1_GPIO_PORT, LOAD_SWITCH2_GPIO_PORT, LOAD_SWITCH3_GPIO_PORT};
static const uint16_t LOAD_SWITCH_PIN[LOAD_SWITCHn] = {LOAD_SWITCH1_PIN, LOAD_SWITCH2_PIN, LOAD_SWITCH3_PIN};

static void BUTTON_SW1_EXTI_Callback(void);
UART_HandleTypeDef STNODE_BSP_debug_usart;
I2C_HandleTypeDef STNODE_BSP_sensor_i2c1;
SPI_HandleTypeDef STNODE_BSP_flash_spi;
TIM_HandleTypeDef STNODE_BSP_buzzer_timer;
DMA_HandleTypeDef STNODE_BSP_hdma_tx;
RTC_HandleTypeDef STNODE_BSP_rtc;

/**
 * LED APIs
 */

/**
  * @brief  Configures LED GPIO.
  * @param  Led: LED to be configured.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LED_Init(Led_TypeDef Led)
{
  GPIO_InitTypeDef gpio_init_structure = {0};

  /* Enable the GPIO_LED Clock */
  LEDx_GPIO_CLK_ENABLE(Led);

  /* Configure the GPIO_LED pin */
  gpio_init_structure.Pin = LED_PIN[Led];
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(LED_PORT[Led], &gpio_init_structure);
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET); /* LEDs are active low*/

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  DeInit LEDs.
  * @param  Led: LED to be de-init.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @note Led DeInit does not disable the GPIO clock nor disable the Mfx
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LED_DeInit(Led_TypeDef Led)
{
  /* Turn off LED */
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET); /* LEDs are active low*/

  /* DeInit the GPIO_LED pin */
  HAL_GPIO_DeInit(LED_PORT[Led], LED_PIN[Led]);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LED_Off(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Get the status of the selected LED.
  * @param  Led Specifies the Led to get its state.
  *         This parameter can be one of following parameters:
  *            @arg LED1
  *            @arg LED2
  * @return LED status
  */
int32_t STNODE_BSP_LED_GetState(Led_TypeDef Led)
{
  return (int32_t)HAL_GPIO_ReadPin(LED_PORT[Led], LED_PIN[Led]);
}

/**
 * Push button (PB) APIs
 */

/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @param  ButtonMode: Specifies Button mode.
  *   This parameter can be one of following parameters:
  *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO
  *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt
  *                            generation capability
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{
  GPIO_InitTypeDef gpio_init_structure = {0};
  static STNODE_BSP_EXTI_LineCallback button_callback[BUTTONn] = {BUTTON_SW1_EXTI_Callback};
  static uint32_t button_interrupt_priority[BUTTONn] = {STNODE_BSP_BUTTON_SWx_IT_PRIORITY};
  static const uint32_t button_exti_line[BUTTONn] = {BUTTON_SW1_EXTI_LINE};

  /* Enable the BUTTON Clock */
  BUTTONx_GPIO_CLK_ENABLE(Button);

  gpio_init_structure.Pin = BUTTON_PIN[Button];
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

  if (ButtonMode == BUTTON_MODE_GPIO)
  {
    /* Configure Button pin as input */
    gpio_init_structure.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(BUTTON_PORT[Button], &gpio_init_structure);
  }
  else /* (ButtonMode == BUTTON_MODE_EXTI) */
  {
    /* Configure Button pin as input with External interrupt */
    gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;

    HAL_GPIO_Init(BUTTON_PORT[Button], &gpio_init_structure);

    (void)HAL_EXTI_GetHandle(&hpb_exti[Button], button_exti_line[Button]);
    (void)HAL_EXTI_RegisterCallback(&hpb_exti[Button], HAL_EXTI_COMMON_CB_ID, button_callback[Button]);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority((BUTTON_IRQn[Button]), button_interrupt_priority[Button], 0x00);
    HAL_NVIC_EnableIRQ((BUTTON_IRQn[Button]));
  }

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Push Button DeInit.
  * @param  Button: Button to be configured
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @note PB DeInit does not disable the GPIO clock
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_PB_DeInit(Button_TypeDef Button)
{
  HAL_NVIC_DisableIRQ((BUTTON_IRQn[Button]));
  HAL_GPIO_DeInit(BUTTON_PORT[Button], BUTTON_PIN[Button]);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @return The Button GPIO pin value.
  */
int32_t STNODE_BSP_PB_GetState(Button_TypeDef Button)
{
  return (int32_t)HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}

/**
  * @brief  This function handles Push-Button interrupt requests.
  * @param  Button Specifies the pin connected EXTI line
  * @return None
  */
void STNODE_BSP_PB_IRQHandler(Button_TypeDef Button)
{
  HAL_EXTI_IRQHandler(&hpb_exti[Button]);
}

/**
  * @brief  BSP Push Button callback
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  * @return None.
  */
__weak void STNODE_BSP_PB_Callback(Button_TypeDef Button)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Button);

  /* This function should be implemented by the user application.
     It is called into this driver when an event on Button is triggered. */
}

/**
  * @brief  Button SW1 EXTI line detection callback.
  * @return None
  */
static void BUTTON_SW1_EXTI_Callback(void)
{
  STNODE_BSP_PB_Callback(BUTTON_SW1);
}

/**
 * Load Switches (LS) control APIs
 */

/**
  * @brief  Configures load switch GPIO.
  * @param  loadSwitch: Load Switch to be configured.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  *            @arg LOAD_SWITCH3
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LS_Init(Load_Switch_TypeDef loadSwitch)
{
  GPIO_InitTypeDef gpio_init_structure = {0};

  /* Enable the GPIO_LOAD_SWITCH Clock */
  LOAD_SWITCHx_GPIO_CLK_ENABLE(loadSwitch); //TODO fix this if the clock port is not the same for all the pins

  /* Configure the GPIO_LOAD_SWITCH pin */
  gpio_init_structure.Pin = LOAD_SWITCH_PIN[loadSwitch];
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(LOAD_SWITCH_PORT[loadSwitch], &gpio_init_structure);
  HAL_GPIO_WritePin(LOAD_SWITCH_PORT[loadSwitch], LOAD_SWITCH_PIN[loadSwitch], GPIO_PIN_RESET);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  DeInit Load Switches.
  * @param  loadSwitch: Load Switch to be de-init.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  *            @arg LOAD_SWITCH3
  * @note Load Switch DeInit does not disable the GPIO clock nor disable the Mfx
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LS_DeInit(Load_Switch_TypeDef loadSwitch)
{
  /* Turn off Load Switch */
  HAL_GPIO_WritePin(LOAD_SWITCH_PORT[loadSwitch], LOAD_SWITCH_PIN[loadSwitch], GPIO_PIN_RESET);

  /* DeInit the GPIO_LOAD_SWITCH pin */
  HAL_GPIO_DeInit(LOAD_SWITCH_PORT[loadSwitch], LOAD_SWITCH_PIN[loadSwitch]);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected Load Switch On.
  * @param  loadSwitch: Specifies the Load Switch to be set on.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  *            @arg LOAD_SWITCH3
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LS_On(Load_Switch_TypeDef loadSwitch)
{
  HAL_GPIO_WritePin(LOAD_SWITCH_PORT[loadSwitch], LOAD_SWITCH_PIN[loadSwitch], GPIO_PIN_SET);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected Load Switch Off.
  * @param  loadSwitch: Specifies the Load Switch to be set off.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  *            @arg LOAD_SWITCH3
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LS_Off(Load_Switch_TypeDef loadSwitch)
{
  HAL_GPIO_WritePin(LOAD_SWITCH_PORT[loadSwitch], LOAD_SWITCH_PIN[loadSwitch], GPIO_PIN_RESET);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Toggles the selected Load Switch.
  * @param  loadSwitch: Specifies the Load Switch to be toggled.
  *         This parameter can be one of the following values:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  *            @arg LOAD_SWITCH3
  * @return STNODE_BSP status
  */
int32_t STNODE_BSP_LS_Toggle(Load_Switch_TypeDef loadSwitch)
{
  HAL_GPIO_TogglePin(LOAD_SWITCH_PORT[loadSwitch], LOAD_SWITCH_PIN[loadSwitch]);

  return STNODE_BSP_ERROR_NONE;
}

/**
  * @brief  Get the status of the selected Load Switch.
  * @param  loadSwitch Specifies the Load Switch to get its state.
  *         This parameter can be one of following parameters:
  *            @arg LOAD_SWITCH1
  *            @arg LOAD_SWITCH2
  *            @arg LOAD_SWITCH3
  * @return Load Switch status
  */
int32_t STNODE_BSP_LS_GetState(Load_Switch_TypeDef loadSwitch)
{
  return (int32_t)HAL_GPIO_ReadPin(LOAD_SWITCH_PORT[loadSwitch], LOAD_SWITCH_PIN[loadSwitch]);
}

/**
 * Battery Monitoring (BM) APIs
 */

/**
 * @brief Configure battery monitoring control GPIO and ADC pin.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_BM_Init(void)
{
  GPIO_InitTypeDef gpio_init_structure = {0};

  /* Enable the VBAT PORT Clock */
  VBAT_GPIO_CLK_ENABLE();

  /* Configure the VBAT pin */
  gpio_init_structure.Pin = VBAT_PIN;
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(VBAT_PORT, &gpio_init_structure);
  HAL_GPIO_WritePin(VBAT_PORT, VBAT_PIN, GPIO_PIN_RESET);

  //TODO: Add ADC init here, see https://github.com/TheThingsIndustries/st-node/issues/29

  return STNODE_BSP_ERROR_NONE;
}

/**
 * @brief DeInit BM pins configurations.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_BM_DeInit(void)
{
  /* Turn off VBAT pin */
  HAL_GPIO_WritePin(VBAT_PORT, VBAT_PIN, GPIO_PIN_RESET);

  /* DeInit the VBAT pin */
  HAL_GPIO_DeInit(VBAT_PORT, VBAT_PIN);

  //TODO: Add ADC Deinit here, see https://github.com/TheThingsIndustries/st-node/issues/29

  return STNODE_BSP_ERROR_NONE;
}

/**
 * @brief Enable battery monitoring PIN.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_BM_Enable(void)
{
  HAL_GPIO_WritePin(VBAT_PORT, VBAT_PIN, GPIO_PIN_SET);

  return STNODE_BSP_ERROR_NONE;
}

/**
 * @brief Disable battery monitoring PIN.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_BM_Disable(void)
{
  HAL_GPIO_WritePin(VBAT_PORT, VBAT_PIN, GPIO_PIN_RESET);

  return STNODE_BSP_ERROR_NONE;
}

/**
 * @brief Get the state of battery monitoring PIN.
 *
 * @return STNODE_BSP status
 */
int32_t STNODE_BSP_BM_GetState(void)
{
  return (int32_t)HAL_GPIO_ReadPin(VBAT_PORT, VBAT_PIN);
}

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

  if (HAL_UART_Init(&STNODE_BSP_debug_usart) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  /* In StopMode: Need UART in FIFO mode so characters are displayed on Hyperterminal */
  HAL_UARTEx_EnableFifoMode(&STNODE_BSP_debug_usart);
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
  __HAL_RCC_DMA1_CLK_ENABLE();
#ifdef HAL_ADC_MODULE_ENABLED
  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
#endif
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

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

// TODO: Add communication init like UART, SPI and I2C, see https://github.com/TheThingsIndustries/st-node/issues/30

int32_t STNODE_BSP_BUZZER_TIM_Init(pTIM_CallbackTypeDef cb)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  STNODE_BSP_buzzer_timer.Instance = BUZZER_TIMER;
  STNODE_BSP_buzzer_timer.Init.Prescaler = BUZZER_PRESCALER;
  STNODE_BSP_buzzer_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  STNODE_BSP_buzzer_timer.Init.Period = BUZZER_PERIOD;
  STNODE_BSP_buzzer_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  STNODE_BSP_buzzer_timer.Init.RepetitionCounter = 0;
  STNODE_BSP_buzzer_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&STNODE_BSP_buzzer_timer) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&STNODE_BSP_buzzer_timer, &sMasterConfig) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&STNODE_BSP_buzzer_timer, &sConfigOC, BUZZER_TIMER_CHANNEL) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&STNODE_BSP_buzzer_timer, &sBreakDeadTimeConfig) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }
  HAL_TIM_RegisterCallback(&STNODE_BSP_buzzer_timer, HAL_TIM_PERIOD_ELAPSED_CB_ID, cb);
  if (HAL_TIM_PWM_Start_IT(&STNODE_BSP_buzzer_timer, BUZZER_TIMER_CHANNEL) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }
  return STNODE_BSP_ERROR_NONE;
}

int32_t STNODE_BSP_RTC_Init(void)
{
  RTC_AlarmTypeDef sAlarm = {0};

  STNODE_BSP_rtc.Instance = RTC;
  STNODE_BSP_rtc.Init.AsynchPrediv = RTC_PREDIV_A;
  STNODE_BSP_rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  STNODE_BSP_rtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  STNODE_BSP_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  STNODE_BSP_rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  STNODE_BSP_rtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  STNODE_BSP_rtc.Init.BinMode = RTC_BINARY_ONLY;
  if (HAL_RTC_Init(&STNODE_BSP_rtc) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }

  // Initialize RTC and set the Time and Date
  if (HAL_RTCEx_SetSSRU_IT(&STNODE_BSP_rtc) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }
  // Enable the Alarm A
  sAlarm.BinaryAutoClr = RTC_ALARMSUBSECONDBIN_AUTOCLR_NO;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDBINMASK_NONE;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&STNODE_BSP_rtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    return STNODE_BSP_ERROR_NO_INIT;
  }
  return STNODE_BSP_ERROR_NONE;
}

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
 * @file node_bsp.c
 *
 * @copyright Copyright (c) 2020 The Things Industries B.V.
 *
 */
#include "node_bsp.h"

/**
  * @brief  Configures LED GPIO.
  * @param  Led: LED to be configured.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  *            @arg LED3
  * @retval BSP status
  */
int32_t BSP_LED_Init(Led_TypeDef Led)
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
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);

  return BSP_ERROR_NONE;
}

/**
  * @brief  DeInit LEDs.
  * @param  Led: LED to be de-init.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @note Led DeInit does not disable the GPIO clock nor disable the Mfx
  * @retval BSP status
  */
int32_t BSP_LED_DeInit(Led_TypeDef Led)
{
  /* Turn off LED */
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);

  /* DeInit the GPIO_LED pin */
  HAL_GPIO_DeInit(LED_PORT[Led], LED_PIN[Led]);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @retval BSP status
  */
int32_t BSP_LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @retval BSP status
  */
int32_t BSP_LED_Off(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled.
  *         This parameter can be one of the following values:
  *            @arg LED1
  *            @arg LED2
  * @retval BSP status
  */
int32_t BSP_LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Get the status of the selected LED.
  * @param  Led Specifies the Led to get its state.
  *         This parameter can be one of following parameters:
  *            @arg LED1
  *            @arg LED2
  *            @arg LED3
  * @retval LED status
  */
int32_t BSP_LED_GetState(Led_TypeDef Led)
{
  return (int32_t)HAL_GPIO_ReadPin(LED_PORT[Led], LED_PIN[Led]);
}

/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @param  ButtonMode: Specifies Button mode.
  *   This parameter can be one of following parameters:
  *     @arg BUTTON_MODE_GPIO: Button will be used as simple IO
  *     @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt
  *                            generation capability
  * @retval BSP status
  */
int32_t BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{
  GPIO_InitTypeDef gpio_init_structure = {0};
  static BSP_EXTI_LineCallback button_callback[BUTTONn] = {BUTTON_SW1_EXTI_Callback, BUTTON_SW2_EXTI_Callback, BUTTON_SW3_EXTI_Callback};
  static uint32_t button_interrupt_priority[BUTTONn] = {BSP_BUTTON_SWx_IT_PRIORITY, BSP_BUTTON_SWx_IT_PRIORITY, BSP_BUTTON_SWx_IT_PRIORITY};
  static const uint32_t button_exti_line[BUTTONn] = {BUTTON_SW1_EXTI_LINE, BUTTON_SW2_EXTI_LINE, BUTTON_SW3_EXTI_LINE};

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

  return BSP_ERROR_NONE;
}

/**
  * @brief  Push Button DeInit.
  * @param  Button: Button to be configured
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @note PB DeInit does not disable the GPIO clock
  * @retval BSP status
  */
int32_t BSP_PB_DeInit(Button_TypeDef Button)
{
  HAL_NVIC_DisableIRQ((BUTTON_IRQn[Button]));
  HAL_GPIO_DeInit(BUTTON_PORT[Button], BUTTON_PIN[Button]);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @retval The Button GPIO pin value.
  */
int32_t BSP_PB_GetState(Button_TypeDef Button)
{
  return (int32_t)HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}

/**
  * @brief  This function handles Push-Button interrupt requests.
  * @param  Button Specifies the pin connected EXTI line
  * @retval None
  */
void BSP_PB_IRQHandler(Button_TypeDef Button)
{
  HAL_EXTI_IRQHandler(&hpb_exti[Button]);
}

/**
  * @brief  BSP Push Button callback
  * @param  Button: Specifies the Button to be checked.
  *         This parameter can be one of following parameters:
  *           @arg BUTTON_SW1
  *           @arg BUTTON_SW2
  *           @arg BUTTON_SW3
  * @retval None.
  */
__weak void BSP_PB_Callback(Button_TypeDef Button)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Button);

  /* This function should be implemented by the user application.
     It is called into this driver when an event on Button is triggered. */
}

/**
  * @brief  Button SW1 EXTI line detection callback.
  * @retval None
  */

static void BUTTON_SW1_EXTI_Callback(void)
{
  BSP_PB_Callback(BUTTON_SW1);
}

// TODO: Add communication init like UART and SPI and I2C if needed.
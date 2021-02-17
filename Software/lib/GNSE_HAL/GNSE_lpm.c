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
 * @file GNSE_lpm.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_lpm.h"
#include "usart_if.h"

/**
  * @brief Power driver callbacks handler
  */
const struct UTIL_LPM_Driver_s UTIL_PowerDriver =
{
  GNSE_LPM_EnterSleepMode,
  GNSE_LPM_ExitSleepMode,

  GNSE_LPM_EnterStopMode,
  GNSE_LPM_ExitStopMode,

  GNSE_LPM_EnterOffMode,
  GNSE_LPM_ExitOffMode,
};

/**
  * @brief Enters Low Power Off Mode
  * @param none
  * @return none
  */
void GNSE_LPM_EnterOffMode(void)
{
  // Not implemented.
}

/**
  * @brief Exits Low Power Off Mode
  * @param none
  * @return none
  */
void GNSE_LPM_ExitOffMode(void)
{
  // Not implemented.
}

/**
  * @brief Enters Low Power Stop Mode
  * @note ARM exists the function when waking up
  * @param none
  * @return none
  */
void GNSE_LPM_EnterStopMode(void)
{
  /* Suspend sysTick : work around for degugger problem in dual core (tickets 71085,  72038, 71087 ) */
  HAL_SuspendTick();
  /* Clear Status Flag before entering STOP/STANDBY Mode */
  LL_PWR_ClearFlag_C1STOP_C1STB();
  HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
}

/**
  * @brief Exits Low Power Stop Mode
  * @note Enable the pll at 32MHz
  * @param none
  * @return none
  */
void GNSE_LPM_ExitStopMode(void)
{
  /* Resume sysTick : work around for degugger problem in dual core */
  HAL_ResumeTick();
  /*Not retained periph:
    ADC interface
    DAC interface USARTx, TIMx, i2Cx, SPIx
    SRAM ctrls, DMAx, DMAMux, AES, RNG, HSEM  */

  /* Resume not retained USARTx and DMA */
  vcom_Resume();
}

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @return none
  */
void GNSE_LPM_EnterSleepMode(void)
{
  /* Suspend sysTick */
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

/**
  * @brief Exists Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @return none
  */
void GNSE_LPM_ExitSleepMode(void)
{
  /* Suspend sysTick */
  HAL_ResumeTick();
}

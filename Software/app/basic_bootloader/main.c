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
#include "GNSE_hal.h"
#include "bootloader.h"

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  if (GNSE_HAL_SysClk_Init() != GNSE_HAL_OP_SUCCESS)
  {
    GNSE_HAL_Error_Handler();
  }

  /* Initialize Tracer/Logger */
  GNSE_TRACER_INIT();

  GNSE_app_printAppInfo();
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

  GNSE_HAL_Error_Handler();
}

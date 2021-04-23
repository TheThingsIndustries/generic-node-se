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
 * @file GNSE_flash.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "GNSE_flash.h"
#include "GNSE_bsp.h"

MxChip GNSE_Flash;
spiffs GNSE_Flash_SPIFFS;

/**
 * Buffers needed for SPIFFS, based on lib author requirements
 * @warning: Avoid changing these parameters unless you are familiar with the change effect
 */
#define GNSE_FLASH_PAGE_SIZE 256U
#define GNSE_FLASH_FS_FD 4U
#define GNSE_FLASH_FS_FD_SIZE 32U
#define GNSE_FLASH_FS_MIN_WORK_BUFFER 2U

static uint8_t spiffs_work_buf[GNSE_FLASH_PAGE_SIZE * GNSE_FLASH_FS_MIN_WORK_BUFFER];
static uint8_t spiffs_fds[GNSE_FLASH_FS_FD_SIZE * GNSE_FLASH_FS_FD];
static uint8_t spiffs_cache_buf[(GNSE_FLASH_PAGE_SIZE + GNSE_FLASH_FS_FD_SIZE) * GNSE_FLASH_FS_FD];

/**
  * @brief Initialize hardware of the external SPI flash
  * @param none
  * @return FLASH_op_result_t, see enum for more information
  */
FLASH_op_result_t GNSE_Flash_Init(void)
{
// TODO: Fix issue of system hanging with this function is called more than once, https://github.com/TheThingsIndustries/generic-node-se/issues/173
  GNSE_BSP_LS_Init(LOAD_SWITCH_FLASH);
  GNSE_BSP_LS_On(LOAD_SWITCH_FLASH);
  HAL_Delay(LOAD_SWITCH_FLASH_DELAY_MS);
  if (MX25R16_Init(&GNSE_Flash) != MXST_SUCCESS)
  {
    return FLASH_OP_FAIL;
  }
  return FLASH_OP_SUCCESS;
}

/**
  * @brief Deinitialize hardware of the external SPI flash
  * @param none
  * @return FLASH_op_result_t, see enum for more information
  */
FLASH_op_result_t GNSE_Flash_DeInit(void)
{
  GNSE_BSP_LS_Off(LOAD_SWITCH_FLASH);
  GNSE_BSP_LS_DeInit(LOAD_SWITCH_FLASH);
  GNSE_BSP_Flash_SPI_DeInit();
  return FLASH_OP_SUCCESS;
}

/**
 * @brief Reads a number of bytes from external flash
 * This function abstracts MxREAD (found as part of MX25R16 APIs in nor_cmd.c)
 *
 * @param addr for MX25R16 SPI flash the address range is from 0x000000 to 0x1FFFFF => 16 Mb
 * @param byte_count
 * @param target_buffer the buffer that will be used to store the read data
 * @return FLASH_op_result_t, see enum for more information
 */
FLASH_op_result_t GNSE_Flash_Read(uint32_t addr, uint32_t byte_count, uint8_t *target_buffer)
{
  FLASH_op_result_t status = FLASH_OP_SUCCESS;
  if (MxIsFlashBusy(&GNSE_Flash) == MXST_DEVICE_READY)
  {
    if (GNSE_Flash.AppGrp._Read(&GNSE_Flash, addr, byte_count, target_buffer) != MXST_SUCCESS)
    {
      status = FLASH_OP_FAIL;
    }
  }
  else
  {
    status = FLASH_OP_FAIL;
  }

  return status;
}

/**
 * @brief Write a number of bytes to external flash
 * This function abstracts MxPP (found as part of MX25R16 APIs in nor_cmd.c)
 *
 * @param addr for MX25R16 SPI flash the address range is from 0x000000 to 0x1FFFFF => 16 Mb
 * @param byte_count
 * @param source_buffer the buffer that written to the external flash
 * @return FLASH_op_result_t, see enum for more information
 */
FLASH_op_result_t GNSE_Flash_Write(uint32_t addr, uint32_t byte_count, uint8_t *source_buffer)
{
  FLASH_op_result_t status = FLASH_OP_SUCCESS;
  if (MxIsFlashBusy(&GNSE_Flash) == MXST_DEVICE_READY)
  {
    if (GNSE_Flash.AppGrp._Write(&GNSE_Flash, addr, byte_count, source_buffer) != MXST_SUCCESS)
    {
      status = FLASH_OP_FAIL;
    }
  }
  else
  {
    status = FLASH_OP_FAIL;
  }

  return status;
}

/**
 * @brief Erases a number or blocks, each block is 64KByte in size
 * This function abstracts MxBE (found as part of MX25R16 APIs in nor_cmd.c)
 * It will erase the block correlating to the given addr and following blocks depending on block_count
 *
 * @param addr for MX25R16 SPI flash the address range is from 0x000000 to 0x1FFFFF => 16 Mb
 * @param block_count for MX25R16 SPI flash the block count is from 0 to 31 => 16 Mb
 * @return FLASH_op_result_t, see enum for more information
 */
FLASH_op_result_t GNSE_Flash_BlockErase(uint32_t addr, uint32_t block_count)
{
  FLASH_op_result_t status = FLASH_OP_SUCCESS;
  if (MxIsFlashBusy(&GNSE_Flash) == MXST_DEVICE_READY)
  {
    if (GNSE_Flash.AppGrp._Erase(&GNSE_Flash, addr, block_count) != MXST_SUCCESS)
    {
      status = FLASH_OP_FAIL;
    }
  }
  else
  {
    status = FLASH_OP_FAIL;
  }

  return status;
}

/**
 * @brief Erases all 16Mb of the external SPI flash
 * @note This function consumes a lot of power and takes long time to execute, use with caution
 *
 * @return FLASH_op_result_t, see enum for more information
 */
FLASH_op_result_t GNSE_Flash_ChipErase(void)
{
  FLASH_op_result_t status = FLASH_OP_SUCCESS;
  if (MxIsFlashBusy(&GNSE_Flash) == MXST_DEVICE_READY)
  {
    if (MxCE(&GNSE_Flash) != MXST_SUCCESS)
    {
      status = FLASH_OP_FAIL;
    }
  }
  else
  {
    status = FLASH_OP_FAIL;
  }

  return status;
}

static s32_t spiffs_read_wrapper(u32_t addr, u32_t size, u8_t *dst)
{
  return (s32_t)GNSE_Flash_Read(addr, size, dst);
}

static s32_t spiffs_write_wrapper(u32_t addr, u32_t size, u8_t *src)
{
  return (s32_t)GNSE_Flash_Write(addr, size, src);
}

static s32_t spiffs_erase_wrapper(u32_t addr, u32_t size)
{
  return (s32_t)GNSE_Flash_BlockErase(addr, size);
}

/**
 * @brief Mounts the external flash as a file system using the SPIFFS library
 * @note This function should be called after GNSE_Flash_Init()
 *
 * @return FLASH_op_result_t, see enum for more information
 * In case of mount failure, invoke GNSE_Flash_ChipErase() "once"
 */
FLASH_op_result_t GNSE_Flash_mount(void)
{
  spiffs_config cfg;
  cfg.phys_size = GNSE_Flash.ChipSz; // use all spi flash
  cfg.phys_addr = 0;                 // start spiffs at start of spi flash
  cfg.phys_erase_block = GNSE_Flash.BlockSz;
  cfg.log_block_size = GNSE_Flash.BlockSz;
  cfg.log_page_size = GNSE_Flash.PageSz;

  cfg.hal_read_f = spiffs_read_wrapper;
  cfg.hal_write_f = spiffs_write_wrapper;
  cfg.hal_erase_f = spiffs_erase_wrapper;

  if (SPIFFS_mount(&GNSE_Flash_SPIFFS, &cfg, spiffs_work_buf, spiffs_fds,
                   sizeof(spiffs_fds),
                   spiffs_cache_buf,
                   sizeof(spiffs_cache_buf),
                   0) != SPIFFS_OK)
  {
    /*
     * Call GNSE_Flash_ChipErase() "once" before GNSE_Flash_mount()
     * to avoid mount issues
     */
    return FLASH_OP_FAIL;
  }
  return FLASH_OP_SUCCESS;
}

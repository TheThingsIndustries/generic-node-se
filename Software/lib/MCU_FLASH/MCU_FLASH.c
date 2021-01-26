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
 * @file MCU_FLASH.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "MCU_FLASH.h"

#define NB_PAGE_SECTOR_PER_ERASE  2U    /*!< Nb page erased per erase */

static __IO uint32_t DoubleECC_Error_Counter = 0U;

static __IO uint8_t DoubleECC_Check;

/**
  * @brief  Gets the page of a given address
  * @param  uAddr: Address of the FLASH Memory
  * @return The page of a given address
  */
static uint32_t GetPage(uint32_t uAddr)
{
  uint32_t page = 0U;

  if (uAddr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (uAddr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (uAddr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @return HAL Status.
  */
HAL_StatusTypeDef MCU_FLASH_Init(void)
{
  HAL_StatusTypeDef ret = HAL_ERROR;

  /* Unlock the Program memory */
  if (HAL_FLASH_Unlock() == HAL_OK)
  {
    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    /* Unlock the Program memory */
    if (HAL_FLASH_Lock() == HAL_OK)
    {
      ret = HAL_OK;
    }
  }
  return ret;
}

/**
  * @brief  This function does an erase of n (depends on Length) pages in user flash area
  * @param  pStart: Start of user flash area
  * @param  uLength: number of bytes.
  * @return HAL status.
  */
HAL_StatusTypeDef MCU_FLASH_Erase(void *pStart, uint32_t uLength)
{
  uint32_t page_error = 0U;
  uint32_t uStart = (uint32_t)pStart;
  FLASH_EraseInitTypeDef x_erase_init;
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;
  uint32_t first_page = 0U, nb_pages = 0U;
  uint32_t chunk_nb_pages;
  uint32_t erase_command = 0U;

  /* Initialize Flash */
  e_ret_status = MCU_FLASH_Init();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    if (HAL_FLASH_Unlock() == HAL_OK)
    {
      do
      {
        /* Get the 1st page to erase */
        first_page = GetPage(uStart);
        /* Get the number of pages to erase from 1st page */
        nb_pages = GetPage(uStart + uLength - 1U) - first_page + 1U;

        /* Fill EraseInit structure*/
        x_erase_init.TypeErase = FLASH_TYPEERASE_PAGES;

        /* Erase flash per NB_PAGE_SECTOR_PER_ERASE to avoid watch-dog */
        do
        {
          chunk_nb_pages = (nb_pages >= NB_PAGE_SECTOR_PER_ERASE) ? NB_PAGE_SECTOR_PER_ERASE : nb_pages;
          x_erase_init.Page = first_page;
          x_erase_init.NbPages = chunk_nb_pages;
          first_page += chunk_nb_pages;
          nb_pages -= chunk_nb_pages;
          if (HAL_FLASHEx_Erase(&x_erase_init, &page_error) != HAL_OK)
          {
            HAL_FLASH_GetError();
            e_ret_status = HAL_ERROR;
          }
          /* Refresh Watchdog */
          /* WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD); */
        }
        while (nb_pages > 0);
        erase_command = 1U;
      }
      while (erase_command == 0);
      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();

    }
    else
    {
      e_ret_status = HAL_ERROR;
    }
  }

  return e_ret_status;
}

/**
  * @brief  This function writes a data buffer in flash (data are 64-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  pDestination: Start address for target location
  * @param  pSource: pointer on buffer with data to write
  * @param  uLength: Length of data buffer in byte. It has to be 64-bit aligned.
  * @return HAL Status.
  */
HAL_StatusTypeDef MCU_FLASH_Write(uint32_t pDestination, uint8_t *pSource, uint32_t uLength)
{
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;
  uint32_t i = 0U;
  uint64_t data;

  /* Initialize Flash */
  e_ret_status = MCU_FLASH_Init();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
      MCU_FLASH_PPRINTF("ERROR ==> Unlock not possible\n");
      return HAL_ERROR;
    }
    else
    {
      MCU_FLASH_PPRINTF("Flash Write : Memory addr 0x%08x length%04d\r\n", pDestination, uLength);

      /* DataLength must be a multiple of 64 bit */
      for (i = 0U; i < uLength; i += 8U)
      {
        UTIL_MEM_cpy_8((void*)&data, (const void*)(pSource + i), 8);
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
        be done by word */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pDestination, data) == HAL_OK)
        {
          /* Check the written value */
          if (*(uint64_t *)pDestination != data)
          {
            /* Flash content doesn't match SRAM content */
            e_ret_status = HAL_ERROR;
            MCU_FLASH_PPRINTF("ERROR ==> Memory check failure\n");
            break;
          }
          /* Increment FLASH Destination address */
          pDestination += 8U;
        }
        else
        {
          /* Error occurred while writing data in Flash memory */
          e_ret_status = HAL_ERROR;
          MCU_FLASH_PPRINTF("ERROR ==> Memory write failure\n");
          break;
        }
      }
      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();
    }
  }
  return e_ret_status;
}

/**
  * @brief  This function reads flash
  * @param  pDestination: Start address for target location
  * @param  pSource: pointer on buffer with data to write
  * @param  Length: Length in bytes of data buffer
  * @return HAL_StatusTypeDef HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef MCU_FLASH_Read(void *pDestination, const void *pSource, uint32_t Length)
{
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;

  DoubleECC_Error_Counter = 0U;
  DoubleECC_Check = 1;
  memcpy(pDestination, pSource, Length);
  DoubleECC_Check = 0;
  if (DoubleECC_Error_Counter == 0U)
  {
    e_ret_status = HAL_OK;
  }
  DoubleECC_Error_Counter = 0U;

  return e_ret_status;
}

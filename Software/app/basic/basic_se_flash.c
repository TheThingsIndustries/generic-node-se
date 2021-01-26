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
 * @file basic_se_flash.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "tnglora_read_helper.h"

static void print_buffer(const char *message, uint8_t *buffer, size_t buffer_size)
{
    APP_PPRINTF(message);
    for (size_t ix = 0; ix < buffer_size; ix++)
    {
        APP_PPRINTF("%02x ", buffer[ix]);
    }
    APP_PPRINTF("\r\n");
}

void secure_element_read_info(void)
{
    // Initialize the on boarad secure element and read serial number
    ATCAIfaceCfg atecc608_i2c_config;
    uint8_t secure_element_serialnum[ATCA_SERIAL_NUM_SIZE];
    uint8_t secure_element_devEUI[DEV_EUI_SIZE_BYTE];
    uint8_t secure_element_joinEUI[JOIN_EUI_SIZE_BYTE];

    atecc608_i2c_config.iface_type = ATCA_I2C_IFACE;

    atecc608_i2c_config.atcai2c.baud = ATCA_HAL_ATECC608A_I2C_FREQUENCY;
    atecc608_i2c_config.atcai2c.bus = ATCA_HAL_ATECC608A_I2C_BUS_PINS;
    atecc608_i2c_config.atcai2c.slave_address = ATCA_HAL_ATECC608A_I2C_ADDRESS;
    atecc608_i2c_config.devtype = ATECC608A;
    atecc608_i2c_config.rx_retries = ATCA_HAL_ATECC608A_I2C_RX_RETRIES;
    atecc608_i2c_config.wake_delay = ATCA_HAL_ATECC608A_I2C_WAKEUP_DELAY;
    if (atcab_init(&atecc608_i2c_config) != ATCA_SUCCESS)
    {
        APP_PPRINTF("\r\n Failed to initialize ATECC608A-TNGLORA secure elelment \r\n");
        return;
    }

    ATCA_STATUS read_status = atcab_read_serial_number(secure_element_serialnum);
    if (read_status != ATCA_SUCCESS)
    {
        APP_PPRINTF("\r\n Failed to read ATECC608A-TNGLORA (%d) \r\n", read_status);
        return;
    }

    read_status = atcab_read_devEUI(secure_element_devEUI);
    if (read_status != ATCA_SUCCESS)
    {
        APP_PPRINTF("\r\n Failed to read ATECC608A-TNGLORA devEUI (%d) \r\n", read_status);
        return;
    }

    read_status = atcab_read_joinEUI(secure_element_joinEUI);
    if (read_status != ATCA_SUCCESS)
    {
        APP_PPRINTF(" \r\n Failed to read ATECC608A-TNGLORA joinEUI (%d) \r\n", read_status);
        return;
    }

    print_buffer("\r\n SE Serial Number:          ", secure_element_serialnum, ATCA_SERIAL_NUM_SIZE);
    print_buffer("\r\n SE Device EUI:             ", secure_element_devEUI, DEV_EUI_SIZE_BYTE);
    print_buffer("\r\n SE Application/Join EUI:   ", secure_element_joinEUI, JOIN_EUI_SIZE_BYTE);

    return;
}

void flash_read_write(void)
{
    MxChip mxic = {0};
    if (MX25R16_Init(&mxic) != MXST_SUCCESS)
    {
        APP_PPRINTF("\r\n Failed to init external SPI flash (MX25R1635F)\r\n");
        return;
    }

    if (MxSimpleTest(&mxic) == MXST_SUCCESS)
    {
        APP_PPRINTF("\r\n Simple external SPI flash (MX25R1635F) test passed!\r\n");
    }
    else
    {
        APP_PPRINTF("\r\n Simple external SPI flash test (MX25R1635F) failed, check UART logs for more details\r\n");
    }
}

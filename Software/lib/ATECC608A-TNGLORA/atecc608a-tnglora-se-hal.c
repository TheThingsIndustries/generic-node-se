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
 * @file hal_LoraMacNode.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include <string.h>
#include <stdio.h>

#include "atca_hal.h"
#include "atca_device.h"
#include "atca_status.h"
#include "stm32wlxx_hal_dma.h"
#include "stm32wlxx_hal_i2c.h"
#include "GNSE_bsp.h"

uint32_t ATECC608ASeHalGetRandomNumber( void )
{
    // Not Implemeted
    //TODO: Add suppport for random generation, see https://github.com/TheThingsIndustries/generic-node-se/issues/137
    return 0;
}

/** @brief This function delays for a number of microseconds.
 *
 * @param[in] delay number of 0.001 milliseconds to delay
 */
void atca_delay_us(uint32_t delay)
{
    HAL_Delay(delay / 1000);
}

/** @brief This function delays for a number of tens of microseconds.
 *
 * @param[in] delay number of 0.01 milliseconds to delay
 */
void atca_delay_10us(uint32_t delay)
{
    HAL_Delay(delay / 100);
}

/** @brief This function delays for a number of milliseconds.
 *
 *         You can override this function if you like to do
 *         something else in your system while delaying.
 * @param[in] delay number of milliseconds to delay
 */
void atca_delay_ms(uint32_t delay)
{
    HAL_Delay(delay);
}

/** @brief discover i2c buses available for this hardware
 * this maintains a list of logical to physical bus mappings freeing the application
 * of the a-priori knowledge
 * @param[in] i2c_buses - an array of logical bus numbers
 * @param[in] max_buses - maximum number of buses the app wants to attempt to discover
 * @return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
    return ATCA_SUCCESS;
}

/** @brief discover any CryptoAuth devices on a given logical bus number
 * @param[in]  bus_num  logical bus number on which to look for CryptoAuth devices
 * @param[out] cfg     pointer to head of an array of interface config structures which get filled in by this method
 * @param[out] found   number of devices found on this bus
 * @return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_discover_devices(int bus_num, ATCAIfaceCfg cfg[], int *found)
{
    return ATCA_SUCCESS;
}

/** @brief hal_i2c_init manages requests to initialize a physical interface.  it manages use counts so when an interface
 * has released the physical layer, it will disable the interface for some other use.
 * You can have multiple ATCAIFace instances using the same bus, and you can have multiple ATCAIFace instances on
 * multiple i2c buses, so hal_i2c_init manages these things and ATCAIFace is abstracted from the physical details.
 */

/** @brief initialize an I2C interface using given config
 * @param[in] hal - opaque ptr to HAL data
 * @param[in] cfg - interface configuration
 * @return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
    // This fuction is a place filler for now, TODO: add an improved init, see https://github.com/TheThingsIndustries/generic-node-se/issues/33
    return ATCA_SUCCESS;
}

/** @brief HAL implementation of I2C post init
 * @param[in] iface  instance
 * @return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    // This fuction is a place filler for now, TODO: add an improved init, see https://github.com/TheThingsIndustries/generic-node-se/issues/33
    return ATCA_SUCCESS;
}

/** @brief HAL implementation of I2C send over ASF
 * @param[in] iface     instance
 * @param[in] txdata    pointer to space to bytes to send
 * @param[in] txlength  number of bytes to send
 * @return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t word_address, uint8_t *txdata, int txlength)
{
    txdata[0] = 0x3;
    txlength++;
    if (HAL_I2C_Master_Transmit(&GNSE_BSP_sensor_i2c1, (uint16_t)iface->mIfaceCFG->atcai2c.slave_address, txdata, (size_t)txlength, ATCA_HAL_ATECC608A_LONG_TIMEOUT) == HAL_OK)
    {
        return ATCA_SUCCESS;
    }
    else
    {
        return ATCA_TX_FAIL;
    }
}

/** @brief HAL implementation of I2C receive function for ASF I2C
 * @param[in]    iface     Device to interact with.
 * @param[out]   rxdata    Data received will be returned here.
 * @param[inout] rxlength  As input, the size of the rxdata buffer.
 *                         As output, the number of bytes received.
 * @return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t word_address, uint8_t *rxdata, uint16_t *rxlength)
{
    // read procedure is:
    // 1. read 1 byte, this will be the length of the package
    // 2. read the rest of the package

    uint8_t lengthPackage[1] = {0};
    HAL_StatusTypeDef r = HAL_BUSY;
    int retries = iface->mIfaceCFG->rx_retries;
    while (--retries > 0 && r != HAL_OK)
    {
        r = HAL_I2C_Master_Receive(&GNSE_BSP_sensor_i2c1, (uint16_t)iface->mIfaceCFG->atcai2c.slave_address, lengthPackage, 1, ATCA_HAL_ATECC608A_LONG_TIMEOUT);
    }

    if (r != HAL_OK)
    {
        return ATCA_RX_TIMEOUT;
    }

    uint8_t bytesToRead = lengthPackage[0] - 1;

    if (bytesToRead > *rxlength)
    {
        return ATCA_SMALL_BUFFER;
    }

    memset(rxdata, 0, *rxlength);
    rxdata[0] = lengthPackage[0];

    r = HAL_BUSY;
    retries = iface->mIfaceCFG->rx_retries;
    while (--retries > 0 && r != HAL_OK)
    {
        r = HAL_I2C_Master_Receive(&GNSE_BSP_sensor_i2c1, (uint16_t)iface->mIfaceCFG->atcai2c.slave_address, rxdata + 1, bytesToRead, ATCA_HAL_ATECC608A_LONG_TIMEOUT);
    }

    if (r != HAL_OK)
    {
        return ATCA_RX_TIMEOUT;
    }

    *rxlength = lengthPackage[0];

    return ATCA_SUCCESS;
}

/** @brief method to change the bus speec of I2C
 * @param[in] iface  interface on which to change bus speed
 * @param[in] speed  baud rate (typically 100000 or 400000)
 */
void change_i2c_speed(ATCAIface iface, uint32_t speed)
{
    return;
}

/** @brief wake up CryptoAuth device using I2C bus
 * @param[in] iface  interface to logical device to wakeup
 * @return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
    // 2. Send NULL buffer to address 0x0 (NACK)
    uint8_t emptybuff[1] = {0};
    HAL_StatusTypeDef r = HAL_I2C_Master_Transmit(&GNSE_BSP_sensor_i2c1, 0x00, emptybuff, (size_t)0, ATCA_HAL_ATECC608A_LONG_TIMEOUT);

    // 3. Wait for wake_delay
    atca_delay_us(iface->mIfaceCFG->wake_delay);

    uint8_t rx_buffer[4] = {0};

    // 4. Read from normal slave_address
    r = HAL_BUSY;
    int retries = iface->mIfaceCFG->rx_retries;
    while (--retries > 0 && r != HAL_OK)
    {
        r = HAL_I2C_Master_Receive(&GNSE_BSP_sensor_i2c1, (uint16_t)iface->mIfaceCFG->atcai2c.slave_address, rx_buffer, 4, ATCA_HAL_ATECC608A_LONG_TIMEOUT);
    }

    // 5. Set frequency back to requested one
    const uint8_t expected_response[4] = {0x04, 0x11, 0x33, 0x43};
    uint8_t selftest_fail_resp[4] = {0x04, 0x07, 0xC4, 0x40};

    if (memcmp(rx_buffer, expected_response, 4) == 0)
    {
        return ATCA_SUCCESS;
    }
    if (memcmp(rx_buffer, selftest_fail_resp, 4) == 0)
    {
        return ATCA_STATUS_SELFTEST_ERROR;
    }
    return ATCA_WAKE_FAILED;
}

/** @brief idle CryptoAuth device using I2C bus
 * @param[in] iface  interface to logical device to idle
 * @return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
    uint8_t buffer[1] = {0x2}; // idle word address value
    HAL_I2C_Master_Transmit(&GNSE_BSP_sensor_i2c1, (uint16_t)iface->mIfaceCFG->atcai2c.slave_address, buffer, (size_t)1, ATCA_HAL_ATECC608A_LONG_TIMEOUT);
    return ATCA_SUCCESS;
}

/** @brief sleep CryptoAuth device using I2C bus
 * @param[in] iface  interface to logical device to sleep
 * @return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
    uint8_t buffer[1] = {0x1}; // sleep word address value
    HAL_I2C_Master_Transmit(&GNSE_BSP_sensor_i2c1, (uint16_t)iface->mIfaceCFG->atcai2c.slave_address, buffer, (size_t)1, ATCA_HAL_ATECC608A_LONG_TIMEOUT);
    return ATCA_SUCCESS;
}

/** @brief manages reference count on given bus and releases resource if no more refences exist
 * @param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_release(void *hal_data)
{
    return ATCA_SUCCESS;
}

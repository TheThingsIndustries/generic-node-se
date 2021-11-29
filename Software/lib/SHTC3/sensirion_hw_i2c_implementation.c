/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "stm32wlxx_hal.h"
#include "sensirion_i2c.h"
#include "GNSE_bsp.h"

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_init(void)
{
// This fuction is a place filler for now, TODO: add an improved init, see https://github.com/TheThingsIndustries/generic-node-se/issues/33
}

/**
 * Release all resources initialized by sensirion_i2c_init().
 */
void sensirion_i2c_release(void)
{
// This fuction is a place filler for now, TODO: add an improved init, see https://github.com/TheThingsIndustries/generic-node-se/issues/33
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_read(uint8_t address, uint8_t *data, uint16_t count)
{
    return (int8_t)HAL_I2C_Master_Receive(&GNSE_BSP_sensor_i2c1, (uint16_t)(address << 1),
                                          data, count, SENSOR_I2C1_TIMOUT);
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_write(uint8_t address, const uint8_t *data,
                           uint16_t count)
{
    return (int8_t)HAL_I2C_Master_Transmit(&GNSE_BSP_sensor_i2c1, (uint16_t)(address << 1),
                                           (uint8_t *)data, count, SENSOR_I2C1_TIMOUT);
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_sleep_usec(uint32_t useconds)
{
    uint32_t msec = useconds / 1000;
    if (useconds % 1000 > 0)
    {
        msec++;
    }
    HAL_Delay(msec);
}
